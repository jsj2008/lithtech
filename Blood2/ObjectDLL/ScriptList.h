// ----------------------------------------------------------------------- //
//
// MODULE  : ScriptList.h
//
// PURPOSE : List of ScriptCmd class objects
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __SCRIPT_LIST_H__
#define __SCRIPT_LIST_H__

#include "dynarray.h"
#include <memory.h>  // for memset

#define MAXDATA_LEN            50

// Cut Scene Commands
#define SCRIPTSCENE_START                           "START"
#define SCRIPTSCENE_END                             "END"

#define SCRIPTSCENE_CUTSCENE                        "CUTSCENE"

#define SCRIPTSCENE_SUBTITLE_DISPLAY                "SUBTITLEDISPLAY"
#define SCRIPTSCENE_SUBTITLE_FADE		            "SUBTITLEFADE"

#define SCRIPTSCENE_PLAY_ANIM                       "PLAYANIM"
#define SCRIPTSCENE_PLAY_SOUNDPOS                   "PLAYSOUNDPOS"
#define SCRIPTSCENE_PLAY_SOUNDOBJ                   "PLAYSOUNDOBJ"

#define SCRIPTSCENE_WAIT							"WAIT"

#define SCRIPTSCENE_KILL_SOUND                      "KILLSOUND"

#define SCRIPTSCENE_OBJECT_CREATE_AT                "OBJECTCREATE"
#define SCRIPTSCENE_OBJECT_CREATE_NEAR              "OBJECTCREATENEAR"

#define SCRIPTSCENE_OBJECT_ASSIGN                   "OBJECTASSIGN"
#define SCRIPTSCENE_OBJECT_CREATE_MODEL_AT          "OBJECTCREATEMODEL"
#define SCRIPTSCENE_OBJECT_CREATE_MODEL_NEAR        "OBJECTCREATEMODELNEAR"
#define SCRIPTSCENE_OBJECT_CREATE_FIRE              "OBJECTCREATEFIRE"

#define SCRIPTSCENE_OBJECT_CREATE_EXPLOSION_AT      "OBJECTCREATE_EXPLOSIONAT"
#define SCRIPTSCENE_OBJECT_CREATE_EXPLOSION_NEAR    "OBJECTCREATE_EXPLOSIONNEAR"

#define SCRIPTSCENE_OBJECT_MOVE                     "OBJECTMOVE"
#define SCRIPTSCENE_OBJECT_MOVE_OVERTIME            "OBJECTMOVETOOBJ"
#define SCRIPTSCENE_OBJECT_TELEPORT                 "OBJECTTELEPORT"
#define SCRIPTSCENE_OBJECT_FLAGS                    "OBJECTFLAGS"
#define SCRIPTSCENE_OBJECT_FLAG_VISIBLE             "OBJECTFLAGVISIBLE"
#define SCRIPTSCENE_OBJECT_FLAG_SOLID               "OBJECTFLAGSOLID"
#define SCRIPTSCENE_OBJECT_FLAG_GRAVITY             "OBJECTFLAGGRAVITY"
#define SCRIPTSCENE_OBJECT_DIMS                     "OBJECTDIMS"
#define SCRIPTSCENE_OBJECT_GIB                      "OBJECTGIB"
#define SCRIPTSCENE_OBJECT_SCALE                    "OBJECTSCALE"
#define SCRIPTSCENE_OBJECT_DESTROY                  "OBJECTDESTROY"

//#define SCRIPTSCENE_CAMERA_FOV                      "CAMERAFOV"
//#define SCRIPTSCENE_CAMERA_RECT                     "CAMERARECT"
#define SCRIPTSCENE_CAMERA_RESET                    "CAMERARESET"

