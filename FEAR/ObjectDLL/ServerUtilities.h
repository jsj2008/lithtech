// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.h
//
// PURPOSE : Server-side Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997 - 2004 Monolith Productions, Inc.  All Rights Reserved.
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_UTILITIES_H__
#define __SERVER_UTILITIES_H__

#include "ltengineobjects.h"
#include "SoundTypes.h"
#include "CommonUtilities.h"
#include "FxFlags.h"
#include "resourceextensions.h"
#include "DatabaseUtils.h"

class CPlayerObj;

// FindNamedObject will return LT_OK if exactly one object of that name is found,
// LT_NOTFOUND if no objects of that name are found, or LT_ERROR if more than one
// of the named object is found (unless bMultipleOkay is true, in which it will
// return LT_OK and just use the first object in the array)

inline LTRESULT FindNamedObject(const char* szName, HOBJECT& hObject, bool bMultipleOkay = false)
{
	if ( !szName || !*szName ) return LT_NOTFOUND;

	hObject = NULL;
	ObjArray<HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects((char*)szName,objArray);

	switch ( objArray.NumObjects() )
	{
		case 1:
		{
			hObject = objArray.GetObject(0);
			return LT_OK;
		}
		case 0:
		{
			return LT_NOTFOUND;
		}
		default:
		{
			if ( bMultipleOkay )
			{
				hObject = objArray.GetObject(0);
				return LT_OK;
			}
			else
			{
                g_pLTServer->CPrint("Error, %d objects named \"%s\" present in level", objArray.NumObjects(), szName);
				return LT_ERROR;
			}
		}
	}
}


// Note : This version of the function will search the lite objects as well.
LTRESULT FindNamedObject(const char* szName, ILTBaseClass *& pObject, bool bMultipleOkay = false);

enum eObjListControl
{
	eObjListDuplicatesOK = 0,
	eObjListNODuplicates
};

inline bool IsObjectInList( ObjectList *pObjList, HOBJECT hTestObj )
{
	if( !pObjList || !hTestObj )
		return false;

	ObjectLink *pObjLink = pObjList->m_pFirstLink;
	while( pObjLink && pObjLink->m_hObject )
	{
		if( pObjLink->m_hObject == hTestObj )
			return true;

		pObjLink = pObjLink->m_pNext;
	}

	return false;
}

inline void AddObjectToList( ObjectList *pObjList, HOBJECT hObj, eObjListControl eControl )
{
	if( !pObjList || !hObj ) return;
	
	if( eControl == eObjListNODuplicates )
	{
		if( IsObjectInList( pObjList, hObj ))
			return;
	}

	g_pLTServer->AddObjectToList( pObjList, hObj );
}

void StartTimingCounterServer();
void EndTimingCounterServer(char *pMsg, ...);

// ToString

inline const char* ToString(const HOBJECT& hObject)
{
 	static char szString[256];
 
 	if (g_pLTServer->GetObjectName(hObject, szString, sizeof(szString)) != LT_OK)
  		return "";
 
  	return szString;
}

// Print object info to console.
void ObjectCPrint(HOBJECT hObject, const char *pMsg, ...);
void ObjectCPrint(const char *pName, const char *pMsg, ...);

bool IsPlayer( HOBJECT hObject );
bool IsCharacter( HOBJECT hObject );
bool IsCharacter( ILTBaseClass *pObject );
bool IsCharacterHitBox( HOBJECT hObject );
bool IsTurret( HOBJECT hObject );
bool IsExplosion( HOBJECT hObject );
bool IsGameBase( HOBJECT hObject );
bool IsWorldModel( HOBJECT hObject );
bool IsKindOf( HOBJECT hObject, const char* szClass );
bool IsKindOf( HOBJECT hObject, HOBJECT hObject2 );
bool AreSameTeam( HOBJECT hPlayer1, HOBJECT hPlayer2 );
uint8 GetPlayerTeamId( HOBJECT hPlayer );
char const* TeamIdToString( uint8 nTeamId );
uint8 TeamStringToTeamId( char const* pszTeamString );

// Fills in a editstringlist with available teams.
void TeamPopulateEditStringList( char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength );


