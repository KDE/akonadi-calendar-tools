/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "options.h"

Options::Options()
{
}

void Options::setAction(Options::Action action)
{
    m_action = action;
}

Options::Action Options::action() const
{
    return m_action;
}

QList<Akonadi::Collection::Id> Options::collections() const
{
    return m_collectionIds;
}

void Options::setCollections(const QList<Akonadi::Collection::Id> &collections)
{
    m_collectionIds = collections;
}

bool Options::testCollection(Akonadi::Collection::Id id) const
{
    return m_collectionIds.isEmpty() || m_collectionIds.contains(id);
}

bool Options::stripOldAlarms() const
{
    return m_stripOldAlarms;
}

void Options::setStripOldAlarms(bool strip)
{
    m_stripOldAlarms = strip;
}
