/******************************************************************************
 * konsolekalendarexports.cpp                                                 *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/
/**
 * @file konsolekalendarexports.cpp
 * Provides the KonsoleKalendarExports class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendarexports.h"

#include "konsolekalendar_debug.h"

#include <KCalendarCore/Event>

#include <KLocalizedString>
#include <QDateTime>
#include <QLocale>

#include <iostream>
#include <stdlib.h>

using namespace KCalendarCore;
using namespace std;

KonsoleKalendarExports::KonsoleKalendarExports(KonsoleKalendarVariables *vars)
{
    m_variables = vars;
    m_firstEntry = true;
}

KonsoleKalendarExports::~KonsoleKalendarExports()
{
}

bool KonsoleKalendarExports::exportAsTxt(QTextStream *ts, const Event::Ptr &event, const QDate &date)
{
    // Export "Text" Format:
    //
    // Date:\t<Incidence Date>(dddd yyyy-MM-dd)
    // [\t<Incidence Start Time>(hh:mm) - <Incidence End Time>(hh:mm)]
    // Summary:
    // \t<Incidence Summary | "(no summary available)">
    // Location:
    // \t<Incidence Location | "(no location available)">
    // Description:
    // \t<Incidence Description | "(no description available)">
    // UID:
    // \t<Incidence UID>
    // --------------------------------------------------

    QLocale locale;

    // Print Event Date (in user's preferred format)
    *ts << i18n("Date:") << "\t" << locale.toString(date) << '\n';

    // Print Event Starttime - Endtime, for Non-All-Day Events Only
    if (!event->allDay()) {
        *ts << "\t" << locale.toString(event->dtStart().time()) << " - " << locale.toString(event->dtEnd().time());
    }
    *ts << '\n';

    // Print Event Summary
    *ts << i18n("Summary:") << '\n';
    if (!event->summary().isEmpty()) {
        *ts << "\t" << event->summary() << '\n';
    } else {
        *ts << "\t" << i18n("(no summary available)") << '\n';
    }

    // Print Event Location
    *ts << i18n("Location:") << '\n';
    if (!event->location().isEmpty()) {
        *ts << "\t" << event->location() << '\n';
    } else {
        *ts << "\t" << i18n("(no location available)") << '\n';
    }

    // Print Event Description
    *ts << i18n("Description:") << '\n';
    if (!event->description().isEmpty()) {
        *ts << "\t" << event->description() << '\n';
    } else {
        *ts << "\t" << i18n("(no description available)") << '\n';
    }

    // Print Event UID
    *ts << i18n("UID:") << '\n' << "\t" << event->uid() << '\n';

    // Print Line Separator
    *ts << "--------------------------------------------------" << '\n';

    return true;
}

bool KonsoleKalendarExports::exportAsTxtShort(QTextStream *ts, const Event::Ptr &event, const QDate &date, bool sameday)
{
    // Export "Text-Short" Format:
    //
    // [--------------------------------------------------]
    // {<Incidence Date>(dddd yyyy-MM-dd)]
    // [<Incidence Start Time>(hh:mm) - <Incidence End Time>(hh:mm) | "\t"]
    // \t<Incidence Summary | \t>[, <Incidence Location>]
    // \t\t<Incidence Description | "\t">
    QLocale locale;

    if (!sameday) {
        // If a new date, then Print the Event Date (in user's preferred format)
        *ts << locale.toString(date) << ":" << '\n';
    }

    // Print Event Starttime - Endtime
    if (!event->allDay()) {
        *ts << locale.toString(event->dtStart().time()) << " - " << locale.toString(event->dtEnd().time());
    } else {
        *ts << i18n("[all day]\t");
    }
    *ts << "\t";

    // Print Event Summary
    *ts << event->summary().replace(QLatin1Char('\n'), QLatin1Char(' '));

    // Print Event Location
    if (!event->location().isEmpty()) {
        if (!event->summary().isEmpty()) {
            *ts << ", ";
        }
        *ts << event->location().replace(QLatin1Char('\n'), QLatin1Char(' '));
    }
    *ts << '\n';

    // Print Event Description
    if (!event->description().isEmpty()) {
        *ts << "\t\t\t" << event->description().replace(QLatin1Char('\n'), QLatin1Char(' ')) << '\n';
    }

    // By user request, no longer print UIDs if export-type==short

    // Print Separator
    *ts << '\n';

    return true;
}

QString KonsoleKalendarExports::processField(const QString &field, const QString &dquote)
{
    // little function that processes a field for CSV compliance:
    //   1. Replaces double quotes by a pair of consecutive double quotes
    //   2. Surrounds field with double quotes

    QString double_dquote = dquote + dquote;
    QString retField = field;
    retField = dquote + retField.replace(dquote, double_dquote) + dquote;
    return retField;
}

//@cond IGNORE
#define pF(x) processField((x), dquote)
//@endcond

bool KonsoleKalendarExports::exportAsCSV(QTextStream *ts, const Event::Ptr &event, const QDate &date)
{
    // Export "CSV" Format:
    //
    // startdate,starttime,enddate,endtime,summary,location,description,UID

    QString delim = i18n(","); // character to use as CSV field delimiter
    QString dquote = i18n("\""); // character to use to quote CSV fields
    if (!event->allDay()) {
        *ts << pF(date.toString(Qt::ISODate)) << delim << pF(event->dtStart().time().toString(Qt::ISODate)) << delim << pF(date.toString(Qt::ISODate)) << delim
            << pF(event->dtEnd().time().toString(Qt::ISODate));
    } else {
        *ts << pF(date.toString(Qt::ISODate)) << delim << pF(QLatin1String("")) << delim << pF(date.toString(Qt::ISODate)) << delim << pF(QLatin1String(""));
    }

    *ts << delim << pF(event->summary().replace(QLatin1Char('\n'), QLatin1Char(' '))) << delim
        << pF(event->location().replace(QLatin1Char('\n'), QLatin1Char(' '))) << delim << pF(event->description().replace(QLatin1Char('\n'), QLatin1Char(' ')))
        << delim << pF(event->uid()) << '\n';

    return true;
}
