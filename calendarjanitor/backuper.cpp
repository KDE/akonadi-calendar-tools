/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "backuper.h"

#include <CalendarSupport/Utils>

#include <KCalendarCore/FileStorage>
#include <KCalendarCore/Incidence>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <KJob>
#include <KLocalizedString>
#include <QDebug>
#include <QTimeZone>

#include <QCoreApplication>

static void printOut(const QString &message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

void Backuper::emitFinished(bool success, const QString &message)
{
    if (success) {
        printOut(
            QLatin1Char('\n')
            + i18np("Backup was successful. %1 incidence was saved.", "Backup was successful. %1 incidences were saved.", m_calendar->incidences().count()));
    } else {
        printOut(message);
    }

    m_calendar.clear();

    Q_EMIT finished(success, message);
    qApp->exit(success ? 0 : -1); // TODO: If we move this class to kdepimlibs, remove this
}

Backuper::Backuper(QObject *parent)
    : QObject(parent)
    , m_backupInProgress(false)
{
}

void Backuper::backup(const QString &filename, const QList<Akonadi::Collection::Id> &collectionIds)
{
    if (filename.isEmpty()) {
        emitFinished(false, i18n("File is empty."));
        return;
    }

    if (m_backupInProgress) {
        emitFinished(false, i18n("A backup is already in progress."));
        return;
    }
    printOut(i18n("Backing up your calendar data..."));
    m_calendar = KCalendarCore::MemoryCalendar::Ptr(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
    m_requestedCollectionIds = collectionIds;
    m_backupInProgress = true;
    m_filename = filename;

    auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);

    job->fetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    connect(job, &Akonadi::CollectionFetchJob::result, this, &Backuper::onCollectionsFetched);
    job->start();
}

void Backuper::onCollectionsFetched(KJob *job)
{
    if (job->error() == 0) {
        const QStringList mimetypes = KCalendarCore::Incidence::mimeTypes();
        QSet<QString> mimeTypeSet = QSet<QString>(mimetypes.begin(), mimetypes.end());
        auto cfj = qobject_cast<Akonadi::CollectionFetchJob *>(job);
        const auto collections = cfj->collections();
        for (const Akonadi::Collection &collection : collections) {
            if (!m_requestedCollectionIds.isEmpty() && !m_requestedCollectionIds.contains(collection.id())) {
                continue;
            }
            const QStringList contentMimeTypesLst = collection.contentMimeTypes();
            QSet<QString> collectionMimeTypeSet = QSet<QString>(contentMimeTypesLst.begin(), contentMimeTypesLst.end());
            if (!mimeTypeSet.intersect(collectionMimeTypeSet).isEmpty()) {
                m_collections << collection;
                loadCollection(collection);
            }
        }

        if (m_collections.isEmpty()) {
            emitFinished(false, i18n("No data to backup."));
        }
    } else {
        qCritical() << job->errorString();
        m_backupInProgress = false;
        emitFinished(false, job->errorString());
    }
}

void Backuper::loadCollection(const Akonadi::Collection &collection)
{
    printOut(i18n("Processing collection %1 (id=%2)...", collection.displayName(), collection.id()));
    auto ifj = new Akonadi::ItemFetchJob(collection, this);
    ifj->setProperty("collectionId", collection.id());
    ifj->fetchScope().fetchFullPayload(true);
    connect(ifj, &Akonadi::ItemFetchJob::result, this, &Backuper::onCollectionLoaded);
    m_pendingCollections << collection.id();
}

void Backuper::onCollectionLoaded(KJob *job)
{
    if (job->error()) {
        m_backupInProgress = false;
        m_calendar.clear();
        emitFinished(false, job->errorString());
    } else {
        auto ifj = qobject_cast<Akonadi::ItemFetchJob *>(job);
        Akonadi::Collection::Id id = ifj->property("collectionId").toInt();
        Q_ASSERT(id != -1);
        const Akonadi::Item::List items = ifj->items();
        m_pendingCollections.removeAll(id);

        for (const Akonadi::Item &item : items) {
            KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
            Q_ASSERT(incidence);
            m_calendar->addIncidence(incidence);
        }

        if (m_pendingCollections.isEmpty()) { // We're done
            KCalendarCore::FileStorage storage(m_calendar, m_filename);
            bool success = storage.save();
            QString message = success ? QString() : i18n("An error occurred");
            emitFinished(success, message);
        }
    }
}
