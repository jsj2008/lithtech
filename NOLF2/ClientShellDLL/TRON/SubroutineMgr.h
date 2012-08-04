// ----------------------------------------------------------------------- //
//
// MODULE  : KeyMgr.h
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// TODO add code that other modules can use to determine what subroutines are
// active, maybe automatically hook functions or something.

// In alpha state a protection subroutine is a 1:1 draw from the energy bar.
// The higher the optimization level the less it draws from the energy changing the ratio.
// Lets say gold draws 50% of the protection subroutine's percentage.

#if !defined(_SUBROUTINE_MGR_H_)
#define _SUBROUTINE_MGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ScreenSpriteMgr.h"

#define SUBROUTINE_DEFAULT_FILE		"Attributes\\Subroutines.txt"
#define SUBROUTINE_MAX_NAME			32
#define SUBROUTINE_MAX_FILE_PATH	64


class CSubroutineMgr;
extern CSubroutineMgr* g_pSubroutineMgr;


// the subroutinemgr needs to maintain two arrays of subroutines.
// the first list is all conceivable types of subroutines, and this
// contains the minimal information on that subroutine.  It's essentially
// the template for that subroutine.

// The second array is the list of subroutines that actually exist at
// any given point in time, in the player's inventory.  This second list
// contains a pointer to the actual subroutine in the first list, and
// adds the game/player-specific data that's required.

extern char * szRingTex[];

typedef enum
{
	VERSION_ALPHA = 0,
	VERSION_BETA,
	VERSION_GOLD,
	LAST_VERSION
} SubVersion;

typedef enum
{
	SUBSTATE_OKAY = 0,
	SUBSTATE_FOREIGN,
	SUBSTATE_CORRUPT,
	SUBSTATE_UNUSABLE,	// bad block or basecode
	SUBSTATE_DELETED,
	SUBSTATE_LAST
} SubState;

typedef enum
{
	FUNCTION_COMBAT = 0,
	FUNCTION_DEFENSE,
	FUNCTION_UTILITY,
	FUNCTION_BADBLOCK,
	FUNCTION_BASECODE,
	FUNCTION_LAST
} SubFunction;

struct TronSubroutine
{
	TronSubroutine();
	char		szName[SUBROUTINE_MAX_NAME];
	uint16		nNameId;
	uint16		nDescriptionId;
	SubFunction	eFunction;			// combat, defense, utility, Bad sector, base code
	char		szSprite[SUBROUTINE_MAX_FILE_PATH];

	// defense-specific fields
	uint8		nDeflection;
	uint8		nAntivirus;
	uint8		nArmorPiece;

	// utility-specific fields
	float		fContinuousEnergyDrain;		// steady drainage in units per second while active
	float		fBurstEnergyDrain;			// drainage in units if it's a utility that you USE.
};

struct PlayerSubroutine
{
	PlayerSubroutine();
	TronSubroutine * pTronSubroutine;
	SubVersion	eVersion;			// alpha,beta,gold
	bool		bErased;			// is this subroutine currently erased?
	int			nTempSlot;			// temp location until player compiles
	int			nSlot;				// slot in player's system memory, -1 for library
	float		fPercentDone;		// Amount of work that a procedural has done to fix a condition
	float		fPercentCorrupt;	// Percentage that a subroutine is corrupt
	SubState	eState;				// okay, foreign, corrupt, unusable, deleted
	bool		bWorking;			// is a procedural working on this subroutine?

	// HACK for Base code so that we know which model to use.
	bool		bPrevious;
	bool		bNext;

	bool		bActive;			// used by Utility Subroutines
};

typedef std::vector<PlayerSubroutine *> PlayerSubroutineArray;

struct Procedural
{
	Procedural();
	char				szName[SUBROUTINE_MAX_NAME];
	uint16				nNameId;
	uint16				nDescriptionId;
	uint16				nCompletionId;	// string to display when a procedural finishes
	int					iProcSlot;	// 1-5 (6?)
	bool				bPlayerHasThis;
	float				fTimeRequired;	// time (in seconds) to perform a complete operation

