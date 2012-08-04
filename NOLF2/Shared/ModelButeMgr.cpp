// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.cpp
//
// PURPOSE : ModelButeMgr implementation - Controls attributes of all ModelButes
//
// CREATED : 12/02/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModelButeMgr.h"
#include "CommonUtilities.h"
#include "UberAssert.h"
#include "SurfaceMgr.h"

// Globals/statics

CModelButeMgr* g_pModelButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];
static char s_szBuffer[1024];

// Defines

#define	MODELBMGR_MODEL					"Model"

	#define	MODELBMGR_MODEL_NAME								"Name"
	#define	MODELBMGR_MODEL_SOUND_TEMPLATE						"SoundTemplate"
	#define	MODELBMGR_MODEL_SKELETON							"Skeleton"
	#define	MODELBMGR_MODEL_TYPE								"Type"
	#define	MODELBMGR_MODEL_MASS								"Mass"
	#define	MODELBMGR_MODEL_HIT_POINTS							"HitPoints"
	#define	MODELBMGR_MODEL_MAX_HIT_POINTS						"MaxHitPoints"
	#define	MODELBMGR_MODEL_ARMOR								"Armor"
	#define	MODELBMGR_MODEL_MAX_ARMOR							"MaxArmor"
	#define	MODELBMGR_MODEL_ENERGY								"Energy"
	#define	MODELBMGR_MODEL_MAX_ENERGY							"MaxEnergy"
	#define	MODELBMGR_MODEL_MODELFILE							"ModelFile"
	#define	MODELBMGR_MODEL_SKIN								"Skin"
	#define	MODELBMGR_MODEL_HANDSSKIN							"HandsSkin"
	#define	MODELBMGR_MODEL_RENDERSTYLE							"RenderStyle"
	#define	MODELBMGR_MODEL_CLIENTFX							"ClientFX"
	#define	MODELBMGR_MODEL_AINAME								"AIName"
	#define MODELBMGR_MODEL_AIONLY								"AIOnly"
	#define MODELBMGR_MODEL_ANIMATION							"Animation"
	#define MODELBMGR_MODEL_ATTACHMENT							"DefaultAttachment"
	#define MODELBMGR_MODEL_PVATTACHMENT						"PlayerViewAttachment"
	#define MODELBMGR_MODEL_CANBECARRIED						"CanBeCarried"
	#define MODELBMGR_MODEL_AIIGNOREBODY						"AIIgnoreBody"
	#define MODELBMGR_MODEL_UNALERTDAMAGEMOD					"UnalertDamageFactor"	
	#define MODELBMGR_MODEL_LOUDMOVEMENTSOUNDBUTE				"LoudMovementSoundBute"
	#define MODELBMGR_MODEL_QUIETMOVEMENTSOUNDBUTE				"QuietMovementSoundBute"
	#define MODELBMGR_MODEL_ALTHEAD								"AltHeadSkin"
	#define MODELBMGR_MODEL_ALTBODY								"AltBodySkin"
	#define MODELBMGR_MODEL_TRANSLUCENT							"Translucent"
	#define MODELBMGR_MODEL_FLESHSURFACETYPE					"FleshSurfaceType"
	#define MODELBMGR_MODEL_ARMORSURFACETYPE					"ArmorSurfaceType"
	#define MODELBMGR_MODEL_NAMEID								"NameId"
	#define MODELBMGR_MODEL_PLAYERPAINSNDDIR					"PlayerPainSoundDir"

	#define	MODELBMGR_MODEL_DEFAULT_MODELFILE					CString("chars\\models\\hero_action.ltb")
	#define	MODELBMGR_MODEL_DEFAULT_HANDSSKIN					CString("guns\\skins_pv\\actionHands_pv.dtx")
	#define	MODELBMGR_MODEL_DEFAULT_ANIMATION					CString("Attributes\\AnimationsHuman.txt")
	#define	MODELBMGR_MODEL_DEFAULT_AINAME						CString("Hero")
	#define	MODELBMGR_MODEL_DEFAULT_AIONLY						LTTRUE
	#define MODELBMGR_MODEL_DEFAULT_CANBECARRIED				LTTRUE
	#define MODELBMGR_MODEL_DEFAULT_AIIGNOREBODY				LTFALSE
	#define MODELBMGR_MODEL_DEFAULT_UNALERTDAMAGEMOD			1.0f
	#define	MODELBMGR_MODEL_DEFAULT_TRANSLUCENT					0
	#define MODELBMGR_MODEL_DEFAULT_NAMEID						0
	#define MODELBMGR_MODEL_DEFAULT_PLAYERPAINSNDDIR			CString("Player")
	#define MODELBMGR_MODEL_DEFAULT_FLESHSURFACETYPE			"Flesh"
	#define MODELBMGR_MODEL_DEFAULT_ARMORSURFACETYPE			"Armor"


#define	MODELBMGR_STYLE					"Style"

	#define	MODELBMGR_STYLE_NAME								"Name"

#define MODELBMGR_SKELETON				"Skeleton"

	#define MODELBMGR_SKELETON_DEFAULT_FRONT_DEATH_ANI			"DefaultFrontDeathAni"
	#define MODELBMGR_SKELETON_DEFAULT_BACK_DEATH_ANI			"DefaultBackDeathAni"
	#define MODELBMGR_SKELETON_DEFAULT_FRONT_SHORTRECOIL_ANI	"DefaultFrontShortRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_BACK_SHORTRECOIL_ANI		"DefaultBackShortRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_HIT_NODE					"DefaultHitNode"

	#define MODELBMGR_SKELETON_TRACKING_NODES_LOOK_AT			"LookAtTrackingNodes"
	#define MODELBMGR_SKELETON_TRACKING_NODES_AIM_AT			"AimAtTrackingNodes"

	#define MODELBMGR_SKELETON_NODE_NAME						"Node%dName"
	#define MODELBMGR_SKELETON_NODE_FLAGS						"Node%dFlags"
	#define MODELBMGR_SKELETON_NODE_FRONT_DEATH_ANI				"Node%dFrontDeathAni"
	#define MODELBMGR_SKELETON_NODE_BACK_DEATH_ANI				"Node%dBackDeathAni"
	#define MODELBMGR_SKELETON_NODE_FRONT_SHORTRECOIL_ANI		"Node%dFrontShortRecoilAni"
	#define MODELBMGR_SKELETON_NODE_BACK_SHORTRECOIL_ANI		"Node%dBackShortRecoilAni"
	#define MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR				"Node%dDamageFactor"
	#define MODELBMGR_SKELETON_NODE_PARENT						"Node%dParent"
	#define MODELBMGR_SKELETON_NODE_LOCATION					"Node%dLocation"
	#define MODELBMGR_SKELETON_NODE_RADIUS						"Node%dRadius"
	#define MODELBMGR_SKELETON_NODE_PRIORITY					"Node%dPriority"
	#define MODELBMGR_SKELETON_NODE_ATTACHSPEARS				"Node%dAttachSpears"

