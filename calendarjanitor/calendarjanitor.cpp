/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "calendarjanitor.h"
#include "collectionloader.h"

#include <CalendarSupport/Utils>

#include <KCalendarCore/Alarm>
#include <KCalendarCore/Attachment>
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <KLocalizedString>

#include <QCoreApplication>
#include <QList>
#include <QTextStream>

#define TEXT_WIDTH 75

static void print(const QString &message, bool newline = true)
{
    QTextStream out(stdout);
    out << message;
    if (newline) {
        out << "\n";
    }
}

static void bailOut()
{
    print(i18n("Bailing out. Fix your akonadi setup first. These kind of errors should not happen."));
    qApp->exit(-1);
}

static bool collectionIsReadOnly(const Akonadi::Collection &collection)
{
    return !(collection.rights() & Akonadi::Collection::CanChangeItem) || !(collection.rights() & Akonadi::Collection::CanDeleteItem);
}

static bool incidenceIsOld(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (incidence->recurs() || incidence->type() == KCalendarCore::Incidence::TypeJournal) {
        return false;
    }

    QDateTime datetime = incidence->dtStart();
    if (!datetime.isValid() && incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        datetime = incidence->dateTime(KCalendarCore::Incidence::RoleEnd);
    }

    return datetime.isValid() && datetime.daysTo(QDateTime::currentDateTime()) > 365;
}

CalendarJanitor::CalendarJanitor(const Options &options, QObject *parent)
    : QObject(parent)
    , m_collectionLoader(new CollectionLoader(this))
    , m_options(options)
    , m_currentSanityCheck(Options::CheckNone)
    , m_pendingModifications(0)
    , m_pendingDeletions(0)
    , m_strippingOldAlarms(false)
    , m_returnCode(0)
{
    m_changer = new Akonadi::IncidenceChanger(this);
    m_changer->setShowDialogsOnError(false);
    connect(m_changer, &Akonadi::IncidenceChanger::modifyFinished, this, &CalendarJanitor::onModifyFinished);
    connect(m_changer, &Akonadi::IncidenceChanger::deleteFinished, this, &CalendarJanitor::onDeleteFinished);
    connect(m_collectionLoader, &CollectionLoader::loaded, this, &CalendarJanitor::onCollectionsFetched);
}

void CalendarJanitor::start()
{
    m_collectionLoader->load();
}

void CalendarJanitor::onCollectionsFetched(bool success)
{
    if (!success) {
        print(i18n("Error while fetching collections"));
        Q_EMIT finished(false);
        qApp->exit(-1);
        return;
    }

    const auto collections = m_collectionLoader->collections();
    for (const Akonadi::Collection &collection : collections) {
        if (m_options.testCollection(collection.id())) {
            m_collectionsToProcess << collection;
        }
    }

    if (m_collectionsToProcess.isEmpty()) {
        print(i18n("There are no collections to process."));
        qApp->exit((-1));
        return;
    }

    // Load all items:
    m_calendar = Akonadi::FetchJobCalendar::Ptr(new Akonadi::FetchJobCalendar());
    connect(m_calendar.data(), &Akonadi::FetchJobCalendar::loadFinished, this, &CalendarJanitor::onItemsFetched);
}

void CalendarJanitor::onItemsFetched(bool success, const QString &errorMessage)
{
    if (!success) {
        print(errorMessage);
        Q_EMIT finished(false);
        qApp->exit(-1);
        return;
    }

    // Start processing collections
    processNextCollection();
}

void CalendarJanitor::onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorMessage)
{
    Q_UNUSED(changeId)
    if (resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess) {
        print(i18n("Error while modifying incidence: %1", errorMessage));
        bailOut();
        return;
    }
    if (!m_options.stripOldAlarms()) {
        print(i18n("Fixed item %1", item.id()));
    }

    m_pendingModifications--;
    if (m_pendingModifications == 0) {
        runNextTest();
    }
}

void CalendarJanitor::onDeleteFinished(int changeId,
                                       const QVector<Akonadi::Item::Id> &items,
                                       Akonadi::IncidenceChanger::ResultCode resultCode,
                                       const QString &errorMessage)
{
    Q_UNUSED(changeId)
    if (resultCode != Akonadi::IncidenceChanger::ResultCodeSuccess) {
        print(i18n("Error while deleting incidence: %1", errorMessage));
        bailOut();
        return;
    }
    print(i18n("Deleted item %1", items.first()));
    m_pendingDeletions--;
    if (m_pendingDeletions == 0) {
        runNextTest();
    }
}

