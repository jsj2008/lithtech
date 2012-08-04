// ----------------------------------------------------------------------- //
//
// MODULE  : ClientServerShared.h
//
// PURPOSE : Types and defines shared between the client and the server
//
// CREATED : 11/25/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SERVER_SHARED_H__
#define __CLIENT_SERVER_SHARED_H__

#include "ltobjref.h"
#include <list>
#include "ltobjectcreate.h"
#include "CommonUtilities.h"
#include "NetDefs.h"


// Object user flags -
// NOTE: The top two bytes of the user flags are reserverd for surface 
// flags and collision properties...

#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_IGNORE_PROJECTILES		(1<<1)

// Used with player attachments (weapon / flash).  USRFLG_ATTACH_HIDE1SHOW3
// indicates that this object is hidden on the client in first person mode,
// and shown on the client in 3rd person mode....
#define USRFLG_ATTACH_HIDE1SHOW3		(1<<2)

// Used only with AI objects.
#define USRFLG_AI_CLIENT_SOLID			(1<<3)

// Used with Camera objects.  This specifies if a specific camera is "live"
#define USRFLG_CAMERA_LIVE				(1<<4)

// Can this object move...
#define USRFLG_MOVEABLE					(1<<5)

// Used with the player object only (checked only on the client, and only
// on the player model)...
#define USRFLG_PLAYER_UNDERWATER		(1<<6)

// Used with character objects...
#define USRFLG_CHARACTER				(1<<7)

// Specifies that this object can be activated...
#define USRFLG_CAN_ACTIVATE				(1<<8)

// Used when a GameBase object is active on the server
#define USRFLG_GAMEBASE_ACTIVE			(1<<9)

// Used with SpecialFX objects to determine, on the client,
// if a SpecialFX object is "on"
#define USRFLG_SFX_ON					(1<<10)

// Is this object a hitbox?  (Used for client-side hit detection)
#define USRFLG_HITBOX					(1<<11)

// Used to specify the object is only in the client physics simulation and should not be
// interacted with outside of the client physics simulation.
// The object is non-solid but it's client side rigidbody is solid.
#define USRFLG_CLIENT_RIGIDBODY_ONLY	(1<<12)

// Indicate that this object is a "gory" object and should not be shown on low-violence clients
#define USRFLG_GORE						(1<<13)

// The level of detail of the object is stored in these bits...
#define USRFLG_OBJECTLOD1				(1<<14)
#define USRFLG_OBJECTLOD2				(1<<15)
#define USRFLG_OBJECTLODMASK			(((uint32)0x3) * USRFLG_OBJECTLOD1)
EEngineLOD UserFlagToObjectLOD( uint32 nUserFlags );
uint32 ObjectLODToUserFlag( EEngineLOD eObjectLOD );

// NOTE: The top two bytes of the user flags are reserverd for surface flags and collision properties...

// CollisionProperty index is stored in these bits.
#define USRFLG_COLLISIONPROPINDEX1		(1<<16)
#define USRFLG_COLLISIONPROPINDEX2		(1<<17)
#define USRFLG_COLLISIONPROPINDEX3		(1<<18)
#define USRFLG_COLLISIONPROPINDEX4		(1<<19)
#define USRFLG_COLLISIONPROPINDEX5		(1<<20)
#define USRFLG_COLLISIONPROPINDEX6		(1<<21)
#define USRFLG_COLLISIONPROPINDEX7		(1<<22)
#define USRFLG_COLLISIONPROPINDEX8		(1<<23)
#define USRFLG_COLLISIONPROPMASK		((( uint32 )0xFF )*USRFLG_COLLISIONPROPINDEX1)
HRECORD UserFlagToCollisionPropertyRecord( uint32 dwUserFlags );
uint32 CollisionPropertyRecordToUserFlag( HRECORD hCollisionProperty );

// SurfaceType is stored in these bits.
#define USRFLG_SURFACE1					(1<<24)
#define USRFLG_SURFACE2					(1<<25)
#define USRFLG_SURFACE3					(1<<26)
#define USRFLG_SURFACE4					(1<<27)
#define USRFLG_SURFACE5					(1<<28)
#define USRFLG_SURFACE6					(1<<29)
#define USRFLG_SURFACE7					(1<<30)
#define USRFLG_SURFACE8					(1<<31)
#define USRFLG_SURFACEMASK				((( uint32 )0xFF )*USRFLG_SURFACE1)

// ALL USRFLG's available - NOTE 15 is the last flag!!!!


// These used to be part of User Flags but are now seperated out
// and sent to the clients through a MID_SFX_MESSAGE.

