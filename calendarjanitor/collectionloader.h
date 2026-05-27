/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include <Akonadi/Collection>

#include <QObject>

class KJob;

class CollectionLoader : public QObject
{
    Q_OBJECT
public:
    explicit CollectionLoader(QObject *parent = nullptr);
    void load();
    Akonadi::Collection::List collections() const;

Q_SIGNALS:
    void loaded(bool success);

private Q_SLOTS:
    void onCollectionsLoaded(KJob *);

private:
    Akonadi::Collection::List m_collections;
};