void CalendarJanitor::processNextCollection()
{
    m_itemsToProcess.clear();
    m_currentSanityCheck = Options::CheckNone;
    m_strippingOldAlarms = false;

    if (m_collectionsToProcess.isEmpty()) {
        print(QLatin1Char('\n') + QString().leftJustified(TEXT_WIDTH, QLatin1Char('*')));
        Q_EMIT finished(true);
        qApp->exit(m_returnCode);
        return;
    }

    m_currentCollection = m_collectionsToProcess.takeFirst();
    print(QLatin1Char('\n') + QString().leftJustified(TEXT_WIDTH, QLatin1Char('*')));
    print(i18n("Processing collection %1 (id=%2)...", m_currentCollection.displayName(), m_currentCollection.id()));

    if (collectionIsReadOnly(m_currentCollection)) {
        if (m_options.action() == Options::ActionScanAndFix) {
            print(i18n("Collection is read only, disabling fix mode."));
        } else if (m_options.stripOldAlarms()) {
            print(i18n("Collection is read only, skipping it."));
            processNextCollection();
            return;
        }
    }

    m_itemsToProcess = m_calendar->items(m_currentCollection.id());
    if (m_itemsToProcess.isEmpty()) {
        print(i18n("Collection is empty, ignoring it."));
        processNextCollection();
    } else {
        m_incidenceMap.clear();
        for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
            KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
            Q_ASSERT(incidence);
            m_incidenceMap.insert(incidence->instanceIdentifier(), incidence);
            m_incidenceToItem.insert(incidence, item);
        }
        runNextTest();
    }
}

void CalendarJanitor::runNextTest()
{
    if (m_options.stripOldAlarms()) {
        if (!m_strippingOldAlarms) {
            m_strippingOldAlarms = true;
            stripOldAlarms();
        } else {
            processNextCollection();
        }

        return;
    }

    int currentType = static_cast<int>(m_currentSanityCheck);
    m_currentSanityCheck = static_cast<Options::SanityCheck>(currentType + 1);

    switch (m_currentSanityCheck) {
    case Options::CheckEmptySummary:
        sanityCheck1();
        break;
    case Options::CheckEmptyUid:
        sanityCheck2();
        break;
    case Options::CheckEventDates:
        sanityCheck3();
        break;
    case Options::CheckTodoDates:
        sanityCheck4();
        break;
    case Options::CheckJournalDates:
        sanityCheck5();
        break;
    case Options::CheckOrphans:
        sanityCheck6();
        break;
    case Options::CheckDuplicateUIDs:
        sanityCheck7();
        break;
    case Options::CheckStats:
        sanityCheck8();
        break;
    case Options::CheckOrphanRecurId:
        sanityCheck9();
        break;
    case Options::CheckCount:
        processNextCollection();
        break;
    default:
        Q_ASSERT(false);
    }
}