	LTIntPt				CenterPos;		// centerpoint of procedural
	int					iRadius;		// size (radius) of control on screen

	char				szIntroFX[SUBROUTINE_MAX_NAME];

	// Various skins needed for visual representation
	char				szIdleSkin[SUBROUTINE_MAX_FILE_PATH];	// skin to use for idle anim
	char				szWorkSkin[SUBROUTINE_MAX_FILE_PATH];	// skin to use while working
	char				szConditionSkin[SUBROUTINE_MAX_FILE_PATH]; // skin to show condition
	char				szProgressSkin[SUBROUTINE_MAX_FILE_PATH];	// skin to show condition removed

	SubState			eAffectState;	// what kinds of subroutines does this affect?
	// other vars having to do with the specifics of how the proc works
	PlayerSubroutine * pSub;
};

typedef std::vector<Procedural *> ProceduralArray;

struct Primitive
{
	Primitive();
	char				szName[SUBROUTINE_MAX_NAME];
	uint16				nNameId;
	bool				bPlayerHasThis;
};
typedef std::vector<Primitive *> PrimitiveArray;

class CSubroutineMgr : public CGameButeMgr
{
public:
	CSubroutineMgr();
	virtual ~CSubroutineMgr();

    LTBOOL      Init(const char* szAttributeFile=SUBROUTINE_DEFAULT_FILE);
	void		Term();

	void		Pause(LTBOOL bPause); // pause and unpause the game
	void		Update();	// advance all timers

	uint16		GetNumSubroutines() {return m_SubroutineArray.size();}

	// Search functions
	TronSubroutine*		GetSubroutine(const char *pszName);
	Procedural *		GetProcedural(const char * name);
	Procedural *		GetProcedural(int iProcNum) {return m_ProceduralArray[iProcNum];}
	
	// Subroutine functions
	void				ClearPlayerSubroutines();
	void				AddPlayerSubroutine(PlayerSubroutine *pSub);
	void				GivePlayerSubroutine(char * name, char * state, char * condition);
	void				PopulateSubroutineScreen();

	// Utility Subroutine Functions
	bool				IsUtilitySubroutineActive(char * name);
	SubVersion			GetActiveSubroutineVersion(char * name);
	float				GetActiveSubroutineCorruption(char * name);

	// Procedural functions
	void				GivePlayerProcedural(char * name);
	bool				CanProcHookSub(Procedural *pProc, PlayerSubroutine *pSub);
	void				HookProcToSub(Procedural *pProc, PlayerSubroutine *pSub);
	Procedural *		WhoHookedToSub(PlayerSubroutine *pSub);
	bool				IsProceduralActive() {return m_bProceduralActive;}

	// Primitive functions
	void				GivePlayerPrimitive(char * name);
	bool				DoesPlayerHavePrimitive(char * name);

	// Gameplay-specific functions
	void				ComputeArmorCoverage();
	bool				HaveArmorPiece(int iPiece);

	void				Compile();

	// To set the system memory configuration (bad blocks, base code, etc...)
	bool				SetSystemMemoryConfiguration(const char* pSystemMemoryConfig);
	void				AddBadBlock(int iSectorNumber);	// create a subroutine that's unusable
	void				AddBaseCodeBlock(int iSectorNumber, bool bPrev, bool bNext); // create a subroutine that's unusable

protected:
	void				CleanArrays();

	void				ProceduralFinished(Procedural * pProc); // clean up a procedural operation

	char				* m_SystemMemoryArray;

	typedef std::vector<TronSubroutine *> SubroutineArray;
	SubroutineArray m_SubroutineArray;

	PlayerSubroutineArray m_PlayerSubroutineArray;

	ProceduralArray m_ProceduralArray;

	PrimitiveArray m_PrimitiveArray;

	LTBOOL				m_bPause;
	float				m_fLastUpdateTime;
	bool				m_bProceduralActive;
};

#endif // !defined(_SUBROUTINE_MGR_H_)