#define MODELBMGR_TRACKINGNODES			"TrackingNodes"

	#define MODELBMGR_TRACKING_NODE_NAME						"Node%dName"
	#define MODELBMGR_TRACKING_NODE_CLONE_NAME					"Node%dCloneName"
	#define MODELBMGR_TRACKING_NODE_FORWARD						"Node%dForwardVector"
	#define MODELBMGR_TRACKING_NODE_UP							"Node%dUpVector"
	#define MODELBMGR_TRACKING_NODE_DISCOMFORT_X				"Node%dDiscomfortX"
	#define MODELBMGR_TRACKING_NODE_DISCOMFORT_Y				"Node%dDiscomfortY"
	#define MODELBMGR_TRACKING_NODE_MAX_X						"Node%dMaxX"
	#define MODELBMGR_TRACKING_NODE_MAX_Y						"Node%dMaxY"
	#define MODELBMGR_TRACKING_NODE_MAX_VEL						"Node%dMaxVel"

#define MODELBMGR_NODE_SCRIPT			"NodeScript"

	#define MODELBMGR_NSCRIPT_NODE_NAME							"NodeName"
	#define MODELBMGR_NSCRIPT_FLAGS								"Flags"

	#define MODELBMGR_NSCRIPT_NODE_TIME							"Pt%dTime"
	#define MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET				"Pt%dPosOffset"
	#define MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET				"Pt%dRotOffset"


#define	MODELBMGR_CP_MODEL				"MultiplayerModels"
#define	MODELBMGR_DM_MODEL				"DeathmatchModels"
#define	MODELBMGR_TEAM_MODEL			"TeamModels"
#define	MODELBMGR_TEAM_DEFAULT			"TeamDefaults"

	#define	MODELBMGR_MP_MODEL_NAME								"Name"
	#define	MODELBMGR_MP_MODEL_TEAM								"Team"

#ifndef _CLIENTBUILD


