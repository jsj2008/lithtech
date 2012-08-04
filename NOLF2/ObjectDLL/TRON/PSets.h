/****************************************************************************
;
;	MODULE:			PSets.h
;
;	PURPOSE:		Permission Set include file
;
;	HISTORY:		12/24/2001 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/

#define ADD_PSETS_PROP(group) \
	PROP_DEFINEGROUP(PermissionSets, group) \
	ADD_BOOLPROP_FLAG(Key1,0,group)\
	ADD_BOOLPROP_FLAG(Key2,0,group)\
	ADD_BOOLPROP_FLAG(Key3,0,group)\
	ADD_BOOLPROP_FLAG(Key4,0,group)\
	ADD_BOOLPROP_FLAG(Key5,0,group)\
	ADD_BOOLPROP_FLAG(Key6,0,group)\
	ADD_BOOLPROP_FLAG(Key7,0,group)\
	ADD_BOOLPROP_FLAG(Key8,0,group)

uint8 MakePSetsByte(LTBOOL bPSets[8])
{
	// Change em into a byte
	uint8 byPSets = 0;
	for (int i = 0; i < 8; i++)
	{
		if (bPSets[i])
		{
			byPSets |= 1<<i;
		}
	}

	return byPSets;
}

#define HANDLE_READ_PSETS_PROPS(var) \
	LTBOOL bPSets[8]; \
	memset(bPSets,0,8*sizeof(LTBOOL)); \
	g_pLTServer->GetPropBool("Key1", &bPSets[0]); \
	g_pLTServer->GetPropBool("Key2", &bPSets[1]); \
	g_pLTServer->GetPropBool("Key3", &bPSets[2]); \
	g_pLTServer->GetPropBool("Key4", &bPSets[3]); \
	g_pLTServer->GetPropBool("Key5", &bPSets[4]); \
	g_pLTServer->GetPropBool("Key6", &bPSets[5]); \
	g_pLTServer->GetPropBool("Key7", &bPSets[6]); \
	g_pLTServer->GetPropBool("Key8", &bPSets[7]); \
	var = MakePSetsByte(bPSets);

#define SET_OBJECT_PSETS_USER_FLAGS(hObj,pSets) \
	uint32 dwFlags = pSets; \
	dwFlags = dwFlags << 13; \
	g_pCommonLT->SetObjectFlags(hObj, OFT_User, dwFlags, dwFlags);