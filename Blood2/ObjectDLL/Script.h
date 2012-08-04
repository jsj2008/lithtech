#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdio.h>
#include "basedefs_de.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "ScriptList.h"
#include "gib.h"
#include "AI_Shared.h"


#define MAX_DEFINES			50
#define MAXSCRIPTOBJ        30
#define MAX_MOVEOBJS        10

// Script Objects
typedef struct ObjectData_t
{    
	char		m_szObjectName[MAXDATA_LEN+1];
	char		m_szClassName[MAXDATA_LEN+1];
    HOBJECT     m_hObject;

// Movement Over Time
    DBOOL       m_bMovement;
	DVector		m_vInitPos;
	DRotation	m_rInitRot;
	DVector		m_vDestPos;
	DRotation	m_rDestRot;
    DFLOAT      m_fStartTime;
    DFLOAT      m_fCurrentTime;
    DFLOAT      m_fDuration;
} ObjectData;


// Sound object
typedef struct SoundData_t 
{
	char		m_szObjectName[MAXDATA_LEN+1];
    HSOUNDDE    m_hSound;
} SoundData;


// Defines
typedef struct DefineData_t 
{
	char		m_szDefineName[MAXDATA_LEN+1];
	char		m_szDefineValue[MAXDATA_LEN+1];
} DefineData;



class Script : public B2BaseClass
{
	public:

		Script();
		virtual ~Script();

	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		DBOOL		HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private:

		DBOOL		ReadProp(ObjectCreateStruct *pStruct);
		void		PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL		InitialUpdate(DVector *pMovement);
		DBOOL		Update(DVector *pMovement);

		DBOOL		ProcessScript();
		DBOOL		CheckStartTime();
		DBOOL		LoadScript(int &nEOF);

		DBOOL		AddArgScript(char *temparg);

		DBOOL		StartScript();
		DBOOL		EndScript();
		DBOOL		SetCutScene();
		DBOOL		SubTitleDisplay();
		DBOOL		SubTitleFade();

		DBOOL		ObjectCreateAt();
		DBOOL		ObjectCreateNear();

		DBOOL		ObjectAssign();
		DBOOL		ObjectCreateModelAt();
		DBOOL		ObjectCreateModelNear();
		DBOOL		ObjectCreateFire();
		DBOOL		ObjectCreateExplosionAt();
		DBOOL		ObjectCreateExplosionNear();

		DBOOL		ObjectMove();
		DBOOL		ObjectTeleport();

		DBOOL		ObjectMoveObj();
		DBOOL		ObjectMoveOverTimeUpdate(ObjectData *pObj);

		DBOOL		ObjectFlags();
		DBOOL		ObjectFlagVisible();
		DBOOL		ObjectFlagSolid();
		DBOOL		ObjectFlagGravity();
		DBOOL		ObjectDims();
		DBOOL		ObjectGib();
		DBOOL		ObjectScale();
		DBOOL		ObjectDestroy();

		DBOOL		WaitDelay();
		DBOOL		CameraCreate();
		DBOOL		CameraDestroy();
		DBOOL		CameraSelect();
		DBOOL		CameraMoveObj();
		DBOOL		CameraTeleport();
		DBOOL		CameraTeleportObj();
		DBOOL		CameraRotate();
		DBOOL		CameraLinkSpot();
		DBOOL		CameraZoom();
		DBOOL		PlaySoundPos();
		DBOOL		PlaySoundObj();
		DBOOL		PlayAnimation();
		DBOOL		KillSound();
		DBOOL		SendAIScript();
		DBOOL		SendTrigger();
		DBOOL		SendTriggerNamed();

		int			StringToScriptCutCmdType(char* pCmdName, int &nArgs, int &nCheckObjID);

		ObjectData	ScriptObject[MAXSCRIPTOBJ];         // Script objects
		ObjectData	ScriptCamera[MAXSCRIPTOBJ];         // Script objects
		SoundData	ScriptSound[MAXSCRIPTOBJ];         // Script objects

		int			m_nObjectIndex;                     // Object buffer index
		int			m_nCameraIndex;                     // Object buffer index

		DFLOAT		m_fNextScriptTime;
		DFLOAT		m_fStartTime;
		int			m_nCurrLine;

		SCRIPTCMD	m_curScriptCmd;		                // The current script command
		CScriptList	m_scriptCmdList;	                // List of all script commands

		HSTRING		m_hstrScriptName;	                // ScriptName

		DBOOL		m_bScriptLoaded;
		DBOOL		m_bCheckOverTime;

		AI_Shared	AIShared;       // Shared functions

		DBOOL		m_bCheckStartTime;

		int			m_nDefineCnt;
		DefineData	sDefineObject[MAX_DEFINES];

		FILE		*infile;

		int			m_nMoveObject[MAX_MOVEOBJS];
		int			m_nMoveCamera[MAX_MOVEOBJS];

		DBOOL		m_bScriptStarted;
};


#endif // __SCRIPT_H__