// Plugin statics
#ifndef __PSX2
LTBOOL CModelButeMgrPlugin::sm_bInitted = LTFALSE;
#endif

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::CModelButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::CModelButeMgr()
{
	m_cModels = 0;
    m_aModels = LTNULL;

	m_cStyles = 0;
    m_aStyles = LTNULL;

	m_cSkeletons = 0;
    m_aSkeletons = LTNULL;

	m_cTrackingNodeGroups = 0;
	m_aTrackingNodeGroups = LTNULL;

	m_cNScripts = 0;
    m_aNScripts = LTNULL;

	m_cCPModels = 0;
	m_aCPModels = LTNULL;

	m_cDMModels = 0;
	m_aDMModels = LTNULL;

	m_cTeamModels = 0;
	m_aTeamModels = LTNULL;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::~CModelButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::~CModelButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CModelButeMgr& CModelButeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CModelButeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::Init(const char* szAttributeFile)
{
    if (g_pModelButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer

	g_pModelButeMgr = this;

	// Count the node groups

	m_cSkeletons = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cSkeletons++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);
	}

	m_aSkeletons = debug_newa(CModelButeMgr::CSkeleton, m_cSkeletons);

	// Read in the nodegroups

	for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, iSkeleton);

		// Get the skeleton's default death/long recoil anis

		
		m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_FRONT_DEATH_ANI,m_aSkeletons[iSkeleton].m_szDefaultFrontDeathAni,MAX_MODELBMGR_NAME_LEN);
		m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_BACK_DEATH_ANI,m_aSkeletons[iSkeleton].m_szDefaultBackDeathAni, MAX_MODELBMGR_NAME_LEN);
		m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_FRONT_SHORTRECOIL_ANI,m_aSkeletons[iSkeleton].m_szDefaultFrontShortRecoilAni,MAX_MODELBMGR_NAME_LEN);
		m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_BACK_SHORTRECOIL_ANI,m_aSkeletons[iSkeleton].m_szDefaultBackShortRecoilAni,MAX_MODELBMGR_NAME_LEN);

		// Get the skeleton's default hit node

        m_aSkeletons[iSkeleton].m_eModelNodeDefaultHit = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_SKELETON_DEFAULT_HIT_NODE);

		// Get the skeleton's tracking node groups.

		m_aSkeletons[iSkeleton].m_eModelTrackingNodesLookAt = (ModelTrackingNodeGroup)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_SKELETON_TRACKING_NODES_LOOK_AT, m_aSkeletons[iSkeleton].m_eModelTrackingNodesLookAt);

		m_aSkeletons[iSkeleton].m_eModelTrackingNodesAimAt  = (ModelTrackingNodeGroup)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_SKELETON_TRACKING_NODES_AIM_AT, m_aSkeletons[iSkeleton].m_eModelTrackingNodesAimAt);

		// Count the number of nodes

		m_aSkeletons[iSkeleton].m_cNodes = 0;
		sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aSkeletons[iSkeleton].m_cNodes++;
			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);
		}

		// Get all of our nodes

		m_aSkeletons[iSkeleton].m_aNodes = debug_newa(CModelButeMgr::CNode, m_aSkeletons[iSkeleton].m_cNodes);

		for ( int iNode = 0 ; iNode < m_aSkeletons[iSkeleton].m_cNodes ; iNode++ )
		{
			// Get the node's name

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName,m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szName,MAX_MODELBMGR_NAME_LEN);

			// Get the node's special flags

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FLAGS, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_dwFlags = m_buteMgr.GetDword(s_aTagName, s_aAttName);

			// Get the node's death/long recoil animations

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FRONT_DEATH_ANI, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName,m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szFrontDeathAni,MAX_MODELBMGR_NAME_LEN);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BACK_DEATH_ANI, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName,m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szBackDeathAni,MAX_MODELBMGR_NAME_LEN);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FRONT_SHORTRECOIL_ANI, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName,m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szFrontShortRecoilAni, MAX_MODELBMGR_NAME_LEN);

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BACK_SHORTRECOIL_ANI, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName,m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szBackShortRecoilAni, MAX_MODELBMGR_NAME_LEN);

			// Get the node's damage factor

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fDamageFactor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's parent

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PARENT, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eModelNodeParent = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName);

			// Get the node's hit location

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_LOCATION, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eHitLocation = (HitLocation)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName, HL_UNKNOWN);

			// Get the node's hit radius

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_RADIUS, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fHitRadius = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's hit radius

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PRIORITY, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fHitPriority = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's attach spear flag

			sprintf( s_aAttName, MODELBMGR_SKELETON_NODE_ATTACHSPEARS, iNode );
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_bAttachSpears = !!m_buteMgr.GetInt( s_aTagName, s_aAttName, 1 );
		}
	}


	// Count the tracking node group.

	m_cTrackingNodeGroups = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_TRACKINGNODES, m_cTrackingNodeGroups);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cTrackingNodeGroups++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_TRACKINGNODES, m_cTrackingNodeGroups);
	}

	m_aTrackingNodeGroups = debug_newa(CModelButeMgr::CTrackingNodeGroup, m_cTrackingNodeGroups);

	// Read in the tracking node group.

	for ( int iGroup = 0 ; iGroup < m_cTrackingNodeGroups ; iGroup++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_TRACKINGNODES, iGroup);

		// Count the number of nodes

		m_aTrackingNodeGroups[iGroup].m_cTrackingNodes = 0;
		sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_NAME, m_aTrackingNodeGroups[iGroup].m_cTrackingNodes);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aTrackingNodeGroups[iGroup].m_cTrackingNodes++;
			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_NAME, m_aTrackingNodeGroups[iGroup].m_cTrackingNodes);
		}

		// Get all of our nodes

		m_aTrackingNodeGroups[iGroup].m_aTrackingNodes = debug_newa(CModelButeMgr::CTrackingNode, m_aTrackingNodeGroups[iGroup].m_cTrackingNodes);

		for ( int iNode = 0 ; iNode < m_aTrackingNodeGroups[iGroup].m_cTrackingNodes ; iNode++ )
		{
			// Get the node's name.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_NAME, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName, m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_szName, MAX_MODELBMGR_NAME_LEN);

			// Get the node's discomfort angle around x-axis.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_DISCOMFORT_X, iNode);
            m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_fDiscomfortAngleX = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's discomfort angle around y-axis.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_DISCOMFORT_Y, iNode);
            m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_fDiscomfortAngleY = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's max angle around x-axis.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_MAX_X, iNode);
            m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_fMaxAngleX = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's max angle around y-axis.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_MAX_Y, iNode);
            m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_fMaxAngleY = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's max angular velocity.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_MAX_VEL, iNode);
            m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_fMaxVelocity = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			//
			// The following fields are optional.
			//

			// Get the cloned node's name.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_CLONE_NAME, iNode);
			if( m_buteMgr.Exist( s_aTagName, s_aAttName ) )
			{
				m_aTrackingNodeGroups[iGroup].m_cClonedTrackingNodes++;
				m_buteMgr.GetString(s_aTagName, s_aAttName, m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_szClonedName, MAX_MODELBMGR_NAME_LEN);
			}

			char szVector[32];
			char* tok;

			// Get the node's forward vector.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_FORWARD, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szVector, sizeof( szVector ) );
			if( m_buteMgr.Success( ))
			{
				if( szVector[0] )
				{
					m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_bAxesSpecified = LTTRUE;

					tok = strtok( szVector, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vForward.x = (LTFLOAT)atof( tok );

					tok = strtok( LTNULL, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vForward.y = (LTFLOAT)atof( tok );

					tok = strtok( LTNULL, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vForward.z = (LTFLOAT)atof( tok );
				}
			}

			// Get the node's up vector.

			sprintf(s_aAttName, MODELBMGR_TRACKING_NODE_UP, iNode);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szVector, sizeof( szVector ) );
			if( m_buteMgr.Success( ))
			{
				if( szVector[0] )
				{
					m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_bAxesSpecified = LTTRUE;

					tok = strtok( szVector, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vUp.x = (LTFLOAT)atof( tok );

					tok = strtok( LTNULL, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vUp.y = (LTFLOAT)atof( tok );

					tok = strtok( LTNULL, "," );
					if( tok ) m_aTrackingNodeGroups[iGroup].m_aTrackingNodes[iNode].m_vUp.z = (LTFLOAT)atof( tok );
				}
			}
		}
	}


	// Count the scripts

	m_cNScripts = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cNScripts++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);
	}

	m_aNScripts = debug_newa(CModelButeMgr::CNScript, m_cNScripts);

	// Read in the scripts

	for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, iNScript);

		// Get the node's name
		m_buteMgr.GetString(s_aTagName, MODELBMGR_NSCRIPT_NODE_NAME,m_aNScripts[iNScript].m_szName,MAX_MODELBMGR_NAME_LEN);

		// Get the scripts flags
        m_aNScripts[iNScript].m_bFlags = (uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_NSCRIPT_FLAGS);

		// Count the number of script sections
		m_aNScripts[iNScript].m_cNScriptPts = 0;
		sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aNScripts[iNScript].m_cNScriptPts++;
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);
		}

		// Get all of our script sections
		m_aNScripts[iNScript].m_aNScriptPts = debug_newa(CModelButeMgr::CNScriptPt, m_aNScripts[iNScript].m_cNScriptPts);

		for ( int iNScriptPt = 0 ; iNScriptPt < m_aNScripts[iNScript].m_cNScriptPts ; iNScriptPt++ )
		{
			// Get the node's time
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, iNScriptPt);
            m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Each time must be greater than the previous... and the first one must be zero
			if(iNScriptPt > 0)
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime > m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt - 1].m_fTime);
			else
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime == 0.0f);

			// Get the node's offset position
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vPosOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);

			// Get the node's offset rotation
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);

			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.x *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.y *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.z *= (MATH_PI / 180.0f);
		}
	}

	// Count the styles

	m_cStyles = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, m_cStyles);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cStyles++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, m_cStyles);
	}

	m_aStyles = debug_newa(CModelButeMgr::CStyle, m_cStyles);

	// Read in the styles

	for ( int iStyle = 0 ; iStyle < m_cStyles ; iStyle++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, iStyle);
		m_buteMgr.GetString(s_aTagName, MODELBMGR_STYLE_NAME,m_aStyles[iStyle].m_szName,MAX_MODELBMGR_NAME_LEN);
	}

	// Count the models

	m_cModels = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, m_cModels);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cModels++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, m_cModels);
	}

	m_aModels = debug_newa(CModelButeMgr::CModel, m_cModels);

	// Read in the models

	for ( int iModel = 0 ; iModel < m_cModels ; iModel++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, iModel);

		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_NAME,m_aModels[iModel].m_szName,MAX_MODELBMGR_NAME_LEN);
 		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_SOUND_TEMPLATE,m_aModels[iModel].m_szSoundTemplate,MAX_MODELBMGR_NAME_LEN);
        m_aModels[iModel].m_eModelSkeleton = (ModelSkeleton)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_MODEL_SKELETON);
        m_aModels[iModel].m_eModelType = (ModelType)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_MODEL_TYPE);
        m_aModels[iModel].m_fModelMass = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MASS);
        m_aModels[iModel].m_fModelHitPoints = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_HIT_POINTS);
        m_aModels[iModel].m_fModelMaxHitPoints = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MAX_HIT_POINTS);
        m_aModels[iModel].m_fModelArmor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_ARMOR);
        m_aModels[iModel].m_fModelMaxArmor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MAX_ARMOR);
		m_aModels[iModel].m_fModelEnergy = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_ENERGY);
        m_aModels[iModel].m_fModelMaxEnergy = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MAX_ENERGY);

		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_ANIMATION, MODELBMGR_MODEL_DEFAULT_ANIMATION, s_szBuffer, ARRAY_LEN(s_szBuffer) );
		if(s_szBuffer[0] == '\0')
		{
			m_aModels[iModel].m_szAnimationMgr = debug_newa(char, strlen(MODELBMGR_MODEL_DEFAULT_ANIMATION) + 1);
			strcpy(m_aModels[iModel].m_szAnimationMgr, MODELBMGR_MODEL_DEFAULT_ANIMATION);
		}
		else {
			m_aModels[iModel].m_szAnimationMgr = debug_newa(char, strlen(s_szBuffer) + 1);
			strcpy(m_aModels[iModel].m_szAnimationMgr, s_szBuffer);
		}

		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_MODELFILE, MODELBMGR_MODEL_DEFAULT_MODELFILE,m_aModels[iModel].m_szModelFile, MAX_MODELBMGR_MAX_PATH);

		m_aModels[iModel].m_blrSkinReader.Read(&m_buteMgr, s_aTagName, MODELBMGR_MODEL_SKIN, MAX_PATH);

		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_HANDSSKIN, MODELBMGR_MODEL_DEFAULT_HANDSSKIN,m_aModels[iModel].m_szHandsSkin, MAX_MODELBMGR_MAX_PATH);

		m_aModels[iModel].m_blrRenderStyleReader.Read(&m_buteMgr, s_aTagName, MODELBMGR_MODEL_RENDERSTYLE, MAX_PATH);

		m_aModels[iModel].m_blrClientFXReader.Read(&m_buteMgr, s_aTagName, MODELBMGR_MODEL_CLIENTFX, MAX_PATH);

		// Get AI values.
		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_AINAME, MODELBMGR_MODEL_DEFAULT_AINAME,m_aModels[iModel].m_szAIName,MAX_MODELBMGR_NAME_LEN);
        m_aModels[iModel].m_bAIOnly = (LTBOOL)m_buteMgr.GetBool(s_aTagName, MODELBMGR_MODEL_AIONLY, MODELBMGR_MODEL_DEFAULT_AIONLY);
        m_aModels[iModel].m_bCanBeCarried = (LTBOOL)m_buteMgr.GetBool(s_aTagName, MODELBMGR_MODEL_CANBECARRIED, MODELBMGR_MODEL_DEFAULT_CANBECARRIED);
        m_aModels[iModel].m_bAIIgnoreBody = (LTBOOL)m_buteMgr.GetBool(s_aTagName, MODELBMGR_MODEL_AIIGNOREBODY, MODELBMGR_MODEL_DEFAULT_AIIGNOREBODY);
		m_aModels[iModel].m_fUnalertDamageFactor = (LTFLOAT)m_buteMgr.GetDouble( s_aTagName, MODELBMGR_MODEL_UNALERTDAMAGEMOD, MODELBMGR_MODEL_DEFAULT_UNALERTDAMAGEMOD );

		// Get sound butes...
		m_buteMgr.GetString( s_aTagName, MODELBMGR_MODEL_LOUDMOVEMENTSOUNDBUTE, "", m_aModels[iModel].m_szLoudMovementSnd, MAX_MODELBMGR_NAME_LEN );
		m_buteMgr.GetString( s_aTagName, MODELBMGR_MODEL_QUIETMOVEMENTSOUNDBUTE, "", m_aModels[iModel].m_szQuietMovementSnd, MAX_MODELBMGR_NAME_LEN );

		// Get alternate head and body skins...

		m_aModels[iModel].m_blrAltHeadSkin.Read( &m_buteMgr, s_aTagName, MODELBMGR_MODEL_ALTHEAD, MAX_PATH );
		m_aModels[iModel].m_blrAltBodySkin.Read( &m_buteMgr, s_aTagName, MODELBMGR_MODEL_ALTBODY, MAX_PATH );

		// Determine whether or not this object is translucent
		m_aModels[iModel].m_bTranslucent = (LTBOOL)!!m_buteMgr.GetInt(s_aTagName, MODELBMGR_MODEL_TRANSLUCENT, MODELBMGR_MODEL_DEFAULT_TRANSLUCENT);

		// Get the surface types for this model
		char pszSurfaceType[MAX_MODELBMGR_NAME_LEN + 1];
		SURFACE* pSurface;
		
		//First armor
		m_buteMgr.GetString( s_aTagName, MODELBMGR_MODEL_ARMORSURFACETYPE, MODELBMGR_MODEL_DEFAULT_ARMORSURFACETYPE, pszSurfaceType, MAX_MODELBMGR_NAME_LEN );
		pSurface = g_pSurfaceMgr->GetSurface(pszSurfaceType);
		m_aModels[iModel].m_eArmorSurfaceType = (pSurface) ? pSurface->eType : ST_ARMOR;

		//then flesh
		m_buteMgr.GetString( s_aTagName, MODELBMGR_MODEL_FLESHSURFACETYPE, MODELBMGR_MODEL_DEFAULT_FLESHSURFACETYPE, pszSurfaceType, MAX_MODELBMGR_NAME_LEN );
		pSurface = g_pSurfaceMgr->GetSurface(pszSurfaceType);
		m_aModels[iModel].m_eFleshSurfaceType = (pSurface) ? pSurface->eType : ST_FLESH;
		
		// Get the optional display name Id...

		m_aModels[iModel].m_nNameId = (uint16)m_buteMgr.GetInt( s_aTagName, MODELBMGR_MODEL_NAMEID, MODELBMGR_MODEL_DEFAULT_NAMEID );

		// Get the directory for the player pain sounds...

		m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_PLAYERPAINSNDDIR, MODELBMGR_MODEL_DEFAULT_PLAYERPAINSNDDIR, m_aModels[iModel].m_szPlayerPainSoundDir, ARRAY_LEN(m_aModels[iModel].m_szPlayerPainSoundDir) );
				
			
		// Get default attachments.

		char* tok;
		char szTemp[64];
		DefaultAttachmentStruct das;

		uint32 iAttachment = 0;
		while ( true )
		{
			sprintf(s_aAttName, "%s%d", MODELBMGR_MODEL_ATTACHMENT, iAttachment);
			m_buteMgr.GetString(s_aTagName, s_aAttName, "", szTemp, 64);
			if( !m_buteMgr.Success( ))
				break;

			tok = strtok( szTemp, ", ");
			das.szAttachmentPosition = debug_newa( char, strlen(tok) + 1 );
			strcpy( das.szAttachmentPosition, tok );

			tok = strtok( LTNULL, ", ");
			das.szAttachment = debug_newa( char, strlen(tok) + 1 );
			strcpy( das.szAttachment, tok );

			m_aModels[iModel].m_lstDefaultAttachments.push_back( das );

			iAttachment++;
		}

		// Get the player view attachment...

		PlayerViewAttachmentStruct pas;

		uint32 iPVAttachment = 0;
		while( true )
		{
			sprintf( s_aAttName, "%s%d", MODELBMGR_MODEL_PVATTACHMENT, iPVAttachment );
			m_buteMgr.GetString( s_aTagName, s_aAttName, "", szTemp, 64 );
			if( !m_buteMgr.Success() )
				break;

			tok = strtok( szTemp, ", ");
			pas.szPVAttachmentPosition = debug_newa( char, strlen( tok ) + 1 );
			strcpy( pas.szPVAttachmentPosition, tok );

			tok = strtok( LTNULL, ", " );
			pas.szPVAttachment = debug_newa( char, strlen( tok ) + 1 );
			strcpy( pas.szPVAttachment, tok );

			m_aModels[iModel].m_lstPVAttachments.push_back( pas );

			++iPVAttachment;
		}
	}

	
	// Count the co-op models
	m_cCPModels = 0;
	sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cCPModels);

	while (m_buteMgr.Exist(MODELBMGR_CP_MODEL,s_aAttName))
	{
		m_cCPModels++;
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cCPModels);
	}

	m_aCPModels = debug_newa(ModelId, m_cCPModels);
	// Read in the models
	for ( int iCPModel = 0 ; iCPModel < m_cCPModels ; iCPModel++ )
	{
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, iCPModel);
		m_buteMgr.GetString(MODELBMGR_CP_MODEL, s_aAttName, "", s_szBuffer, ARRAY_LEN(s_szBuffer) );
		m_aCPModels[iCPModel] = GetModelId(s_szBuffer);
	}


	// Count the DM models
	m_cDMModels = 0;
	sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cDMModels);

	while (m_buteMgr.Exist(MODELBMGR_DM_MODEL,s_aAttName))
	{
		m_cDMModels++;
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cDMModels);
	}

	m_aDMModels = debug_newa(ModelId, m_cDMModels);

	// Read in the models

	for ( int iDMModel = 0 ; iDMModel < m_cDMModels ; iDMModel++ )
	{
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, iDMModel);
		m_buteMgr.GetString(MODELBMGR_DM_MODEL, s_aAttName, "", s_szBuffer, ARRAY_LEN(s_szBuffer) );
		m_aDMModels[iDMModel] = GetModelId(s_szBuffer);
	}

	// Count the  team models
	m_cTeamModels = 0;
	sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cTeamModels);

	while (m_buteMgr.Exist(MODELBMGR_TEAM_MODEL,s_aAttName))
	{
		m_cTeamModels++;
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, m_cTeamModels);
	}

	m_aTeamModels = debug_newa(ModelId, m_cTeamModels);

	// Read in the models

	for ( int iTModel = 0 ; iTModel < m_cTeamModels ; iTModel++ )
	{
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_NAME, iTModel);
	
		m_buteMgr.GetString(MODELBMGR_TEAM_MODEL, s_aAttName, "", s_szBuffer, ARRAY_LEN(s_szBuffer) );
		m_aTeamModels[iTModel] = GetModelId(s_szBuffer);
	}

	for ( int iTeam = 0 ; iTeam < 2 ; iTeam++ )
	{
		sprintf(s_aAttName, "%s%d", MODELBMGR_MP_MODEL_TEAM, iTeam);
		
		m_aTeamDefaults[iTeam] = m_buteMgr.GetInt(MODELBMGR_TEAM_DEFAULT, s_aAttName, 0);
	}



	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CModelButeMgr::Term()
{
	if (m_aModels)
	{
		debug_deletea(m_aModels);
        m_aModels = LTNULL;
	}

	if (m_aStyles)
	{
		debug_deletea(m_aStyles);
        m_aStyles = LTNULL;
	}

	if (m_aSkeletons)
	{
		for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
		{
			debug_deletea(m_aSkeletons[iSkeleton].m_aNodes);
		}

		debug_deletea(m_aSkeletons);
        m_aSkeletons = LTNULL;
	}

	if (m_aTrackingNodeGroups)
	{
		for ( int iGroup = 0 ; iGroup < m_cTrackingNodeGroups ; iGroup++ )
		{
			debug_deletea(m_aTrackingNodeGroups[iGroup].m_aTrackingNodes);
		}

		debug_deletea(m_aTrackingNodeGroups);
        m_aTrackingNodeGroups = LTNULL;
	}

	if (m_aNScripts)
	{
		for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
		{
			debug_deletea(m_aNScripts[iNScript].m_aNScriptPts);
		}

		debug_deletea(m_aNScripts);
        m_aNScripts = LTNULL;
	}

	if (m_aCPModels)
	{
		debug_deletea(m_aCPModels);
        m_aCPModels = LTNULL;
	}

	if (m_aDMModels)
	{
		debug_deletea(m_aDMModels);
        m_aDMModels = LTNULL;
	}

	if (m_aTeamModels)
	{
		debug_deletea(m_aTeamModels);
        m_aTeamModels = LTNULL;
	}

    g_pModelButeMgr = LTNULL;

	CGameButeMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetModel...
//
//	PURPOSE:	Various model attribute lookups
//
// ----------------------------------------------------------------------- //

ModelId CModelButeMgr::GetModelId(const char *szName)
{
	_ASSERT(szName);

    int iModel;
    for ( iModel = 0 ; iModel < m_cModels ; iModel++ )
	{
        if ( !_stricmp(szName, m_aModels[iModel].m_szName) )
		{
			return (ModelId)iModel;
		}
	}

	// Couldn't find anything, return any intentionally bogus value

// !!!    g_pLTBase->CPrint("CModelButeMgr::GetModel - Could not find Model \"%s\"", szName);
    _ASSERT(LTFALSE);

	return (ModelId)iModel;
}

const char* CModelButeMgr::GetModelName(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szName;
	}

	_ASSERT( !"CModelButeMgr::GetModelName: Invalid ModelId" );
	return LTNULL;
}

