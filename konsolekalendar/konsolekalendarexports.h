/*******************************************************************************
 * konsolekalendarexports.h                                                    *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                             *
 ******************************************************************************/

#pragma once

#include <QTextStream>

#include "konsolekalendarvariables.h"

/**
 * @file konsolekalendarexports.h
 * Provides the KonsoleKalendarExports class definition.
 */

/**
 * @brief
 * Class to manage the Export functionality.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
class KonsoleKalendarExports
{
public:
    /**
     * Constructs a KonsoleKalendarChange object from command line arguments.
     * @param vars is a KonsoleKalendarVariable object with Event information.
     */
    explicit KonsoleKalendarExports(KonsoleKalendarVariables *vars = nullptr);

    /**
     * Destructor
     */
    ~KonsoleKalendarExports();

    /**
     * Export the Event in Text Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     */
    bool exportAsTxt(QTextStream *ts, const KCalendarCore::Event::Ptr &event, const QDate &date);

    /**
     * Export the Event in Short Text Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     * @param sameday flags that this Event is on the same date as the
     * previously exported Event.
     */
    bool exportAsTxtShort(QTextStream *ts, const KCalendarCore::Event::Ptr &event, const QDate &date, bool sameday);

    /**
     * Export the Event in Comma-Separated Values (CSV) Mode.
     * @param ts pointer to the output QTextStream.
     * @param event pointer to the Event to export.
     * @param date is the QDate to be exported for.
     */
    bool exportAsCSV(QTextStream *ts, const KCalendarCore::Event::Ptr &event, const QDate &date);

private:
    //@cond PRIVATE
    KonsoleKalendarVariables *m_variables = nullptr;
    bool m_firstEntry;
    //@endcond

    /**
     * Processes a field for Comma-Separated Value (CSV) compliance:
     *   1. Replaces double quotes by a pair of consecutive double quotes
     *   2. Surrounds field with double quotes
     * @param field is the field value to be processed.
     * @param dquote is a QString containing the double quote character.
     */
    QString processField(const QString &field, const QString &dquote);
};

