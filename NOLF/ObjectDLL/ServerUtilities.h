// ----------------------------------------------------------------------- //
//
// MODULE  : ServerUtilities.h
//
// PURPOSE : Server-side Utility functions
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_UTILITIES_H__
#define __SERVER_UTILITIES_H__

#include "ltengineobjects.h"
#include "SoundTypes.h"
#include "CommonUtilities.h"

// Save/Load Macros

#define SAVE_BYTE(variable) g_pLTServer->WriteToMessageByte(hWrite, variable);
#define LOAD_BYTE(variable) variable = g_pLTServer->ReadFromMessageByte(hRead);
#define LOAD_BYTE_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageByte(hRead);
#define SAVE_WORD(variable) g_pLTServer->WriteToMessageWord(hWrite, variable);
#define LOAD_WORD(variable) variable = g_pLTServer->ReadFromMessageWord(hRead);
#define LOAD_WORD_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageWord(hRead);
#define SAVE_INT(variable) g_pLTServer->WriteToMessageFloat(hWrite, (float)variable);
#define LOAD_INT(variable) variable = (int)g_pLTServer->ReadFromMessageFloat(hRead);
#define LOAD_INT_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageFloat(hRead);
#define SAVE_BOOL(variable) g_pLTServer->WriteToMessageByte(hWrite, variable);
#define LOAD_BOOL(variable) variable = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
#define SAVE_DWORD(variable) g_pLTServer->WriteToMessageDWord(hWrite, variable);
#define LOAD_DWORD(variable) variable = g_pLTServer->ReadFromMessageDWord(hRead);
#define LOAD_DWORD_CAST(variable, clazz) variable = (clazz)g_pLTServer->ReadFromMessageDWord(hRead);
#define SAVE_FLOAT(variable) g_pLTServer->WriteToMessageFloat(hWrite, variable);
#define LOAD_FLOAT(variable) variable = g_pLTServer->ReadFromMessageFloat(hRead);
#define SAVE_HOBJECT(variable) g_pLTServer->WriteToLoadSaveMessageObject(hWrite, variable);
#define LOAD_HOBJECT(variable) g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &variable);
#define SAVE_VECTOR(variable)   g_pLTServer->WriteToMessageVector(hWrite, &variable);
#define LOAD_VECTOR(variable)   g_pLTServer->ReadFromMessageVector(hRead, &variable);
#define SAVE_ROTATION(variable) g_pLTServer->WriteToMessageRotation(hWrite, &variable);
#define LOAD_ROTATION(variable) g_pLTServer->ReadFromMessageRotation(hRead, &variable);
#define SAVE_HSTRING(variable)  g_pLTServer->WriteToMessageHString(hWrite, variable);
#define LOAD_HSTRING(variable)  variable = g_pLTServer->ReadFromMessageHString(hRead);
#define SAVE_RANGE(variable) g_pLTServer->WriteToMessageFloat(hWrite, (float)variable.GetMin()); g_pLTServer->WriteToMessageFloat(hWrite, (float)variable.GetMax());
#define LOAD_RANGE(variable) variable.Set(g_pLTServer->ReadFromMessageFloat(hRead), g_pLTServer->ReadFromMessageFloat(hRead));
#define LOAD_RANGE_CAST(variable, type) variable.Set((type)g_pLTServer->ReadFromMessageFloat(hRead), (type)g_pLTServer->ReadFromMessageFloat(hRead));

// FindNamedObject will return LT_OK if exactly one object of that name is found,
// LT_NOTFOUND if no objects of that name are found, or LT_ERROR if more than one
// of the named object is found (unless bMultipleOkay is LTTRUE, in which it will
// return LT_OK and just use the first object in the array)

inline LTRESULT FindNamedObject(const char* szName, HOBJECT& hObject, LTBOOL bMultipleOkay = LTFALSE)
{
	if ( !szName || !*szName ) return LT_NOTFOUND;

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

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)

#define FREE_HSTRING(x) if ( x ) { g_pLTServer->FreeString(x); x = LTNULL; }

void StartTimingCounter();
void EndTimingCounter(char *pMsg, ...);

LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
LTBOOL SendTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);
void SendTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL bBogus, const char* pStr);

LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
LTBOOL SendMixedTriggerMsgToObjects(LPBASECLASS pSender, const char* pName, const char* pMsg);
void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, HSTRING hMsg);
void SendMixedTriggerMsgToObject(LPBASECLASS pSender, HOBJECT hObj, LTBOOL bBogus, const char* pStr);

LTBOOL IsPlayer( HOBJECT hObject );
LTBOOL IsAI( HOBJECT hObject );
LTBOOL IsVehicle( HOBJECT hObject );
LTBOOL IsDog( HOBJECT hObject );
LTBOOL IsCharacter( HOBJECT hObject );
LTBOOL IsCharacterHitBox( HOBJECT hObject );
LTBOOL IsBody( HOBJECT hObject );
LTBOOL IsGameBase( HOBJECT hObject );
LTBOOL IsKindOf( HOBJECT hObject, const char* szClass );
LTBOOL IsKindOf( HOBJECT hObject, HOBJECT hObject2 );

LTBOOL IsVector(const char* szString);

uint16 Color255VectorToWord( LTVector *pVal );

LTBOOL MoveObjectToFloor(HOBJECT hObj);

inline void SetNextUpdate(HOBJECT hObj, LTFLOAT fDelta)
{
	if (!hObj) return;

	fDelta = fDelta <= 0.0f ? 0.0f : fDelta;
    g_pLTServer->SetNextUpdate(hObj, fDelta);

	if (fDelta == 0.0f)
	{
		g_pLTServer->SetDeactivationTime(hObj, 0.001f);
		g_pLTServer->SetObjectState(hObj, OBJSTATE_AUTODEACTIVATE_NOW);
	}
	else
	{
		g_pLTServer->SetDeactivationTime(hObj, fDelta + 1.0f);
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

LTBOOL DoesSegmentIntersectAABB(const LTVector& Point1, const LTVector& Point2, const LTVector& MinBox, const LTVector& MaxBox, LTVector *pIntersectPt = NULL, LTPlane *pIntersectPlane = NULL);

int		GetConsoleInt(char* sKey, int nDefault);
void	GetConsoleString(char* sKey, char* sDest, char* sDefault);
void	WriteConsoleString(char* sKey, char* sValue);
void	WriteConsoleInt(char* sKey, int nValue);
LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault);
void	WriteConsoleFloat(char* sKey, LTFLOAT fValue);

#define BUILD_NOPAIN_WAV	"null.wav"

#endif // __SERVER_UTILITIES_H__