const char* CModelButeMgr::GetModelSoundTemplate(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szSoundTemplate;
	}

	_ASSERT( !"CModelButeMgr::GetModelName: Invalid ModelId" );
	return LTNULL;
}

const char*	CModelButeMgr::GetModelAnimationMgr(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szAnimationMgr;
	}

	_ASSERT( !"CModelButeMgr::GetModelAnimationMgr: Invalid ModelId" );
	return LTNULL;
}

const char* CModelButeMgr::GetModelAIName(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szAIName;
	}

	_ASSERT( !"CModelButeMgr::GetModelAnimationMgr: Invalid ModelId" );
	return LTNULL;
}

ModelType CModelButeMgr::GetModelType(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_eModelType;
	}

	_ASSERT( !"CModelButeMgr::GetModelType: Invalid ModelId" );
	return eModelTypeInvalid;
}

ModelSkeleton CModelButeMgr::GetModelSkeleton(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_eModelSkeleton;
	}

	_ASSERT( !"CModelButeMgr::GetModelSkeleton: Invalid ModelId" );
	return eModelSkeletonInvalid;
}

LTFLOAT CModelButeMgr::GetModelMass(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelMass;
	}

	_ASSERT( !"CModelButeMgr::GetModelMass: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelHitPoints(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelHitPoints;
	}

	_ASSERT( !"CModelButeMgr::GetModelHitPoints: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelMaxHitPoints(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelMaxHitPoints;
	}

	_ASSERT( !"CModelButeMgr::GetModelMaxHitPoints: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelEnergy(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelEnergy;
	}

	_ASSERT( !"CModelButeMgr::GetModelEnergy: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelMaxEnergy(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelMaxEnergy;
	}

	_ASSERT( !"CModelButeMgr::GetModelMaxEnergy: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelArmor(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelArmor;
	}

	_ASSERT( !"CModelButeMgr::GetModelArmor: Invalid ModelId" );
	return 0.f;
}

