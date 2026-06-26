/******************************************************************************
 * konsolekalendarchange.cpp                                                  *
 *                                                                            *
 * KonsoleKalendar is a command line interface to KDE calendars               *
 * SPDX-FileCopyrightText: 2002-2004 Tuukka Pasanen <illuusio@mailcity.com>   *
 * SPDX-FileCopyrightText: 2003-2005 Allen Winter <winter@kde.org>            *
 *                                                                            *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0 *
 *                                                                            *
 ******************************************************************************/
/**
 * @file konsolekalendarchange.cpp
 * Provides the KonsoleKalendarChange class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include "konsolekalendarchange.h"

#include <KLocalizedString>

#include <QElapsedTimer>
#include <QEventLoop>

#include <cstdlib>
#include <iostream>

using namespace KCalendarCore;
using namespace std;

KonsoleKalendarChange::KonsoleKalendarChange(KonsoleKalendarVariables *vars)
{
    m_variables = vars;
}

bool KonsoleKalendarChange::changeEvent()
{
    bool status = false;

    /*
     * Retrieve event on the basis of the unique string ID
     */
    Event::Ptr const event = m_variables->getCalendar()->event(m_variables->getUID());
    if (event) {
        if (m_variables->isDryRun()) {
            cout << i18n("Dry Run: Change Event:").toLocal8Bit().data() << '\n';
            printSpecs(event);

            cout << i18n("Dry Run: To Event:").toLocal8Bit().data() << '\n';
            printSpecs();
        } else {
            if (m_variables->isVerbose()) {
                cout << i18n("Change Event:").toLocal8Bit().data() << '\n';
                printSpecs(event);

                cout << i18n("To Event:").toLocal8Bit().data() << '\n';
                printSpecs();
            }

            event->startUpdates();
            Akonadi::CalendarBase::Ptr const calendar = m_variables->getCalendar();
            const auto timeZone = calendar->timeZone();
            if (m_variables->isStartDateTime()) {
                event->setDtStart(m_variables->getStartDateTime().toTimeZone(timeZone));
            }

            if (m_variables->isEndDateTime()) {
                event->setDtEnd(m_variables->getEndDateTime().toTimeZone(timeZone));
            }

            event->setAllDay(m_variables->getFloating());

            if (m_variables->isSummary()) {
                event->setSummary(m_variables->getSummary());
            }

            if (m_variables->isDescription()) {
                event->setDescription(m_variables->getDescription());
            }

            if (m_variables->isLocation()) {
                event->setLocation(m_variables->getLocation());
            }
            event->endUpdates();
            QEventLoop loop;
            QObject::connect(calendar.data(), &Akonadi::CalendarBase::modifyFinished, &loop, &QEventLoop::quit);
            QElapsedTimer t;
            t.start();
            calendar->modifyIncidence(event);
            loop.exec();

            status = *event == *calendar->incidence(event->uid());

            if (status) {
                cout << i18n("Success: \"%1\" changed", event->summary()).toLocal8Bit().data() << '\n';
            } else {
                cout << i18n("Failure: \"%1\" not changed", event->summary()).toLocal8Bit().data() << '\n';
            }
        }
    }

    return status;
}

void KonsoleKalendarChange::printSpecs(const KCalendarCore::Event::Ptr &event)
{
    cout << i18n("  UID:   %1", event->uid()).toLocal8Bit().data() << '\n';

    cout << i18n("  What:  %1", event->summary()).toLocal8Bit().data() << '\n';

    const auto timeZone = m_variables->getCalendar()->timeZone();
    cout << i18n("  Begin: %1", event->dtStart().toTimeZone(timeZone).toString(Qt::TextDate)).toLocal8Bit().data() << '\n';

    cout << i18n("  End:   %1", event->dtEnd().toTimeZone(timeZone).toString(Qt::TextDate)).toLocal8Bit().data() << '\n';

    cout << i18n("  Desc:  %1", event->description()).toLocal8Bit().data() << '\n';

    cout << i18n("  Location:  %1", event->location()).toLocal8Bit().data() << '\n';
}

void KonsoleKalendarChange::printSpecs()
{
    cout << i18n("  UID:   %1", m_variables->getUID()).toLocal8Bit().data() << '\n';

    cout << i18n("  What:  %1", m_variables->getSummary()).toLocal8Bit().data() << '\n';

    cout << i18n("  Begin: %1", m_variables->getStartDateTime().toString(Qt::TextDate)).toLocal8Bit().data() << '\n';

    cout << i18n("  End:   %1", m_variables->getEndDateTime().toString(Qt::TextDate)).toLocal8Bit().data() << '\n';

    cout << i18n("  Desc:  %1", m_variables->getDescription()).toLocal8Bit().data() << '\n';

    cout << i18n("  Location:  %1", m_variables->getLocation()).toLocal8Bit().data() << '\n';
}
