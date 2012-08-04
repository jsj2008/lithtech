// ----------------------------------------------------------------------- //
//
// MODULE  : TO2VersionMgr.h
//
// PURPOSE : Definition of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _TO2VERSIONMGR_H_
#define _TO2VERSIONMGR_H_

#include "VersionMgr.h"

class CTO2VersionMgr : public CVersionMgr
{
	public :

		CTO2VersionMgr();
		virtual ~CTO2VersionMgr() {}

		virtual void Update();

		virtual const char* GetVersion();
		virtual const char* GetBuild();
		virtual const uint32 GetSaveVersion();
		virtual const char* GetNetGameName();
		virtual const char* GetNetVersion();
		virtual LTGUID const* GetBuildGuid();
};

#endif _TO2VERSIONMGR_H_