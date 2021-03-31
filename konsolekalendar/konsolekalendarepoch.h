/******************************************************************************
 * konsolekalendarepoch.h                                                     *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/

#pragma once

#include <QDateTime>

/**
 * @file konsolekalendarepoch.h
 * Provides the KonsoleKalendarEpoch class definition.
 */

/**
 * @brief
 * Class for timestamps expressed as epochs.
 * @author Allen Winter
 */
class KonsoleKalendarEpoch
{
public:
    /**
     * Constructor.
     */
    KonsoleKalendarEpoch();

    /**
     * Destructor
     */
    ~KonsoleKalendarEpoch();

    /**
     * Converts epoch time to QDateTime format.
     * @param epoch epoch time.
     */
    static QDateTime epoch2QDateTime(uint epoch);

    /**
     * Converts QT DateTime to epoch format.
     * @param dt is a QDateTime to convert to an epoch.
     */
    static uint QDateTime2epoch(const QDateTime &dt);
};

