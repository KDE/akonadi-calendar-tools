/******************************************************************************
 * main.cpp                                                                   *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2009 Allen Winter <winter@kde.org>            *
 * SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>          *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 *****************************************************************************/
/**
 * @file main.cpp
 * KonsoleKalendar main program.
 * @author Tuukka Pasanen
 * @author Allen Winter
 * @author Sérgio Martins
 */

#include "konsolekalendar.h"
#include "konsolekalendarepoch.h"

#include "konsolekalendarvariables.h"

#include <KLocalizedString>
#include <kconfig.h>

#include "konsolekalendar_debug.h"

#include <KCalendarCore/CalFormat>

#include <QDateTime>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>

#include <KAboutData>
#include <QApplication>
#include <QCommandLineParser>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace KCalendarCore;
using namespace std;

//@cond IGNORE
static const char progName[] = "konsolekalendar";
static const char progDisplay[] = "KonsoleKalendar";
#include "console-version.h"
static const char progVersion[] = KDEPIM_VERSION;

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("konsolekalendar");
    QApplication app(argc, argv);

    KAboutData aboutData(QLatin1String(progName),
                         i18n("KonsoleKalendar"),
                         QStringLiteral(KDEPIM_VERSION),
                         i18n("A command line interface to KDE calendars"),
                         KAboutLicense::GPL,
                         i18n("(c) 2002-2009, Tuukka Pasanen and Allen Winter"));

    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Bug fixing"), QStringLiteral("montel@kde.org"));
    aboutData.addAuthor(i18n("Allen Winter"), i18n("Maintainer"), QStringLiteral("winter@kde.org"));
    aboutData.addAuthor(i18n("Tuukka Pasanen"), i18n("Author"), QStringLiteral("illuusio@mailcity.com"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), i18n("Print helpful runtime messages")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("dry-run"), i18n("Print what would have been done, but do not execute")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("allow-gui"), i18n("Allow calendars which might need an interactive user interface")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("event"), i18n("Operate for Events only (Default)")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("todo"), i18n("Operate for To-dos only [NOT WORKING YET]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("journal"), i18n("Operate for Journals only [NOT WORKING YET]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("view"), i18n("Print incidences in specified export format")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("add"), i18n("Insert an incidence into the calendar")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("change"), i18n("Modify an existing incidence")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("delete"), i18n("Remove an existing incidence")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("create"), i18n("Create new Akonadi Resource for file"), i18n("file")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("import"), i18n("Import this calendar to main calendar"), QStringLiteral("[import-file]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("list-calendars"), i18n("List available calendars")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("all"), i18n("View all calendar entries, ignoring date/time options")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("next"), i18n("View next activity in calendar")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("show-next"), i18n("From start date show next # days' activities"), QStringLiteral("[days]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("uid"), i18n("Incidence Unique-string identifier"), QStringLiteral("[uid]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("date"), i18n("Start from this day [YYYY-MM-DD]"), QStringLiteral("[start-date]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("time"), i18n("Start from this time [HH:MM:SS]"), QStringLiteral("[start-time]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("end-date"), i18n("End at this day [YYYY-MM-DD]"), QStringLiteral("[end-date]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("end-time"), i18n("End at this time [HH:MM:SS]"), QStringLiteral("[end-time]")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("epoch-start"), i18n("Start from this time [secs since epoch]"), QStringLiteral("[epoch-time]")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("epoch-end"), i18n("End at this time [secs since epoch]"), QStringLiteral("[epoch-time]")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("summary"), i18n("Add summary to incidence (for add/change modes)"), QStringLiteral("[summary]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("description"),
                                        i18n("Add description to incidence (for add/change modes)"),
                                        QStringLiteral("[description]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("location"),
                                        i18n("Add location to incidence (for add/change modes)"),
                                        QStringLiteral("[location]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("calendar"),
                                        i18n("Calendar to use when creating a new incidence"),
                                        QStringLiteral("[calendar id]")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("export-type"), i18n("Export file type (Default: text)"), QStringLiteral("[export-type]")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("export-file"), i18n("Export to file (Default: stdout)"), QStringLiteral("[export-type]")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("export-list"), i18n("Print list of export types supported and exit")));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // Laurent: allow or not gui FIXME
    //        // when not set (default) GUI is not enabled - disable all GUI stuff
    //        parser.isSet(QStringLiteral("allow-gui"));

    // Default values for start date/time (today at 07:00)
    QDate startdate = QDate::currentDate();
    QTime starttime(7, 0);

    // Default values for end date/time (today at 17:00)
    QDate enddate = QDate::currentDate();
    QTime endtime(17, 0);

    // Default values for switches
    bool view = true;
    bool add = false;
    bool change = false;
    bool del = false;
    bool create = false;
    // bool calendarFile = false;
    bool importFile = false;

    QString option;

    KonsoleKalendarVariables variables;
    KonsoleKalendarEpoch epochs;

    variables.setFloating(false); // by default, new events do NOT float

    if (parser.isSet(QStringLiteral("verbose"))) {
        variables.setVerbose(true);
    }

    if (parser.isSet(QStringLiteral("dry-run"))) {
        variables.setDryRun(true);
    }

    if (parser.isSet(QStringLiteral("allow-gui"))) {
        variables.setAllowGui(true);
    }

    /*
     * Switch on export list
     */
    if (parser.isSet(QStringLiteral("export-list"))) {
        cout << endl;
        cout << i18n("%1 supports these export formats:", QString::fromLatin1(progDisplay)).toLocal8Bit().data() << endl;
        cout << i18nc("the default export format", "  %1 [Default]", QStringLiteral("Text")).toLocal8Bit().data() << endl;
        cout << i18nc("short text export", "  %1 (like %2, but more compact)", QStringLiteral("Short"), QStringLiteral("Text")).toLocal8Bit().data() << endl;
        cout << i18nc("comma-separated values export", "  %1 (Comma-Separated Values)", QStringLiteral("CSV")).toLocal8Bit().data() << endl;
        cout << endl;
        return 0;
    }

    /*
     * Set incidence type(s)
     */
    if (parser.isSet(QStringLiteral("event"))) {
        variables.setUseEvents(true);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options | use Events";
    }
    if (parser.isSet(QStringLiteral("todo"))) {
        variables.setUseTodos(true);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options | use To-dos";
        cout << i18n("Sorry, To-dos are not working yet.").toLocal8Bit().data() << endl;
        return 1;
    }
    if (parser.isSet(QStringLiteral("journal"))) {
        variables.setUseJournals(true);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options | use Journals";
        cout << i18n("Sorry, Journals are not working yet.").toLocal8Bit().data() << endl;
        return 1;
    }
    // Use Events if no incidence type is specified on the command line
    if (!parser.isSet(QStringLiteral("event")) && !parser.isSet(QStringLiteral("todo")) && !parser.isSet(QStringLiteral("journal"))) {
        variables.setUseEvents(true);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options | use Events (Default)";
    }

    /*
     * Switch on exporting
     */
    variables.setExportType(ExportTypeText);
    if (parser.isSet(QStringLiteral("export-type"))) {
        option = parser.value(QStringLiteral("export-type"));

        if (option.toUpper() == QLatin1String("CSV")) {
            qCDebug(KONSOLEKALENDAR_LOG) << "main | export-type | Export to CSV";
            variables.setExportType(ExportTypeCSV);
        } else if (option.toUpper() == QLatin1String("TEXT")) {
            qCDebug(KONSOLEKALENDAR_LOG) << "main | export-type | Export to TEXT (default)";
            variables.setExportType(ExportTypeText);
        } else if (option.toUpper() == QLatin1String("SHORT")) {
            qCDebug(KONSOLEKALENDAR_LOG) << "main | export-type | Export to TEXT-SHORT";
            variables.setExportType(ExportTypeTextShort);
        } else {
            cout << i18n("Invalid Export Type Specified: %1", option).toLocal8Bit().data() << endl;
            return 1;
        }
    }

    /*
     * Switch on export file name
     */
    if (parser.isSet(QStringLiteral("export-file"))) {
        option = parser.value(QStringLiteral("export-file"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Export File:"
                                     << "(" << option << ")";

        variables.setExportFile(option);
    }

    /*
     * Switch on View (Print Entries).  This is the default mode of operation.
     */
    if (parser.isSet(QStringLiteral("view"))) {
        view = true;

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Mode: (Print incidences)";
    }

    /*
     * Switch on Add (Insert Entry)
     */
    if (parser.isSet(QStringLiteral("add"))) {
        view = false;
        add = true;

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Mode: (Add incidence)";
    }

    /*
     * Switch on Change (Modify Entry)
     */
    if (parser.isSet(QStringLiteral("change"))) {
        view = false;
        change = true;

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Mode: (Change incidence)";
    }

    /*
     * Switch on Delete (Remove Entry)
     */
    if (parser.isSet(QStringLiteral("delete"))) {
        view = false;
        del = true;

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Mode: (Delete incidence)";
    }

    /*
     * Switch on Create
     */
    if (parser.isSet(QStringLiteral("create"))) {
        view = false;
        create = true;

        option = parser.value(QStringLiteral("create"));
        variables.setCalendarFile(option);

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Create Akonadi Resource for"
                                     << "(" << option << ")";
    }

    /*
     * If there is summary attached.
     */
    if (parser.isSet(QStringLiteral("summary"))) {
        option = parser.value(QStringLiteral("summary"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Summary:"
                                     << "(" << option << ")";

        variables.setSummary(option);
    }

    /*
     * If there is description attached.
     */
    if (parser.isSet(QStringLiteral("description"))) {
        option = parser.value(QStringLiteral("description"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Description:"
                                     << "(" << option << ")";

        variables.setDescription(option);
    }

    if (parser.isSet(QStringLiteral("calendar"))) {
        option = parser.value(QStringLiteral("calendar"));
        bool ok = false;
        int colId = option.toInt(&ok);
        if (ok) {
            variables.setCollectionId(colId);
        }
    }

    /*
     * If there is location information
     */
    if (parser.isSet(QStringLiteral("location"))) {
        option = parser.value(QStringLiteral("location"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Location:"
                                     << "(" << option << ")";

        variables.setLocation(option);
    }

    /*
     * Show next happening and exit
     */
    if (parser.isSet(QStringLiteral("next"))) {
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Show next incidence only";

        variables.setNext(true);
    }

    /*
     * Set incidence unique string identifier
     */
    if (parser.isSet(QStringLiteral("uid"))) {
        option = parser.value(QStringLiteral("uid"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "incidence UID:"
                                     << "(" << option << ")";

        variables.setUID(option);
    }

    /*
     * Set starting date for calendar
     */
    if (parser.isSet(QStringLiteral("date"))) {
        option = parser.value(QStringLiteral("date"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Start date before conversion:"
                                     << "(" << option << ")";

        startdate = QDate::fromString(option, Qt::ISODate);
        if (!startdate.isValid()) {
            cout << i18n("Invalid Start Date Specified: %1", option).toLocal8Bit().data() << endl;
            return 1;
        }
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Start date after conversion:"
                                     << "(" << startdate.toString() << ")";
    }

    /*
     * Set starting time
     */
    if (parser.isSet(QStringLiteral("time"))) {
        option = parser.value(QStringLiteral("time"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Start time before conversion :"
                                     << "(" << option << ")";

        if (option.toUpper() != QLatin1String("FLOAT")) {
            if (option.count(QLatin1Char(':')) < 2) {
                // need to append seconds
                option.append(QLatin1String(":00"));
            }
            starttime = QTime::fromString(option, Qt::ISODate);
            if (!starttime.isValid()) {
                cout << i18n("Invalid Start Time Specified: %1", option).toLocal8Bit().data() << endl;
                return 1;
            }
            qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                         << "Start time after conversion:"
                                         << "(" << starttime.toString() << ")";
        } else {
            variables.setFloating(true);
            qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                         << "Floating event time specified";
        }
    }

    /*
     * Set end date for calendar
     */
    if (parser.isSet(QStringLiteral("end-date"))) {
        option = parser.value(QStringLiteral("end-date"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "End date before conversion:"
                                     << "(" << option << ")";

        enddate = QDate::fromString(option, Qt::ISODate);
        if (!enddate.isValid()) {
            cout << i18n("Invalid End Date Specified: %1", option).toLocal8Bit().data() << endl;
            return 1;
        }
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "End date after conversion:"
                                     << "(" << enddate.toString() << ")";
    }

    /*
     * Show next # days and exit
     */
    if (parser.isSet(QStringLiteral("show-next"))) {
        bool ok;

        option = parser.value(QStringLiteral("show-next"));
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Show" << option << "days ahead";
        variables.setDaysCount(option.toInt(&ok, 10));

        if (!ok) {
            cout << i18n("Invalid Date Count Specified: %1", option).toLocal8Bit().data() << endl;
            return 1;
        }

        enddate = startdate;
        enddate = enddate.addDays(variables.getDaysCount());
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "End date after conversion:"
                                     << "(" << enddate.toString() << ")";
    }

    /*
     * Set ending time
     */
    if (parser.isSet(QStringLiteral("end-time"))) {
        option = parser.value(QStringLiteral("end-time"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "End time before conversion:"
                                     << "(" << option << ")";

        if (option.toUpper() != QLatin1String("FLOAT")) {
            if (option.count(QLatin1Char(':')) < 2) {
                // need to append seconds
                option.append(QLatin1String(":00"));
            }
            endtime = QTime::fromString(option, Qt::ISODate);
            if (!endtime.isValid()) {
                cout << i18n("Invalid End Time Specified: %1", option).toLocal8Bit().data() << endl;
                return 1;
            }

            qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                         << "End time after conversion:"
                                         << "(" << endtime.toString() << ")";
        } else {
            variables.setFloating(true);
            qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                         << "Floating event time specified";
        }
    }

    /*
     * Set start date/time from epoch
     */
    time_t epochstart = 0;
    if (parser.isSet(QStringLiteral("epoch-start"))) {
        option = parser.value(QStringLiteral("epoch-start"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Epoch start:"
                                     << "(" << option << ")";

        epochstart = (time_t)option.toULong(nullptr, 10);
    }

    /*
     * Set end date/time from epoch
     */
    time_t epochend = 0;
    if (parser.isSet(QStringLiteral("epoch-end"))) {
        option = parser.value(QStringLiteral("epoch-end"));

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "Epoch end:"
                                     << "(" << option << ")";

        epochend = (time_t)option.toULong(nullptr, 10);
    }

    if (parser.isSet(QStringLiteral("all"))) {
        variables.setAll(true);
    } else {
        variables.setAll(false);
    }

    if (parser.isSet(QStringLiteral("import"))) {
        view = false;
        importFile = true;
        option = parser.value(QStringLiteral("import"));
        variables.setImportFile(option);

        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "importing file from:"
                                     << "(" << option << ")";
    }

    auto konsolekalendar = new KonsoleKalendar(&variables);

    if (parser.isSet(QStringLiteral("list-calendars"))) {
        konsolekalendar->printCalendarList();
        return 0;
    }

    QEventLoop loop;
    Akonadi::FetchJobCalendar::Ptr calendar = Akonadi::FetchJobCalendar::Ptr(new Akonadi::FetchJobCalendar());
    QObject::connect(calendar.data(), &Akonadi::FetchJobCalendar::loadFinished, &loop, &QEventLoop::quit);
    qCDebug(KONSOLEKALENDAR_LOG) << "Starting to load calendar";
    QElapsedTimer t;
    t.start();
    loop.exec();
    qCDebug(KONSOLEKALENDAR_LOG) << "Calendar loaded in" << t.elapsed() << "ms; success=" << !calendar->isLoading()
                                 << "; num incidences=" << calendar->incidences().count();

    variables.setCalendar(calendar);

    /***************************************************************************
     * Glorious date/time checking and setting code                            *
     ***************************************************************************/
    QDateTime startdatetime;
    QDateTime enddatetime;

    // Handle case with either date or end-date unspecified
    if (!parser.isSet(QStringLiteral("end-date")) && !parser.isSet(QStringLiteral("show-next")) && parser.isSet(QStringLiteral("date"))) {
        enddate = startdate;
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting enddate to startdate";
    } else if (parser.isSet(QStringLiteral("end-date")) && !parser.isSet(QStringLiteral("date"))) {
        startdate = enddate;
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting startdate to enddate";
    }

    // NOTE: If neither date nor end-date specified, then event will be today.

    // Case:
    //   End time (or epoch) unspecified, and start time (or epoch) IS specified.
    //   In this case, set the ending to 1 hour after starting.
    if (!parser.isSet(QStringLiteral("end-time")) && !parser.isSet(QStringLiteral("epoch-end"))) {
        if (parser.isSet(QStringLiteral("time"))) {
            endtime = starttime.addSecs(60 * 60); // end is 1 hour after start
            qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                         << "setting endtime 1 hour after starttime";
        } else if (parser.isSet(QStringLiteral("epoch-start"))) {
            startdatetime = epochs.epoch2QDateTime(epochstart);
            enddatetime = startdatetime.addSecs(60 * 60);
            qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                         << "setting endtime 1 hour after epochstart";
        }
    }

    // Case:
    //   Time (or epoch) unspecified, and end-time (or epoch) IS specified.
    //   In this case, set the starting to 1 hour before ending.
    if (!parser.isSet(QStringLiteral("time")) && !parser.isSet(QStringLiteral("epoch-start"))) {
        if (parser.isSet(QStringLiteral("end-time"))) {
            starttime = endtime.addSecs(-60 * 60); // start is 1 hour before end
            qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                         << "setting starttime 1 hour before endtime";
        } else if (parser.isSet(QStringLiteral("epoch-end"))) {
            enddatetime = epochs.epoch2QDateTime(epochend);
            startdatetime = enddatetime.addSecs(-60 * 60);
            qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                         << "setting starttime 1 before after epochend";
        }
    }

    // Case:
    //   Time (or epoch) unspecified, and end-time (or epoch) unspecified.
    if (!parser.isSet(QStringLiteral("time")) && !parser.isSet(QStringLiteral("epoch-start")) && !parser.isSet(QStringLiteral("end-time"))
        && !parser.isSet(QStringLiteral("epoch-end"))) {
        // set default start date/time
        startdatetime = QDateTime(startdate, starttime);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting startdatetime from"
                                     << "default startdate (today) and starttime";
        // set default end date/time
        enddatetime = QDateTime(enddate, endtime);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting enddatetime from"
                                     << "default enddate (today) and endtime";
    }

    // Set startdatetime, enddatetime if still necessary
    if (startdatetime.isNull()) {
        startdatetime = QDateTime(startdate, starttime);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting startdatetime from startdate and starttime";
    }
    if (enddatetime.isNull()) {
        enddatetime = QDateTime(enddate, endtime);
        qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp |"
                                     << "setting enddatetime from enddate and endtime";
    }

    // Float check for add mode:
    //   Events float if time AND end-time AND epoch times are UNspecified
    if (add) {
        if (!parser.isSet(QStringLiteral("time")) && !parser.isSet(QStringLiteral("end-time")) && !parser.isSet(QStringLiteral("epoch-start"))
            && !parser.isSet(QStringLiteral("epoch-end"))) {
            variables.setFloating(true);
            qCDebug(KONSOLEKALENDAR_LOG) << "main | floatingcheck |"
                                         << "turn-on floating event";
        }
    }

    // Finally! Set the start/end date times
    if (!change) {
        variables.setStartDateTime(startdatetime);
        variables.setEndDateTime(enddatetime);
    } else {
        // Do NOT set start/end datetimes in change mode,
        //   unless they were specified on commandline
        if (parser.isSet(QStringLiteral("time")) || parser.isSet(QStringLiteral("epoch-start")) || parser.isSet(QStringLiteral("end-time"))
            || parser.isSet(QStringLiteral("epoch-end"))) {
            variables.setStartDateTime(startdatetime);
            variables.setEndDateTime(enddatetime);
        }
    }

    // Some more debug prints
    qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp | StartDate=" << startdatetime.toString(Qt::TextDate);
    qCDebug(KONSOLEKALENDAR_LOG) << "main | datetimestamp | EndDate=" << enddatetime.toString(Qt::TextDate);

    /***************************************************************************
     * Sanity checks                                                           *
     ***************************************************************************/

    // Cannot combine modes
    if (create + view + add + change + del > 1) {
        cout << i18n(
                    "Only 1 operation mode "
                    "(view, add, change, delete, create) "
                    "permitted at any one time")
                    .toLocal8Bit()
                    .data()
             << endl;
        return 1;
    }

    // Cannot have a ending before starting
    if (startdatetime > enddatetime) {
        cout << i18n("Ending Date/Time occurs before the Starting Date/Time").toLocal8Bit().data() << endl;
        return 1;
    }

    /***************************************************************************
     * And away we go with the real work...                                    *
     ***************************************************************************/

    // Free up some memory.

    /*
     * Set our application name for use in unique IDs and error messages,
     * and product ID for incidence PRODID property
     */
    QString prodId = QStringLiteral("-//K Desktop Environment//NONSGML %1 %2//EN");
    CalFormat::setApplication(QLatin1String(progDisplay), prodId.arg(QLatin1String(progDisplay)).arg(QLatin1String(progVersion)));

    if (importFile) {
        if (konsolekalendar->importCalendar()) {
            cout << i18n("Calendar %1 successfully imported", variables.getImportFile()).toLocal8Bit().data() << endl;
            return 0;
        } else {
            cout << i18n("Unable to import calendar: %1", variables.getImportFile()).toLocal8Bit().data() << endl;
            return 1;
        }
    }

    if (add) {
        if (!konsolekalendar->isEvent(startdatetime, enddatetime, variables.getSummary())) {
            qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                         << "calling addEvent()";
            konsolekalendar->addEvent();
        } else {
            cout << i18n("Attempting to insert an event that already exists").toLocal8Bit().data() << endl;
            return 1;
        }
    }

    if (change) {
        qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                     << "calling changeEvent()";
        if (!variables.isUID()) {
            cout << i18n(
                        "Missing event UID: "
                        "use --uid command line option")
                        .toLocal8Bit()
                        .data()
                 << endl;
            return 1;
        }
        if (!konsolekalendar->changeEvent()) {
            cout << i18n("No such event UID: change event failed").toLocal8Bit().data() << endl;
            return 1;
        }
        qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                     << "successful changeEvent()";
    }

    if (del) {
        qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                     << "calling deleteEvent()";
        if (!variables.isUID()) {
            cout << i18n(
                        "Missing event UID: "
                        "use --uid command line option")
                        .toLocal8Bit()
                        .data()
                 << endl;
            return 1;
        }
        if (!konsolekalendar->deleteEvent()) {
            cout << i18n("No such event UID: delete event failed").toLocal8Bit().data() << endl;
            return 1;
        }
        qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                     << "successful deleteEvent()";
    }

    if (view) {
        qCDebug(KONSOLEKALENDAR_LOG) << "main | modework |"
                                     << "calling showInstance() to view events";
        if (!konsolekalendar->showInstance()) {
            cout << i18n("Cannot open specified export file: %1", variables.getExportFile()).toLocal8Bit().data() << endl;
            return 1;
        }
    }

    if (create) {
        qCDebug(KONSOLEKALENDAR_LOG) << "main | parse options |"
                                     << "creating Akonadi resource from file:"
                                     << "(" << variables.getCalendarFile() << ")";
        if (!konsolekalendar->createCalendar()) {
            cout << i18n("Cannot create Akonadi resource from file: %1", variables.getCalendarFile()).toLocal8Bit().data() << endl;
            return 1;
        }
    }

    delete konsolekalendar;

    qCDebug(KONSOLEKALENDAR_LOG) << "main | exiting";

    return 0;
}

//@endcond
