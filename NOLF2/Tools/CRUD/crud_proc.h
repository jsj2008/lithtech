// Main CRUD processing header

#ifndef __CRUD_PROC_H__
#define __CRUD_PROC_H__

#include <list.h>
#include <string.h>

typedef list<string> TFileList;

class CCRUDResults
{
public:
	TDateTime m_ProcessingTime;
	int m_iFilesFound;
	int m_iFilesCopied;
};

class CCRUDProcessor
{
public:
	CCRUDProcessor();
	~CCRUDProcessor();
	void ProcessFiles(TFileList &fileList, const char *pSourceDir, const char *pDestDir, bool bRecursive, CCRUDResults &results);
    void Stop();
private:
	HANDLE m_hStopEvent;
};

#endif // __CRUD_PROC_H__
 