#define DMGFLG_CHAR_BLEEDING			(1<<0)
#define DMGFLG_CHAR_POISONED			(1<<1)
#define DMGFLG_CHAR_STUNNED				(1<<2)
#define DMGFLG_CHAR_SLEEPING			(1<<3)
#define DMGFLG_CHAR_CHOKE				(1<<4)
#define DMGFLG_CHAR_BURN				(1<<5)
#define DMGFLG_CHAR_ELECTROCUTE			(1<<6)
#define DMGFLG_CHAR_GLUE				(1<<7)
#define DMGFLG_CHAR_BEARTRAP			(1<<8)
#define DMGFLG_CHAR_LAUGHING			(1<<9)
#define DMGFLG_CHAR_ASSS				(1<<10)
#define DMGFLG_CHAR_FREEZING			(1<<11)
#define DMGFLG_CHAR_SLIPPING			(1<<12)



// Camera related flags...

#define CT_FULLSCREEN				0
#define CT_LETTERBOX				1

#define INFINITE_MASS				100000.0f

#define MAX_DECISION_CHOICES		6
#define MAX_MIXER_MSG				256

typedef std::vector< LTObjRef, LTAllocator<LTObjRef, LT_MEM_TYPE_GAMECODE> > ObjRefVector;
typedef std::list< LTObjRef, LTAllocator<LTObjRef, LT_MEM_TYPE_GAMECODE> > ObjRefList;
typedef std::list< LTObjRefNotifierWithCopy, LTAllocator<LTObjRefNotifierWithCopy, LT_MEM_TYPE_GAMECODE> > ObjRefNotifierList;

// A quick accessor to the object type if you're absolutely sure the object is valid
inline uint32 GetObjectType(HOBJECT hObj)
{
	ASSERT(g_pCommonLT);

	uint32 nObjType;

	uint32 nResult = g_pCommonLT->GetObjectType(hObj, &nObjType);

	ASSERT(nResult == LT_OK);

	return nObjType;
}

// Utility functions for filtering intersection queries

//this will filter out all objects in the list, which is a list of HOBJECTs, terminated
//by NULL
inline bool ObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
            return false;
		++hList;
	}
    return true;
}

//Filters out anything but world models and the main world
inline bool WorldOnlyFilterFn(HOBJECT hTest, void *pUserData)
{
	LTUNREFERENCED_PARAMETER( pUserData );

	//only accept it if it is a world model or if it is the main world
	if (IsMainWorld(hTest) || GetObjectType(hTest) == OT_WORLDMODEL)
	{
		//its a world
		return true;
	}
	//not either, don't want it
	return false;
}

// Old-style format for g_pCommonLT->SetObjectFilenames
inline LTRESULT SetObjectFilenames(HOBJECT hObj, const char *pFilename, const char *pMaterialName)
{
	ObjectCreateStruct cOCS;
	cOCS.Clear();
	cOCS.SetFileName(pFilename);
	uint32 nObjType;
	LTRESULT nResult = g_pCommonLT->GetObjectType(hObj, &nObjType);
	if (nResult != LT_OK)
		return nResult;
	if (nObjType == OT_MODEL)
	{
		for(uint32 nCurrMaterial = 0; nCurrMaterial < MAX_MATERIALS_PER_MODEL; nCurrMaterial++)
			cOCS.SetMaterial(nCurrMaterial, pMaterialName);
	}
	return g_pCommonLT->SetObjectFilenames(hObj, &cOCS);
}

void SetObjectMaterial(HOBJECT hObj, uint8 nMaterialNum, const char* pszMaterial);

//retrieve the hit location enum from the text name of the location
HitLocation HitLocationFromString(const char* szLocation);
const char* StringFromHitLocation(HitLocation eLoc);

//Default world gravity. This is -980 cm/s^2 which equals -9.8 m/s^2 which is earth gravity
#define DEFAULT_WORLD_GRAVITY			-980.0f

inline LTVector ConvertToDEditPos( LTVector &vOrigPosition )
{
	LTVector vWorldOffsetFromDEditToRuntime = vOrigPosition;

	g_pLTBase->GetSourceWorldOffset( vWorldOffsetFromDEditToRuntime );

	return vWorldOffsetFromDEditToRuntime + vOrigPosition;
}

// __STR_LINE__ is a macro that converts the integer constant __LINE__ into a string literal
#define STR_LINE2(x) #x 
#define STR_LINE(x)	STR_LINE2(x)
#define __STR_LINE__ STR_LINE(__LINE__)

// MESSAGE is a useful macro to display a text string in the output window of the MS IDE that will open 
// the file and go to the line number of the message when clicked in the output window.
// Used in conjunction with #pragma:
// Ex: #pragma TODO( "Finish this task." )
#define MESSAGE(x) message( __FILE__ "("__STR_LINE__") : MESSAGE : " x )
#define TODO(x) message ( __FILE__ "("__STR_LINE__") : TODO : " x )
#define WARNING(x) message ( __FILE__ "("__STR_LINE__") : warning : " x )


