#include <QtCore>
#include <QtGui>
#include "widget.h"
#include <dirmngr.h>

DirMngr dirMngr;
QTranslator translator;

void VerifyDir(QString dirname)
{
   QDir qdir(dirname);
   if(!qdir.exists())qdir.mkpath(qdir.absolutePath());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("attCalc");

    dirMngr.SetProgramDir(QString::fromLocal8Bit(argv[0]));

    // Auf Android liegt argv[0] in einem read-only APK-Pfad.
    // Beschreibbares Datenverzeichnis über QStandardPaths ermitteln.
#ifdef Q_OS_ANDROID
    QString cableDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                       + "/cable/";
#else
    QString cableDir = dirMngr.GetDir(DIR_PROGRAM) + "cable/";
#endif
    dirMngr.SetDir(cableDir, DIR_DATA1);

    // Auf Android kein externer Speicher (Scoped Storage), DIR_DATA2 = DIR_DATA1
#ifdef Q_OS_ANDROID
    dirMngr.SetDir(dirMngr.GetDir(DIR_DATA1), DIR_DATA2);
#else
    QString dirname = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    dirname += QString("/cable/");
    dirMngr.SetDir(dirname.toLocal8Bit().constData(), DIR_DATA2);
#endif

    VerifyDir(dirMngr.GetDir(DIR_DATA1));
#ifndef PORTABLE
    VerifyDir(dirMngr.GetDir(DIR_DATA2));
#endif

    // Übersetzungsdatei aus Qt-Resources laden
    translator.load(":/english.qm");
    a.installTranslator(&translator);

    Widget w;
    w.show();

    return a.exec();
}
