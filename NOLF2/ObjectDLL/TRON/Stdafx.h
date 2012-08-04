// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

// This removes warnings in Vis C about the CodeWarrior pragma 'force_active'
//
#pragma warning( disable : 4068 )

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <limits.h>

#include "mfcstub.h"

#include "DebugNew.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "globals.h"

#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"

#include "ltobjref.h"

#include "ltobjref.h"

#include "Factory.h"

#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "CommonUtilities.h"

// Infrequently changed, often included files:
#include "GameBase.h"
#include "ButeListReader.h"
#include "ClientLightFX.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"
#include "CommandObject.h"
#include "AIClassFactory.h"
#include "CVarTrack.h"
#include "Controller.h"
#include "DebrisFuncs.h"
#include "DestructibleModel.h"
#include "FastHeap.h"
#include "FastStack.h"
#include "Prop.h"
#include "BankedList.h"
#include "Editable.h"
#include "ServerUtilities.h"

#endif // __STDAFX_H__