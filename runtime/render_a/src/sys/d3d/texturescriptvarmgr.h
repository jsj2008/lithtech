//----------------------------------------------------------------------
// TextureScriptVarMgr.h
//
// Provides the definition for a class that holds variables used to
// control rendering of texture transforms so that they are accessible
// to game code and the renderer.
//
//----------------------------------------------------------------------

#ifndef __TEXTURESCRIPTVARMGR_H__
#define __TEXTURESCRIPTVARMGR_H__

#ifndef __TEXTURESCRIPTDEFS_H__
#	include "texturescriptdefs.h"
#endif

//forward declarations
class CTextureScriptVarNode;

class CTextureScriptVarMgr
{
public:

	//the number of user variables for each ID
	enum	{	NUM_VARS	= TS_NUMUSERVARS	};

	CTextureScriptVarMgr();
	~CTextureScriptVarMgr();

	//singleton accessor
	static CTextureScriptVarMgr&	GetSingleton();

	//static function that when given a string will return a hashed ID to be used
	//for accessing the actual variables
	static uint32	GetID(const char* pszName, uint32 nTransform);

	//used to add a new entry into the table. If an array is passed into the function
	//it will be assumed to be at least the size specified and any other elements will
	//be initialized with 0. Returns false if unable to allocate room
	bool			CreateVars(uint32 nID, uint32 nNumDefaults = 0, float* pDefaults = NULL);

	//gets the variable list from the specified ID. Returns NULL if the ID is not found
	float*			GetVars(uint32 nID);

	//called to set a specific variable. The return value indicates whether or not
	//that ID was found. Assumes the var is in the range of [0..NUM_VARS - 1]
	bool			SetVar(uint32 nID, uint32 nVar, float fVal);

	//releases an ID, returns true if it found the ID to be released
	bool			DeleteVars(uint32 nID);

	//frees all variables
	void			DeleteAllVars();

private:
	
	//prevent copying and assignment
	CTextureScriptVarMgr(const CTextureScriptVarMgr&)	{}
	void operator=(const CTextureScriptVarMgr&)			{}

	//finds the actual node given an ID. Will return NULL if none found
	CTextureScriptVarNode*	GetNode(uint32 nID);

	CTextureScriptVarNode* m_pHead;
};


#endif