LTFLOAT CModelButeMgr::GetModelMaxArmor(ModelId eModelId)
{
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fModelMaxArmor;
	}

	_ASSERT( !"CModelButeMgr::GetModelMaxArmor: Invalid ModelId" );
	return 0.f;
}

const char* CModelButeMgr::GetModelLoudMovementSnd( ModelId eModelId )
{
	if( eModelId >= 0 && eModelId < m_cModels )
	{
		return m_aModels[eModelId].m_szLoudMovementSnd;
	}

	_ASSERT( !"CModelButeMgr::GetModelLoudMovementSnd: Invalid ModelId" );
	return LTNULL;
}

const char* CModelButeMgr::GetModelQuietMovementSnd( ModelId eModelId )
{
	if( eModelId >= 0 && eModelId < m_cModels )
	{
		return m_aModels[eModelId].m_szQuietMovementSnd;
	}

	_ASSERT( !"CModelButeMgr::GetModelQuietMovementSnd: Invalid ModelId" );
	return LTNULL;
}

uint16 CModelButeMgr::GetModelNameId( ModelId eModelId )
{
	if( eModelId >= 0 && eModelId < m_cModels )
	{
		return m_aModels[eModelId].m_nNameId;
	}

	_ASSERT( !"CModelButeMgr::GetModelNameId: Invalid ModelId" );
	return 0;
}

