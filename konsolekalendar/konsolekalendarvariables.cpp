/******************************************************************************
 * konsolekalendarvariables.cpp                                               *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 *****************************************************************************/
/**
 * @file konsolekalendarvariables.cpp
 * Provides the KonsoleKalendarVariables class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendarvariables.h"

#include "konsolekalendar_debug.h"
#include <KLocalizedString>
#include <kconfig.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace KCalendarCore;
using namespace std;

KonsoleKalendarVariables::KonsoleKalendarVariables()
{
    m_bIsUID = false;
    m_bIsStartDateTime = false;
    m_bIsEndDateTime = false;
    m_bNext = false;
    m_bVerbose = false;
    m_bDryRun = false;
    m_bUseEvents = false;
    m_bUseTodos = false;
    m_bUseJournals = false;
    m_exportType = ExportTypeText;
    m_bIsExportFile = false;
    m_bDescription = false;
    m_description.clear();
    m_bLocation = false;
    m_location = QStringLiteral("Default location"); // i18n ?
    m_bSummary = false;
    m_summary = QStringLiteral("Default summary"); // i18n?
    m_bFloating = true;
    m_bAllowGui = false;
    m_collectionId = -1;
}

KonsoleKalendarVariables::~KonsoleKalendarVariables() = default;

void KonsoleKalendarVariables::setUID(const QString &uid)
{
    m_bIsUID = true;
    m_UID = uid;
}

QString KonsoleKalendarVariables::getUID() const
{
    return m_UID;
}

bool KonsoleKalendarVariables::isUID() const
{
    return m_bIsUID;
}

void KonsoleKalendarVariables::setStartDateTime(const QDateTime &start)
{
    m_bIsStartDateTime = true;
    m_startDateTime = start;
}

QDateTime KonsoleKalendarVariables::getStartDateTime() const
{
    return m_startDateTime;
}

bool KonsoleKalendarVariables::isStartDateTime() const
{
    return m_bIsStartDateTime;
}

void KonsoleKalendarVariables::setEndDateTime(const QDateTime &end)
{
    m_bIsEndDateTime = true;
    m_endDateTime = end;
}

QDateTime KonsoleKalendarVariables::getEndDateTime() const
{
    return m_endDateTime;
}

bool KonsoleKalendarVariables::isEndDateTime() const
{
    return m_bIsEndDateTime;
}

void KonsoleKalendarVariables::setNext(bool next)
{
    m_bNext = next;
}

bool KonsoleKalendarVariables::isNext() const
{
    return m_bNext;
}

void KonsoleKalendarVariables::setVerbose(bool verbose)
{
    m_bVerbose = verbose;
}

bool KonsoleKalendarVariables::isVerbose() const
{
    return m_bVerbose;
}

void KonsoleKalendarVariables::setDryRun(bool dryrun)
{
    m_bDryRun = dryrun;
}

bool KonsoleKalendarVariables::isDryRun() const
{
    return m_bDryRun;
}

void KonsoleKalendarVariables::setUseEvents(bool useEvents)
{
    m_bUseEvents = useEvents;
}

bool KonsoleKalendarVariables::getUseEvents() const
{
    return m_bUseEvents;
}

void KonsoleKalendarVariables::setUseTodos(bool useTodos)
{
    m_bUseTodos = useTodos;
}

bool KonsoleKalendarVariables::getUseTodos() const
{
    return m_bUseTodos;
}

void KonsoleKalendarVariables::setUseJournals(bool useJournals)
{
    m_bUseJournals = useJournals;
}

bool KonsoleKalendarVariables::getUseJournals() const
{
    return m_bUseJournals;
}

void KonsoleKalendarVariables::setCalendarFile(const QString &calendar)
{
    m_calendarFile = calendar;
}

QString KonsoleKalendarVariables::getCalendarFile() const
{
    return m_calendarFile;
}

void KonsoleKalendarVariables::setImportFile(const QString &calendar)
{
    m_import = calendar;
}

QString KonsoleKalendarVariables::getImportFile() const
{
    return m_import;
}

void KonsoleKalendarVariables::setCalendar(const Akonadi::FetchJobCalendar::Ptr &resources)
{
    m_calendar = resources;
}

Akonadi::FetchJobCalendar::Ptr KonsoleKalendarVariables::getCalendar() const
{
    return m_calendar;
}

void KonsoleKalendarVariables::setExportType(ExportType exportType)
{
    m_exportType = exportType;
}

ExportType KonsoleKalendarVariables::getExportType() const
{
    return m_exportType;
}

void KonsoleKalendarVariables::setExportFile(const QString &export_file)
{
    m_exportFile = export_file;
    m_bIsExportFile = true;
}

bool KonsoleKalendarVariables::isExportFile() const
{
    return m_bIsExportFile;
}

QString KonsoleKalendarVariables::getExportFile() const
{
    return m_exportFile;
}

bool KonsoleKalendarVariables::isAll() const
{
    return m_bAll;
}

void KonsoleKalendarVariables::setAll(bool all)
{
    m_bAll = all;
}

bool KonsoleKalendarVariables::getAll() const
{
    return m_bAll;
}

void KonsoleKalendarVariables::setDescription(const QString &description)
{
    m_bDescription = true;
    m_description = description;
}

QString KonsoleKalendarVariables::getDescription() const
{
    return m_description;
}

bool KonsoleKalendarVariables::isDescription() const
{
    return m_bDescription;
}

void KonsoleKalendarVariables::setLocation(const QString &location)
{
    m_bLocation = true;
    m_location = location;
}

QString KonsoleKalendarVariables::getLocation() const
{
    return m_location;
}

bool KonsoleKalendarVariables::isLocation() const
{
    return m_bLocation;
}

void KonsoleKalendarVariables::setSummary(const QString &summary)
{
    m_bSummary = true;
    m_summary = summary;
}

QString KonsoleKalendarVariables::getSummary() const
{
    return m_summary;
}

bool KonsoleKalendarVariables::isSummary() const
{
    return m_bSummary;
}

void KonsoleKalendarVariables::setFloating(bool floating)
{
    m_bFloating = floating;
}

bool KonsoleKalendarVariables::getFloating() const
{
    return m_bFloating;
}

void KonsoleKalendarVariables::setDaysCount(int count)
{
    m_daysCount = count;
    m_bDaysCount = true;
}

int KonsoleKalendarVariables::getDaysCount() const
{
    return m_daysCount;
}

bool KonsoleKalendarVariables::isDaysCount() const
{
    return m_bDaysCount;
}

void KonsoleKalendarVariables::setAllowGui(bool allow)
{
    m_bAllowGui = allow;
}

void KonsoleKalendarVariables::setCollectionId(Akonadi::Collection::Id id)
{
    m_collectionId = id;
}

Akonadi::Collection::Id KonsoleKalendarVariables::collectionId() const
{
    return m_collectionId;
}

bool KonsoleKalendarVariables::allowGui() const
{
    return m_bAllowGui;
}
