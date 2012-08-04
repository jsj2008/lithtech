// ----------------------------------------------------------------------- //
//
// MODULE  : TronVersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronVersionMgr.h"

static const char* s_szVersion		= "TRON Build 20 (05/09/02) DO NOT DISTRIBUTE";
static const char* s_szBuild		= "Build 20 (05/09/02)";
static const uint32 s_nSaveVersion  = 0;
static const char* s_szNetVersion	= "1.0.0.0";

CVersionMgr* g_pVersionMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::CTronVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronVersionMgr::CTronVersionMgr() : CVersionMgr()
{
	g_pVersionMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetVersion()
//
//	PURPOSE:	Get the current version string
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetVersion()
{
	return s_szVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetSaveVersion()
//
//	PURPOSE:	Get the current save version 
//
// ----------------------------------------------------------------------- //

const uint32 CTronVersionMgr::GetSaveVersion()
{
	return s_nSaveVersion;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr::GetBuild()
//
//	PURPOSE:	Get the current build string
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetBuild()
{
	return s_szBuild;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronVersionMgr:GetNetVersion()
//
//	PURPOSE:	Get the current net version 
//
// ----------------------------------------------------------------------- //

const char* CTronVersionMgr::GetNetVersion()
{
	return s_szNetVersion;
}

