/******************************************************************************
 * konsolekalendar.cpp                                                        *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/
/**
 * @file konsolekalendar.cpp
 * Provides the KonsoleKalendar class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendar.h"
#include "konsolekalendaradd.h"
#include "konsolekalendarchange.h"
#include "konsolekalendardelete.h"
#include "konsolekalendarexports.h"

#include "konsolekalendar_debug.h"
#include <KLocalizedString>

#include <Akonadi/AgentInstance>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>

#include <QDBusInterface>
#include <QDBusReply>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace KCalendarCore;
using namespace std;

KonsoleKalendar::KonsoleKalendar(KonsoleKalendarVariables *variables)
{
    m_variables = variables;
}

KonsoleKalendar::~KonsoleKalendar() = default;

bool KonsoleKalendar::importCalendar()
{
    KonsoleKalendarAdd add(m_variables);

    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::importCalendar() | importing now!";
    return add.addImportedCalendar();
}

bool KonsoleKalendar::printCalendarList()
{
    auto job = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
    const QStringList mimeTypes = QStringList() << QStringLiteral("text/calendar") << KCalendarCore::Event::eventMimeType()
                                                << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType();
    job->fetchScope().setContentMimeTypes(mimeTypes);
    QEventLoop loop;
    QObject::connect(job, &Akonadi::CollectionFetchJob::result, &loop, &QEventLoop::quit);
    job->start();
    loop.exec();

    if (job->error() != 0) {
        return false;
    }

    const Akonadi::Collection::List collections = job->collections();

    if (collections.isEmpty()) {
        cout << i18n("There are no calendars available.").toLocal8Bit().data() << endl;
    } else {
        cout << "--------------------------" << endl;
        auto mimeTypeSet = QSet<QString>(mimeTypes.begin(), mimeTypes.end()); // set changes by run method intersect
        for (const Akonadi::Collection &collection : collections) {
            const QStringList contentMimeTypes = collection.contentMimeTypes();
            auto collectionMimeTypeSet = QSet<QString>(contentMimeTypes.begin(), contentMimeTypes.end());

            if (mimeTypeSet.intersects(collectionMimeTypeSet)) {
                QString colId = QString::number(collection.id()).leftJustified(6, QLatin1Char(' '));
                colId += QLatin1StringView("- ");

                bool readOnly = !(collection.rights() & Akonadi::Collection::CanCreateItem || collection.rights() & Akonadi::Collection::CanChangeItem
                                  || collection.rights() & Akonadi::Collection::CanDeleteItem);

                QString readOnlyString = readOnly ? i18n("(Read only)") + QLatin1Char(' ') : QString();

                cout << colId.toLocal8Bit().data() << readOnlyString.toLocal8Bit().constData() << collection.displayName().toLocal8Bit().data() << endl;
            }
        }
    }

    return true;
}

bool KonsoleKalendar::createAkonadiResource(const QString &icalFileUri)
{
    Akonadi::AgentType type = Akonadi::AgentManager::self()->type(QStringLiteral("akonadi_ical_resource"));
    auto job = new Akonadi::AgentInstanceCreateJob(type);
    job->exec();
    if (job->error() != 0) {
        return false;
    }
    auto inst = job->instance();
    inst.setName(QFileInfo(icalFileUri).baseName());
    QDBusInterface iface(QStringLiteral("org.freedesktop.Akonadi.Resource.") + inst.identifier(), QStringLiteral("/Settings"));
    QDBusReply<void> reply = iface.call(QStringLiteral("setDisplayName"), QFileInfo(icalFileUri).baseName());
    if (!reply.isValid()) {
        qCWarning(KONSOLEKALENDAR_LOG) << "Could not set setting 'name': " << reply.error().message();
        return false;
    }
    reply = iface.call(QStringLiteral("setPath"), icalFileUri);
    if (!reply.isValid()) {
        qCWarning(KONSOLEKALENDAR_LOG) << "Could not set setting 'path': " << reply.error().message();
        return false;
    }
    reply = iface.call(QStringLiteral("save"));
    if (!reply.isValid()) {
        qCWarning(KONSOLEKALENDAR_LOG) << "Could not save settings: " << reply.error().message();
        return false;
    }
    inst.reconfigure();
    return true;
}

bool KonsoleKalendar::createCalendar()
{
    bool status = false;

    const QString filename = m_variables->getCalendarFile();

    if (m_variables->isDryRun()) {
        cout << i18n("Create Calendar <Dry Run>: %1", filename).toLocal8Bit().data() << endl;
    } else {
        qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::createCalendar() |"
                                     << "Creating calendar file: " << filename.toLocal8Bit().data();

        if (m_variables->isVerbose()) {
            cout << i18n("Create Calendar <Verbose>: %1", filename).toLocal8Bit().data() << endl;
        }

        status = createAkonadiResource(QStringLiteral("file://%1").arg(filename));
    }

    return status;
}

bool KonsoleKalendar::showInstance()
{
    bool status = true;
    QFile f;
    QString title;
    Event::Ptr event;
    const auto timeZone = m_variables->getCalendar()->timeZone();
    Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();

    if (m_variables->isDryRun()) {
        cout << qPrintable(i18n("View Events <Dry Run>:")) << endl;
        printSpecs();
    } else {
        qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                     << "open export file";

        if (m_variables->isExportFile()) {
            f.setFileName(m_variables->getExportFile());
            if (!f.open(QIODevice::WriteOnly)) {
                status = false;
                qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                             << "unable to open export file" << m_variables->getExportFile();
            }
        } else {
            f.open(stdout, QIODevice::WriteOnly);
        }

        if (status) {
            qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                         << "opened successful";

            if (m_variables->isVerbose()) {
                cout << i18n("View Event <Verbose>:").toLocal8Bit().data() << endl;
                printSpecs();
            }

            QTextStream ts(&f);

            if (m_variables->getAll()) {
                qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                             << "view all events sorted list";

                const Event::List sortedList = calendar->events(EventSortStartDate);
                qCDebug(KONSOLEKALENDAR_LOG) << "Found" << sortedList.count() << "events";
                if (!sortedList.isEmpty()) {
                    // The code that was here before the akonadi port was really slow with 200 events
                    // this is much faster:
                    for (const KCalendarCore::Event::Ptr &event : sortedList) {
                        status &= printEvent(&ts, event, event->dtStart().date());
                    }
                }
            } else if (m_variables->isUID()) {
                qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                             << "view events by uid list";
                // TODO: support a list of UIDs
                event = calendar->event(m_variables->getUID());
                // If this UID represents a recurring Event,
                // only the first day of the Event will be printed
                status = printEvent(&ts, event, event->dtStart().date());
            } else if (m_variables->isNext()) {
                qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                             << "Show next activity in calendar";

                QDateTime datetime = m_variables->getStartDateTime();
                datetime = datetime.addDays(720);

                QDate dt;
                for (dt = m_variables->getStartDateTime().date(); dt <= datetime.date(); dt = dt.addDays(1)) {
                    Event::List events = calendar->events(dt, timeZone, EventSortStartDate, SortDirectionAscending);
                    qCDebug(KONSOLEKALENDAR_LOG) << "2-Found" << events.count() << "events on date" << dt;
                    // finished here when we get the next event
                    if (!events.isEmpty()) {
                        qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::showInstance() |"
                                                     << "Got the next event";
                        printEvent(&ts, events.first(), dt);
                        return true;
                    }
                }
            }
            f.close();
        }
    }
    return status;
}

bool KonsoleKalendar::printEventList(QTextStream *ts, Event::List *eventList, QDate date)
{
    bool status = true;

    qCDebug(KONSOLEKALENDAR_LOG) << eventList->count();
    if (!eventList->isEmpty()) {
        Event::Ptr singleEvent;
        Event::List::ConstIterator it;

        for (it = eventList->constBegin(); it != eventList->constEnd() && status != false; ++it) {
            singleEvent = *it;

            status = printEvent(ts, singleEvent, date);
        }
    }

    return status;
}

bool KonsoleKalendar::printEvent(QTextStream *ts, const Event::Ptr &event, QDate dt)
{
    bool status = false;
    KonsoleKalendarExports exports;

    if (event) {
        switch (m_variables->getExportType()) {
        case ExportTypeCSV:
            qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::printEvent() |"
                                         << "CSV export";
            status = exports.exportAsCSV(ts, event, dt);
            break;

        case ExportTypeTextShort: {
            qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::printEvent() |"
                                         << "TEXT-SHORT export";
            bool sameDay = true;
            if (dt.daysTo(m_saveDate)) {
                sameDay = false;
                m_saveDate = dt;
            }
            status = exports.exportAsTxtShort(ts, event, dt, sameDay);
        } break;

        default: // Default export-type is ExportTypeText
            qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendar.cpp::printEvent() |"
                                         << "TEXT export";
            status = exports.exportAsTxt(ts, event, dt);
            break;
        }
    }
    return status;
}

bool KonsoleKalendar::addEvent()
{
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::addEvent() |"
                                 << "Create Adding";
    KonsoleKalendarAdd add(m_variables);
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::addEvent() |"
                                 << "Adding Event now!";
    return add.addEvent();
}

bool KonsoleKalendar::changeEvent()
{
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::changeEvent() |"
                                 << "Create Changing";
    KonsoleKalendarChange change(m_variables);
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::changeEvent() |"
                                 << "Changing Event now!";
    return change.changeEvent();
}

bool KonsoleKalendar::deleteEvent()
{
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::deleteEvent() |"
                                 << "Create Deleting";
    KonsoleKalendarDelete del(m_variables);
    qCDebug(KONSOLEKALENDAR_LOG) << "konsolecalendar.cpp::deleteEvent() |"
                                 << "Deleting Event now!";
    return del.deleteEvent();
}

bool KonsoleKalendar::isEvent(const QDateTime &startdate, const QDateTime &enddate, const QString &summary)
{
    // Search for an event with specified start and end datetime stamp and summary

    Event::Ptr event;
    Event::List::ConstIterator it;

    bool found = false;

    const auto timeZone = m_variables->getCalendar()->timeZone();
    Event::List eventList(m_variables->getCalendar()->rawEventsForDate(startdate.date(), timeZone, EventSortStartDate, SortDirectionAscending));
    for (it = eventList.constBegin(); it != eventList.constEnd(); ++it) {
        event = *it;
        if (event->dtEnd().toTimeZone(timeZone) == enddate && event->summary() == summary) {
            found = true;
            break;
        }
    }
    return found;
}

void KonsoleKalendar::printSpecs()
{
    cout << i18n("  What:  %1", m_variables->getSummary()).toLocal8Bit().data() << endl;

    cout << i18n("  Begin: %1", m_variables->getStartDateTime().toString(Qt::TextDate)).toLocal8Bit().data() << endl;

    cout << i18n("  End:   %1", m_variables->getEndDateTime().toString(Qt::TextDate)).toLocal8Bit().data() << endl;

    if (m_variables->getFloating() == true) {
        cout << i18n("  No Time Associated with Event").toLocal8Bit().data() << endl;
    }

    cout << i18n("  Desc:  %1", m_variables->getDescription()).toLocal8Bit().data() << endl;

    cout << i18n("  Location:  %1", m_variables->getLocation()).toLocal8Bit().data() << endl;
}
