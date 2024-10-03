#include <QtCore>
#include "dirmngr.h"

#ifdef Q_WS_WIN
//#define DIR_SEP '\\'
//weil Qt alles mit Slash macht
#define DIR_SEP '/'
#else
#define DIR_SEP '/'
#endif


DirMngr::DirMngr()
{

}

void DirMngr::SetProgramDir(QString dir)
{
	QFileInfo applInfo(dir);
    programDir=applInfo.absolutePath();
	if(programDir.right(1)!="\\" && programDir.right(1)!="/")programDir+=DIR_SEP;
	configDir=programDir;
	data1Dir=programDir;
	data2Dir=programDir;
	data3Dir=programDir;
	data4Dir=programDir;
	data5Dir=programDir;
	tempDir=programDir;
}

void DirMngr::SetDir(QString dir,int where)
{
//	QFileInfo applInfo(dir);
//    	QString tmp=applInfo.absolutePath();
	QString tmp=dir;
	if(tmp.right(1)!="\\" && tmp.right(1)!="/")tmp+=DIR_SEP;
	switch(where){
		case DIR_PROGRAM:programDir=tmp;break;
		case DIR_CONFIG:configDir=tmp;break;
		case DIR_DATA1:data1Dir=tmp;break;
		case DIR_DATA2:data2Dir=tmp;break;
		case DIR_DATA3:data3Dir=tmp;break;
		case DIR_DATA4:data4Dir=tmp;break;
		case DIR_DATA5:data5Dir=tmp;break;
		case DIR_TEMP:tempDir=tmp;break;
	}//switch

}

QString DirMngr::GetDir(int where)
{
	switch(where){
		case DIR_PROGRAM:return programDir;
		case DIR_CONFIG:return configDir;
		case DIR_DATA1:return data1Dir;
		case DIR_DATA2:return data2Dir;
		case DIR_DATA3:return data3Dir;
		case DIR_DATA4:return data4Dir;
		case DIR_DATA5:return data5Dir;
		case DIR_TEMP:return tempDir;
	}//switch
	return QString("");
}

QString DirMngr::CreatePath(QString fileName,int where)
{
	return GetDir(where)+fileName;
}
