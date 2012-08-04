//----------------------------------------------------------
//
// MODULE  : B2BaseClass.CPP
//
// PURPOSE : A BaseClass for Blood2 objects
//
// CREATED : 8/4/98
//
//----------------------------------------------------------

#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"
#include "BloodServerShell.h"
#include "SharedDefs.h"


BEGIN_CLASS(B2BaseClass)
	PROP_DEFINEGROUP(GameSettings, PF_GROUP6)
		ADD_BOOLPROP_FLAG(DifficultyEasy, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(DifficultyMed, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(DifficultyHard, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeCustom, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeSingle, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeActionMode, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeBloodBath, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeCTF, DTRUE, PF_GROUP6)
		ADD_BOOLPROP_FLAG(TypeCoop, DTRUE, PF_GROUP6)
END_CLASS_DEFAULT_FLAGS(B2BaseClass, BaseClass, NULL, NULL, CF_HIDDEN)


// Constructor
B2BaseClass::B2BaseClass(DBYTE nType) : BaseClass(nType)
{
	m_dwOptFlags = 0;
}


B2BaseClass::B2BaseClass() : BaseClass(OT_NORMAL)
{
	m_dwOptFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	B2BaseClass::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD B2BaseClass::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	if (!g_pServerDE) return 0;
	// Set object type...

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				ReadProp((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData == INITIALUPDATE_WORLDFILE)
			{
				DBOOL bResult = InitialUpdate();
				if (!bResult)
					g_pServerDE->RemoveObject(m_hObject);
			}
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	B2BaseClass::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL B2BaseClass::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE || !pStruct) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("DifficultyEasy", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_EASY : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_EASY;

	if (g_pServerDE->GetPropGeneric("DifficultyMed", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_MED : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_MED;

	if (g_pServerDE->GetPropGeneric("DifficultyHard", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_HARD : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_HARD;

	if (g_pServerDE->GetPropGeneric("DifficultyInsane", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_INSANE : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_INSANE;

	if (g_pServerDE->GetPropGeneric("TypeCustom", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_CUSTOM : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_CUSTOM;

	if (g_pServerDE->GetPropGeneric("TypeSingle", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_SINGLE : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_SINGLE;

	if (g_pServerDE->GetPropGeneric("TypeActionMode", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_ACTION: 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_ACTION;

	if (g_pServerDE->GetPropGeneric("TypeBloodbath", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_BLOODBATH : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_BLOODBATH;

	if (g_pServerDE->GetPropGeneric("TypeCTF", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_CTF : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_CTF;

	if (g_pServerDE->GetPropGeneric("TypeCoop", &genProp) == DE_OK)
	{
		m_dwOptFlags |= genProp.m_Bool ? OPT_FLAG_COOP : 0;
	}
	else
		m_dwOptFlags |= OPT_FLAG_COOP;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	B2BaseClass::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL B2BaseClass::InitialUpdate()
{
	if (!g_pServerDE) return DFALSE;

	DBOOL bAllow = DTRUE;
	
	DBYTE nDifficulty = GetGameDifficulty();
	DBYTE nGameType = GetGameType();

	// See if this object is allowed at this skill level
	switch (nDifficulty)
	{
		case DIFFICULTY_EASY: 
			if ((m_dwOptFlags & OPT_FLAG_EASY) == 0)
				bAllow = DFALSE;
			break;
		case DIFFICULTY_MEDIUM: 
			if ((m_dwOptFlags & OPT_FLAG_MED) == 0)
				bAllow = DFALSE;
			break;
		case DIFFICULTY_HARD: 
			if ((m_dwOptFlags & OPT_FLAG_HARD) == 0)
				bAllow = DFALSE;
			break;
		case DIFFICULTY_INSANE: 
			if ((m_dwOptFlags & OPT_FLAG_INSANE) == 0)
				bAllow = DFALSE;
			break;
	}

	// See if this object is allowed for this game type
	switch (nGameType)
	{
		case GAMETYPE_CUSTOM: 
			if ((m_dwOptFlags & OPT_FLAG_CUSTOM) == 0)
				bAllow = DFALSE;
			break;
		case GAMETYPE_SINGLE: 
			if ((m_dwOptFlags & OPT_FLAG_SINGLE) == 0)
				bAllow = DFALSE;
			break;
		case GAMETYPE_ACTION: 
			if ((m_dwOptFlags & OPT_FLAG_ACTION) == 0)
				bAllow = DFALSE;
			break;
		case GAMETYPE_BLOODBATH: 
			if ((m_dwOptFlags & OPT_FLAG_BLOODBATH) == 0)
				bAllow = DFALSE;
			break;
		case GAMETYPE_CTF: 
			if ((m_dwOptFlags & OPT_FLAG_CTF) == 0)
				bAllow = DFALSE;
			break;
		case GAMETYPE_COOP: 
			if ((m_dwOptFlags & OPT_FLAG_CTF) == 0)
				bAllow = DFALSE;
			break;
	}

	DDWORD dwFlags = g_pServerDE->GetObjectFlags(m_hObject);
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_DONTFOLLOWSTANDING);

	return bAllow;
}



