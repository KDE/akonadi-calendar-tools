/******************************************************************************
 * konsolekalendarchange.h                                                    *
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
 * @file konsolekalendarchange.h
 * Provides the KonsoleKalendarChange class definition.
 */

/**
 * @brief
 * Class to manage the Event modification capability.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
class KonsoleKalendarChange
{
public:
    /**
     * Constructs a KonsoleKalendarChange object from command line arguments.
     * @param vars is a KonsoleKalendarVariable object with Event information.
     */
    explicit KonsoleKalendarChange(KonsoleKalendarVariables *vars);

    /**
     * Destructor
     */
    ~KonsoleKalendarChange();

    /**
     * Modify the Event.
     */
    bool changeEvent();

private:
    /**
     * Print Event specs for dryrun and verbose options.
     *
     * @param event is a pointer to an Event to print.
     */
    void printSpecs(const KCalendarCore::Event::Ptr &event);

    /**
     * Print Event specs as provided from the command line arguments.
     */
    void printSpecs();

    //@cond PRIVATE
    KonsoleKalendarVariables *m_variables = nullptr;
    //@endcond
};

