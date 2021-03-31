/******************************************************************************
 * konsolekalendaradd.h                                                       *
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

/**
 * @file konsolekalendaradd.h
 * Provides the KonsoleKalendarAdd class definition.
 */

/**
 * @brief
 * Class to manage the Event insertion capability.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
class KonsoleKalendarAdd
{
public:
    /**
     * Constructs a KonsoleKalendarAdd object from command line arguments.
     * @param vars is a KonsoleKalendarVariable object with Event information.
     */
    explicit KonsoleKalendarAdd(KonsoleKalendarVariables *vars);
    /**
     * Destructor
     */
    ~KonsoleKalendarAdd();

    /**
     * Add the Event.
     */
    bool addEvent();

    /**
     * Imports calendar file to current Calendar.
     */
    bool addImportedCalendar();

private:
    /**
     * Print event specs for dryrun and verbose options
     */
    void printSpecs();

    /**
     * Variable to how to make it
     */
    KonsoleKalendarVariables *m_variables = nullptr;
};