const char* CModelButeMgr::GetModelPlayerPainSndDir( ModelId eModelId )
{
	if( eModelId >= 0 && eModelId < m_cModels )
	{
		return m_aModels[eModelId].m_szPlayerPainSoundDir;
	}

	_ASSERT( !"CModelButeMgr::GetModelLoudMovementSnd: Invalid ModelId" );
	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetSkeleton...
//
//	PURPOSE:	Various skeleton attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetSkeletonNumNodes(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0;

	return m_aSkeletons[eModelSkeleton].m_cNodes;
}

ModelNode CModelButeMgr::GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	for ( int iNode = 0 ; iNode < m_aSkeletons[eModelSkeleton].m_cNodes ; iNode++ )
	{
        if ( !_stricmp(m_aSkeletons[eModelSkeleton].m_aNodes[iNode].m_szName, szName) )
		{
			return (ModelNode)iNode;
		}
	}

	return eModelNodeInvalid;
}

const char* CModelButeMgr::GetSkeletonDefaultFrontDeathAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultFrontDeathAni;
}

const char* CModelButeMgr::GetSkeletonDefaultBackDeathAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultBackDeathAni;
}

const char* CModelButeMgr::GetSkeletonDefaultFrontShortRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultFrontShortRecoilAni;
}

const char* CModelButeMgr::GetSkeletonDefaultBackShortRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultBackShortRecoilAni;
}

ModelNode CModelButeMgr::GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_eModelNodeDefaultHit;
}

const char* CModelButeMgr::GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szName;
}

uint32 CModelButeMgr::GetSkeletonNodeFlags(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_dwFlags;
}

const char* CModelButeMgr::GetSkeletonNodeFrontDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szFrontDeathAni;
}

const char* CModelButeMgr::GetSkeletonNodeBackDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szBackDeathAni;
}

const char* CModelButeMgr::GetSkeletonNodeFrontShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szFrontShortRecoilAni;
}

const char* CModelButeMgr::GetSkeletonNodeBackShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szBackShortRecoilAni;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fDamageFactor;
}

ModelNode CModelButeMgr::GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eModelNodeParent;
}

HitLocation CModelButeMgr::GetSkeletonNodeLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return HL_UNKNOWN;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eHitLocation;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fHitRadius;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeHitPriority(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fHitPriority;
}

bool CModelButeMgr::GetSkeletonNodeAttachSpears( ModelSkeleton eModelSkeleton, ModelNode eModelNode )
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_bAttachSpears;
}

ModelTrackingNodeGroup CModelButeMgr::GetSkeletonTrackingNodesLookAt(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	return m_aSkeletons[eModelSkeleton].m_eModelTrackingNodesLookAt;
}

