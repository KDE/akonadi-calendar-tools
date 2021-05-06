/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "backuper.h"
#include "calendarjanitor.h"
#include "options.h"

#include "console-version.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <qglobal.h>

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static const QString progName = QStringLiteral("calendarjanitor");
static const char progDisplay[] = I18N_NOOP("CalendarJanitor");

static const QString progVersion = QStringLiteral(KDEPIM_VERSION);
static const char progDesc[] = I18N_NOOP("A command line interface to report and fix errors in your calendar data");

#ifndef COMPILE_WITH_UNITY_CMAKE_SUPPORT
static void printOut(const QString &message)
{
    QTextStream out(stdout);
    out << message << "\n";
}
#endif

static void printCollectionsUsage()
{
    printOut(i18n("Error while parsing %1", QStringLiteral("--collections")));
    printOut(i18n("Example usage: %1", QStringLiteral("--collections 90,23,40")));
}

static void silenceStderr()
{
#ifdef Q_OS_UNIX
    if (qgetenv("KDE_CALENDARJANITOR_DEBUG") != "1") {
        // krazy:cond=syscalls since UNIX-only code
        // Disable stderr so we can actually read what's going on
        int fd = ::open("/dev/null", O_WRONLY);
        ::dup2(fd, 2);
        ::close(fd);
        // krazy:endcond=syscalls
    }
#endif
}

int main(int argv, char *argc[])
{
    KLocalizedString::setApplicationDomain("calendarjanitor");
    KAboutData aboutData(progName, i18n(progDisplay), progVersion);
    aboutData.setLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
    aboutData.addAuthor(i18n("Sérgio Martins"), i18n("Maintainer"), QStringLiteral("iamsergiogmail.com"));
    aboutData.setShortDescription(i18n(progDesc));

    QCoreApplication app(argv, argc);

    QCommandLineParser parser;
    QCommandLineOption colsOpt(QStringLiteral("collections"), i18n("Comma-separated list of collection ids to scan"), QStringLiteral("ids"));
    QCommandLineOption fixOpt(QStringLiteral("fix"), i18n("Fix broken incidences"));
    QCommandLineOption backupOpt(QStringLiteral("backup"), i18n("Backup your calendar"), QStringLiteral("output.ics"));
    QCommandLineOption stripOldAlarmsOpt(QStringLiteral("strip-old-alarms"), i18n("Delete alarms older than 365 days"));
    parser.addOptions({colsOpt, fixOpt, backupOpt, stripOldAlarmsOpt});
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    Options janitorOptions;
    if (parser.isSet(colsOpt)) {
        QString option = parser.value(colsOpt);
        const QStringList collections = option.split(QLatin1Char(','));
        QList<Akonadi::Collection::Id> ids;
        for (const QString &collection : collections) {
            bool ok = false;
            int num = collection.toInt(&ok);
            if (ok) {
                ids << num;
            } else {
                printCollectionsUsage();
                return -1;
            }

            if (ids.isEmpty()) {
                printCollectionsUsage();
                return -1;
            } else {
                janitorOptions.setCollections(ids);
            }
        }
    }

    if (parser.isSet(fixOpt) && parser.isSet(backupOpt)) {
        printOut(i18n("--fix is incompatible with --backup"));
        return -1;
    }

    if (parser.isSet(stripOldAlarmsOpt) && parser.isSet(backupOpt)) {
        printOut(i18n("--strip-old-alarms is incompatible with --backup"));
        return -1;
    }

    if (parser.isSet(stripOldAlarmsOpt) && parser.isSet(fixOpt)) {
        printOut(i18n("--strip-old-alarms is incompatible with --fix"));
        return -1;
    }

    silenceStderr(); // Switching off mobile phones, movie is about to start

    janitorOptions.setStripOldAlarms(parser.isSet(stripOldAlarmsOpt));

    QString backupFile;
    if (parser.isSet(fixOpt)) {
        janitorOptions.setAction(Options::ActionScanAndFix);
        printOut(i18n("Running in fix mode."));
    } else if (parser.isSet(backupOpt)) {
        backupFile = parser.value(backupOpt);
        if (backupFile.isEmpty()) {
            printOut(i18n("Please specify a output file."));
            return -1;
        }
        janitorOptions.setAction(Options::ActionBackup);
    } else {
        printOut(i18n("Running in scan only mode."));
        janitorOptions.setAction(Options::ActionScan);
    }

    switch (janitorOptions.action()) {
    case Options::ActionBackup: {
        auto backuper = new Backuper();
        backuper->backup(backupFile, janitorOptions.collections());
        break;
    }
    case Options::ActionScan:
    case Options::ActionScanAndFix: {
        auto janitor = new CalendarJanitor(janitorOptions);
        janitor->start();
        break;
    }
    default:
        Q_ASSERT(false);
    }

    return app.exec();
}
