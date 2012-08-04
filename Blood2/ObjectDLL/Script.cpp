//----------------------------------------------------------
//
// MODULE  : SCRIPT.CPP
//
// PURPOSE : Script object 
//
// CREATED : 01/13/98
//
//----------------------------------------------------------

// Includes...

#include <stdio.h>
#include "generic_msg_de.h"
#include "Script.h"
#include "SharedDefs.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"

#include "CameraObj.h"
#include "FireFX.h"
#include "Trigger.h"
#include "Explosion.h"
#include "SoundTypes.h"

#include <mbstring.h>

// Subgroup commands
// 015 :OBJECT
//  001 CREATE
//  002 MOVE
//


char *NextToken(char *pszString, const char *pszSeps);


BEGIN_CLASS(Script)
	ADD_STRINGPROP(ScriptName, "script.txt")	//  Script file to load
	ADD_REALPROP(StartTime, 0.0f)	            //  Start time, or wait for Trigger
END_CLASS_DEFAULT_FLAGS(Script, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Script::Script()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Script::Script() : B2BaseClass(OT_NORMAL)
{
	m_nObjectIndex		= 0;
	m_nCameraIndex		= 0;    
	m_nCurrLine			= 0;

	m_hstrScriptName	= DNULL;
	m_bScriptLoaded		= DFALSE;
	m_bCheckOverTime	= DFALSE;
	m_bCheckStartTime	= DTRUE;

	m_nDefineCnt		= 0;

	m_bScriptStarted	= DFALSE;


	memset(ScriptObject, 0, sizeof(ScriptObject));
	memset(ScriptCamera, 0, sizeof(ScriptCamera));
	memset(ScriptSound, 0, sizeof(ScriptSound));

	for (int i=0; i<MAX_MOVEOBJS+1; i++)
	{
		m_nMoveObject[i] = -1;
		m_nMoveCamera[i] = -1;
	}            
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Script::~Script()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Script::~Script()
{
	if (!g_pServerDE) return;

	if (m_hstrScriptName)
		g_pServerDE->FreeString(m_hstrScriptName);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::ReadProp()
//
//	PURPOSE:	Reads properties
//
// --------------------------------------------------------------------------- //

DBOOL Script::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return DFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("ScriptName", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) m_hstrScriptName = g_pServerDE->CreateString(genProp.m_String);
	}

	if (g_pServerDE->GetPropGeneric("StartTime", &genProp) == DE_OK)
	{
		m_fStartTime = genProp.m_Float;
		if (m_fStartTime <= 0.0f) m_fStartTime = 0.001f;
	}

	return DTRUE;
}

      

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::PostPropRead()
//
//	PURPOSE:	Initializes data
//
// --------------------------------------------------------------------------- //

void Script::PostPropRead(ObjectCreateStruct *pStruct)
{
	// Set the Update!
	pStruct->m_NextUpdate = 0.01f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::InitialUpdate()
//
//	PURPOSE:	Initializes data.
//
// --------------------------------------------------------------------------- //

DBOOL Script::InitialUpdate(DVector *pMovement)
{
	// load the script
	if (!ProcessScript())
		return DFALSE;
	
	// Set next update to the script start time
	g_pServerDE->SetNextUpdate(m_hObject, m_fStartTime);

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::ProcessScript()
//
//	PURPOSE:	Load the Script and parse the commands (1 line per update)
//
// --------------------------------------------------------------------------- //
DBOOL Script::ProcessScript()
{
	int nEOF = 0;

	// LoadScriptStart
	// Open the File

	// Make sure the Script data is Clear
	m_scriptCmdList.RemoveAll();

	// Initial Update... Lets read in the Script File.
	if (m_hstrScriptName)
	{
		if (!(infile = fopen(g_pServerDE->GetStringData(m_hstrScriptName), "r")))
		{
			return DFALSE;
		}
	}
	else
	{
		g_pServerDE->BPrint("ERROR Scriptfile not found or PreProcessing Error.");
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		g_pServerDE->RemoveObject(m_hObject);
		return DFALSE;
	}

	// Initialize the time cout
	m_fNextScriptTime = 0.0f;

	while (nEOF == 0)
	{
		if (LoadScript(nEOF) == DFALSE)
		{
			// If Returns DFALSE then ERROR
			return DFALSE;
		}
	}

	m_bScriptLoaded = DTRUE;
	fclose(infile);

	return DTRUE;
}


/*


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::CheckStartTime()
//
//	PURPOSE:	Check to see if its time to run the script
//
// --------------------------------------------------------------------------- //
DBOOL Script::CheckStartTime()
{
	// StartTime check 
	// If StartTime = 0.0 then wait for a Trigger
	// if StartTime = -1.0  then Stop the Script (set the StartTime to 0.0

	// Trigger STOP
	if (m_fStartTime < 0.0f)
	{
		// Slow down the Update
		g_pServerDE->SetNextUpdate(m_hObject, 0.1f);
		m_fStartTime = 0.0f;
		return DTRUE;        
	}   
	else if (m_fStartTime == 0.0f)
	{
		// Wait for Trigger to Set timer
		return DTRUE;
	}

	// Check if its TIME to Start Script
	if (g_pServerDE->GetTime() > m_fScriptTime + m_fStartTime )
	{
		// Increase the Update
		g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		m_fScriptTime = g_pServerDE->GetTime();
		m_bCheckStartTime = DFALSE;
	}
	else
	{
		return DTRUE;
	}

	return DTRUE;
}

*/


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::Update()
//
//	PURPOSE:	Script update function, updates the current state.
//
// --------------------------------------------------------------------------- //
DBOOL Script::Update(DVector *pMovement)
{
	g_pServerDE->SetNextUpdate(m_hObject, (DFLOAT).01);

	//
	// First Load the script
	//
	if (m_bScriptLoaded == DFALSE)
	{
		return DFALSE;
	}

	m_curScriptCmd.command = SCRIPT_SCMD_UNKNOWN;

	// Check for Movement of Objects...
	if (m_bCheckOverTime == DTRUE)
	{
		DBOOL bStillUpdate = DFALSE;
		// Only Support MAX_MOVEOBJS at 1 time
		for (int i=0; i<MAX_MOVEOBJS+1; i++)
		{
			if (m_nMoveObject[i] != -1)
			{
				if (ScriptObject[m_nMoveObject[i]].m_bMovement == DTRUE)
				{
					ObjectData *pObj = &ScriptObject[m_nMoveObject[i]];
					if (g_pServerDE->GetTime() > pObj->m_fStartTime + pObj->m_fDuration)
					{
						pObj->m_bMovement = DFALSE;
						m_nMoveObject[i] = -1;
					}        
					else
					{
						pObj->m_fCurrentTime = g_pServerDE->GetTime();
						ObjectMoveOverTimeUpdate(pObj);
						bStillUpdate = DTRUE;
					}        
				}
			}                
			if (m_nMoveCamera[i] != -1)
			{
				if (ScriptCamera[m_nMoveCamera[i]].m_bMovement == DTRUE)
				{
					ObjectData *pObj = &ScriptCamera[m_nMoveCamera[i]];
					if (g_pServerDE->GetTime() > pObj->m_fStartTime + pObj->m_fDuration)
					{
						pObj->m_bMovement = DFALSE;
						m_nMoveCamera[i] = -1;
					}        
					else
					{
						pObj->m_fCurrentTime = g_pServerDE->GetTime();
						ObjectMoveOverTimeUpdate(pObj);
						bStillUpdate = DTRUE;
					}        
				}
			}                
		}    
		if (!bStillUpdate)
			m_bCheckOverTime = DFALSE;
	}


	//
	// Check for script Updates
	//
	while (m_scriptCmdList.GetNumItems() > m_nCurrLine && 
			m_scriptCmdList[m_nCurrLine]->m_bExecuted == DFALSE &&
			g_pServerDE->GetTime() >= m_fNextScriptTime)
	{
		switch (m_scriptCmdList[m_nCurrLine]->command)
		{
			case SCRIPT_SCMD_START:                StartScript();      break;

			case SCRIPT_SCMD_CUTSCENE               :      SetCutScene();      break;

			case SCRIPT_SCMD_SUBTITLE_DISPLAY       :      SubTitleDisplay();  break;
			case SCRIPT_SCMD_SUBTITLE_FADE          :      SubTitleFade();     break;

			case SCRIPT_SCMD_OBJECT_CREATE_AT       :      ObjectCreateAt();   break;
			case SCRIPT_SCMD_OBJECT_CREATE_NEAR     :      ObjectCreateNear();   break;

			case SCRIPT_SCMD_OBJECT_ASSIGN          :      ObjectAssign();    break;
			case SCRIPT_SCMD_OBJECT_CREATE_MODEL_AT :      ObjectCreateModelAt();     break;
			case SCRIPT_SCMD_OBJECT_CREATE_MODEL_NEAR:     ObjectCreateModelNear();     break;
			case SCRIPT_SCMD_OBJECT_CREATE_FIRE     :      ObjectCreateFire();      break;
			case SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_AT:   ObjectCreateExplosionAt();  break;
			case SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_NEAR: ObjectCreateExplosionNear();  break;

			case SCRIPT_SCMD_OBJECT_MOVE            :      ObjectMove();       break;
			case SCRIPT_SCMD_OBJECT_MOVE_OVERTIME   :      ObjectMoveObj();    break;
			case SCRIPT_SCMD_OBJECT_TELEPORT        :      ObjectTeleport();   break;
			case SCRIPT_SCMD_OBJECT_FLAGS           :      ObjectFlags();      break;
			case SCRIPT_SCMD_OBJECT_FLAG_VISIBLE    :      ObjectFlagVisible();break;
			case SCRIPT_SCMD_OBJECT_FLAG_SOLID      :      ObjectFlagSolid();break;
			case SCRIPT_SCMD_OBJECT_FLAG_GRAVITY    :      ObjectFlagGravity();break;
			case SCRIPT_SCMD_OBJECT_DIMS            :      ObjectDims();       break;
			case SCRIPT_SCMD_OBJECT_GIB             :      ObjectGib();        break;
			case SCRIPT_SCMD_OBJECT_SCALE           :      ObjectScale();      break;
			case SCRIPT_SCMD_OBJECT_DESTROY         :      ObjectDestroy();    break;

			case SCRIPT_SCMD_WAIT                   :      WaitDelay();        break;
			case SCRIPT_SCMD_PLAY_ANIM              :      PlayAnimation();    break;
			case SCRIPT_SCMD_PLAY_SOUND_POS         :      PlaySoundPos();     break;
			case SCRIPT_SCMD_PLAY_SOUND_OBJ         :      PlaySoundObj();     break;

			case SCRIPT_SCMD_KILL_SOUND             :      KillSound();     break;

//				case SCRIPT_SCMD_CAMERA_RESET           :      CameraReset();      break;

			case SCRIPT_SCMD_CAMERA_CREATE          :      CameraCreate();     break;
			case SCRIPT_SCMD_CAMERA_DESTROY			:	   CameraDestroy();    break;
			case SCRIPT_SCMD_CAMERA_SELECT          :      CameraSelect();     break;
			case SCRIPT_SCMD_CAMERA_MOVE_OBJ        :      CameraMoveObj();    break;
			case SCRIPT_SCMD_CAMERA_TELEPORT        :      CameraTeleport();   break;
			case SCRIPT_SCMD_CAMERA_TELEPORT_OBJ    :      CameraTeleportObj();break;
			case SCRIPT_SCMD_CAMERA_ROTATE          :      CameraRotate();     break;
			case SCRIPT_SCMD_CAMERA_LINKSPOT        :      CameraLinkSpot();   break;
			case SCRIPT_SCMD_CAMERA_ZOOM            :      CameraZoom();       break;
			case SCRIPT_SCMD_ARG                    :      break;

			case SCRIPT_SCMD_SEND_AI_SCRIPT         :      SendAIScript();     break;
			case SCRIPT_SCMD_SEND_TRIGGER           :      SendTrigger();      break;
			case SCRIPT_SCMD_SEND_TRIGGER_NAMED     :      SendTriggerNamed(); break;

			case SCRIPT_SCMD_END                    :      EndScript();        break;     

			default:
			{
				g_pServerDE->BPrint("Error: Unknown Command: %d", m_scriptCmdList[m_nCurrLine]->command);

				return DFALSE;
			}
		}

		m_scriptCmdList[m_nCurrLine]->m_bExecuted = DTRUE;

		// Check for Last line
		if (m_scriptCmdList.GetNumItems() > m_nCurrLine)  
		{   
			m_nCurrLine++;
		}                    
		else
		{
			break;
			// Turn off update, maybe remove object?
			//  g_pServerDE->BPrint("Script DONE!");
		}
	}

	return DTRUE;
}


 


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

DDWORD Script::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
			if (Update((DVector*)pData) == DFALSE)
            {
				g_pServerDE->RemoveObject(m_hObject);
                return 0;
            }
			break;

		case MID_PRECREATE:
			{
				DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

				if (fData == 1.0f)
					ReadProp((ObjectCreateStruct*)pData);

				    PostPropRead((ObjectCreateStruct*)pData);

				return dwRet;
			}

		case MID_INITIALUPDATE:
		    InitialUpdate((DVector*)pData);
			break;
            
		// If we created a link to the target, this will tell us that it no longer exists
		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
            for (int x=0; x<MAXSCRIPTOBJ; x++)
            {
                if (ScriptObject[x].m_hObject == hObj)
                {
	    			ScriptObject[x].m_hObject = DNULL;
                    break;
		    	}
            }                
		}
		break;            
	}
	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Script::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
			HandleTrigger(hSender, hRead);
			break;
	}
	
	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Script::HandleTrigger()