ModelTrackingNodeGroup CModelButeMgr::GetSkeletonTrackingNodesAimAt(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	return m_aSkeletons[eModelSkeleton].m_eModelTrackingNodesAimAt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetTrackingNode...
//
//	PURPOSE:	Various tracking node group attribute lookups
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::IsTrackingNodeGroupValid(ModelTrackingNodeGroup eModelTrackingNodeGroup)
{
	if(eModelTrackingNodeGroup < 0 || eModelTrackingNodeGroup >= m_cTrackingNodeGroups)
	{
		UBER_ASSERT( 0, "CModelButeMgr::IsTrackingNodeGroupValid: TrackingNodeGroup index out of range." );
		return LTFALSE;
	}
	return LTTRUE;
}

LTBOOL CModelButeMgr::IsTrackingNodeValid(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( eModelTrackingNode < 0 || eModelTrackingNode >= m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_cTrackingNodes )
	{
		UBER_ASSERT( 0, "CModelButeMgr::IsTrackingNodeValid: TrackingNode index out of range." );
		return LTFALSE;
	}
	return LTTRUE;
}

int	CModelButeMgr::GetNumTrackingNodes(ModelTrackingNodeGroup eModelTrackingNodeGroup)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_cTrackingNodes;
	}
	return 0;
}

int	CModelButeMgr::GetNumClonedTrackingNodes(ModelTrackingNodeGroup eModelTrackingNodeGroup)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_cClonedTrackingNodes;
	}
	return 0;
}

const char*	CModelButeMgr::GetTrackingNodeName(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_szName;
	}
	return LTNULL;
}

const char*	CModelButeMgr::GetTrackingNodeClonedName(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_szClonedName;
	}
	return LTNULL;
}

void CModelButeMgr::GetTrackingNodeForward(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode, LTVector* pvForward)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		*pvForward = m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_vForward;
	}
}

void CModelButeMgr::GetTrackingNodeUp(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode, LTVector* pvUp)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		*pvUp = m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_vUp;
	}
}

const LTBOOL CModelButeMgr::GetTrackingNodeAxesSpecified(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_bAxesSpecified;
	}
	return LTFALSE;
}

const LTFLOAT CModelButeMgr::GetTrackingNodeDiscomfortAngleX(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_fDiscomfortAngleX;
	}
	return 0.f;
}

const LTFLOAT CModelButeMgr::GetTrackingNodeDiscomfortAngleY(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_fDiscomfortAngleY;
	}
	return 0.f;
}

const LTFLOAT CModelButeMgr::GetTrackingNodeMaxAngleX(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_fMaxAngleX;
	}
	return 0.f;
}

const LTFLOAT CModelButeMgr::GetTrackingNodeMaxAngleY(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_fMaxAngleY;
	}
	return 0.f;
}

const LTFLOAT CModelButeMgr::GetTrackingNodeMaxVelocity(ModelTrackingNodeGroup eModelTrackingNodeGroup, ModelTrackingNode eModelTrackingNode)
{
	if( IsTrackingNodeGroupValid( eModelTrackingNodeGroup ) && IsTrackingNodeValid( eModelTrackingNodeGroup, eModelTrackingNode ) )
	{
		return m_aTrackingNodeGroups[eModelTrackingNodeGroup].m_aTrackingNodes[eModelTrackingNode].m_fMaxVelocity;
	}
	return 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetNScript...
//
//	PURPOSE:	Various node script attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetNScriptNumPts(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_cNScriptPts;
}

const char* CModelButeMgr::GetNScriptNodeName(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_szName;
}

uint8 CModelButeMgr::GetNScriptFlags(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_bFlags;
}

LTFLOAT CModelButeMgr::GetNScriptPtTime(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_fTime;
}

const LTVector& CModelButeMgr::GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vPosOffset;
}

const LTVector& CModelButeMgr::GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vRotOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Get*Filename
//
//	PURPOSE:	Filename generating functions
//
// ----------------------------------------------------------------------- //

const char* CModelButeMgr::GetModelFilename(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szModelFile;
	}

	return LTNULL;
}

uint8 CModelButeMgr::GetNumSkins(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrSkinReader.GetNumItems();
	}

	return 0;
}

const char* CModelButeMgr::GetSkinFilename(ModelId eModelId, uint8 iSkin)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(iSkin < m_aModels[eModelId].m_blrSkinReader.GetNumItems());
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrSkinReader.GetItem(iSkin);
	}

	return LTNULL;
}

void CModelButeMgr::CopySkinFilenames(ModelId eModelId, uint8 iStart, char* paszDest, int strLen)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		m_aModels[eModelId].m_blrSkinReader.CopyList(iStart, paszDest, strLen);
	}
}

CButeListReader* CModelButeMgr::GetSkinReader(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return &(m_aModels[eModelId].m_blrSkinReader); 
	}

	return LTNULL;
}

CButeListReader* CModelButeMgr::GetRenderStyleReader(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return &(m_aModels[eModelId].m_blrRenderStyleReader); 
	}

	return LTNULL;
}


const char*	CModelButeMgr::GetHandsSkinFilename(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szHandsSkin;
	}

	return LTNULL;
}

const char* CModelButeMgr::GetMultiModelFilename(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_szModelFile;
	}

	return LTNULL;
}

void CModelButeMgr::CopyRenderStyleFilenames(ModelId eModelId, ObjectCreateStruct* pCreateStruct)
{
	ASSERT(eModelId >= 0 && eModelId < m_cModels);
	ASSERT(pCreateStruct);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		m_aModels[eModelId].m_blrRenderStyleReader.CopyList(0, pCreateStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);
	}
}
uint8 CModelButeMgr::GetNumClientFX(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrClientFXReader.GetNumItems();
	}

	return 0;
}

const char* CModelButeMgr::GetClientFX(ModelId eModelId, uint8 iClientFX)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(iClientFX < m_aModels[eModelId].m_blrClientFXReader.GetNumItems());
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrClientFXReader.GetItem(iClientFX);
	}

	return LTNULL;
}

void CModelButeMgr::CopyClientFX(ModelId eModelId, uint8 iStart, char* paszDest, int strLen)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		m_aModels[eModelId].m_blrClientFXReader.CopyList(iStart, paszDest, strLen);
	}
}

CButeListReader* CModelButeMgr::GetClientFXReader(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return &(m_aModels[eModelId].m_blrClientFXReader); 
	}

	return LTNULL;
}

uint8 CModelButeMgr::GetNumAltHeadSkins( ModelId eModelId )
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrAltHeadSkin.GetNumItems();
	}

	return 0;
}

