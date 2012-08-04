/****************************************************************************
;
;   MODULE:     LTDirectMusicControlFile (.H)
;
;   PURPOSE:    Implement DirectMusic special Control File class
;
;   HISTORY:    Nov-20-1999 [blb]
;
;   NOTICE:     Copyright (c) 1999 Monolith Procutions, Inc.
;
***************************************************************************/

#ifndef __LTDIRECTMUSICCONTROLFILE_H__
#define __LTDIRECTMUSICCONTROLFILE_H__

#ifndef __CONTROLFILEMGR_H__
#include "controlfilemgr.h"
#endif

#ifdef NOLITHTECH

#ifndef __REZMGR_H__
#include "rezmgr.h"
#endif

#else

#ifndef __CLIENTMGR_H__
#include "clientmgr.h"
#endif

#ifndef __CLIENTDE_IMPL_H__
#include "clientde_impl.h"
#endif

#endif

#ifndef NOLITHTECH
///////////////////////////////////////////////////////////////////////////////////////////
// special derived controlfilemgr that uses dstreams
///////////////////////////////////////////////////////////////////////////////////////////
class CControlFileMgrDStream : public CControlFileMgr
{
public:
    // open a file for reading in read only text mode
    virtual BOOL FileOpen(const char* sName);

    // close file 
    virtual void FileClose();
    
    // get next character in file
    virtual int FileGetChar();
    
    // return TRUE if we are at the end of the file
    virtual BOOL FileEOF();

protected:
    char* m_pData;
    char* m_pPos;
    char* m_pEnd;
    
};
#endif

#ifdef NOLITHTECH
///////////////////////////////////////////////////////////////////////////////////////////
// special derived controlfilemgr that uses the rez file
///////////////////////////////////////////////////////////////////////////////////////////
class CControlFileMgrRezFile : public CControlFileMgr
{
public:
	CControlFileMgrRezFile(const char* sRezFileName) { m_RezMgr.Open(sRezFileName); };
	~CControlFileMgrRezFile() { if (m_RezMgr.IsOpen()) m_RezMgr.Close(); };

	// open a file for reading in read only text mode
	virtual BOOL FileOpen(const char* sName);

	// close file 
	virtual void FileClose();
	
	// get next character in file
	virtual int FileGetChar();
	
	// return TRUE if we are at the end of the file
	virtual BOOL FileEOF();

protected:
	char* m_pData;
	char* m_pPos;
	char* m_pEnd;
	
	CRezMgr m_RezMgr;
	CRezItm* m_pItem;
};
#endif

#endif