bool IsVector(const char* szString);

bool MoveObjectToFloor(HOBJECT hObj, HOBJECT *pFilterList = NULL, ObjectFilterFn pFilterFn = NULL);

void ShowObjectAttachments(HOBJECT hObj, bool bShow);

//Macros to make the update intervals more consistant and easier to understand
#define	UPDATE_NEXT_FRAME			(0.0000001f)
#define UPDATE_NEVER				(0.0f)

#define INVALID_ANI				((HMODELANIM)-1)

inline void SetNextUpdate(HOBJECT hObj, float fDelta)
{
	if (!hObj) return;

	fDelta = fDelta <= 0.0f ? 0.0f : fDelta;
    g_pLTServer->SetNextUpdate(hObj, fDelta);

	if (fDelta == 0.0f)
	{
		g_pLTServer->SetObjectState(hObj, OBJSTATE_INACTIVE);
	}
	else
	{
		g_pLTServer->SetObjectState(hObj, OBJSTATE_ACTIVE);
	}
}

// This function converts an .ltb model filename to .ltc (yes, it just changes
// the last letter of the extention)...
//
// NOTE:  DO NOT pass const or static strings to this function!

inline void ConvertLTBFilename( char* szFilename, int nBufferLen )
{
	CResExtUtil::SetFileExtension( szFilename, nBufferLen, RESEXT_MODEL_COMPRESSED );
}


bool DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt = NULL, LTPlane *pIntersectPlane = NULL);

struct SAABB
{
	bool IntersectPoint( const LTVector& vPoint );

	LTVector vMin;
	LTVector vMax;
};


int	GetConsoleInt(char* sKey, int nDefault);
char const* GetConsoleString(char* pszKey, char const* pszDefault);
void WriteConsoleString(char const* sKey, char const* sValue);
void WriteConsoleInt(char* sKey, int nValue);
float GetConsoleFloat(char* sKey, float fDefault);
void WriteConsoleFloat(char* sKey, float fValue);

void Warn(const char* szFormat, ...);

// Send a dataless message to the specified client
LTRESULT SendEmptyClientMsg(uint32 nMsgID, HCLIENT hClient, uint32 nFlags = MESSAGE_GUARANTEED);

// Send a dataless object message
LTRESULT SendEmptyObjectMsg(uint32 nMsgID, HOBJECT hSource, HOBJECT hDest, uint32 nFlags = MESSAGE_GUARANTEED);

// Sends a message to all clients except the one specified.
LTRESULT SendToClientsExcept( ILTMessage_Read& msg, HCLIENT hExceptClient, uint32 nFlags = MESSAGE_GUARANTEED);

// Replacement for the old format of GetObjectName
const char *GetObjectName(HOBJECT hObj);

// Check if we're in a multiplayer game.
bool IsMultiplayerGameServer();

// Gets the playerobj from a client id.
CPlayerObj* GetPlayerFromClientId( uint32 nClientId );

// Returns the number of materials on a model
int GetObjectMaterialCount(HOBJECT hObj);

// simple macro to clean up code and reduce typing overhead
inline CPlayerObj* GetPlayerFromHClient( HCLIENT hClient ) 
{
	return ( CPlayerObj* )g_pLTServer->HandleToObject(g_pLTServer->GetClientObject(hClient));
}

// helper to show or hide the model pieces of a weapon
void DisplayWeaponModelPieces(LTObjRef hObject, HRECORD hWeapon, const char* ModelPieceListAtt, bool bShow, bool bUseAIWeapons);

#define DEFINE_CAST( class ) \
	static class* DynamicCast( HOBJECT hObj ) \
	{ \
		if( !hObj )	\
			return NULL;\
		\
		HCLASS hObjClass = g_pLTServer->GetObjectClass( hObj ); \
		static HCLASS hClass = g_pLTServer->GetClass( #class ); \
		if( g_pLTServer->IsKindOf( hObjClass, hClass )) \
			return static_cast< class* >( g_pLTServer->HandleToObject( hObj )); \
		else \
			return NULL; \
	}


#endif // __SERVER_UTILITIES_H__
