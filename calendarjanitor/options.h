/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <AkonadiCore/Collection>
#include <QList>

class Options
{
public:
    enum SanityCheck {
        CheckNone,
        CheckEmptySummary, // Checks for empty summary and description. In fix mode, it deletes them.
        CheckEmptyUid, // Checks for an empty UID. In fix mode, a new UID is assigned.
        CheckEventDates, // Check for missing DTSTART or DTEND. New dates will be assigned.
        CheckTodoDates, // Check for recurring to-dos without DTSTART. DTDUE will be assigned to DTSTART, or current date if DTDUE is also invalid.
        CheckJournalDates, // Check for journals without DTSTART
        CheckOrphans, // Check for orphan to-dos. Will be unparented." <disabled for now>
        CheckDuplicateUIDs, // Check for duplicated UIDs. Copies will be deleted if the payload is the same. Otherwise a new UID is assigned.
        CheckOrphanRecurId, // Check if incidences with recurrence id belong to an nonexistent master incidence
        CheckStats, // Gathers some statistics. No fixing is done.
        CheckCount // For iteration purposes. Keep at end.
    };

    enum Action { ActionNone, ActionScan, ActionScanAndFix, ActionBackup };

    Options();

    void setAction(Action);
    Action action() const;

    /**
     * List of collections for backup or fix modes.
     * If empty, all collections will be considered.
     */
    QList<Akonadi::Collection::Id> collections() const;
    void setCollections(const QList<Akonadi::Collection::Id> &);
    bool testCollection(Akonadi::Collection::Id) const;

    bool stripOldAlarms() const;
    void setStripOldAlarms(bool strip);

private:
    QList<Akonadi::Collection::Id> m_collectionIds;
    Action m_action = ActionNone;
    bool m_stripOldAlarms = false;
};

