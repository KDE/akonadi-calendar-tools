/******************************************************************************
 * konsolekalendardelete.h                                                    *
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

#include <KCalendarCore/Event>

/**
 * @file konsolekalendardelete.h
 * Provides the KonsoleKalendarDelete class definition.
 */

/**
 * @brief
 * Class to manage the Event removal capability.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
class KonsoleKalendarDelete
{
public:
    /**
     * Constructs a KonsoleKalendarDelete object from command line arguments.
     *
     * @param vars is a pointer to the #KonsoleKalendarVariables object
     * which contains all the command line arguments.
     */
    explicit KonsoleKalendarDelete(KonsoleKalendarVariables *vars);

    /**
     * Destructor
     */
    ~KonsoleKalendarDelete();

    /**
     * Delete the Event.
     */
    bool deleteEvent();

private:
    /**
     * Print event specs for dryrun and verbose options.
     *
     * @param event is a pointer to an Event that is to be printed.
     */
    void printSpecs(const KCalendarCore::Event::Ptr &event);

    //@cond PRIVATE
    KonsoleKalendarVariables *m_variables = nullptr;
    //@endcond
};