void CalendarJanitor::sanityCheck1()
{
    beginTest(i18n("Checking for incidences with empty summary and description..."));

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->summary().isEmpty() && incidence->description().isEmpty() && incidence->attachments().isEmpty()) {
            printFound(item);
            deleteIncidence(item);
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck2()
{
    beginTest(i18n("Checking for incidences with empty UID..."));

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->uid().isEmpty()) {
            printFound(item);
            if (m_fixingEnabled) {
                incidence->recreate();
                m_pendingModifications++;
                m_changer->modifyIncidence(item);
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck3()
{
    beginTest(i18n("Checking for events with invalid DTSTART..."));
    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        KCalendarCore::Event::Ptr event = incidence.dynamicCast<KCalendarCore::Event>();
        if (!event) {
            continue;
        }

        QDateTime start = event->dtStart();
        QDateTime end = event->dtEnd();

        bool modify = false;
        QString message;
        if (!start.isValid() && end.isValid()) {
            modify = true;
            printFound(item);
            event->setDtStart(end);
        } else if (!start.isValid() && !end.isValid()) {
            modify = true;
            printFound(item);
            event->setDtStart(QDateTime::currentDateTime());
            event->setDtEnd(event->dtStart().addSecs(3600));
        }

        if (modify) {
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck4()
{
    beginTest(i18n("Checking for recurring to-dos with invalid DTSTART..."));
    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        KCalendarCore::Todo::Ptr todo = incidence.dynamicCast<KCalendarCore::Todo>();
        if (!todo) {
            continue;
        }

        QDateTime start = todo->dtStart();
        QDateTime due = todo->dtDue();
        bool modify = false;
        if (todo->recurs() && !start.isValid() && due.isValid()) {
            modify = true;
            printFound(item);
            todo->setDtStart(due);
        }

        if (todo->recurs() && !start.isValid() && !due.isValid()) {
            modify = true;
            printFound(item);
            todo->setDtStart(QDateTime::currentDateTime());
        }

        if (modify) {
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest();
}

void CalendarJanitor::sanityCheck5()
{
    beginTest(i18n("Checking for journals with invalid DTSTART..."));
    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->type() != KCalendarCore::Incidence::TypeJournal) {
            continue;
        }

        if (!incidence->dtStart().isValid()) {
            printFound(item);
            incidence->setDtStart(QDateTime::currentDateTime());
            if (m_fixingEnabled) {
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }
    endTest();
}

void CalendarJanitor::sanityCheck6()
{
    beginTest(i18n("Checking for orphans...")); // Incidences without a parent

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        const QString parentUid = incidence->relatedTo();
        if (!parentUid.isEmpty() && !m_calendar->incidence(parentUid)) {
            printFound(item, i18n("The following incidences are children of nonexistent parents"));
            if (m_fixingEnabled) {
                incidence->setRelatedTo(QString());
                m_changer->modifyIncidence(item);
                m_pendingModifications++;
            }
        }
    }

    endTest(true, i18n("In fix mode these children will be unparented."), i18n("Children were successfully unparented."));
}

void CalendarJanitor::sanityCheck7()
{
    beginTest(i18n("Checking for duplicate UIDs..."));

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        const QList<KCalendarCore::Incidence::Ptr> existingIncidences = m_incidenceMap.values(incidence->instanceIdentifier());

        if (existingIncidences.count() == 1) {
            continue;
        }

        for (const KCalendarCore::Incidence::Ptr &existingIncidence : existingIncidences) {
            if (existingIncidence != incidence && *incidence == *existingIncidence) {
                printFound(item);
                deleteIncidence(item);
                break;
            }
        }
    }

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        const QList<KCalendarCore::Incidence::Ptr> existingIncidences = m_incidenceMap.values(incidence->instanceIdentifier());

        if (existingIncidences.count() == 1) {
            continue;
        }

        for (int i = 1; i < existingIncidences.count(); ++i) {
            printFound(item);
            if (m_fixingEnabled) {
                KCalendarCore::Incidence::Ptr existingIncidence = existingIncidences.at(i);
                Akonadi::Item item = m_incidenceToItem.value(existingIncidence);
                Q_ASSERT(item.isValid());
                if (item.isValid()) {
                    existingIncidence->recreate();
                    m_changer->modifyIncidence(item);
                    m_pendingModifications++;
                    m_incidenceMap.remove(incidence->instanceIdentifier(), existingIncidence);
                }
            }
        }
    }

    endTest();
}

static void printStat(const QString &message, int arg)
{
    if (arg > 0) {
        print(message.leftJustified(50), false);
        const QString s = QStringLiteral(": %1");
        print(s.arg(arg));
    }
}

void CalendarJanitor::sanityCheck8()
{
    beginTest(i18n("Gathering statistics..."));
    print(QStringLiteral("\n"));

    int numOldAlarms = 0;
    int numAttachments = 0;
    int totalAttachmentSize = 0;
    int numOldIncidences = 0;
    int numEmptyRID = 0;
    QHash<KCalendarCore::Incidence::IncidenceType, int> m_counts;

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (!incidence->attachments().isEmpty()) {
            const auto attachments = incidence->attachments();
            for (const KCalendarCore::Attachment &attachment : attachments) {
                if (!attachment.isUri()) {
                    numAttachments++;
                    totalAttachmentSize += attachment.size();
                }
            }
        }

        m_counts[incidence->type()]++;

        if (incidenceIsOld(incidence)) {
            if (!incidence->alarms().isEmpty()) {
                numOldAlarms++;
            }
            numOldIncidences++;
        }

        numAttachments += incidence->attachments().count();

        if (item.remoteId().isEmpty()) {
            numEmptyRID++;
        }
    }

    printStat(i18n("Events"), m_counts[KCalendarCore::Incidence::TypeEvent]);
    printStat(i18n("Todos"), m_counts[KCalendarCore::Incidence::TypeTodo]);
    printStat(i18n("Journals"), m_counts[KCalendarCore::Incidence::TypeJournal]);
    printStat(i18n("Passed events and to-dos (>365 days)"), numOldIncidences);
    printStat(i18n("Old incidences with alarms"), numOldAlarms);
    printStat(i18n("Inline attachments"), numAttachments);
    printStat(i18n("Items with empty remote id [!!]"), numEmptyRID);

    if (totalAttachmentSize < 1024) {
        printStat(i18n("Total size of inline attachments (bytes)"), totalAttachmentSize);
    } else {
        printStat(i18n("Total size of inline attachments (KB)"), totalAttachmentSize / 1024);
    }

    if (numEmptyRID > 0) {
        m_returnCode = -2;
    }

    endTest(/**print=*/false);
}

void CalendarJanitor::sanityCheck9()
{
    beginTest(i18n("Checking for RECURRING-ID incidences with nonexistent master incidence..."));
    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (incidence->recurs() && incidence->hasRecurrenceId() && !m_calendar->incidence(incidence->uid())) {
            printFound(item);
            if (m_fixingEnabled) {
                bool modified = false;

                QDateTime recId = incidence->recurrenceId();
                QDateTime start = incidence->dtStart();
                QDateTime end = incidence->dateTime(KCalendarCore::Incidence::RoleEnd);

                KCalendarCore::Event::Ptr event = incidence.dynamicCast<KCalendarCore::Event>();
                KCalendarCore::Todo::Ptr todo = incidence.dynamicCast<KCalendarCore::Todo>();

                if (event && start.isValid() && end.isValid()) {
                    modified = true;
                    const int duration = start.daysTo(end.toTimeSpec(start.timeSpec()));
                    incidence->setDtStart(recId);
                    event->setDtEnd(recId.addDays(duration));
                } else if (todo && start.isValid()) {
                    modified = true;
                    incidence->setDtStart(recId);

                    if (end.isValid()) {
                        const int duration = start.daysTo(end.toTimeSpec(start.timeSpec()));
                        todo->setDtDue(recId.addDays(duration));
                    }
                }

                if (modified) {
                    m_pendingModifications++;
                    incidence->recreate(); // change uid
                    incidence->clearRecurrence(); // make it non-recurring
                    incidence->setRecurrenceId(QDateTime());
                    m_changer->modifyIncidence(item);
                }
            }
        }
    }

    endTest(true, i18n("In fix mode the RECURRING-ID property will be unset and UID changed."), i18n("Recurrence cleared."));
}

void CalendarJanitor::stripOldAlarms()
{
    beginTest(i18n("Deleting alarms older than 365 days..."));

    for (const Akonadi::Item &item : std::as_const(m_itemsToProcess)) {
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        if (!incidence->alarms().isEmpty() && incidenceIsOld(incidence)) {
            incidence->clearAlarms();
            m_pendingModifications++;
            m_changer->modifyIncidence(item);
        }
    }

    endTest();
}

static QString dateString(const KCalendarCore::Incidence::Ptr &incidence)
{
    QDateTime start = incidence->dtStart();
    QDateTime end = incidence->dateTime(KCalendarCore::Incidence::RoleEnd);
    QString str = QLatin1String("DTSTART=") + (start.isValid() ? start.toString() : i18n("invalid")) + QLatin1String("; ");

    if (incidence->type() == KCalendarCore::Incidence::TypeJournal) {
        return str;
    }

    str += QLatin1String("\n        ");

    if (incidence->type() == KCalendarCore::Incidence::TypeTodo) {
        str += QLatin1String("DTDUE=");
    } else if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
        str += QLatin1String("DTEND=");
    }

    str += (start.isValid() ? end.toString() : i18n("invalid")) + QLatin1String("; ");

    if (incidence->recurs()) {
        str += i18n("recurrent");
    }

    return str;
}

