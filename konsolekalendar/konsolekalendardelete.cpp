/******************************************************************************
 * konsolekalendardelete.cpp                                                  *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/
/**
 * @file konsolekalendardelete.cpp
 * Provides the KonsoleKalendarDelete class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendardelete.h"

#include <iostream>
#include <stdlib.h>

#include "konsolekalendar_debug.h"
#include <KLocalizedString>
#include <QEventLoop>

using namespace KCalendarCore;
using namespace std;

KonsoleKalendarDelete::KonsoleKalendarDelete(KonsoleKalendarVariables *vars)
{
    m_variables = vars;
}

KonsoleKalendarDelete::~KonsoleKalendarDelete()
{
}

bool KonsoleKalendarDelete::deleteEvent()
{
    bool status = false;

    qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendardelete.cpp::deleteEvent()";

    /*
     * Retrieve event on the basis of the unique string ID
     */
    Event::Ptr event = m_variables->getCalendar()->event(m_variables->getUID());
    if (event) {
        if (m_variables->isDryRun()) {
            cout << i18n("Delete Event <Dry Run>:").data() << endl;
            printSpecs(event);
        } else {
            qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendardelete.cpp:deleteEvent() :" << m_variables->getUID().data();

            if (m_variables->isVerbose()) {
                cout << i18n("Delete Event <Verbose>:").data() << endl;
                printSpecs(event);
            }

            QEventLoop loop;
            Akonadi::CalendarBase::Ptr calendar = m_variables->getCalendar();
            QObject::connect(calendar.data(), &Akonadi::CalendarBase::deleteFinished, &loop, &QEventLoop::quit);
            calendar->deleteEvent(event);
            loop.exec();
            qCDebug(KONSOLEKALENDAR_LOG) << "Finished deleting";
            status = calendar->incidence(event->uid()) == nullptr;

            if (status) {
                cout << i18n("Success: \"%1\" deleted", event->summary()).data() << endl;
            } else {
                cout << i18n("Error deleting: \"%1\"", event->summary()).data() << endl;
            }
        }
    }

    qCDebug(KONSOLEKALENDAR_LOG) << "konsolekalendardelete.cpp::deleteEvent() | Done";
    return status;
}

void KonsoleKalendarDelete::printSpecs(const Event::Ptr &event)
{
    cout << i18n("  UID:   %1", m_variables->getUID()).data() << endl;

    cout << i18n("  What:  %1", event->summary()).data() << endl;

    const auto timeZone = m_variables->getCalendar()->timeZone();
    cout << i18n("  Begin: %1", event->dtStart().toTimeZone(timeZone).toString(Qt::TextDate)).data() << endl;

    cout << i18n("  End:   %1", event->dtEnd().toTimeZone(timeZone).toString(Qt::TextDate)).data() << endl;

    cout << i18n("  Desc:  %1", event->description()).data() << endl;

    cout << i18n("  Location:  %1", event->location()).data() << endl;
}
