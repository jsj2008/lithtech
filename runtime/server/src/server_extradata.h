
// This module initializes the 'extra data' for each object (like Model 
// and WorldModel pointers).

#ifndef __SERVER_EXTRADATA_H__
#define __SERVER_EXTRADATA_H__

#ifndef __SERVER_FILEMGR_H__
#include "server_filemgr.h"
#endif


class ExtraDataBackup
{
public:
	LTAnimTracker	m_AnimTracker;
	UsedFile		*m_pFile;
	UsedFile		*m_pSkin;
};

LTRESULT sm_InitExtraData(LTObject *pObject, ObjectCreateStruct *pStruct);
LTRESULT sm_TermExtraData(LTObject *pObject);

// Used when changing filenames.
LTRESULT BackupExtraData(LTObject *pObject, ExtraDataBackup *pBackup);
LTRESULT RestoreExtraData(LTObject *pObject, ExtraDataBackup *pBackup);


#endif  // __SERVER_EXTRADATA_H__