//
//	PURPOSE:	Trigger function 
//
// --------------------------------------------------------------------------- //

DBOOL Script::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	if (!g_pServerDE) return DFALSE;

	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);

	char *pCommand = _strupr(g_pServerDE->GetStringData(hMsg));

	g_pServerDE->BPrint(pCommand);

	if ( _mbsncmp((const unsigned char*)pCommand, (const unsigned char*)"SCRIPT_STOP", 11) == 0)
	{
		m_fStartTime = -1.0f;
		m_bCheckStartTime = DTRUE;
	}

	// Dont Restart it...
	if ( m_bScriptStarted == DTRUE )  return DFALSE;

	if ( _mbsncmp((const unsigned char*)pCommand, (const unsigned char*)"SCRIPT_START", 12) == 0)
	{
		m_fStartTime = 1.0f;
		m_bCheckStartTime = DTRUE;
	}

	g_pServerDE->FreeString( hMsg );

	return DFALSE;
}






// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StringToScriptCutCmdType
//
//	PURPOSE:	Cut Scene Script Commands and num of args
//
// ----------------------------------------------------------------------- //
int Script::StringToScriptCutCmdType(char* pCmdName, int &nArgs, int &nCheckObjID)
{
	if (!pCmdName) return SCRIPT_SCMD_UNKNOWN;

	int nType = SCRIPT_SCMD_UNKNOWN;
    
    // Return the number of args required
    nArgs = 0;
    
    // Return the ObjectID Arg position, to check to make sure we are not over MAXSCRIPTOBJ
    nCheckObjID = 0;
    	
	if (_mbsicmp((const unsigned char*)SCRIPTSCENE_START, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_START;
        nArgs = 0;
	}
                    
	if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CUTSCENE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CUTSCENE;
        nArgs = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_SUBTITLE_DISPLAY, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_SUBTITLE_DISPLAY;
        nArgs = 2;          // Length of time, Text to Display.
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_SUBTITLE_FADE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_SUBTITLE_FADE;
        nArgs = 1;          // Delay
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_PLAY_ANIM, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_PLAY_ANIM;
        nArgs = 2;          // Object, Anim Num
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_PLAY_SOUNDPOS, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_PLAY_SOUND_POS;
        nArgs = 7;          // SoundID, X, Y, Z, Sound, Radius,Looping
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_PLAY_SOUNDOBJ, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_PLAY_SOUND_OBJ;
        nArgs = 5;          // SoundID, ObjectID(for Position), Sound, Radius,Looping
        nCheckObjID = 2;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_KILL_SOUND, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_KILL_SOUND;
        nArgs = 1;          // SoundID
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_WAIT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_WAIT;
        nArgs = 1;          // Delay in secs
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_AT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_AT;
        nArgs = 5;          // Object ID, Class to create, X, Y, Z
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_NEAR, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_NEAR;
        nArgs = 3;          // Object ID, Class to create, NearObjectID
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_ASSIGN, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_ASSIGN;
        nArgs = 2;          // Object ID, Target Name (object to assign the ID to, must be in level) 
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_MODEL_AT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_MODEL_AT;
        nArgs = 6;          // Object ID, Model Name, Skin Name, X, Y, Z
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_MODEL_NEAR, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_MODEL_NEAR;
        nArgs = 4;          // Object ID, NearObjectID, Model Name, Skin Name
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_FIRE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_FIRE;
        nArgs = 3;          // Object ID, Target ID (object to attach to), BurnTime
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_EXPLOSION_AT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_AT;
        nArgs = 8;          // X, Y, Z, Duration, Sound Radius, Damage Radius, Min Scale, Max Scale
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_CREATE_EXPLOSION_NEAR, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_CREATE_EXPLOSION_NEAR;
        nArgs = 6;          // TargetObject ID, Duration, Sound Radius, Damage Radius, Min Scale, Max Scale
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_GIB, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_GIB;
        nArgs = 1;          // TargetObject ID
        nCheckObjID = 1;
	}

    else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_SCALE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_SCALE;
        nArgs = 4;          // TargetObject ID, Scale X, Scale Y, Scale Z
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_MOVE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_MOVE;
        nArgs = 4;          // ObjectID, X, Y, Z
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_TELEPORT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_TELEPORT;
        nArgs = 4;          // ObjectID, X, Y ,Z
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_MOVE_OVERTIME, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_MOVE_OVERTIME;
        nArgs = 6;          // ObjectID, X, Y ,Z, Interval, Duration
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_FLAGS, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_FLAGS;
        nArgs = 2;          // ObjectID, FLAGS STRING
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_FLAG_VISIBLE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_FLAG_VISIBLE;
        nArgs = 2;          // ObjectID, FLAGS STRING
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_FLAG_SOLID, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_FLAG_SOLID;
        nArgs = 2;          // ObjectID, FLAGS STRING
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_FLAG_GRAVITY, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_FLAG_GRAVITY;
        nArgs = 2;          // ObjectID, FLAGS STRING
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_DIMS, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_DIMS;
        nArgs = 4;          // ObjectID, Dim X, Dim Y, Dim Z
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_OBJECT_DESTROY, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_OBJECT_DESTROY;
        nArgs = 1;          // ObjectID 
        nCheckObjID = 1;
	}
    
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_SEND_AI_SCRIPT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_SEND_AI_SCRIPT;
        nArgs = 2;
        nCheckObjID = 1;    // ObjectID, Script to Send
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_SEND_TRIGGER, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_SEND_TRIGGER;
        nArgs = 2;
        nCheckObjID = 1;    // ObjectID, Script to Send
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_SEND_TRIGGER_NAMED, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_SEND_TRIGGER_NAMED;
        nArgs = 2;
        nCheckObjID = 1;    // ObjectID, Script to Send
	}
    
/*	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_FOV, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_FOV;
        nArgs = 2;          // X Fov, Y Fov
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_RECT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_RECT;
        nArgs = 4;          // Rect L,T,R,B
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_RESET, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_RESET;
        nArgs = 0;
	}
*/
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_CREATE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_CREATE;
        nArgs = 2;          // CameraID, Type
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_DESTROY, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_DESTROY;
        nArgs = 1;          // CameraID
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_SELECT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_SELECT;
        nArgs = 1;          // CameraID
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_MOVE_OBJ, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_MOVE_OBJ;
        nArgs = 3;          // CameraID, Time, ObjID
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_TELEPORT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_TELEPORT;
        nArgs = 4;          // CameraID, X, Y, Z
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_TELEPORT_OBJ, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_TELEPORT_OBJ;
        nArgs = 2;          // CameraID, ObjID
        nCheckObjID = 1;
	}
    
	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_ROTATE, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_ROTATE;
        nArgs = 4;          // CameraID, X, Y ,Z
        nCheckObjID = 1;
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_LINKSPOT, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_LINKSPOT;
        nArgs = 2;          // CameraID, ObjectID   (if objectID is 0 then Break Link
        nCheckObjID = 2;    // Could be a problem here...
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_CAMERA_ZOOM, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_CAMERA_ZOOM;
        nArgs = 1;          // 0 Zoom False, 1 Zoom True
	}

	else if (_mbsicmp((const unsigned char*)SCRIPTSCENE_END, (const unsigned char*)pCmdName) == 0)
	{
		nType = SCRIPT_SCMD_END;
        nArgs = 0;
	}
    

	return nType;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadScript
//
//	PURPOSE:	Reads Script Scene
//
// ----------------------------------------------------------------------- //
DBOOL Script::LoadScript(int &nEOF) 
{
	char line[256];

	char seps[] = " ,\t\n";

	char *tokenname;            // Script Command
	char *temparg;

	nEOF = 0;

	// Get a line, check for End of File
	if (fgets(line, 256, infile) == NULL)
	{
		nEOF = 1;
		return DTRUE;
	}

	// GET THE FIRST TOKEN
	/////////////////////////////////////////////////////////////////
	tokenname = NextToken(line, seps);

	// See if its a comment
	if ( !tokenname || tokenname[0] == ';' || _mbsncmp((const unsigned char*)tokenname, (const unsigned char*)"//", 2) == 0) return DTRUE;

	// Check for DEFINES
	if ( m_nDefineCnt < MAX_DEFINES && _mbsncmp((const unsigned char*)tokenname, (const unsigned char*)"define", 6) == 0)
	{
		if ( (temparg = NextToken(NULL, " =\t\n")) )
		{
			// Copy the Define data to structure
			memset(sDefineObject[m_nDefineCnt].m_szDefineName, 0, MAXDATA_LEN);
			_mbsncpy((unsigned char*)sDefineObject[m_nDefineCnt].m_szDefineName, (const unsigned char*)temparg, MAXDATA_LEN);

			if ( (temparg = NextToken(NULL, " =\t\n")) )
			{
				// Copy the Define data to structure
				memset(sDefineObject[m_nDefineCnt].m_szDefineValue, 0, MAXDATA_LEN);
				_mbsncpy((unsigned char*)sDefineObject[m_nDefineCnt].m_szDefineValue, (const unsigned char*)temparg, MAXDATA_LEN);
			}
			else
			{
				// Error
				g_pServerDE->BPrint("ERROR...define missing value");
				return DFALSE;
			}
		}
		else
		{
			// Error
			g_pServerDE->BPrint("ERROR...define missing name");
			return DFALSE;
		}

		// Increate the Array counter    
		m_nDefineCnt++;

		// next line
		return DTRUE;
	}


	// GET THE NEXT TOKEN
	/////////////////////////////////////////////////////////////////
//	tokenname = NextToken(NULL, seps);
	// If no command then skip
//	if (!(tokenname) ) return DTRUE;

	// make sure the command is always uppercase
	_strupr(tokenname);

	SCRIPTCMD* pCmd = new SCRIPTCMD;
	if (!pCmd)
	{
		g_pServerDE->BPrint( "ERROR...Could not create Temp Script" );
		return DFALSE;
	}

	int nArgs;
	int nCheckObjID;

	// Store the command in Temp cmd
	pCmd->command = StringToScriptCutCmdType(tokenname, nArgs, nCheckObjID);

	// Make sure the command is Valid!
	if (pCmd->command == 0)
	{    
		g_pServerDE->BPrint("ERROR...Unknown command %s", tokenname);
		return DFALSE;
	}                 

	// Store the number of args in data
	sprintf(pCmd->data, "%d", nArgs);

	// set the start time
//	pCmd->fStartTime = m_fScriptTime;
	pCmd->m_bExecuted = DFALSE;

	m_scriptCmdList.Add(pCmd);

	if (nArgs > 0)
	{
		for (int i=0; i < nArgs; i++)
		{
			// Get Next Arg (based on the number of args for this command)
			/////////////////////////////////////////////////////////////////
			if ( (temparg = NextToken(NULL, " ,\t\n")) )
			{
				// Add Arg Script
				if (AddArgScript(temparg) == DFALSE)   return DFALSE;


				// NEED TO GET THE DEFINED ARG OBJECT TO CHECK FOR THIS...
				//            
				//                // Check the ObjectID
				//                if ((nCheckObjID > 0) && (i+1 == nCheckObjID) )
				//                {
				//                    if (atoi(temparg) > MAXSCRIPTOBJ)
				//                    {
				//                        g_pServerDE->BPrint( "ERROR...ObjectID Greater than MAX" );
				//                  	return DFALSE;
				//                    }
				//                }

				//
				// Do some pre processing checking!
				//
				if ( ((pCmd->command == SCRIPT_SCMD_OBJECT_CREATE_NEAR)||(pCmd->command == SCRIPT_SCMD_OBJECT_CREATE_AT)) && (i==1)  )
				{
					// Check to make sure the class is Valid!
					HCLASS pClass = g_pServerDE->GetClass(temparg);
					if (!pClass)
					{    
						g_pServerDE->BPrint("ERROR...Invalid ObjectCLASS: %s",temparg);
						return DFALSE;
					}                 
				}
			}
			else
			{
				g_pServerDE->BPrint( "ERROR...Invalid Args for script command %s", tokenname );
				return DFALSE;
			}
		}
	}    

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddArgScript
//
//	PURPOSE:	Add a Arg to the Script list
//
// ----------------------------------------------------------------------- //
DBOOL Script::AddArgScript(char *temparg)
{
    char szCopyArg[256];
    
    memset(szCopyArg,0,256);
    _mbsncpy((unsigned char*)szCopyArg,(const unsigned char*)temparg,MAXDATA_LEN);

    // Do the Define replacements
    // Only check the args that have a (
    if (m_nDefineCnt > 0 && szCopyArg[0] == '#')
    {
		char *pToken = &szCopyArg[1];
        for (int i=0; i < m_nDefineCnt; i++)
        {
            if ( _mbsncmp((const unsigned char*)pToken, (const unsigned char*)sDefineObject[i].m_szDefineName, _mbstrlen(sDefineObject[i].m_szDefineName) ) == 0)
            {
                memset(szCopyArg,0,256);
                _mbsncpy((unsigned char*)szCopyArg, (const unsigned char*)sDefineObject[i].m_szDefineValue, MAXDATA_LEN);
            
                break;
            }
        }
        
    }        

    //
    // Create a Temp Arg Scriptbuffer
    //
    SCRIPTCMD* pArg = new SCRIPTCMD;
    if (!pArg)
    {
        g_pServerDE->BPrint( "ERROR...Could not create Temp Script" );
        return DFALSE;
    }

    // Store the Arg command
    pArg->command = SCRIPT_SCMD_ARG;

    char* pData = szCopyArg;
    if (pData) 
    {
        _mbsncpy((unsigned char*)pArg->data, (const unsigned char*)pData, MAXDATA_LEN);
	    m_scriptCmdList.Add(pArg);
    }
    else
    {
        g_pServerDE->BPrint( "ERROR...Arg not found" );
        return DFALSE;
    }

    return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Start
//
//	PURPOSE:	Init the script
//
// ----------------------------------------------------------------------- //
DBOOL Script::StartScript()
{
	if (!g_pServerDE) return DFALSE;

// If Script is already running, return

    if (m_bScriptStarted == DFALSE)
    {
        m_bScriptStarted = DTRUE;
    
    }

// Init the Execute commands to DFALSE

    return DTRUE;
}    

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Start
//
//	PURPOSE:	Init the script
//
// ----------------------------------------------------------------------- //
DBOOL Script::SetCutScene()
{
	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "SetCutScene" );

    int nCutMode;
    
    // Arg0 = On or Off (1 or 0)
    m_nCurrLine++;
    nCutMode = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    if (nCutMode == 1)
    {    
//        g_pServerDE->BPrint( "CutSceneStart" );

	    HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CUTSCENE_START);
    	g_pServerDE->EndMessage(hMsg);
    }
    else
    {
  //      g_pServerDE->BPrint( "CutSceneEnd" );

	    HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CUTSCENE_END);
    	g_pServerDE->EndMessage(hMsg);

		// Deactivate all cameras
		for (int i=0; i<MAXSCRIPTOBJ; i++)
		{
			if (ScriptCamera[i].m_hObject)
			{
    			CameraObj* pCamera = (CameraObj*)g_pServerDE->HandleToObject(ScriptCamera[i].m_hObject);
				pCamera->SetActive(DFALSE);
			}
		}
    
    }
    return DTRUE;
}
    
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	End
//
//	PURPOSE:	End the script
//
// ----------------------------------------------------------------------- //
DBOOL Script::EndScript()
{
	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint("Script DONE!");
    g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
    //return DFALSE;
    
    return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Display Title
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::SubTitleDisplay()
{
	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "DisplayTitle" );

    char szTitle[128];
    
	// Arg0 = Length of time to display subtitle
    // Arg1 = Text to display

    m_nCurrLine++;
    DFLOAT fTime = (DFLOAT)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    m_nCurrLine++;
    _mbscpy((unsigned char*)szTitle, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_DISPLAYTEXT);
	g_pServerDE->WriteToMessageFloat(hMsg, fTime);
	g_pServerDE->WriteToMessageString(hMsg, szTitle);
	g_pServerDE->EndMessage(hMsg);

    return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fade
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::SubTitleFade()
{
	if (!g_pServerDE) return DFALSE;
    
//    g_pServerDE->BPrint( "FadeSubTitle" );
    
    int nFadeMode;
    
    // Arg0 = Class
    m_nCurrLine++;
    nFadeMode = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_SCREENFADE);
	g_pServerDE->WriteToMessageByte(hMsg, (DBYTE)nFadeMode);
	g_pServerDE->EndMessage(hMsg);

    
    return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObjectAt
//
//	PURPOSE:	Creates a object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateAt()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = Object number
    // Arg1 = Class
    // Arg2 = X
    // Arg3 = Y
    // Arg4 = Z
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Object Number
    m_nCurrLine++;
    sprintf(pName,"ScriptObj%d", atoi(m_scriptCmdList[m_nCurrLine]->data) );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);

    // Arg1 = Class
    m_nCurrLine++;
    _mbscpy((unsigned char*)pClassName, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
	DVector vPos;
    // Arg2 = X Position
    m_nCurrLine++;
    vPos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg3 = Y
    m_nCurrLine++;
    vPos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg4 = Z
    m_nCurrLine++;
    vPos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	VEC_COPY(theStruct.m_Pos, vPos);
    
    // Store the names into the Object Array    
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szClassName, (const unsigned char*)pClassName, MAXDATA_LEN);
    
	DRotation rRot;
    ROT_INIT(rRot);
    ROT_COPY(theStruct.m_Rotation, rRot);
    
	theStruct.m_NextUpdate = 0.1f;
    
	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
    // Check to make sure this is a valid class.
    if (pClass)
    {    
    	LPBASECLASS pObject = g_pServerDE->CreateObject(pClass, &theStruct);
        
        if (pObject)
        {
            ScriptObject[m_nObjectIndex].m_hObject = g_pServerDE->ObjectToHandle(pObject);
			// Link to the target so we are notified if it's destroyed
			g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
            
            return DTRUE;
        }
    }        
    
    g_pServerDE->BPrint("ObjectCreateAt");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObjectNear
//
//	PURPOSE:	Creates a object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateNear()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Object ID, Class to create, NearObjectID
    
    // Arg0 = Object number
    // Arg1 = Class
    // Arg2 = NearObjectID
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Object Number
    m_nCurrLine++;
    sprintf(pName,"ScriptObj%d", atoi(m_scriptCmdList[m_nCurrLine]->data) );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);

    // Arg1 = Class
    m_nCurrLine++;
    _mbscpy((unsigned char*)pClassName, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg2 = NearObjectID
    m_nCurrLine++;
    int nNearObject = atoi(m_scriptCmdList[m_nCurrLine]->data);
    HOBJECT hNearObject = ScriptObject[nNearObject].m_hObject;
    
    
    // Store the names into the Object Array    
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szClassName, (const unsigned char*)pClassName, MAXDATA_LEN);
    
	DRotation rRot;
    ROT_INIT(rRot);
    ROT_COPY(theStruct.m_Rotation, rRot);

	DVector vPos;
    g_pServerDE->GetObjectPos(m_hObject, &vPos);

    if (hNearObject)
    {
        DVector vNearPos;
        g_pServerDE->GetObjectPos(hNearObject, &vNearPos);
        
        vPos.x = vNearPos.x;
        vPos.y = vNearPos.y;
        vPos.z = vNearPos.z;
    }    

	VEC_COPY(theStruct.m_Pos, vPos);
    
	theStruct.m_NextUpdate = 0.1f;
    
	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
    // Check to make sure this is a valid class.
    if (pClass)
    {    
    	LPBASECLASS pObject = g_pServerDE->CreateObject(pClass, &theStruct);
        
        if (pObject)
        {
            ScriptObject[m_nObjectIndex].m_hObject = g_pServerDE->ObjectToHandle(pObject);
			// Link to the target so we are notified if it's destroyed
			g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
            
            return DTRUE;
        }
    }        
    
    g_pServerDE->BPrint("ObjectCreateNear");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectAssign
//
//	PURPOSE:	Assign a Object ID to a Object already in the world
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectAssign()
{
    char pName[MAXDATA_LEN];
    DBOOL bFound = DFALSE;
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = Object number
    // Arg1 = Target Name (search for name in the world)


    // Arg0 = Object number
    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nCurrLine++;
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg1 = Target Name (search for name in the world)
    m_nCurrLine++;
    _mbsncpy((unsigned char*)pName, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data, MAXDATA_LEN);
    
	if (pName && pName[0] != '\0')
    {
    	ObjectList*	pList = g_pServerDE->FindNamedObjects(pName);
    	if (pList)
        {
	        ObjectLink* pLink = pList->m_pFirstLink;
        	while(pLink)
	        {
		        if (pLink)
    		    {
                    ScriptObject[m_nObjectIndex].m_hObject = pLink->m_hObject;
                    
                    // Set the Object Name
                    memset(ScriptObject[m_nObjectIndex].m_szObjectName, 0, MAXDATA_LEN);
                    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
                    
                    // Dont know the Base Class!
                    
        			// Link to the target so we are notified if it's destroyed
		        	g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
                    bFound = DTRUE;
	    	    }
            
	    	    pLink = pLink->m_pNext;
        	}
	    
	        g_pServerDE->RelinquishList(pList);
        }
        
        if (bFound) return DTRUE;
    }
        
    g_pServerDE->BPrint("Error: ObjectCreateAssign");
    g_pServerDE->BPrint(pName);
    return DFALSE;
} 



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObjectModelAt
//
//	PURPOSE:	Creates a object Model
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateModelAt()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = Object ID
    // Arg1 = Model Name
    // Arg2 = Skin Name
    // Arg3 = X Position
    // Arg4 = Y
    // Arg5 = Z
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Object Number
    m_nCurrLine++;
    sprintf(pName,"ScriptObj%d", atoi(m_scriptCmdList[m_nCurrLine]->data) );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
//    g_pServerDE->BPrint(pName);

    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);

	theStruct.m_ObjectType = OT_MODEL; 
    _mbscpy((unsigned char*)pClassName, (const unsigned char*)"BaseClass");

    // Arg1 = ModelFileName
    m_nCurrLine++;
	_mbscpy((unsigned char*)theStruct.m_Filename, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
//    g_pServerDE->BPrint(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg2 = ModelSkinName
    m_nCurrLine++;
	_mbscpy((unsigned char*)theStruct.m_SkinName, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
//    g_pServerDE->BPrint(m_scriptCmdList[m_nCurrLine]->data);
    
    
	DVector vPos;
    // Arg3 = X Position
    m_nCurrLine++;
    vPos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg4 = Y
    m_nCurrLine++;
    vPos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg5 = Z
    m_nCurrLine++;
    vPos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	VEC_COPY(theStruct.m_Pos, vPos);

    // Store the names into the Object Array    
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
    _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szClassName, (const unsigned char*)pClassName, MAXDATA_LEN);
    
	DRotation rRot;
    ROT_INIT(rRot);
    ROT_COPY(theStruct.m_Rotation, rRot);
    
	theStruct.m_NextUpdate = 0.1f;
    
	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
    // Check to make sure this is a valid class.
    if (pClass)
    {    
    	LPBASECLASS pObject = g_pServerDE->CreateObject(pClass, &theStruct);
        
        if (pObject)
        {
            ScriptObject[m_nObjectIndex].m_hObject = g_pServerDE->ObjectToHandle(pObject);

            if (ScriptObject[m_nObjectIndex].m_hObject)
            {
                // Link to the target so we are notified if it's destroyed
			    g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
            
                // Set the Dims
                DVector vDims;
                g_pServerDE->GetModelAnimUserDims(ScriptObject[m_nObjectIndex].m_hObject, &vDims, g_pServerDE->GetModelAnimation(ScriptObject[m_nObjectIndex].m_hObject));
            
        	    g_pServerDE->SetObjectDims(ScriptObject[m_nObjectIndex].m_hObject, &vDims);
            
                return DTRUE;
            }                
        }
    }        
    
    g_pServerDE->BPrint("ObjectCreateModel");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObjectModelAt
//
//	PURPOSE:	Creates a object Model
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateModelNear()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = Object ID
    // Arg1 = NearObject ID
    // Arg2 = Model Name
    // Arg3 = Skin Name
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Object Number
    m_nCurrLine++;
    sprintf(pName,"ScriptObj%d", atoi(m_scriptCmdList[m_nCurrLine]->data) );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);

    // Arg1 = NearObject ID
    m_nCurrLine++;
    int nTargetID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    HOBJECT hLinkObject = ScriptObject[nTargetID].m_hObject;
    
    if (hLinkObject)
    {
        // Random explosion position (from the FireFX code)
            DRotation rRot;
            DVector m_vUp, m_vRight, m_vForward;
            DVector vPos, vTemp, vDir, vDims;

            VEC_INIT(m_vUp);
            VEC_INIT(m_vRight);
            VEC_INIT(m_vForward);

            g_pServerDE->GetObjectRotation(hLinkObject, &rRot);
            g_pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);    

            g_pServerDE->GetObjectDims(hLinkObject, &vDims);
            g_pServerDE->GetObjectPos(hLinkObject, &vPos);
            
            // Need to use the Dims of the Object to set these!!!
//			VEC_MULSCALAR(vDims, vDims, 0.8f);
            DFLOAT m_fForwardOffset = g_pServerDE->Random(-vDims.x, vDims.x);
            DFLOAT m_fUpOffset = g_pServerDE->Random(-vDims.y, vDims.y/2.0f);
            DFLOAT m_fRightOffset = g_pServerDE->Random(-vDims.z, vDims.z);
    
    	    VEC_MULSCALAR(vTemp, m_vForward, m_fForwardOffset);
        	VEC_ADD(vPos, vPos, vTemp);
    
            // vPos is a point in front of you.
            // vDir is a point in front of you, but down
        	VEC_COPY(vDir, vPos);
            vDir.y = vDir.y - m_fUpOffset;
            vDir.x = vDir.x - m_fRightOffset;
			vDir.z = vDir.z - m_fForwardOffset;
            
            VEC_COPY(vPos, vDir);
    
        	VEC_COPY(theStruct.m_Pos, vPos);
    
        theStruct.m_NextUpdate = 0.01f;

    	theStruct.m_ObjectType = OT_MODEL; 
        _mbscpy((unsigned char*)pClassName, (const unsigned char*)"BaseClass");

        // Arg2 = ModelFileName
        m_nCurrLine++;
    	_mbscpy((unsigned char*)theStruct.m_Filename, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
//    g_pServerDE->BPrint(m_scriptCmdList[m_nCurrLine]->data);
    
        // Arg3 = ModelSkinName
        m_nCurrLine++;
    	_mbscpy((unsigned char*)theStruct.m_SkinName, (const unsigned char*)m_scriptCmdList[m_nCurrLine]->data);
    
//    g_pServerDE->BPrint(m_scriptCmdList[m_nCurrLine]->data);
   
        // Store the names into the Object Array    
        _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
        _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szClassName, (const unsigned char*)pClassName, MAXDATA_LEN);
    
//    	DRotation rRot;
        ROT_INIT(rRot);
        ROT_COPY(theStruct.m_Rotation, rRot);
    
	    theStruct.m_NextUpdate = 0.1f;
    
    	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
        // Check to make sure this is a valid class.
        if (pClass)
        {    
        	LPBASECLASS pObject = g_pServerDE->CreateObject(pClass, &theStruct);
        
            if (pObject)
            {
                ScriptObject[m_nObjectIndex].m_hObject = g_pServerDE->ObjectToHandle(pObject);

                if (ScriptObject[m_nObjectIndex].m_hObject)
                {
                    // Link to the target so we are notified if it's destroyed
    			    g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
            
                    // Set the Dims
                    DVector vDims;
                    g_pServerDE->GetModelAnimUserDims(ScriptObject[m_nObjectIndex].m_hObject, &vDims, g_pServerDE->GetModelAnimation(ScriptObject[m_nObjectIndex].m_hObject));
            
            	    g_pServerDE->SetObjectDims(ScriptObject[m_nObjectIndex].m_hObject, &vDims);
            
                    return DTRUE;
                }                
            }
        }        
    }
    
    g_pServerDE->BPrint("ObjectCreateModel");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObjectFire
//
//	PURPOSE:	Creates a Fire object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateFire()
{
    char    pName[MAXDATA_LEN];
    char    pClassName[MAXDATA_LEN];
    int     nTargetID;
    char    pTargetName[MAXDATA_LEN];
    HOBJECT hTargetObject = DNULL;
    DFLOAT  fBurnTime = 0.0f;
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = Object ID
    // Arg1 = Target ID (object to attach to)
    // Arg2 = BurnTime
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Object ID
    m_nCurrLine++;
    sprintf(pName,"ScriptObj%d", atoi(m_scriptCmdList[m_nCurrLine]->data) );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nObjectIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg1 = TargetID
    // TargetID
    m_nCurrLine++;
    nTargetID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    _mbscpy((unsigned char*)pTargetName, (const unsigned char*)ScriptObject[nTargetID].m_szObjectName);
    
    // Arg2 = BurnTime
    m_nCurrLine++;
    fBurnTime = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
	if (pTargetName && pTargetName[0] != '\0')
    {
    	ObjectList*	pList = g_pServerDE->FindNamedObjects(pTargetName);
    	if (pList)
        {
	        ObjectLink* pLink = pList->m_pFirstLink;
        	while(pLink)
	        {
		        if (pLink)
    		    {
                    hTargetObject = pLink->m_hObject;
	    	    }
            
	    	    pLink = pLink->m_pNext;
        	}
	    
	        g_pServerDE->RelinquishList(pList);
        }
        
    }

    _mbscpy((unsigned char*)pClassName, (const unsigned char*)"FireFX");
    
    // Found object, now add Fire
    if (hTargetObject)
    {
    	theStruct.m_ObjectType = OT_NORMAL; 

        // Store the names into the Object Array    
        _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
        _mbsncpy((unsigned char*)ScriptObject[m_nObjectIndex].m_szClassName, (const unsigned char*)pClassName, MAXDATA_LEN);
    
    	DRotation rRot;
        ROT_INIT(rRot);

    	DVector vPos;
        g_pServerDE->GetObjectPos(m_hObject, &vPos);

        ROT_COPY(theStruct.m_Rotation, rRot);
	    VEC_COPY(theStruct.m_Pos, vPos);
    
    	theStruct.m_NextUpdate = 0.01f;
    
	    HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
        // Check to make sure this is a valid class.
        if (pClass)
        {    
        	FireFX* pObject = (FireFX*)g_pServerDE->CreateObject(pClass, &theStruct);
            
            if (pObject)
            {
                ScriptObject[m_nObjectIndex].m_hObject = g_pServerDE->ObjectToHandle(pObject);
                
    			// Link to the target so we are notified if it's destroyed
	    		g_pServerDE->CreateInterObjectLink(m_hObject, ScriptObject[m_nObjectIndex].m_hObject);
            
                pObject->Setup(hTargetObject, fBurnTime);
                return DTRUE;
            }
        }
    }
    
    g_pServerDE->BPrint("ObjectCreateFire");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObject Exposion
//
//	PURPOSE:	Creates a Exposion object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateExplosionAt()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    DFLOAT fDuration = 0.0f;
    DFLOAT fSoundRadius = 1000.0f;
    DFLOAT fDamageRadius = 1000.0f;
    DFLOAT fScaleMin = 0.1f;
	DFLOAT fScaleMax = 1.0f;
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = X
    // Arg1 = Y
    // Arg2 = Z
    // Arg3 = Duration
    // Arg4 = Sound Radius
    // Arg5 = Damage Radius
    // Arg6 = Min Scale
    // Arg7 = Max Scale
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    sprintf(pName,"ScriptObjExp");
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    _mbscpy((unsigned char*)pClassName, (const unsigned char*)"Explosion");

    DRotation rRot;
    ROT_INIT(rRot);
    ROT_COPY(theStruct.m_Rotation, rRot);

	DVector vPos;
    // Arg0 = X Position
    m_nCurrLine++;
    vPos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg1 = Y
    m_nCurrLine++;
    vPos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg2 = Z
    m_nCurrLine++;
    vPos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg3 = Duration
    m_nCurrLine++;
    fDuration = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg4 = Sound Radius
    m_nCurrLine++;
    fSoundRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg5 = Damage Radius
    m_nCurrLine++;
    fDamageRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg6 = Min Scale
    m_nCurrLine++;
    fScaleMin = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg7 = Max Scale
    m_nCurrLine++;
    fScaleMax = (float)atof(m_scriptCmdList[m_nCurrLine]->data);


	VEC_COPY(theStruct.m_Pos, vPos);
    
    theStruct.m_NextUpdate = 0.01f;
    
	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
    // Check to make sure this is a valid class.
    if (pClass)
    {    
    	Explosion* pExplosion = (Explosion *)g_pServerDE->CreateObject(pClass, &theStruct);
        
        if (pExplosion)
        {
//            HOBJECT hObject = g_pServerDE->ObjectToHandle(pExplosion);
        
            DFLOAT fDamage = 50.0f;
            DBOOL bCreateMark = DTRUE;
            DBOOL bAddSparks  = DFALSE;
            DBOOL bCreateSmoke = DTRUE;
            
            DVector vLightColor;
	        VEC_SET(vLightColor, 1, 1, 1);
            
    		pExplosion->Setup("Sounds\\exp_tnt.wav", fSoundRadius, fDuration,
						  NULL, fDamageRadius, fDamage, fScaleMin,
						  fScaleMax, bCreateMark, bAddSparks, bCreateSmoke);


		    pExplosion->SetupLight(DTRUE, vLightColor, fScaleMin*10, fScaleMax*10);

            // Boom!
        	pExplosion->Explode();

            return DTRUE;
        }
    }
    
    g_pServerDE->BPrint("ObjectCreateExposionAt");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 
                    


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateObject Exposion Near a Object
//
//	PURPOSE:	Creates a Exposion object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectCreateExplosionNear()
{
    char pName[MAXDATA_LEN];
    char pClassName[MAXDATA_LEN];
    DFLOAT fDuration = 0.0f;
    DFLOAT fSoundRadius = 1000.0f;
    DFLOAT fDamageRadius = 1000.0f;
    DFLOAT fScaleMin = 0.1f;
    DFLOAT fScaleMax = 1.0f;
    int nTargetID;
    
	if (!g_pServerDE || !m_hObject) return DFALSE;

    // Arg0 = TargetID
    // Arg1 = Duration
    // Arg2 = Sound Radius
    // Arg3 = Damage Radius
    // Arg4 = Min Scale
    // Arg5 = Max Scale
    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    sprintf(pName,"ScriptObjExp");
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);
    
    // Arg0 = Object ID
    m_nCurrLine++;
    nTargetID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    _mbscpy((unsigned char*)pClassName, (const unsigned char*)"Explosion");

//    DRotation rRot;
//    ROT_INIT(rRot);
//    ROT_COPY(theStruct.m_Rotation, rRot);

    HOBJECT hLinkObject = ScriptObject[nTargetID].m_hObject;
    
    if (hLinkObject)
    {
        // Random explosion position (from the FireFX code)
            DRotation rRot;
            DVector m_vUp, m_vRight, m_vForward;
            DVector vPos, vTemp, vDir, vDims;

            VEC_INIT(m_vUp);
            VEC_INIT(m_vRight);
            VEC_INIT(m_vForward);

            g_pServerDE->GetObjectRotation(hLinkObject, &rRot);
            g_pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);    

            g_pServerDE->GetObjectDims(hLinkObject, &vDims);
            g_pServerDE->GetObjectPos(hLinkObject, &vPos);
            
            // Need to use the Dims of the Object to set these!!!
//			VEC_MULSCALAR(vDims, vDims, 0.8f);
            DFLOAT m_fForwardOffset = g_pServerDE->Random(-vDims.x, vDims.x);
            DFLOAT m_fUpOffset = g_pServerDE->Random(-vDims.y, vDims.y/2.0f);
            DFLOAT m_fRightOffset = g_pServerDE->Random(-vDims.z, vDims.z);
    
    	    VEC_MULSCALAR(vTemp, m_vForward, m_fForwardOffset);
        	VEC_ADD(vPos, vPos, vTemp);
    
            // vPos is a point in front of you.
            // vDir is a point in front of you, but down
        	VEC_COPY(vDir, vPos);
            vDir.y = vDir.y - m_fUpOffset;
            vDir.x = vDir.x - m_fRightOffset;
			vDir.z = vDir.z - m_fForwardOffset;
            
            VEC_COPY(vPos, vDir);
    

        // Arg1 = Duration
        m_nCurrLine++;
        fDuration = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

        // Arg2 = Sound Radius
        m_nCurrLine++;
        fSoundRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

        // Arg3 = Damage Radius
        m_nCurrLine++;
        fDamageRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
        
        // Arg4 = Min Scale
        m_nCurrLine++;
        fScaleMin = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

        // Arg5 = Max Scale
        m_nCurrLine++;
        fScaleMax = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    	VEC_COPY(theStruct.m_Pos, vPos);
    
        theStruct.m_NextUpdate = 0.01f;
    
    	HCLASS pClass = g_pServerDE->GetClass(pClassName);
    
        // Check to make sure this is a valid class.
        if (pClass)
        {    
    	    Explosion* pExplosion = (Explosion *)g_pServerDE->CreateObject(pClass, &theStruct);
        
            if (pExplosion)
            {
//            HOBJECT hObject = g_pServerDE->ObjectToHandle(pExplosion);
        
                DFLOAT fDamage = 50.0f;
                DBOOL bCreateMark = DTRUE;
                DBOOL bAddSparks  = DFALSE;
                DBOOL bCreateSmoke = DTRUE;
            
                DVector vLightColor;
	            VEC_SET(vLightColor, 1, 1, 1);
            
    		    pExplosion->Setup("Sounds\\exp_tnt.wav", fSoundRadius, fDuration,
						  NULL, fDamageRadius, fDamage, fScaleMin,
						  fScaleMax, bCreateMark, bAddSparks, bCreateSmoke);


    		    pExplosion->SetupLight(DTRUE, vLightColor, fScaleMin*10, fScaleMax*10);

                // Boom!
            	pExplosion->Explode();

                return DTRUE;
            }
        }
    }
    
    g_pServerDE->BPrint("ObjectCreateExposionNear");
    g_pServerDE->BPrint("Error creating");
    g_pServerDE->BPrint(pClassName);
    return DFALSE;
} 
 


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Move the Object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectMove()
{
	DVector Pos;
    int nObjectID;
    
	if (!g_pServerDE) return DFALSE;
    
//    g_pServerDE->BPrint( "ObjectMove" );
    // Arg0 = object number (m_nObjectIndex)
    // Arg1 = x
    // Arg2 = y
    // Arg3 = z
    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    m_nCurrLine++;
    Pos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    Pos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    Pos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    // If any of the points are zero, then we are going to Offset from current Point
    
    if (ScriptObject[nObjectID].m_hObject)
    {
//        if (Pos.x == 0 || Pos.y == 0 || Pos.z == 0)
//        {

        // Always Move from current position
            DVector vCurPos;
            g_pServerDE->GetObjectPos(ScriptObject[nObjectID].m_hObject, &vCurPos);
            
    	    VEC_ADD(Pos, Pos, vCurPos);
//        }    
        
//    	g_pServerDE->SetVelocity(ScriptObject[nObjectID].m_hObject, &Pos);
    	g_pServerDE->MoveObject(ScriptObject[nObjectID].m_hObject, &Pos);
        return DTRUE;
    }            
    
    g_pServerDE->BPrint( "ObjectMove" );
    g_pServerDE->BPrint( "Error Moving" );
    return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Teleport the Object
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectTeleport()
{
	DVector vPos;
    int nObjectID;

	if (!g_pServerDE) return DFALSE;
    
    // Arg0 = object number (m_nObjectIndex)
    // Arg1 = x
    // Arg2 = y
    // Arg3 = z

    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    vPos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    vPos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    vPos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    if (ScriptObject[nObjectID].m_hObject)
    {
        // If any of the points are zero, then we are going to Offset from current Point
//        if (vPos.x == 0 || vPos.y == 0 || vPos.z == 0)
//        {
// Always Move from current position
            DVector vCurPos;
            g_pServerDE->GetObjectPos(ScriptObject[nObjectID].m_hObject, &vCurPos);
        
        	VEC_ADD(vPos, vPos, vCurPos);
//        }    
        
	    g_pServerDE->TeleportObject(ScriptObject[nObjectID].m_hObject, &vPos);
        return DTRUE;
    }            
    
    g_pServerDE->BPrint( "ObjectTeleport" );
    g_pServerDE->BPrint( "Error Teleporting");
    return DFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Move the Object Over Time (Setup)
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectMoveObj()
{
	if (!g_pServerDE) return DFALSE;

	// Arg0 = camera ID (m_nObjectIndex)
	// Arg1 = Time interval to destination
	// Arg2 = object ID
	m_nCurrLine++;
	int nObjectId = atoi(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	DFLOAT fDuration = (DFLOAT)atof(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	int nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptObject[nObjectId].m_hObject && ScriptObject[nObjectID].m_hObject)
	{
		int MoveObject = -1;
        
		for (int i=0; i<MAX_MOVEOBJS; i++)
		{
			if (m_nMoveObject[i] == -1)
			{
				MoveObject = i;
				break;
			} 
		}
    
		if (MoveObject != -1)
		{
			ObjectData *pObj = &ScriptObject[nObjectId];

			g_pServerDE->GetObjectPos(pObj->m_hObject, &pObj->m_vInitPos);
			g_pServerDE->GetObjectRotation(pObj->m_hObject, &pObj->m_rInitRot);

			g_pServerDE->GetObjectPos(ScriptObject[nObjectID].m_hObject, &pObj->m_vDestPos);
			g_pServerDE->GetObjectRotation(ScriptObject[nObjectID].m_hObject, &pObj->m_rDestRot);

			pObj->m_fStartTime = pObj->m_fCurrentTime = g_pServerDE->GetTime();
			pObj->m_fDuration = fDuration;
			pObj->m_bMovement = DTRUE;

			m_nMoveObject[MoveObject] = nObjectId;
	        m_bCheckOverTime = DTRUE;
		}
		
		return DTRUE;
	}            

	g_pServerDE->BPrint("ObjectMoveObj: Error");
	return DFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Move the Object Over Time (Update)
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectMoveOverTimeUpdate(ObjectData *pObj)
{
	if (!g_pServerDE) return DFALSE;

	float t = (pObj->m_fCurrentTime - pObj->m_fStartTime) / (pObj->m_fDuration);
	
	// get the new position from position 1's position

	DVector pos1;
	VEC_COPY(pos1, pObj->m_vInitPos);

	DVector pos2;
	VEC_COPY(pos2, pObj->m_vDestPos);
	
	DVector posNew;
	VEC_LERP(posNew, pos1, pos2, t);

	// get the new angle from position 1's angle

	DRotation rot1;
	ROT_COPY(rot1, pObj->m_rInitRot);
	
	DRotation rot2;
	ROT_COPY(rot2, pObj->m_rDestRot);

	DRotation rotNew;
	ROT_INIT(rotNew);
	g_pServerDE->InterpolateRotation(&rotNew, &rot1, &rot2, t);
	
	// If object is a camera, and it has a link, don't rotate it.
	DBOOL bRotate = DTRUE;
	if(g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(pObj->m_hObject), g_pServerDE->GetClass("CameraObj"))) 
	{
		CameraObj *pCamera = (CameraObj*)g_pServerDE->HandleToObject(pObj->m_hObject);
		if (pCamera->GetLinkObject())
		{
			bRotate = DFALSE;
		}
	}
	
	g_pServerDE->MoveObject(pObj->m_hObject, &posNew);
	if (bRotate)
		g_pServerDE->RotateObject(pObj->m_hObject, &rotNew);


    return DFALSE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectFlags()
{
	if (!g_pServerDE) return DFALSE;

	int nObjectID;
	//    DFLOAT fFlags;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = Flags
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	unsigned long lFlags = atol(m_scriptCmdList[m_nCurrLine]->data);

	// Force Visible ONLY
	if (ScriptObject[nObjectID].m_hObject)
	{
		g_pServerDE->SetObjectFlags(ScriptObject[nObjectID].m_hObject, FLAG_VISIBLE | FLAG_GOTHRUWORLD);
		//    	g_pServerDE->SetObjectFlags(ScriptObject[nObjectID].m_hObject, lFlags);
		return DTRUE;
	}
	g_pServerDE->BPrint( "ObjectFlags" );
	g_pServerDE->BPrint( "Error Setting Flags");
	return DFALSE;        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE: Script::ObjectFlagVisible
//
//	PURPOSE: Sets the visible flag for an object
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectFlagVisible()
{
	if (!g_pServerDE) return DFALSE;

	int nObjectID;
	//    DFLOAT fFlags;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = 1 = on, 0 = off
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	HOBJECT hObject = ScriptObject[nObjectID].m_hObject;

	m_nCurrLine++;
	int nOn = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (hObject)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectFlags(hObject);
		if (nOn)
			dwFlags |= FLAG_VISIBLE;
		else
			dwFlags &= ~FLAG_VISIBLE;

		g_pServerDE->SetObjectFlags(hObject, dwFlags);
		return DTRUE;
	}
	g_pServerDE->BPrint( "ObjectFlagVisible" );
	g_pServerDE->BPrint( "Error Setting Flags");
	return DFALSE;        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE: Script::ObjectFlagSolid
//
//	PURPOSE: Sets the solid flag for an object
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectFlagSolid()
{
	if (!g_pServerDE) return DFALSE;

	int nObjectID;
	//    DFLOAT fFlags;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = 1 = on, 0 = off
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	HOBJECT hObject = ScriptObject[nObjectID].m_hObject;

	m_nCurrLine++;
	int nOn = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (hObject)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectFlags(hObject);
		if (nOn)
			dwFlags |= FLAG_SOLID;
		else
			dwFlags &= ~FLAG_SOLID;

		g_pServerDE->SetObjectFlags(hObject, dwFlags);
		return DTRUE;
	}
	g_pServerDE->BPrint( "ObjectFlagSolid" );
	g_pServerDE->BPrint( "Error Setting Flags");
	return DFALSE;        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE: Script::ObjectFlagGravity
//
//	PURPOSE: Sets the gravity flag for an object
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectFlagGravity()
{
	if (!g_pServerDE) return DFALSE;

	int nObjectID;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = 1 = on, 0 = off
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	HOBJECT hObject = ScriptObject[nObjectID].m_hObject;

	m_nCurrLine++;
	int nOn = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (hObject)
	{
		DDWORD dwFlags = g_pServerDE->GetObjectFlags(hObject);
		if (nOn)
			dwFlags |= FLAG_GRAVITY;
		else
			dwFlags &= ~FLAG_GRAVITY;

		g_pServerDE->SetObjectFlags(hObject, dwFlags);
		return DTRUE;
	}
	g_pServerDE->BPrint( "ObjectFlagGravity" );
	g_pServerDE->BPrint( "Error Setting Flags" );
	return DFALSE;        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DBOOL Script::ObjectDims()
{
	if (!g_pServerDE) return DFALSE;
    
    int nObjectID;
	DVector vDims;
	VEC_SET(vDims, 10, 10, 10);
    
    
    // Arg0 = Object ID (m_nObjectIndex)
    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    // Arg1 = Dim X
    m_nCurrLine++;
    vDims.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg2 = Dim Y
    m_nCurrLine++;
    vDims.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

    // Arg3 = Dim Z
    m_nCurrLine++;
    vDims.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    
    if (ScriptObject[nObjectID].m_hObject)
    {
    	g_pServerDE->SetObjectDims(ScriptObject[nObjectID].m_hObject, &vDims);
        return DTRUE;
    }
    
    g_pServerDE->BPrint("ObjectDims");
    g_pServerDE->BPrint("Error Setting Dims");
    return DFALSE;        
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectGib()
{
	if (!g_pServerDE) return DFALSE;
    
    int nObjectID;
    
    // Arg0 = TargetObject ID
    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    if (ScriptObject[nObjectID].m_hObject)
    {
        DDWORD nGibFlag = GIB_HUMAN | GIB_LARGE;
//        DDWORD nGibFlag = GIB_HUMAN | GIB_CORPSE;
        
        AIShared.CreateGibs(ScriptObject[nObjectID].m_hObject, nGibFlag);
                
  		g_pServerDE->RemoveObject( ScriptObject[nObjectID].m_hObject );
//  		ScriptObject[nObjectID].m_hObject = DNULL;
        
        return DTRUE;
    }
    
    g_pServerDE->BPrint("ObjectGib: Error");
    return DFALSE;        
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectDestroy()
{
	if (!g_pServerDE) return DFALSE;
    
    int nObjectID;
    
    // Arg0 = TargetObject ID
    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    if (ScriptObject[nObjectID].m_hObject)
    {
  		g_pServerDE->RemoveObject( ScriptObject[nObjectID].m_hObject );
//  		ScriptObject[nObjectID].m_hObject = DNULL;
        
        return DTRUE;
    }
    
    g_pServerDE->BPrint("ObjectDestroy: Error");
    return DFALSE;        
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::WaitDelay()
{
	DFLOAT fTime;

	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "ChangeCameraFOV" );
    // Arg0 = Float fTime - Time to wait for next command

    m_nCurrLine++;
    fTime = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// If it's a delay, set the next update so that it gets called later
	if (fTime > 0.0f)
	{
		m_fNextScriptTime = g_pServerDE->GetTime() + fTime;

		// See if anything is moving
		if (m_bCheckOverTime)
			g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
		else
			g_pServerDE->SetNextUpdate(m_hObject, fTime);
	}

    return DTRUE;
}


/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Change Camera FOV
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraFOV()
{
	DFLOAT x, y;

	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "ChangeCameraFOV" );
    // Arg0 = Float x FOV
    // Arg1 = Float y FOV

    m_nCurrLine++;
    x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CAMERA_FOV);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)x);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)y);
	g_pServerDE->EndMessage(hMsg);

    return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Change Camera Rect
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraRect()
{
	int l, t, r, b;

	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "CameraRect" );
    // Arg0 = int left
    // Arg1 = int top
    // Arg2 = int right
    // Arg3 = int bottom

    m_nCurrLine++;
    l = atoi(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    t = atoi(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    r = atoi(m_scriptCmdList[m_nCurrLine]->data);
    m_nCurrLine++;
    b = atoi(m_scriptCmdList[m_nCurrLine]->data);

	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CAMERA_RECT);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)l);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)t);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)r);
    g_pServerDE->WriteToMessageFloat(hMsg, (float)b);
	g_pServerDE->EndMessage(hMsg);

    return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraReset
//
//	PURPOSE:	Change Camera reset
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraReset()
{
	if (!g_pServerDE) return DFALSE;

//    g_pServerDE->BPrint( "CameraReset" );

	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_CAMERA_RESET);
	g_pServerDE->EndMessage(hMsg);

    return DTRUE;
}
*/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraCreate()
{
    char pName[20];

	if (!g_pServerDE) return DFALSE;

    
    ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    // Arg0 = Camera Object Number
    m_nCurrLine++;
    sprintf(pName,"CameraObj%s", m_scriptCmdList[m_nCurrLine]->data );
    _mbscpy((unsigned char*)theStruct.m_Name, (const unsigned char*)pName);

    // Set the Object Index  NEED TO CHECK FOR OBJECT ALREADY USING THIS...
    m_nCameraIndex = atoi(m_scriptCmdList[m_nCurrLine]->data);

    // Arg1 = Camera type (0 = fullscreen, non-zero = letterboxed)
	m_nCurrLine++;
	DBYTE nCamType = atoi(m_scriptCmdList[m_nCurrLine]->data) ? CAMTYPE_LETTERBOX : CAMTYPE_FULLSCREEN;

    // Store the Name and Class inside the Object data
    
    // Store the names into the Object Array    
    _mbsncpy((unsigned char*)ScriptCamera[m_nCameraIndex].m_szObjectName, (const unsigned char*)pName, MAXDATA_LEN);
    _mbscpy((unsigned char*)ScriptCamera[m_nCameraIndex].m_szClassName, (const unsigned char*)"CameraObj");
    
	DRotation rRot;
    ROT_INIT(rRot);

	DVector vPos;
    g_pServerDE->GetObjectPos(m_hObject, &vPos);

    ROT_COPY(theStruct.m_Rotation, rRot);
	VEC_COPY(theStruct.m_Pos, vPos);
    
	theStruct.m_NextUpdate = 0.1f;
	theStruct.m_UserData = (DDWORD)nCamType;
    
	HCLASS pClass = g_pServerDE->GetClass("CameraObj");
    
    // Check to make sure this is a valid class, and create the camera
    if (pClass)
    {    
    	CameraObj* pCamera = (CameraObj*)g_pServerDE->CreateObject(pClass, &theStruct);
        
        if (pCamera)
        {
//			pCamera->Setup(nCamType);

            ScriptCamera[m_nCameraIndex].m_hObject = g_pServerDE->ObjectToHandle(pCamera);
            return DTRUE;
        }
    }        
    
    g_pServerDE->BPrint( "CameraCreate: ERROR...creating camera");
    return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraDestroy()
{
	if (!g_pServerDE) return DFALSE;
    
    int nObjectID;
    
    // Arg0 = TargetObject ID
    m_nCurrLine++;
    nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
    
    if (ScriptCamera[nObjectID].m_hObject)
    {
  		g_pServerDE->RemoveObject( ScriptObject[nObjectID].m_hObject );
  		ScriptCamera[nObjectID].m_hObject = DNULL;
        
        return DTRUE;
    }
    
    g_pServerDE->BPrint("CameraDestroy: Error");
    return DFALSE;        
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraSelect()
{
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	// Arg0 = CameraID
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Activate this camera, deactivate all others
	for (int i=0; i<MAXSCRIPTOBJ; i++)
	{
		if (ScriptCamera[i].m_hObject)
		{
    		CameraObj* pCamera = (CameraObj*)g_pServerDE->HandleToObject(ScriptCamera[i].m_hObject);

			DBOOL bActive = (nObjectID == i);
			pCamera->SetActive(bActive);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraMoveObj()
{
	if (!g_pServerDE) return DFALSE;

	// Arg0 = camera ID (m_nObjectIndex)
	// Arg1 = Time interval to destination
	// Arg2 = object ID
	m_nCurrLine++;
	int nCameraID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	DFLOAT fDuration = (DFLOAT)atof(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	int nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptCamera[nCameraID].m_hObject && ScriptObject[nObjectID].m_hObject)
	{
		int MoveCamera = -1;
        
		for (int i=0; i<MAX_MOVEOBJS; i++)
		{
			if (m_nMoveCamera[i] == -1)
			{
				MoveCamera = i;
				break;
			} 
		}
    
		if (MoveCamera != -1)
		{
			ObjectData *pObj = &ScriptCamera[nCameraID];

			g_pServerDE->GetObjectPos(pObj->m_hObject, &pObj->m_vInitPos);
			g_pServerDE->GetObjectRotation(pObj->m_hObject, &pObj->m_rInitRot);

			g_pServerDE->GetObjectPos(ScriptObject[nObjectID].m_hObject, &pObj->m_vDestPos);
			g_pServerDE->GetObjectRotation(ScriptObject[nObjectID].m_hObject, &pObj->m_rDestRot);

			pObj->m_fStartTime = pObj->m_fCurrentTime = g_pServerDE->GetTime();
			pObj->m_fDuration = fDuration;
			pObj->m_bMovement = DTRUE;

			m_nMoveCamera[MoveCamera] = nCameraID;
	        m_bCheckOverTime = DTRUE;
		}
		
		return DTRUE;
	}            

	g_pServerDE->BPrint("CameraMoveObj: Error");
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraTeleport()
{
	DVector vPos;
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = x
	// Arg2 = y
	// Arg3 = z
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	m_nCurrLine++;
	vPos.x = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
	m_nCurrLine++;
	vPos.y = (float)atof(m_scriptCmdList[m_nCurrLine]->data);
	m_nCurrLine++;
	vPos.z = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptCamera[nObjectID].m_hObject)
	{
		g_pServerDE->TeleportObject(ScriptCamera[nObjectID].m_hObject, &vPos);
		return DTRUE;
	}            

	g_pServerDE->BPrint("CameraTeleport: Error");
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraTeleportObj()
{
	int nCameraID;
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	// Arg0 = camera ID (m_nObjectIndex)
	// Arg1 = object ID
	m_nCurrLine++;
	nCameraID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptCamera[nCameraID].m_hObject && ScriptObject[nObjectID].m_hObject)
	{
		DVector vPos;
		DRotation rRot;
		g_pServerDE->GetObjectPos(ScriptObject[nObjectID].m_hObject, &vPos);
		g_pServerDE->GetObjectRotation(ScriptObject[nObjectID].m_hObject, &rRot);

		g_pServerDE->TeleportObject(ScriptCamera[nCameraID].m_hObject, &vPos);
		g_pServerDE->SetObjectRotation(ScriptCamera[nCameraID].m_hObject, &rRot);
		return DTRUE;
	}            

	g_pServerDE->BPrint("CameraTeleportObj: Error");
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraRotate()
{
	DVector vPitchYawRoll;
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	// Arg0 = object number (m_nObjectIndex)
	// Arg1 = x
	// Arg2 = y
	// Arg3 = z
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);
	m_nCurrLine++;
	vPitchYawRoll.x = DEG2RAD((float)atof(m_scriptCmdList[m_nCurrLine]->data));
	m_nCurrLine++;
	vPitchYawRoll.y = DEG2RAD((float)atof(m_scriptCmdList[m_nCurrLine]->data));
	m_nCurrLine++;
	vPitchYawRoll.z = DEG2RAD((float)atof(m_scriptCmdList[m_nCurrLine]->data));

	DRotation rRot;
	g_pServerDE->SetupEuler(&rRot, vPitchYawRoll.x, vPitchYawRoll.y, vPitchYawRoll.z);	
	if (ScriptCamera[nObjectID].m_hObject)
	{
		g_pServerDE->SetObjectRotation(ScriptCamera[nObjectID].m_hObject, &rRot);
		return DTRUE;
	}            

	g_pServerDE->BPrint("CameraRotate: Error");
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraLinkSpot()
{
	int nCameraID;
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	m_nCurrLine++;
	nCameraID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptCamera[nCameraID].m_hObject)
	{
		CameraObj *pCameraObj = (CameraObj*)g_pServerDE->HandleToObject(ScriptCamera[nCameraID].m_hObject);
		pCameraObj->SetLinkObject(ScriptObject[nObjectID].m_hObject);
		return DTRUE;
	}

	g_pServerDE->BPrint("CameraLinkSpot: Error");
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::CameraZoom()
{
	if (!g_pServerDE) return DFALSE;

	m_nCurrLine++;
	DBOOL bZoomView = atoi(m_scriptCmdList[m_nCurrLine]->data);

	HMESSAGEWRITE hMsg = g_pServerDE->StartMessage(NULL, SMSG_ZOOMVIEW);
	g_pServerDE->WriteToMessageByte(hMsg, (DBYTE)bZoomView);
	g_pServerDE->WriteToMessageByte(hMsg, 0);
	g_pServerDE->EndMessage(hMsg);

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::KillSound()
{
	if (!g_pServerDE) return DFALSE;
 
	// SoundID to Destory

	// Get the Args
	m_nCurrLine++;
	int nSoundID  = atoi(m_scriptCmdList[m_nCurrLine]->data);

	if (ScriptSound[nSoundID].m_hSound)
	{
		g_pServerDE->KillSound(ScriptSound[nSoundID].m_hSound);
		ScriptSound[nSoundID].m_hSound = DNULL;
		return DTRUE;
	}

	return DFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::PlaySoundPos()
{
	if (!g_pServerDE) return DFALSE;

	DVector vPos;
	DFLOAT fRadius;
	char *szSound = DNULL;
	// SoundID, X, Y, Z, Sound, Radius,Looping

	// Get the Args
	// Sound ID
	m_nCurrLine++;
	int nSoundID  = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// X
	m_nCurrLine++;
	vPos.x  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Y
	m_nCurrLine++;
	vPos.y  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Z
	m_nCurrLine++;
	vPos.z  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// SoundFilename
	m_nCurrLine++;
	szSound = m_scriptCmdList[m_nCurrLine]->data;

	// Radius
	m_nCurrLine++;
	fRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Looping
	m_nCurrLine++;
	int nLoopit = atoi(m_scriptCmdList[m_nCurrLine]->data);

	DBOOL bLoopit = DFALSE;
	if (nLoopit == 1)  
	{
		bLoopit = DTRUE;
	//        g_pServerDE->BPrint("SoundLoopTRUE");
	}

	//    char buf[128];
	//    sprintf(buf, "Sound=%s", szSound);
	//    g_pServerDE->BPrint(buf);

	if (szSound && _mbstrlen(szSound) > 0)
	{
		if (ScriptSound[nSoundID].m_hSound)
		{
			g_pServerDE->KillSound(ScriptSound[nSoundID].m_hSound);
			ScriptSound[nSoundID].m_hSound = DNULL;
		}

		//m_sndLastSound = PlaySoundFromObject(m_hObject,           pSoundName, 1000.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_MED, DTRUE, DTRUE);
		ScriptSound[nSoundID].m_hSound = PlaySoundFromPos(&vPos, szSound, fRadius, SOUNDPRIORITY_MISC_HIGH, bLoopit, bLoopit);

		return DTRUE;
	}

	g_pServerDE->BPrint("PlaySoundPos: Error");
	return DFALSE;    
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::PlaySoundObj()
{
	if (!g_pServerDE) return DFALSE;

	DFLOAT fRadius;
	char *szSound = DNULL;
	HOBJECT hObject;
	int nObjectID;

	// SoundID, PositionObj, Sound, Radius,Looping
	// Sound ID
	m_nCurrLine++;
	int nSoundID  = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Get the Args
	// Position Obj
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// SoundFilename
	m_nCurrLine++;
	szSound = m_scriptCmdList[m_nCurrLine]->data;

	// Radius    
	m_nCurrLine++;
	fRadius = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Looping
	m_nCurrLine++;
	int nLoopit = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Set the Object from object data
	hObject = ScriptObject[nObjectID].m_hObject;

	//    char buf[128];
	//    sprintf(buf, "Sound=%s", szSound);
	//    g_pServerDE->BPrint(buf);

	DBOOL bLoopit = DFALSE;
	if (nLoopit == 1)
	{
		bLoopit = DTRUE;
	//        g_pServerDE->BPrint("SoundLoopTRUE");
	}        
    
	if (szSound && _mbstrlen(szSound) > 0)
	{
		if (hObject)
		{
			if (ScriptSound[nSoundID].m_hSound)
			{
				g_pServerDE->KillSound(ScriptSound[nSoundID].m_hSound);
				ScriptSound[nSoundID].m_hSound = DNULL;
			}
			//m_sndLastSound = PlaySoundFromObject(m_hObject,           pSoundName, 1000.0f, SOUNDTYPE_MISC, SOUNDPRIORITY_MED, DTRUE, DTRUE);
			ScriptSound[nSoundID].m_hSound = PlaySoundFromObject(hObject, szSound, fRadius, SOUNDPRIORITY_MISC_HIGH, bLoopit, bLoopit);

			return DTRUE;
		}            
	}

	g_pServerDE->BPrint("PlaySoundObj: Error");
	return DFALSE;    
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::PlayAnimation()
{
	char *pAniName = DNULL;
	HOBJECT hObject;
	int nObjectID;

	if (!g_pServerDE) return DFALSE;

	// Arg0 = Object
	// Arg1 = AnimationName

	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	m_nCurrLine++;
	pAniName = m_scriptCmdList[m_nCurrLine]->data;

	if (pAniName)
	{    
		// Set the Object from object data
		hObject = ScriptObject[nObjectID].m_hObject;

		if (hObject)
		{
			DDWORD dwAniIndex = g_pServerDE->GetAnimIndex(hObject, pAniName);
			g_pServerDE->SetModelLooping(hObject, DFALSE);
			g_pServerDE->SetModelAnimation(hObject, dwAniIndex);
		}            
	}

	//    g_pServerDE->BPrint( "PlayAnimation" );
	return DTRUE;    
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::SendAIScript()
{
	if (!g_pServerDE) return DFALSE;

	// g_pServerDE->BPrint( "SendAIScript" );

	// Arg0 = Object
	// Arg1 = AIScriptName

	// Arg0 = Object
	m_nCurrLine++;
	int nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Arg1 = AIScriptName
	m_nCurrLine++;
	char *pAIScript = m_scriptCmdList[m_nCurrLine]->data;

	// Make a HSTRING to pass to the AIScript
	HSTRING hMsg = g_pServerDE->CreateString(pAIScript);

	if (pAIScript)
	{    
		// Set the Object from object data
		HOBJECT hObject = ScriptObject[nObjectID].m_hObject;

		if (hObject)
		{
			HMESSAGEWRITE hMessage;

			hMessage = g_pServerDE->StartMessageToObject(this, hObject, MID_TRIGGER);
			g_pServerDE->WriteToMessageHString(hMessage, hMsg);
			g_pServerDE->EndMessage(hMessage);
		}    
	}

	g_pServerDE->FreeString( hMsg );

	return DTRUE;    
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::SendTrigger()
{
	if (!g_pServerDE) return DFALSE;

	// Arg0 = Object
	// Arg1 = Trigger Message

	m_nCurrLine++;
	int nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Arg1 = AIScriptName
	m_nCurrLine++;
	char *pMsg = m_scriptCmdList[m_nCurrLine]->data;

	// Make a HSTRING to send to the object
	HSTRING hMsg = g_pServerDE->CreateString(pMsg);

	if (hMsg)
	{    
		// Set the Object from object data
		HOBJECT hObject = ScriptObject[nObjectID].m_hObject;

		if (hObject)
		{
			HMESSAGEWRITE hMessage;

			hMessage = g_pServerDE->StartMessageToObject(this, hObject, MID_TRIGGER);
			g_pServerDE->WriteToMessageHString(hMessage, hMsg);
			g_pServerDE->EndMessage(hMessage);
		}    
	}

	g_pServerDE->FreeString( hMsg );

	return DTRUE;    
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::SendTriggerNamed()
{
	if (!g_pServerDE) return DFALSE;

	// Arg0 = Object name
	// Arg1 = Trigger Message

	m_nCurrLine++;
	char *pName = m_scriptCmdList[m_nCurrLine]->data;
	HSTRING hName = g_pServerDE->CreateString(pName);

	// Arg1 = AIScriptName
	m_nCurrLine++;
	char *pMsg = m_scriptCmdList[m_nCurrLine]->data;
	HSTRING hMsg = g_pServerDE->CreateString(pMsg);

	if (hMsg && hName)
	{    
		SendTriggerMsgToObjects(this, hName, hMsg);
	}

	g_pServerDE->FreeString( hName );
	g_pServerDE->FreeString( hMsg );

	return DTRUE;    
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
DBOOL Script::ObjectScale()
{
	if (!g_pServerDE) return DFALSE;

	HOBJECT hObject;
	DVector vScale;
	int nObjectID;

	// Arg0 = Object
	// Arg1 = Scale X
	// Arg2 = Scale Y
	// Arg3 = Scale Z

	// Arg0 = Object
	m_nCurrLine++;
	nObjectID = atoi(m_scriptCmdList[m_nCurrLine]->data);

	// Arg1 = Scale X
	m_nCurrLine++;
	vScale.x  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Arg2 = Scale Y
	m_nCurrLine++;
	vScale.y  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Arg3 = Scale Z
	m_nCurrLine++;
	vScale.z  = (float)atof(m_scriptCmdList[m_nCurrLine]->data);

	// Set the Object from object data
	hObject = ScriptObject[nObjectID].m_hObject;

	if (hObject)
	{
		g_pServerDE->ScaleObject(hObject, &vScale);
		return DTRUE;    
	}            

	g_pServerDE->BPrint( "ObjectScale: Error" );
	return DFALSE;    
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE: NextToken
//
//	PURPOSE: Modified strtok, ignores seperators inside quotes.	
//
// ----------------------------------------------------------------------- //
char *NextToken (char *string, const char *control)
{
	char *str;

	static char *nextoken;

	if (string)
		str = string;
	else
		str = nextoken;

	while ( *str && _mbschr((const unsigned char*)control, (unsigned int)*str) )
		str++;

	// Find the end of the token. If it is not the end of the string,
	// put a null there.
	DBOOL bInQuotes = DFALSE;

	if (*str == '"')
	{
		bInQuotes = DTRUE;
		str++;
	}

	string = str;

	for ( ; *str ; str++ )
	{
		if ( _mbschr((const unsigned char*)control, (unsigned int)*str) )
		{
			if (!bInQuotes)
			{
				*str++ = '\0';
				break;
			}
		}
		if (*str == '"')
		{
			bInQuotes = !bInQuotes;
			*str++ = '\0';
			break;
		}
	}

	// Update nextoken
	nextoken = str;

	// Determine if a token has been found. 
	if ( string == str )
		return NULL;
	else
		return string;
}

