// ----------------------------------------------------------------------- //
//
// MODULE  : B2BaseClass.h
//
// PURPOSE : A BaseClass extension for Blood2 objects
//
// CREATED : 8/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __B2BASECLASS_H__
#define __B2BASECLASS_H__

#include "cpp_engineobjects_de.h"


extern char g_szVarDifficulty[];
extern char g_szVarGameType[];


#define OPT_FLAG_EASY		1
#define OPT_FLAG_MED		2
#define OPT_FLAG_HARD		4
#define OPT_FLAG_INSANE		8

#define OPT_FLAG_CUSTOM		16
#define OPT_FLAG_SINGLE		32
#define OPT_FLAG_ACTION		64
#define OPT_FLAG_BLOODBATH	128
#define OPT_FLAG_CTF		256
#define OPT_FLAG_COOP		512

	

class B2BaseClass : public BaseClass
{
	public :

		B2BaseClass();
		B2BaseClass(DBYTE nType);
		virtual ~B2BaseClass() {}
		DBYTE GetGameDifficulty()
		{
			if (!g_pServerDE) return 0;
			HCONVAR hConVar = g_pServerDE->GetGameConVar(g_szVarDifficulty);
			return hConVar ? (DBYTE)g_pServerDE->GetVarValueFloat(hConVar) : 0;
		}

		DBYTE GetGameType()
		{
			if (!g_pServerDE) return 0;
			HCONVAR hConVar = g_pServerDE->GetGameConVar(g_szVarGameType);
			return hConVar ? (DBYTE)g_pServerDE->GetVarValueFloat(hConVar) : 0;
		}

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate();

		DDWORD	m_dwOptFlags;
};



#endif  // __B2BASECLASS_H__

