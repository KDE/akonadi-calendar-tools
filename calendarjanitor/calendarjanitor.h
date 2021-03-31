/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "options.h"

#include <KCalendarCore/Incidence>

#include <Akonadi/Calendar/FetchJobCalendar>
#include <Akonadi/Calendar/IncidenceChanger>
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <QMultiMap>
#include <QObject>
#include <QString>

class CollectionLoader;

class CalendarJanitor : public QObject
{
    Q_OBJECT
public:
    explicit CalendarJanitor(const Options &options, QObject *parent = nullptr);

    void start();

Q_SIGNALS:
    void finished(bool success);

private Q_SLOTS:
    void onCollectionsFetched(bool success);
    void onItemsFetched(bool success, const QString &errorMessage);
    void onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);
    void onDeleteFinished(int changeId, const QVector<Akonadi::Item::Id> &, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage);

    void processNextCollection();

    // For each collection we process, we run a bunch of tests on it.
    void runNextTest();

    void sanityCheck1();
    void sanityCheck2();
    void sanityCheck3();
    void sanityCheck4();
    void sanityCheck5();
    void sanityCheck6();
    void sanityCheck7();
    void sanityCheck8();
    void sanityCheck9();

    void stripOldAlarms();

    void printFound(const Akonadi::Item &item, const QString &explanation = QString());

    void beginTest(const QString &message);
    void endTest(bool print = true, const QString &fixExplanation = QString(), const QString &fixExplanation2 = QString());

    void deleteIncidence(const Akonadi::Item &item);

private:
    CollectionLoader *m_collectionLoader = nullptr;
    Akonadi::Collection::List m_collectionsToProcess;
    Akonadi::Item::List m_itemsToProcess;
    Options m_options;
    Akonadi::IncidenceChanger *m_changer = nullptr;
    Akonadi::Collection m_currentCollection;
    Options::SanityCheck m_currentSanityCheck;
    int m_pendingModifications = 0;
    int m_pendingDeletions = 0;
    bool m_strippingOldAlarms = false;

    QList<Akonadi::Item::Id> m_test1Results;
    QStringList m_test2Results;

    int m_numDamaged = 0;
    bool m_fixingEnabled = false;

    QString m_summary; // to print at the end.
    QMultiMap<QString, KCalendarCore::Incidence::Ptr> m_incidenceMap;
    QMap<KCalendarCore::Incidence::Ptr, Akonadi::Item> m_incidenceToItem;

    Akonadi::FetchJobCalendar::Ptr m_calendar;

    int m_returnCode;
};

