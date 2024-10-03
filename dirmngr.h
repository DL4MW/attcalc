#ifndef DIRMNGR_H_
#define DIRMNGR_H_

#define DIR_PROGRAM   1
#define DIR_CONFIG    2
#define DIR_DATA1     3
#define DIR_DATA2     4
#define DIR_DATA3     5
#define DIR_TEMP      6
#define DIR_DATA4     7
#define DIR_DATA5     8

#include <QString>

class DirMngr{
	protected:
	QString programDir,
	        configDir,
	        data1Dir,
	        data2Dir,
	        data3Dir,
	        data4Dir,
	        data5Dir,
	        tempDir
	        ;

	public:
	DirMngr();
	void SetProgramDir(QString dir);
	void SetDir(QString dir,int where);
	QString GetDir(int where);
	QString CreatePath(QString fileName,int where);
};

#endif /*DIRMNGR_H_*/
