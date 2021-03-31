/******************************************************************************
 * konsolekalendar.h                                                          *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/

#pragma once

#include "konsolekalendarvariables.h"

#include <Akonadi/Calendar/FetchJobCalendar>
#include <KCalendarCore/Event>

#include <QDateTime>

class QTextStream;

/**
 * @file konsolekalendar.h
 * Provides the KonsoleKalendar class definition.
 */

/**
 * @brief
 * The base class of the project.
 * @author Tuukka Pasanen
 */
class KonsoleKalendar
{
public:
    /**
     * Constructs a KonsoleKalendar object from command line arguments.
     *
     * @param variables is a pointer to a #KonsoleKalendarVariables object
     * containing all the command line arguments.
     */
    explicit KonsoleKalendar(KonsoleKalendarVariables *variables);

    /**
     * Destructor
     */
    ~KonsoleKalendar();

    /**
     * Visualize what we need.
     */
    bool showInstance();

    /**
     * Imports calendar file
     */
    bool importCalendar();

    /**
     * Add event to calendar
     */
    bool addEvent();

    /**
     * Change event
     */
    bool changeEvent();

    /**
     * Delete event
     */
    bool deleteEvent();

    /**
     * Detect if event already exists
     *
     * @param  startdate Starting date
     * @param  enddate   Ending date
     * @param  summary   Which summary event should have have
     */
    bool isEvent(const QDateTime &startdate, const QDateTime &enddate, const QString &summary);

    /**
     * Creates calendar file (If it doesn't exists)
     */
    bool createCalendar();

    /**
     * Prints the available calendars.
     */
    bool printCalendarList();

private:
    /**
     * Print event specs for dryrun and verbose options
     */
    void printSpecs();

    /**
     * Creates an akonadi resource of type ical.
     */
    bool createAkonadiResource(const QString &icalFileUri);

    /**
     * Prints event list in many formats
     *
     * @param ts is the #QTextStream to be printed
     * @param eventList which event we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEventList(QTextStream *ts, KCalendarCore::Event::List *eventList, QDate dt);

    /**
     * Prints a single event in many formats
     *
     * @param ts is the #QTextStream to be printed
     * @param event which we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEvent(QTextStream *ts, const KCalendarCore::Event::Ptr &event, QDate dt);

    /**
     * Variables that changes stuff in program
     */
    KonsoleKalendarVariables *m_variables = nullptr;

    /**
     * Calendar file itself
     */
    Akonadi::FetchJobCalendar::Ptr m_calendar;

    /**
     * This is useful if we like to have same day events to same system
     */
    QDate m_saveDate;
};

