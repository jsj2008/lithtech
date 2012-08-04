// ----------------------------------------------------------------------- //
//
// MODULE  : TronVersionMgr.h
//
// PURPOSE : Definition of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _TRONVERSIONMGR_H_
#define _TRONVERSIONMGR_H_

#include "VersionMgr.h"

class CTronVersionMgr : public CVersionMgr
{
	public :

		CTronVersionMgr();
		virtual ~CTronVersionMgr() {}

		virtual const char* GetVersion();
		virtual const char* GetBuild();
		virtual const uint32 GetSaveVersion();
		virtual const char* GetNetVersion();
};

#endif _TRONVERSIONMGR_H_