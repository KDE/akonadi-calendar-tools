/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "options.h"

#include <AkonadiCore/Collection>
#include <KCalendarCore/MemoryCalendar>

#include <QList>
#include <QObject>

class KJob;

class Backuper : public QObject
{
    Q_OBJECT
public:
    explicit Backuper(QObject *parent = nullptr);
    void backup(const QString &filename, const QList<Akonadi::Collection::Id> &collectionIds);

Q_SIGNALS:
    void finished(bool success, const QString &errorMessage);

private Q_SLOTS:
    void onCollectionsFetched(KJob *);
    void onCollectionLoaded(KJob *);

private:
    void loadCollection(const Akonadi::Collection &collection);
    void emitFinished(bool success, const QString &message);

    QList<Akonadi::Collection::Id> m_requestedCollectionIds;
    QList<Akonadi::Collection::Id> m_pendingCollections;

    Akonadi::Collection::List m_collections;
    QString m_filename;
    KCalendarCore::MemoryCalendar::Ptr m_calendar;

    bool m_backupInProgress;
};

