// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.h
//
// PURPOSE : Server-side Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997 - 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_UTILITIES_H__
#define __SERVER_UTILITIES_H__

#include "ltengineobjects.h"
#include "SoundTypes.h"
#include "CommonUtilities.h"
#include "FXFlags.h"
#include "LiteObjectMgr.h"

class CPlayerObj;

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
#define SAVE_BOOL(variable) pMsg->Writebool(variable != LTFALSE);
#define LOAD_BOOL(variable) variable = (pMsg->Readbool() ? LTTRUE : LTFALSE);
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
#define SAVE_HOBJECT(variable) pMsg->WriteObject(variable);
#define LOAD_HOBJECT(variable) variable = pMsg->ReadObject();
#define SAVE_VECTOR(variable)   pMsg->WriteLTVector(variable);
#define LOAD_VECTOR(variable)   variable = pMsg->ReadLTVector();
#define SAVE_ROTATION(variable) pMsg->WriteLTRotation(variable);
#define LOAD_ROTATION(variable) variable = pMsg->ReadLTRotation();
#define SAVE_HSTRING(variable)  pMsg->WriteHString(variable);
#define LOAD_HSTRING(variable)  variable = pMsg->ReadHString();
#define SAVE_CHARSTRING(variable)  pMsg->WriteString(variable);
#define LOAD_CHARSTRING(variable, nsize)  pMsg->ReadString(variable, nsize);
#define SAVE_STDSTRING(variable) pMsg->WriteString((variable).c_str());
#define LOAD_STDSTRING(variable) { (variable).resize(pMsg->PeekString(0,0) + 1); pMsg->ReadString((variable).begin(), (variable).size()); }
#define SAVE_RANGE(variable) pMsg->Writefloat((float)variable.GetMin()); pMsg->Writefloat((float)variable.GetMax());
#define LOAD_RANGE(variable) variable.Set(pMsg->Readfloat(), pMsg->Readfloat());
#define LOAD_RANGE_CAST(variable, type) variable.Set((type)pMsg->Readfloat(), (type)pMsg->Readfloat());
#define SAVE_TIME(variable) pMsg->Writefloat(( variable ) - g_pLTServer->GetTime( ));
#define LOAD_TIME(variable) variable = g_pLTServer->GetTime( ) + pMsg->Readfloat( );
#define SAVE_LITEOBJECT(variable) pMsg->Writeuint32(g_pGameServerShell->GetLiteObjectMgr()->GetSerializeID((GameBaseLite*)variable));
#define LOAD_LITEOBJECT(variable, type) (variable) = (type*)g_pGameServerShell->GetLiteObjectMgr()->GetSerializeObject(pMsg->Readuint32());
#define SAVE_COBJECT_INTERNAL(variable) pMsg->WriteObject(variable ? ((ILTBaseClass*)variable)->m_hObject : LTNULL);
#define LOAD_COBJECT_INTERNAL(variable, type) { HOBJECT hObject = pMsg->ReadObject(); variable = hObject ? (type*)g_pLTServer->HandleToObject(hObject) : LTNULL;}
#define SAVE_COBJECT(variable) { if (variable && ((ILTBaseClass*)variable)->m_hObject == 0) { SAVE_bool(true); SAVE_LITEOBJECT(variable); } else { SAVE_bool(false); SAVE_COBJECT_INTERNAL(variable); } }
#define LOAD_COBJECT(variable, type) { if (pMsg->Readbool()) { LOAD_LITEOBJECT(variable, type); } else { LOAD_COBJECT_INTERNAL(variable, type); } }

void PrintObjectFlags(HOBJECT hObj, char* pMsg=LTNULL);

// FindNamedObject will return LT_OK if exactly one object of that name is found,
// LT_NOTFOUND if no objects of that name are found, or LT_ERROR if more than one
// of the named object is found (unless bMultipleOkay is LTTRUE, in which it will
// return LT_OK and just use the first object in the array)

inline LTRESULT FindNamedObject(const char* szName, HOBJECT& hObject, LTBOOL bMultipleOkay = LTFALSE)
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

inline LTRESULT FindNamedObject(HSTRING hstrName, HOBJECT& hObject, LTBOOL bMultipleOkay = LTFALSE)
{
    return FindNamedObject(g_pLTServer->GetStringData(hstrName), hObject, bMultipleOkay);
}

// Note : This version of the function will search the lite objects as well.
LTRESULT FindNamedObject(const char* szName, ILTBaseClass *& pObject, LTBOOL bMultipleOkay = LTFALSE);

enum eObjListControl
{
	eObjListDuplicatesOK = 0,
	eObjListNODuplicates
};