void CalendarJanitor::printFound(const Akonadi::Item &item, const QString &explanation)
{
    KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    m_numDamaged++;
    if (m_numDamaged == 1) {
        print(QStringLiteral(" [!!]"));
        if (!explanation.isEmpty()) {
            print(QStringLiteral("    "), false);
            print(explanation, false);
            print(QStringLiteral(":\n"));
        }
    }
    print(QLatin1String("    * ") + i18n("Found buggy incidence:"));
    print(QLatin1String("        ") + i18n("id=%1; summary=\"%2\"", item.id(), incidence->summary()));
    print(QLatin1String("        ") + dateString(incidence));
}

void CalendarJanitor::beginTest(const QString &message)
{
    m_numDamaged = 0;
    m_fixingEnabled = m_options.action() == Options::ActionScanAndFix && !collectionIsReadOnly(m_currentCollection);
    print(message.leftJustified(TEXT_WIDTH), false);
}

void CalendarJanitor::endTest(bool printEnabled, const QString &fixExplanation, const QString &fixExplanation2)
{
    if (m_numDamaged == 0 && printEnabled) {
        print(QStringLiteral(" [OK]"));
    } else if (m_numDamaged > 0) {
        print(QStringLiteral("\n    "), false);
        if (m_options.action() == Options::ActionScanAndFix) {
            print(fixExplanation2);
        } else {
            print(fixExplanation);
        }

        print(QString());
    }

    if (m_pendingDeletions == 0 && m_pendingModifications == 0) {
        runNextTest();
    }
}

void CalendarJanitor::deleteIncidence(const Akonadi::Item &item)
{
    if (m_fixingEnabled && !collectionIsReadOnly(m_currentCollection)) {
        m_pendingDeletions++;
        m_changer->deleteIncidence(item);
        KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
        m_incidenceMap.remove(incidence->instanceIdentifier(), incidence);
        m_incidenceToItem.remove(incidence);
    }
}