#define SCRIPTSCENE_CAMERA_CREATE                   "CAMERACREATE"         // 
#define SCRIPTSCENE_CAMERA_DESTROY                  "CAMERADESTROY"         // 
#define SCRIPTSCENE_CAMERA_SELECT                   "CAMERASELECT"         // arg 0=camera object to view from
#define SCRIPTSCENE_CAMERA_MOVE_OBJ                 "CAMERAMOVETOOBJ"        // moves camera over time to a specified object
#define SCRIPTSCENE_CAMERA_TELEPORT                 "CAMERATELEPORT"       // 
#define SCRIPTSCENE_CAMERA_TELEPORT_OBJ				"CAMERATELEPORTTOOBJ"		// Teleport to a specified object
#define SCRIPTSCENE_CAMERA_ROTATE                   "CAMERAROTATE"
#define SCRIPTSCENE_CAMERA_LINKSPOT                 "CAMERALINKSPOT"
#define SCRIPTSCENE_CAMERA_ZOOM                     "CAMERAZOOM"

#define SCRIPTSCENE_EXECUTESCRIPT                   "EXECUTESCRIPT"

#define SCRIPTSCENE_SEND_AI_SCRIPT                  "SENDAISCRIPT"
#define SCRIPTSCENE_SEND_TRIGGER                    "SENDTRIGGER"
#define SCRIPTSCENE_SEND_TRIGGER_NAMED              "SENDTRIGGERNAMED"



#define SCRIPT_SCMD_UNKNOWN                     0
#define SCRIPT_SCMD_ARG                         1
#define SCRIPT_SCMD_START                       2
#define SCRIPT_SCMD_END                         3
#define SCRIPT_SCMD_SUBTITLE_DISPLAY            4
#define SCRIPT_SCMD_SUBTITLE_FADE		        5

#define SCRIPT_SCMD_PLAY_ANIM                   6
#define SCRIPT_SCMD_PLAY_SOUND_POS              7
#define SCRIPT_SCMD_PLAY_SOUND_OBJ              8

#define SCRIPT_SCMD_WAIT                        9

#define SCRIPT_SCMD_OBJECT_CREATE_AT            10
#define SCRIPT_SCMD_OBJECT_MOVE                 11
#define SCRIPT_SCMD_OBJECT_TELEPORT             12
#define SCRIPT_SCMD_OBJECT_FLAGS                14
#define SCRIPT_SCMD_OBJECT_FLAG_VISIBLE         15
#define SCRIPT_SCMD_OBJECT_FLAG_SOLID           16
#define SCRIPT_SCMD_OBJECT_FLAG_GRAVITY         17

#define SCRIPT_SCMD_CAMERA_CREATE               18
#define SCRIPT_SCMD_CAMERA_DESTROY              19
//#define SCRIPT_SCMD_CAMERA_RECT                 20
//#define SCRIPT_SCMD_CAMERA_RESET                21
#define SCRIPT_SCMD_CAMERA_SELECT               22
#define SCRIPT_SCMD_CAMERA_MOVE_OBJ             23
#define SCRIPT_SCMD_CAMERA_TELEPORT             24
#define SCRIPT_SCMD_CAMERA_TELEPORT_OBJ         25
#define SCRIPT_SCMD_CAMERA_ROTATE               26
#define SCRIPT_SCMD_CAMERA_LINKSPOT             27
#define SCRIPT_SCMD_CAMERA_ZOOM                 28

#define SCRIPT_SCMD_CUTSCENE                    40

#define SCRIPT_SCMD_OBJECT_ASSIGN               41
#define SCRIPT_SCMD_OBJECT_CREATE_MODEL_AT      42
#define SCRIPT_SCMD_OBJECT_CREATE_FIRE          43
#define SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_AT  44
#define SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_NEAR 45

#define SCRIPT_SCMD_OBJECT_DIMS                 46
#define SCRIPT_SCMD_OBJECT_MOVE_OVERTIME        47
#define SCRIPT_SCMD_SEND_AI_SCRIPT              48
#define SCRIPT_SCMD_OBJECT_GIB                  49
#define SCRIPT_SCMD_OBJECT_SCALE                50

#define SCRIPT_SCMD_OBJECT_DESTROY              51

#define SCRIPT_SCMD_OBJECT_CREATE_NEAR          52