inline LTBOOL IsObjectInList( ObjectList *pObjList, HOBJECT hTestObj )
{
	if( !pObjList || !hTestObj )
		return LTFALSE;

	ObjectLink *pObjLink = pObjList->m_pFirstLink;
	while( pObjLink && pObjLink->m_hObject )
	{
		if( pObjLink->m_hObject == hTestObj )
			return LTTRUE;

		pObjLink = pObjLink->m_pNext;
	}

	return LTFALSE;
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


#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)

#define FREE_HSTRING(x) if ( x ) { g_pLTServer->FreeString(x); x = LTNULL; }

void StartTimingCounter();
void EndTimingCounter(char *pMsg, ...);

// Print object info to console.
void ObjectCPrint(HOBJECT hObject, const char *pMsg, ...);
void ObjectCPrint(const char *pName, const char *pMsg, ...);

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, LPBASECLASS pObj, LTBOOL bBogus, const char* pStr);
inline void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL bBogus, const char* pStr)
{
	SendTriggerMsgToObject(pSender, g_pLTServer->HandleToObject(hObj), LTTRUE, pStr);
}

LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg);
void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);
void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL bBogus, const char* pStr);

LTBOOL IsPlayer( HOBJECT hObject );
LTBOOL IsVehicle( HOBJECT hObject );
LTBOOL IsDog( HOBJECT hObject );
LTBOOL IsCharacter( HOBJECT hObject );
LTBOOL IsCharacter( ILTBaseClass *pObject );
LTBOOL IsCharacterHitBox( HOBJECT hObject );
LTBOOL IsBody( HOBJECT hObject );
LTBOOL IsExplosion( HOBJECT hObject );
LTBOOL IsGameBase( HOBJECT hObject );
LTBOOL IsWorldModel( HOBJECT hObject );
LTBOOL IsKindOf( HOBJECT hObject, const char* szClass );
LTBOOL IsKindOf( HOBJECT hObject, HOBJECT hObject2 );
bool AreSameTeam( HOBJECT hPlayer1, HOBJECT hPlayer2 );
uint8 GetPlayerTeamId( HOBJECT hPlayer );
char const* TeamIdToString( uint8 nTeamId );
uint8 TeamStringToTeamId( char const* pszTeamString );

LTBOOL IsVector(const char* szString);

uint16 Color255VectorToWord( LTVector *pVal );

LTBOOL MoveObjectToFloor(HOBJECT hObj, HOBJECT *pFilterList = NULL, ObjectFilterFn pFilterFn = NULL);

void   ShowObjectAttachments(HOBJECT hObj, bool bShow);

//Macros to make the update intervals more consistant and easier to understand
#define	UPDATE_NEXT_FRAME			(0.001f)
#define UPDATE_NEVER				(0.0f)

#define INVALID_ANI				((HMODELANIM)-1)

inline void SetNextUpdate(HOBJECT hObj, LTFLOAT fDelta)
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

inline LTBOOL Equal(const LTVector & v1, const LTVector & v2)
{
    LTBOOL bRet = LTTRUE;

    const LTFLOAT c_fError = 0.001f;

    LTVector vTemp;
	VEC_SUB(vTemp, v1, v2);

	if (vTemp.x < -c_fError || vTemp.x > c_fError)
	{
        bRet = LTFALSE;
	}
	else if (vTemp.y < -c_fError || vTemp.y > c_fError)
	{
        bRet = LTFALSE;
	}
	else if (vTemp.z < -c_fError || vTemp.z > c_fError)
	{
        bRet = LTFALSE;
	}

	return bRet;
}
	
// This function converts an .ltb model filename to .ltc (yes, it just changes
// the last letter of the extention)...
//
// NOTE:  DO NOT pass const or static strings to this function!

inline void ConvertLTBFilename(char* szFilename)
{
	// Make sure string is valid 
	if (!szFilename || !(*szFilename)) return;

	int nLen = strlen(szFilename);
	
	if (nLen > 0)
	{
		szFilename[nLen-1] = 'c';
	}
}


extern char g_szString[4096];
extern GenericProp g_gp;

#define READPROP_BOOL(x, y) \
    if ( g_pLTServer->GetPropGeneric( x, &g_gp ) == LT_OK ) \
		y = g_gp.m_Bool; \

#define READPROP_HSTRING(x, y) \
    if ( g_pLTServer->GetPropGeneric( x, &g_gp ) == LT_OK && g_gp.m_String[0] ) \
		y = g_pLTServer->CreateString( g_gp.m_String ); \
	else \
		y = LTNULL; \

