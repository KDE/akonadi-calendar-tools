/*
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "calendarjanitor.h"
#include "options.h"
#include "backuper.h"

#include "console-version.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <QString>
#include <qglobal.h>

#ifdef Q_OS_UNIX
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <unistd.h>
#endif

static const QString progName = QStringLiteral("calendarjanitor");
static const char progDisplay[] = I18N_NOOP("CalendarJanitor");

static const QString progVersion = QStringLiteral(KDEPIM_VERSION);
static const char progDesc[] = I18N_NOOP("A command line interface to report and fix errors in your calendar data");

static void print(const QString &message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

static void printCollectionsUsage()
{
    print(i18n("Error while parsing %1", QStringLiteral("--collections")));
    print(i18n("Example usage: %1", QStringLiteral("--collections 90,23,40")));
}

static void silenceStderr()
{
#ifdef Q_OS_UNIX
    if (qgetenv("KDE_CALENDARJANITOR_DEBUG") != "1") {
        //krazy:cond=syscalls since UNIX-only code
        // Disable stderr so we can actually read what's going on
        int fd = ::open("/dev/null", O_WRONLY);
        ::dup2(fd, 2);
        ::close(fd);
        //krazy:endcond=syscalls
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
    parser.addOptions({ colsOpt, fixOpt, backupOpt, stripOldAlarmsOpt });
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    Options janitorOptions;
    if (parser.isSet(colsOpt)) {
        QString option = parser.value(colsOpt);
        QStringList collections = option.split(QLatin1Char(','));
        QList<Akonadi::Collection::Id> ids;
        foreach (const QString &collection, collections) {
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
        print(i18n("--fix is incompatible with --backup"));
        return -1;
    }

    if (parser.isSet(stripOldAlarmsOpt) && parser.isSet(backupOpt)) {
        print(i18n("--strip-old-alarms is incompatible with --backup"));
        return -1;
    }

    if (parser.isSet(stripOldAlarmsOpt) && parser.isSet(fixOpt)) {
        print(i18n("--strip-old-alarms is incompatible with --fix"));
        return -1;
    }

    silenceStderr(); // Switching off mobile phones, movie is about to start

    janitorOptions.setStripOldAlarms(parser.isSet(stripOldAlarmsOpt));

    QString backupFile;
    if (parser.isSet(fixOpt)) {
        janitorOptions.setAction(Options::ActionScanAndFix);
        print(i18n("Running in fix mode."));
    } else if (parser.isSet(backupOpt)) {
        backupFile = parser.value(backupOpt);
        if (backupFile.isEmpty()) {
            print(i18n("Please specify a output file."));
            return -1;
        }
        janitorOptions.setAction(Options::ActionBackup);
    } else {
        print(i18n("Running in scan only mode."));
        janitorOptions.setAction(Options::ActionScan);
    }

    switch (janitorOptions.action()) {
    case Options::ActionBackup: {
        Backuper *backuper = new Backuper();
        backuper->backup(backupFile, janitorOptions.collections());
        break;
    }
    case Options::ActionScan:
    case Options::ActionScanAndFix: {
        CalendarJanitor *janitor = new CalendarJanitor(janitorOptions);
        janitor->start();
        break;
    }
    default:
        Q_ASSERT(false);
    }

    return app.exec();
}
