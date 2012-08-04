//----------------------------------------------------------
//
// MODULE  : LoadSave.h
//
// PURPOSE : Load and Save game info
//
// CREATED : 4/19/98
//
//----------------------------------------------------------


#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"


// world menu items (temp)
#define	MAX_SAVESLOTS	8		// Save game slots (not counting quick save)
#define MAX_SAVENAME	100		// Max length of the string containing the save name

#define SLOT_CURRENT	-2		// Special index defines for these slots
#define SLOT_QUICK		-1
#define SLOT_NONE		-255	// No slot


struct SavedInfo
{
	char	szName[MAX_SAVENAME+1];
	char	szCurrentLevel[MAX_CS_FILENAME_LEN+1];
	int		nGameType;
	int		nCharacter;
};

// A class to keep track of saved game info.
class CSavedGameInfo
{
	public:
		
		CSavedGameInfo();

		void	LoadInfo();
		void	SaveInfo(int nIndex, int nGameType, int nCharacter);
		void	ClearSaveSlot(int nIndex);
		void	CopySlot(int nSrcIndex, int nDestIndex);
		int		GetReservedSlot() { return m_nReservedSlot; }
		void	SetReservedSlot(int nSlot) { m_nReservedSlot = nSlot; }
		void	SetSlotName(int nSlot, char *pName);

		static  SavedInfo gQuickSaveInfo;
		static  SavedInfo gCurrentSaveInfo;
		static	SavedInfo gSaveSlotInfo[MAX_SAVESLOTS];

	private:

		int		m_nReservedSlot;		// Keep track of which slot we are saving to.
};


#endif	// __MENU_H__