void DebugCPrint(int nLevel, const char* szFormat, ...);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline bool SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return false;

	return (hObj != (HOBJECT)pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExceptObjectFilterFn()
//
//	PURPOSE:	Filter all objects except that specified in userdata.
//
// ----------------------------------------------------------------------- //

inline bool ExceptObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return false;

	return (hObj == (HOBJECT)pUserData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddObjectToFilter()
//
//	PURPOSE:	Helper function for adding objects to the object list.
//
//				Adds the passed in object to pObjectList if:
//				1) It is not NULL (as this would terminate the list)
//				2) There is enough room in the list.
//
//				Returns the number of objects in the list.
//
//				If the passed in object is NULL or if the list is 
//				overflowed, an assert will occur.
//
// ----------------------------------------------------------------------- //

inline int32 AddObjectToFilter( HOBJECT hObject, HOBJECT* pObjectList, int32 nCurrentListSize, int32 nMaxListSize )
{
	// Check the preconditions.

	if ( NULL == hObject )
	{
		LTERROR( "AddObjectToFilter : Attempting to add a NULL object to the filter list.  NULL objects indicate the end of this list and should not be added.  Verify that this object is correct.");
		return nCurrentListSize;
	}

	if ( nCurrentListSize >= nMaxListSize ) 
	{
		LTERROR( "AddObjectToFilter : Overflowing object list.  Increase the size of the list.");
		return nCurrentListSize;
	}

	// Add the object to the list.

	pObjectList[nCurrentListSize] = hObject;
	return nCurrentListSize + 1;
}

#define WMGR_MAX_NAME_LENGTH				32
#define WMGR_MAX_FILE_PATH					64

// maximum length of a connection error message
const uint32 g_nMaximumConnectionErrorLength = 512;

// file names for use with Multiplayer Overrides
const char* const g_pszOverridesFilename   = "MPCustomizations.txt";
const char* const g_pszConstraintsFilename = "Database/DBConstraints.dat";

// constants for use with the server scoring log
const char* const  g_pszScoringLogFilename		 = "mp_scores.log";
const char* const  g_pszScoringLogBackupFilename = "mp_scores.log.previous";
const uint32	   g_nMaximumEntryLength		 = 128;
const uint32	   g_nScoringTimeBufferSize		 = 32;
const uint32	   g_nScoringEntryBufferSize     = 256;

// simulation log file
const char* const g_pszSimulationLogFilename = "simulation.log";
enum ESimulationAction 
{ 
	eSimulationActionPlayerUpdate = 1, 
	eSimulationActionFire, 
	eSimulationActionWeaponChange,
	eSimulationActionRespawn
};

// utility functions for the game database
HDATABASE OpenGameDatabase(const char* pszFilename);
HDATABASE OpenOverridesDatabase(ILTInStream& OverridesStream, const char* pszConstraintsFilename);

// separators used in the GameSpy downloadable files string 
const char* const g_pszDownloadableFileCountSeparator = ":";
const char* const g_pszDownloadableFileEntrySeparator = ";";
const char* const g_pszDownloadableFileSizeSeparator  = ",";
const uint32 g_nMaxDownloadableFileEntryLength  = 256;
const uint32 g_nMaxDownloadableFileStringLength = 1024;

// Amount we scale the character hitbox from the model dims.
#define HB_DIMS_ENLARGE_PERCENT 1.2f

// String loading function for LOAD_STDSTRING
inline void Read_StdString(ILTMessage_Read *pMsg, std::string &sDest)
{
	static std::vector<char, LTAllocator<char, LT_MEM_TYPE_OBJECTSHELL> > aBuffer(1024);
	aBuffer.resize( pMsg->PeekString(0,0) + 1 ); 
	pMsg->ReadString( &aBuffer[0], aBuffer.size() ); 
	sDest = &aBuffer[0];
}

inline void Read_HRECORD(ILTMessage_Read *pMsg, HRECORD& hRecord, HCATEGORY hCategory)
{
	static std::vector<char, LTAllocator<char, LT_MEM_TYPE_OBJECTSHELL> > aBuffer(1024);
	aBuffer.resize( pMsg->PeekString(0,0) + 1 ); 
	pMsg->ReadString( &aBuffer[0], aBuffer.size() ); 
	hRecord = g_pLTDatabase->GetRecord( hCategory, &aBuffer[0] );
}

// Save/Load Macros

#define SAVE_BYTE(variable) pMsg->Writeuint8(variable);
#define LOAD_BYTE(variable) variable = pMsg->Readuint8();
#define LOAD_BYTE_CAST(variable, clazz) variable = (clazz)pMsg->Readuint8();
#define SAVE_WORD(variable) pMsg->Writeuint16(variable);
#define LOAD_WORD(variable) variable = pMsg->Readuint16();
#define LOAD_WORD_CAST(variable, clazz) variable = (clazz)pMsg->Readuint16();
#define SAVE_INT(variable) pMsg->Writeint32(variable);
#define LOAD_INT(variable) variable = pMsg->Readint32();
#define LOAD_INT_CAST(variable, clazz) variable = (clazz)pMsg->Readint32();
#define SAVE_BOOL(variable) pMsg->Writebool(variable);
#define LOAD_BOOL(variable) variable = pMsg->Readbool();
#define SAVE_bool(variable) pMsg->Writebool(variable);
#define LOAD_bool(variable) variable = pMsg->Readbool();
#define SAVE_DWORD(variable) pMsg->Writeuint32(variable);
#define LOAD_DWORD(variable) variable = pMsg->Readuint32();
#define LOAD_DWORD_CAST(variable, clazz) variable = (clazz)pMsg->Readuint32();
#define SAVE_QWORD(variable) pMsg->Writeuint64( variable );
#define LOAD_QWORD(variable) variable = pMsg->Readuint64();
#define LOAD_QWORD_CAST(variable, clazz) variable = (clazz)pMsg->Readuint64();
#define SAVE_FLOAT(variable) pMsg->Writefloat(variable);
#define LOAD_FLOAT(variable) variable = pMsg->Readfloat();
#define SAVE_DOUBLE(variable) pMsg->Writedouble(variable);
#define LOAD_DOUBLE(variable) variable = pMsg->Readdouble();
#define SAVE_HOBJECT(variable) pMsg->WriteObject(variable);
#define LOAD_HOBJECT(variable) variable = pMsg->ReadObject();
#define SAVE_VECTOR(variable)   pMsg->WriteLTVector(variable);
#define LOAD_VECTOR(variable)   variable = pMsg->ReadLTVector();
#define SAVE_ROTATION(variable) pMsg->WriteLTRotation(variable);
#define LOAD_ROTATION(variable) variable = pMsg->ReadLTRotation();
#define SAVE_CHARSTRING(variable)  pMsg->WriteString(variable);
#define LOAD_CHARSTRING(variable, nsize)  pMsg->ReadString(variable, nsize);
#define SAVE_STDSTRING(variable) pMsg->WriteString((variable).c_str());
#define LOAD_STDSTRING(variable) Read_StdString(pMsg, variable);
#define SAVE_RANGE(variable) pMsg->Writefloat((float)variable.GetMin()); pMsg->Writefloat((float)variable.GetMax());
#define LOAD_RANGE(variable) { float fMin = pMsg->Readfloat( ); variable.Set(fMin, pMsg->Readfloat()); }
#define LOAD_RANGE_CAST(variable, type) { float fMin = pMsg->Readfloat( ); variable.Set((type)fMin, (type)pMsg->Readfloat()); }
#define SAVE_TIME(variable) pMsg->Writefloat((float)(( variable ) - g_pLTServer->GetTime( )));
#define LOAD_TIME(variable) variable = g_pLTServer->GetTime( ) + pMsg->Readfloat( );
#define SAVE_TIME_MS(variable) pMsg->Writeuint32((( variable ) - g_pLTServer->GetTimeMS( )));
#define LOAD_TIME_MS(variable) variable = g_pLTServer->GetTimeMS( ) + pMsg->Readuint32( );
#define SAVE_COBJECT(variable) pMsg->WriteObject(variable ? ((ILTBaseClass*)variable)->m_hObject : NULL);
#define LOAD_COBJECT(variable, type) { HOBJECT hObject = pMsg->ReadObject(); variable = hObject ? (type*)g_pLTServer->HandleToObject(hObject) : NULL;}
#define SAVE_HRECORD(variable) pMsg->WriteString( g_pLTDatabase->GetRecordName( variable ));
#define LOAD_HRECORD(variable, hcategory) Read_HRECORD( pMsg, variable, hcategory );
#define SAVE_TRANSFORM(variable) pMsg->WriteLTTransform( variable );
#define LOAD_TRANSFORM(variable) variable = pMsg->ReadLTTransform();
#define SAVE_RIGIDTRANSFORM(variable) pMsg->WriteLTRigidTransform( variable );
#define LOAD_RIGIDTRANSFORM(variable) variable = pMsg->ReadLTRigidTransform();
#define SAVE_TYPE(variable) pMsg->WriteType( variable );
#define LOAD_TYPE(variable) pMsg->ReadType( &variable );

#endif  // __CLIENT_SERVER_SHARED_H__

// EOF