const char* CModelButeMgr::GetAltHeadSkin( ModelId eModelId, uint8 iSkin )
{
	_ASSERT( eModelId >= 0 && eModelId < m_cModels );
	_ASSERT( iSkin < m_aModels[eModelId].m_blrAltHeadSkin.GetNumItems() );
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrAltHeadSkin.GetItem(iSkin);
	}

	return LTNULL;
}

uint8 CModelButeMgr::GetNumAltBodySkins( ModelId eModelId )
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrAltBodySkin.GetNumItems();
	}

	return 0;
}

const char* CModelButeMgr::GetAltBodySkin( ModelId eModelId, uint8 iSkin )
{
	_ASSERT( eModelId >= 0 && eModelId < m_cModels );
	_ASSERT( iSkin < m_aModels[eModelId].m_blrAltBodySkin.GetNumItems() );
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_blrAltBodySkin.GetItem(iSkin);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::IsModelAIOnly
//
//	PURPOSE:	Flags if the model is only for AI, or if it can be used
//				as a GameStartPoint.
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::IsModelAIOnly(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_bAIOnly;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::CanModelBeCarried
//
//	PURPOSE:	Flags if the body associated with this model can be carried.
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::CanModelBeCarried(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_bCanBeCarried;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::AIIgnoreBody
//
//	PURPOSE:	Flags if the body associated with this model should not register a stimulus.
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::AIIgnoreBody(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_bAIIgnoreBody;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::IsModelTranslucent
//
//	PURPOSE:	determines if the specified model is translucent or not
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::IsModelTranslucent(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_bTranslucent;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetArmorSurfaceType
//
//	PURPOSE:	Gets the surface type of this model when it is using armor
//
// ----------------------------------------------------------------------- //

SurfaceType CModelButeMgr::GetArmorSurfaceType(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_eArmorSurfaceType;
	}

	return ST_UNKNOWN;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetFleshSurfaceType
//
//	PURPOSE:	Gets the surface type of this model when it is not using armor
//
// ----------------------------------------------------------------------- //

SurfaceType CModelButeMgr::GetFleshSurfaceType(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_eFleshSurfaceType;
	}

	return ST_UNKNOWN;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Get*DefaultAttachment
//
//	PURPOSE:	DefaultAttachment functions
//
// ----------------------------------------------------------------------- //

uint8 CModelButeMgr::GetNumDefaultAttachments(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_lstDefaultAttachments.size();
	}

	return 0;
}

void CModelButeMgr::GetDefaultAttachment(ModelId eModelId, uint8 iAttachment, const char*& pszAttachmentPosition, const char*& pszAttachment)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		pszAttachmentPosition = m_aModels[eModelId].m_lstDefaultAttachments[iAttachment].szAttachmentPosition;
		pszAttachment = m_aModels[eModelId].m_lstDefaultAttachments[iAttachment].szAttachment;
	}
}

uint8 CModelButeMgr::GetNumPlayerViewAttachments( ModelId eModelId )
{
	_ASSERT( eModelId >= 0 && eModelId < m_cModels );
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_lstPVAttachments.size();
	}

	return 0;
}

void CModelButeMgr::GetPlayerViewAttachment( ModelId eModelId, uint8 iPVAttachment, const char*& pszPVAttachmentPosition, const char*& pszPVAttachment )
{
	_ASSERT( eModelId >= 0 && eModelId < m_cModels );
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		pszPVAttachmentPosition = m_aModels[eModelId].m_lstPVAttachments[iPVAttachment].szPVAttachmentPosition;
		pszPVAttachment = m_aModels[eModelId].m_lstPVAttachments[iPVAttachment].szPVAttachment;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetUnalertDamageFactor
//
//	PURPOSE:	Get the AI unalert damage modifier for this model
//
// ----------------------------------------------------------------------- //

LTFLOAT CModelButeMgr::GetUnalertDamageFactor( ModelId eModelId )
{
	_ASSERT( eModelId >= 0 && eModelId < m_cModels );
	if(eModelId >= 0 && eModelId < m_cModels)
	{
		return m_aModels[eModelId].m_fUnalertDamageFactor;
	}

	return 0.0f;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetCPModel
//
//	PURPOSE:	Co-op model id lookup
//
// ----------------------------------------------------------------------- //

ModelId CModelButeMgr::GetCPModel(int num)
{
	ASSERT(m_aCPModels);
	if (!m_aCPModels) return eModelIdInvalid;

	if (num >= m_cCPModels)
	{
		ASSERT("Error in CModelButeMgr::GetCPModel() : Invalid CPModel index");
		num = 0;
	}
	return m_aCPModels[num];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetDMModel
//
//	PURPOSE:	Deathmatch model id lookup
//
// ----------------------------------------------------------------------- //

ModelId CModelButeMgr::GetDMModel(int num)
{
	ASSERT(m_aDMModels);
	if (!m_aDMModels) return eModelIdInvalid;

	if (num >= m_cDMModels)
	{
		ASSERT("Error in CModelButeMgr::GetDMModel() : Invalid DMModel index");
		num = 0;
	}
	return m_aDMModels[num];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetTeamModel
//
//	PURPOSE:	Team model id lookup
//
// ----------------------------------------------------------------------- //

ModelId CModelButeMgr::GetTeamModel(int num)
{
	ASSERT(m_aTeamModels);
	if (!m_aTeamModels) return eModelIdInvalid;

	if (num >= m_cTeamModels)
	{
		ASSERT("Error in CModelButeMgr::GetTeamModel() : Invalid TeamModel index");
		num = 0;
	}
	return m_aTeamModels[num];
}

int CModelButeMgr::GetTeamDefaultModel(int nTeam)
{
	if (nTeam >= 2)
	{
		ASSERT("Error in CModelButeMgr::GetDefaultTeamDMModel() : Invalid team index");
		nTeam = 0;
	}
	return m_aTeamDefaults[nTeam];
}



////////////////////////////////////////////////////////////////////////////
//
// CModelButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD  // Server-side only
#ifndef __PSX2

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Initialize the bute mgr...
//
// ----------------------------------------------------------------------- //

LTRESULT CModelButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if (!sm_bInitted)
	{
		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, MBMGR_DEFAULT_FILE);

		// Get the singleton instance of the weaponmgr.
		CModelButeMgr& modelButeMgr = CModelButeMgr::Instance( );
        modelButeMgr.SetInRezFile(LTFALSE);
        modelButeMgr.Init(szFile);
        sm_bInitted = LTTRUE;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Initialize the bute mgr...
//
// ----------------------------------------------------------------------- //

void CModelButeMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;
}

#endif // !__PSX2
#endif // _CLIENTBUILD