#define READPROP_STRING(x, y) \
    if ( g_pLTServer->GetPropGeneric( x, &g_gp ) == LT_OK && g_gp.m_String[0] ) \
		y = g_gp.m_String; \
	else \
		y = LTNULL; \

#define READPROP_STRINGENUM(x, y, a1, a2, n) \
    if ( g_pLTServer->GetPropGeneric( x, &g_gp ) == LT_OK && g_gp.m_String[0] ) \
	{ \
		const char* szValue = g_gp.m_String; \
		for ( uint32 i = 0 ; i < n ; i++ ) \
		{ \
			if ( !_stricmp(a1[i], szValue) ) \
			{ \
				y = a2[i]; \
				break; \
			} \
		} \
	} \

#define READPROP_VECTOR(x, y) \
    g_pLTServer->GetPropVector(x, &y);

#define READPROP_FLOAT(x, y) \
    g_pLTServer->GetPropReal(x, &y);

#define READPROP_INT(x, y) \
    g_pLTServer->GetPropLongInt(x, (long*)&y);

LTBOOL DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt = NULL, LTPlane *pIntersectPlane = NULL);

int		GetConsoleInt(char* sKey, int nDefault);
void	GetConsoleString(char* sKey, char* sDest, char* sDefault);
void	WriteConsoleString(char* sKey, char* sValue);
void	WriteConsoleInt(char* sKey, int nValue);
LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault);
void	WriteConsoleFloat(char* sKey, LTFLOAT fValue);

void Warn(const char* szFormat, ...);

#define BUILD_NOPAIN_WAV	"null.wav"

#include "IObjectPlugin.h"
#pragma warning( disable : 4786 )
#include <vector>
class CEditStringPlugin : public IObjectPlugin
{
	typedef IObjectPlugin super;

	protected :

		struct REGISTER
		{
			REGISTER() : cStrings(0), szPropName(LTNULL), aszStrings(LTNULL) {}
			const char* szPropName;
			const char** aszStrings;
			uint32 cStrings;
		};

	public :

		void Register(const char* szPropName, const char** aszStrings, const uint32 cStrings)
		{
			REGISTER reg;
			reg.szPropName = szPropName;
			reg.aszStrings = aszStrings;
			reg.cStrings = cStrings;

			m_vecRegister.push_back(reg);
		}

        LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
		{
			LTBOOL bFoundMatch = LTFALSE;

			for ( std::vector<REGISTER>::iterator iter = m_vecRegister.begin() ; iter != m_vecRegister.end() ; iter++ )
			{
				if ( DoesPropNameMatch(iter->szPropName, szPropName) )
				{
					for ( uint32 iString = 0 ; iString < iter->cStrings ; iString++ )
					{
						strcpy(aszStrings[(*pcStrings)++], iter->aszStrings[iString]);
					}

					bFoundMatch = LTTRUE;
				}
			}

			if( bFoundMatch )
			{
				return LT_OK;
			}

			return super::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
		}

		virtual LTBOOL DoesPropNameMatch(const char* szPropNameRegistered, const char* szPropNameIncoming)
		{
			return !strcmp(szPropNameRegistered, szPropNameIncoming);
		}

	protected :

		std::vector<REGISTER>	m_vecRegister;
};

// Send a dataless message to the specified client
LTRESULT SendEmptyClientMsg(uint32 nMsgID, HCLIENT hClient, uint32 nFlags = MESSAGE_GUARANTEED);

// Send a dataless object message
LTRESULT SendEmptyObjectMsg(uint32 nMsgID, HOBJECT hSource, HOBJECT hDest, uint32 nFlags = MESSAGE_GUARANTEED);

// Replacement for the old format of GetObjectName
const char *GetObjectName(HOBJECT hObj);
const char *GetObjectName(ILTBaseClass *pObj);

// Check if we're in a multiplayer game.
bool IsMultiplayerGame();

// Plays a fire-and-forget clientFX at a specific location in the world.
void PlayClientFX(char* szFXName, HOBJECT hParent, LTVector* pvPos = NULL, LTRotation *prRot = NULL, uint32 dwFlags = 0);

// Plays a fire-and-forget clientFX at a specific location in the world using target information.
void PlayClientFX(char* szFXName, HOBJECT hParent, HOBJECT hTarget, LTVector *pvPos = NULL, LTRotation *prRot = NULL, LTVector *pvTargetPos = NULL, uint32 dwFlags = 0);

// Gets the playerobj from a client id.
CPlayerObj* GetPlayerFromClientId( uint32 nClientId );

#endif // __SERVER_UTILITIES_H__