#define SCRIPT_SCMD_KILL_SOUND                  53
#define SCRIPT_SCMD_OBJECT_CREATE_MODEL_NEAR    54

#define SCRIPT_SCMD_SEND_TRIGGER                60
#define SCRIPT_SCMD_SEND_TRIGGER_NAMED          61

                    
// SCRIPTCMD struct -> CScriptList nodes...
struct SCRIPTCMD
{
	SCRIPTCMD::SCRIPTCMD();
    
    DFLOAT  fStartTime;
	int		command;
                        //	char args[MAX_ARGS_LENGTH];
	char	data[MAXDATA_LEN];
    DBOOL	m_bExecuted;
};

// Store commands like this:
// command = COMMAND
// data = num of args
// command = ARG
// data = "STRING"
// etc... (for all the args, this will give unlimited number of args per each command


inline SCRIPTCMD::SCRIPTCMD()
{
	memset(this, 0, sizeof(SCRIPTCMD));
}


class CScriptList
{
	public :

		int GetNumItems()	const { return m_nNumItems; }
		DBOOL IsEmpty()		const { return (DBOOL)(m_nNumItems == 0); }

		CScriptList()
		{
			m_pArray	= new CDynArray<SCRIPTCMD*> (DTRUE, 16);
			m_nNumItems = 0;
		}

		~CScriptList()
		{
			if (m_pArray)
			{
				for (int i=0; i < m_nNumItems; i++)
				{
					delete (*m_pArray)[i];
				}

				delete m_pArray;
			}
		}

		SCRIPTCMD* & operator[] (int nIndex)
		{
			assert (nIndex >= 0 && nIndex < m_nNumItems);
			return ((*m_pArray)[nIndex]);
		}

		void Add(SCRIPTCMD* pFX)
		{
			if (m_pArray && pFX)
			{
				(*m_pArray)[m_nNumItems++] = pFX;
			}
		}

		DBOOL Remove(SCRIPTCMD* pFX, DBOOL bRebuildArray=DTRUE)
		{
			DBOOL bRet = DFALSE;
			if (!m_pArray || !pFX) return DFALSE;

			for (int i=0; i < m_nNumItems; i++)
			{
				if ((*m_pArray)[i] == pFX)
				{
					delete (*m_pArray)[i];
					(*m_pArray)[i] = DNULL;
					bRet = DTRUE;
					break;
				}
			}

			if (bRet && bRebuildArray) RebuildArray();

			return bRet;
		}

		DBOOL Remove(int nIndex, DBOOL bRebuildArray=DTRUE)
		{
			if (!m_pArray || nIndex < 0 || nIndex > m_nNumItems) return DFALSE;

			delete (*m_pArray)[nIndex];
			(*m_pArray)[nIndex] = DNULL;

			if (bRebuildArray) RebuildArray();

			return DTRUE;
		}


		void RemoveAll()
		{
			if (m_pArray)
			{
				for (int i=0; i < m_nNumItems; i++)
				{
					delete (*m_pArray)[i];
				}

				delete m_pArray;
			}

			m_pArray	= new CDynArray<SCRIPTCMD*> (DTRUE, 16);
			m_nNumItems = 0;
		}

		// Remove all NULL entries from the array...

		void RebuildArray()
		{
			if (!m_pArray) return;

			CDynArray<SCRIPTCMD*> *pNewArray = new CDynArray<SCRIPTCMD*> (DTRUE, 16);
			if (!pNewArray) return;

			int nNewCount = 0;
			for (int i=0; i < m_nNumItems; i++)
			{
				if ((*m_pArray)[i])
				{
					(*pNewArray)[nNewCount++] = (*m_pArray)[i];
				}
			}

			delete m_pArray;

			m_pArray	= pNewArray;
			m_nNumItems = nNewCount;
		}
	
	private :

		CDynArray<SCRIPTCMD*> *m_pArray;	// Dynamic array 
		int	m_nNumItems;					// Number of elements in array

};


#endif // __SCRIPT_LIST_H__