/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "collectionloader.h"

#include <KCalendarCore/Incidence>

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <QSet>
#include <QString>

#include <QDebug>

CollectionLoader::CollectionLoader(QObject *parent)
    : QObject(parent)
{
}

void CollectionLoader::load()
{
    auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);

    job->fetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    connect(job, &Akonadi::CollectionFetchJob::result, this, &CollectionLoader::onCollectionsLoaded);
    job->start();
}

Akonadi::Collection::List CollectionLoader::collections() const
{
    return m_collections;
}

void CollectionLoader::onCollectionsLoaded(KJob *job)
{
    if (job->error() == 0) {
        auto cfj = qobject_cast<Akonadi::CollectionFetchJob *>(job);
        Q_ASSERT(cfj);
        const QStringList mimetypes = KCalendarCore::Incidence::mimeTypes();
        QSet<QString> mimeTypeSet = QSet<QString>(mimetypes.begin(), mimetypes.end());
        const auto collections = cfj->collections();
        for (const Akonadi::Collection &collection : collections) {
            const QStringList contentMimeTypesLst = collection.contentMimeTypes();
            QSet<QString> collectionMimeTypeSet = QSet<QString>(contentMimeTypesLst.begin(), contentMimeTypesLst.end());
            if (mimeTypeSet.intersects(collectionMimeTypeSet)) {
                m_collections << collection;
            }
        }
        Q_EMIT loaded(true);
    } else {
        qCritical() << job->errorString();
        Q_EMIT loaded(false);
    }
}
