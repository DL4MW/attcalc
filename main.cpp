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
    dirMngr.SetDir(dirMngr.GetDir(DIR_PROGRAM)+QString("cable/"),DIR_DATA1);
    QString dirname=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    dirname+=QString("/cable/");
    dirMngr.SetDir(dirname.toLocal8Bit().constData(),DIR_DATA2);

    //ggf. Verzeichnisse noch anlegen, wenn nicht existent
    VerifyDir(dirMngr.GetDir(DIR_DATA1));
#ifndef PORTABLE
    VerifyDir(dirMngr.GetDir(DIR_DATA2));
#endif
//    if(!langFileName.isEmpty()){
        translator.load(dirMngr.CreatePath("english.qm",DIR_PROGRAM));
       a.installTranslator(&translator);
//    }


    Widget w;
    w.show();

    return a.exec();
}
