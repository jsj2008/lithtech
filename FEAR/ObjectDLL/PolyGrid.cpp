// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.cpp
//
// PURPOSE : PolyGrid - Implementation
//
// CREATED : 10/20/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PolyGrid.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "CommonUtilities.h"
#include "PolyGridModifier.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"
#include "iobjectresourcegatherer.h"
#include "CollisionsDB.h"

LINKFROM_MODULE( PolyGrid );

//plugin class that can be used by derived lighting classes that will fill in the LOD strings
class PolyGrid_Plugin: 
	public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(
		const char* /*szRezPath*/,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength);
};

//the prefetching function for the polygrid
static void PolyGridPrefetch(const char* pszObjectName, IObjectResourceGatherer* pInterface);

BEGIN_CLASS(PolyGrid)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS | PF_LOCALDIMS, "Determines the size of the grid. Polygrids normally face up (Y axis), so a typical polygrid Dims setting would be 256 8 256, for a thin grid that fills a large hole in a floor.")
    ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f, "Specifies the color of the polygrid")
    ADD_STRINGPROP_FLAG(Material, "", PF_FILENAME, "The material that will be used to render this polygrid")

	PROP_DEFINEGROUP(TextureInfo, PF_GROUP(3), "Information describing how the texturing is applied to the polygrid surface")
		ADD_REALPROP_FLAG(XScaleMin, 100.0f, PF_GROUP(3), "Specifies the minimum scale that the texture used on the PolyGrid will scale to. This allows you to have the texture on the PolyGrid swim up and down in size. The number represents the number of units over which a texture will repeat")
		ADD_REALPROP_FLAG(XScaleMax, 100.0f, PF_GROUP(3), "See XScaleMin.")
		ADD_REALPROP_FLAG(YScaleMin, 100.0f, PF_GROUP(3), "Specifies the maximum texture scale on the PolyGrid. See XScaleMin.")
		ADD_REALPROP_FLAG(YScaleMax, 100.0f, PF_GROUP(3), "See XscaleMax.")
	    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_GROUP(3), "Determines the length of time the PolyGrid will take to scale between XScaleMin and XScaleMax.")
	    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_GROUP(3), "See XScaleDuration.")
	    ADD_REALPROP_FLAG(XPan, 10.0f, PF_GROUP(3), "If the PolyGrid is textured, you can enter a panning speed here to have the texture pan along the PolyGrid's X-axis. Speed is in game units/second. Typical values would be from –20 to 20 for fairly fast speed and –5 to 5 for slow.")
	    ADD_REALPROP_FLAG(YPan, 10.0f, PF_GROUP(3), "See XPan.")

	ADD_REALPROP(Alpha, 0.7f, "The translucency of the grid as a whole. If you want to use a texture without an alpha mask or fine-tune the alpha for a texture that does have an alpha mask, this is a useful property. A value of 1 is fully opaque, .5 is half-translucent and 0 is transparent.")
	ADD_BOOLPROP(RenderSolid, FALSE, "Determines if this polygrid should be rendered as a solid or translucent object")
	ADD_LONGINTPROP(NumPoliesX, 16, "The number of segments the polygrid will have along the X direction. The higher the number of polies you add, the smoother the grid will look, but it will also take longer to render.")
	ADD_LONGINTPROP(NumPoliesY, 16, "The number of segments the polygrid will have along the Y direction. The higher the number of polies you add, the smoother the grid will look, but it will also take longer to render.")
	ADD_REALPROP(MinResolutionScale, 0.0f, "A value in the range of [0..1] that indicates the minimum scale that can be used for the resolution of the polygrid. 0 allows it to go down to 2x2, 1 prevents the resolution from changing with LOD, 0.5 allows dropping of 50% resolution at lowest LOD, etc.")
		
	PROP_DEFINEGROUP(WavePropInfo, PF_GROUP(2), "Properties that control how the surface of the water moves")
		ADD_REALPROP_FLAG(TimeScale, 1.0f, PF_GROUP(2), "A multiplier used to speed up time on the surface which causes waves to move up and down faster.")
		ADD_REALPROP_FLAG(DampenScale, 0.99f, PF_GROUP(2), "A multiplier used to remove energy from the system. So if it was .95, it would use 95% of the previous energy, allowing 5% to dissappear, this prevents water simulations from exploding.")
		ADD_REALPROP_FLAG(SpringCoeff, 40.0f, PF_GROUP(2), "Coefficient for the rigidness of the water springs. Raising this value will cause the water to move back into place faster, and lower will make it look more viscous.")		
		ADD_REALPROP_FLAG(ModelDisplace, 10.0f, PF_GROUP(2), "Specifies how much a model moving through the surface should displace. Higher values will create higher waves.")
		ADD_REALPROP_FLAG(MinFrameRate, 10.0f, PF_GROUP(2), "The minimum frame rate that the polygrid can run at. If the frame rate drops below this the polygrid will just pretent it is running at this frame rate")
		ADD_LONGINTPROP_FLAG(NumStartupFrames, 0, PF_GROUP(2), "This indicates how many frames will be simulated when the polygrid is created. This helps prevent a completely flat surface when a level is loaded")
		ADD_STRINGPROP_FLAG(Modifier1, "", PF_OBJECTLINK | PF_GROUP(2), "Name of a modifier that will affect the surface.")
		ADD_STRINGPROP_FLAG(Modifier2, "", PF_OBJECTLINK | PF_GROUP(2), "Name of a modifier that will affect the surface.")
		ADD_STRINGPROP_FLAG(Modifier3, "", PF_OBJECTLINK | PF_GROUP(2), "Name of a modifier that will affect the surface.")
		ADD_STRINGPROP_FLAG(Modifier4, "", PF_OBJECTLINK | PF_GROUP(2), "Name of a modifier that will affect the surface.")

	PROP_DEFINEGROUP(RenderTargets, PF_GROUP(1), "Properties that control automatically creating render targets")
		ADD_BOOLPROP_FLAG(ReflectionMap, false, PF_GROUP(1), "Create a reflection map render target, bound to the tReflectionMap parameter of the material")
		ADD_STRINGPROP_FLAG(ReflectionMapLOD, "Low", PF_STATICLIST | PF_GROUP(1), "Indicates at which this reflection map LOD. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on. Never disables this map.")
		ADD_STRINGPROP_FLAG(ReflectionMapRTGroup, "", PF_OBJECTLINK | PF_GROUP(1), "The render target group that the reflection map should use. Note that this should not be the same as the refraction map, and is not needed if the reflection is disabled")
		
		ADD_BOOLPROP_FLAG(RefractionMap, false, PF_GROUP(1), "Create a refraction map render target, bound to the tRefractionMap parameter of the material")
		ADD_STRINGPROP_FLAG(RefractionMapLOD, "Low", PF_STATICLIST | PF_GROUP(1), "Indicates at which this refraction map LOD. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on. Never disables this map.")
		ADD_STRINGPROP_FLAG(RefractionMapRTGroup, "", PF_OBJECTLINK | PF_GROUP(1), "The render target group that the refraction map should use. Note that this should not be the same as the reflection map, and is not needed if the refraction is disabled")

	ADD_BOOLPROP(EnableCollisions, TRUE, "Enables collisions with physics objects other than models")
	ADD_STRINGPROP_FLAG(CollisionProperty,	StringList_None, PF_STATICLIST,	"This allows the selection of an overriding CollisionProperty.")

END_CLASS_FLAGS_PLUGIN_PREFETCH(PolyGrid, GameBase, 0, PolyGrid_Plugin, PolyGridPrefetch, "Class that simulates the surface of a body of water.  If used with a liquid volume brush make sure the top of the volume brush is at the same Y value as the polygrid object if you want bullet vectors to perturb the surface of the water.")


//the prefetching function for the polygrid
void PolyGridPrefetch(const char* pszObjectName, IObjectResourceGatherer* pInterface)
{
	char pszMaterial[MAX_PATH + 1];
	if(!pInterface->GetPropString(pszObjectName, "Material", pszMaterial, LTARRAYSIZE(pszMaterial), ""))
		return;

	//now add this material to the overlapping volume of space
	LTRigidTransform tObjTrans = pInterface->GetTransform(pszObjectName);
	LTVector vHalfDims = pInterface->GetPropVector(pszObjectName, "Dims", LTVector::GetIdentity());

	pInterface->AddResourceToOBB(pszMaterial, tObjTrans, vHalfDims);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMsgModifier
//
//  PURPOSE:	Validate the color message...
//
// ----------------------------------------------------------------------- //

static bool ValidateMsgModifier( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs != 2 )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateMsgModifier()" );
		pInterface->CPrint( "    MSG - Invalid number of arguments." );
		return false;
	}
	//make sure that the second parameter is a value in range
	uint32 nVal = (uint32)(atoi(cpMsgParams.m_Args[1]) - 1);
	if(nVal >= PolyGrid::MAX_MODIFIERS)
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - Polygrid modifier index is too large" );
	}

	return true;
}

CMDMGR_BEGIN_REGISTER_CLASS( PolyGrid )

	ADD_MESSAGE( ON,	2,		ValidateMsgModifier,	MSG_HANDLER( PolyGrid, HandleOnMsg ),	"ON <Modifier>", "Turns a PolyGridModifier object specified in the WavePropInfo property of the PolyGrid on. The command is targeted at the Modifier number", "To turn on the PolyGridModifier specified in the Modifier1 property field of a PolyGrid named \"PolyGrid\" the command would look like:<BR><BR>msg PolyGrid (ON 1)" ) 
	ADD_MESSAGE( OFF,	2,		ValidateMsgModifier,	MSG_HANDLER( PolyGrid, HandleOffMsg ),	"OFF <Modifier>", "Turns a PolyGridModifier object specified in the WavePropInfo property of the PolyGrid off. The command is targeted at the Modifier number", "To turn off the PolyGridModifier specified in the Modifier1 property field of a PolyGrid named \"PolyGrid\" the command would look like:<BR><BR>msg PolyGrid (OFF 1)" ) 

CMDMGR_END_REGISTER_CLASS( PolyGrid, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::PolyGrid
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

PolyGrid::PolyGrid() : GameBase( OT_NORMAL )
{
	//non-saved data
	m_bCreatedFromSave = false;

	m_bReflectionMap			= false;
	m_bRefractionMap			= false;

	//saved data

	m_vDims = LTVector(32.0f, 32.0f, 0.0f);
	m_vColor = LTVector(255.0f, 255.0f, 255.0f);

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 15.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 15.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
	m_fXPan = 10.0f;
	m_fYPan = 10.0f;
	m_fAlpha = 0.7f;
	m_bRenderSolid = false;

	m_bEnableCollisions = true;
	
	m_dwNumPoliesX = 16;
	m_dwNumPoliesY = 16;

	m_nNumStartupFrames = 0;

	//wave prop info
	m_fDampenScale = 0.99f;
	m_fTimeScale = 1.0f;
	m_fSpringCoeff = 40.0f;
	m_fModelDisplace = 10.0f;
	m_fMinFrameRate = 10.0f;
	m_fMinResolutionScale = 0.0f;

	//init each modifier
	m_nActiveModifiers = 0;

	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_nNumAccelPoints[nCurrMod]		= 0;
		m_fAccelAmount[nCurrMod]		= 0.0f;
		m_fXMin[nCurrMod]				= 0.0f;
		m_fYMin[nCurrMod]				= 0.0f;
		m_fXMax[nCurrMod]				= 0.0f;
		m_fYMax[nCurrMod]				= 0.0f;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::~PolyGrid
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

PolyGrid::~PolyGrid()
{
	FreeModifierStrings();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::CreateSFXMessage
//
//	PURPOSE:	given a message, this will fill it out with the polygrid data
//
// ----------------------------------------------------------------------- //
void PolyGrid::CreateSFXMessage(ILTMessage_Write& cMsg)
{
	uint16 wColor = Color255VectorToWord( &m_vColor );

	cMsg.WriteLTVector(m_vDims);
	cMsg.Writeuint16(wColor);
	cMsg.Writefloat(m_fXScaleMin);
	cMsg.Writefloat(m_fXScaleMax);
	cMsg.Writefloat(m_fYScaleMin);
	cMsg.Writefloat(m_fYScaleMax);
	cMsg.Writefloat(m_fXScaleDuration);
	cMsg.Writefloat(m_fYScaleDuration);
	cMsg.Writefloat(m_fXPan);
	cMsg.Writefloat(m_fYPan);
	cMsg.Writefloat(m_fAlpha);
	cMsg.Writefloat(m_fTimeScale);
	cMsg.Writefloat(m_fDampenScale);
	cMsg.Writefloat(m_fSpringCoeff);
	cMsg.Writefloat(m_fModelDisplace);
	cMsg.Writefloat(m_fMinFrameRate);
	cMsg.Writefloat(m_fMinResolutionScale);
	cMsg.WriteString(m_sMaterial.c_str( ));
	cMsg.Writeuint16((uint16)m_dwNumPoliesX);
	cMsg.Writeuint16((uint16)m_dwNumPoliesY);
	cMsg.Writeuint16((uint16)m_nNumStartupFrames);
	cMsg.Writebool(m_bRenderSolid);
	cMsg.Writebool(m_bEnableCollisions);

	//write out our modifier data
	cMsg.Writeuint8(m_nActiveModifiers);
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		cMsg.Writefloat(m_fAccelAmount[nCurrMod]);
		cMsg.Writeuint16(m_nNumAccelPoints[nCurrMod]);
		cMsg.Writefloat(m_fXMin[nCurrMod]);
		cMsg.Writefloat(m_fYMin[nCurrMod]);
		cMsg.Writefloat(m_fXMax[nCurrMod]);
		cMsg.Writefloat(m_fYMax[nCurrMod]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::InitialUpdate
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void PolyGrid::InitialUpdate( const GenericPropList* pProps )
{
	if ((m_dwNumPoliesX < 2) || (m_dwNumPoliesY < 2)) 
		return;

	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);

	// Set the collisionproperty override.
	char const* pszCollisionProperty = pProps->GetString( "CollisionProperty", "" );
	if( !LTStrEmpty( pszCollisionProperty ))
	{
		HRECORD hRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( CollisionProperty ).GetCategory(), pszCollisionProperty );
		if( hRecord )
		{
			uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hRecord );
			g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );
		}
	}

	// Limit the number of polies in each direciton to 32k...
	if ( m_dwNumPoliesX > 0x7FFF )
		m_dwNumPoliesX = 0x7FFF;

	if ( m_dwNumPoliesY > 0x7FFF )
		m_dwNumPoliesY = 0x7FFF;

	// Create the RenderTarget objects
	if (m_bReflectionMap || m_bRefractionMap)
	{
		bool bIsAttached;
		g_pLTServer->IsObjectAttached(m_hObject, bIsAttached);
		if (bIsAttached)
		{
			g_pLTServer->CPrint("Warning initializing PolyGrid: RenderTarget detected on attached PolyGrid.  The RenderTarget may not move with the PolyGrid.");
		}
		
		HCLASS hRenderTargetClass = g_pLTServer->GetClass("RenderTarget");
		if ( hRenderTargetClass == NULL )
		{
			LTERROR("Error initializing PolyGrid: Unable to find RenderTarget class");
			g_pLTServer->CPrint("Error initializing PolyGrid: Unable to find RenderTarget class");
		}
		else
		{
			// Set up the parameters that are the same between both RenderTargets
			ObjectCreateStruct ocs;
			ocs.m_cProperties.ReserveProps(10, false);
			ocs.m_cProperties.AddProp("FrameLatency", GenericProp(0, LT_PT_LONGINT));
			ocs.m_cProperties.AddProp("Mirror", GenericProp(true, LT_PT_BOOL));
			ocs.m_cProperties.AddProp("Refraction", GenericProp(true, LT_PT_BOOL));
			ocs.m_cProperties.AddProp("RefractionClipPlaneBias", GenericProp(m_vDims.y, LT_PT_REAL));
			ocs.m_cProperties.AddProp("Material", GenericProp(m_sMaterial.c_str(), LT_PT_STRING));
			LTRigidTransform tObjTrans;
			g_pLTServer->GetObjectTransform(m_hObject, &tObjTrans);
			ocs.m_Pos = tObjTrans.m_vPos;
			char aObjName[MAX_PATH];
			g_pLTServer->GetObjectName(m_hObject, aObjName, LTARRAYSIZE(aObjName));
			// Set up the reflection map
			if ( m_bReflectionMap )
			{
				ocs.m_cProperties.AddProp("RenderTargetGroup", GenericProp(m_sReflectionRTGroup.c_str(), LT_PT_STRING));
				ocs.m_cProperties.AddProp("Parameter", GenericProp("tReflectionMap", LT_PT_STRING));
				ocs.m_cProperties.AddProp("LOD", GenericProp(m_sReflectionMapLOD.c_str(), LT_PT_STRING));
				ocs.m_Rotation = LTRotation(tObjTrans.m_rRot.Up(), tObjTrans.m_rRot.Forward());
				char aName[MAX_PATH];
				LTSNPrintF(aName, LTARRAYSIZE(aName), "%s.Reflection", aObjName);
				ocs.SetName(aName);
				ILTBaseClass *pResult = g_pLTServer->CreateObject(hRenderTargetClass, &ocs);
				if (pResult == NULL)
				{
					LTERROR("Error initializing PolyGrid: Unable to create reflection map RenderTarget object");
					g_pLTServer->CPrint("Error initializing PolyGrid: Unable to create reflection map RenderTarget object");
				}
				else
				{
					HATTACHMENT hAttachment;
					LTRigidTransform tRelativeTrans = tObjTrans.GetDifference(LTRigidTransform(ocs.m_Pos, ocs.m_Rotation));
					g_pLTServer->CreateAttachment(m_hObject, pResult->GetHOBJECT(), NULL, &tRelativeTrans.m_vPos, &tRelativeTrans.m_rRot, &hAttachment);
				}
			}
			// Set up the refraction map
			if ( m_bRefractionMap )
			{
				ocs.m_cProperties.AddProp("RenderTargetGroup", GenericProp(m_sRefractionRTGroup.c_str(), LT_PT_STRING));
				ocs.m_cProperties.AddProp("Parameter", GenericProp("tRefractionMap", LT_PT_STRING));
				ocs.m_cProperties.AddProp("LOD", GenericProp(m_sRefractionMapLOD.c_str(), LT_PT_STRING));
				ocs.m_Rotation = LTRotation(-tObjTrans.m_rRot.Up(), tObjTrans.m_rRot.Forward());
				char aName[MAX_PATH];
				LTSNPrintF(aName, LTARRAYSIZE(aName), "%s.Refraction", aObjName);
				ocs.SetName(aName);
				ILTBaseClass *pResult = g_pLTServer->CreateObject(hRenderTargetClass, &ocs);
				if (pResult == NULL)
				{
					LTERROR("Error initializing PolyGrid: Unable to create refraction map RenderTarget object");
					g_pLTServer->CPrint("Error initializing PolyGrid: Unable to create refraction map RenderTarget object");
				}
				else
				{
					HATTACHMENT hAttachment;
					LTRigidTransform tRelativeTrans = tObjTrans.GetDifference(LTRigidTransform(ocs.m_Pos, ocs.m_Rotation));
					g_pLTServer->CreateAttachment(m_hObject, pResult->GetHOBJECT(), NULL, &tRelativeTrans.m_vPos, &tRelativeTrans.m_rRot, &hAttachment);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PolyGrid::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
				ReadProp(&pStruct->m_cProperties);
				PostReadProp(pStruct);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(( GenericPropList* )pData );
			}
			else
			{
				m_bCreatedFromSave = true;
			}
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

		case MID_ALLOBJECTSCREATED:
		{
			if(!m_bCreatedFromSave)
			{
				SetupModifiers();
				UpdateClients();
			}

			// Don't eat ticks please...
			SetNextUpdate(UPDATE_NEVER);

			break;
		}


		case MID_UPDATE:
		{
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PolyGrid::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return;

	m_sMaterial				= pProps->GetString( "Material", "" );
	
	m_vDims					= pProps->GetVector("Dims", m_vDims);
	m_vColor				= pProps->GetColor("Color", m_vColor);
	m_fXScaleMin			= pProps->GetReal("XScaleMin", m_fXScaleMin);
	m_fXScaleMax			= pProps->GetReal("XScaleMax", m_fXScaleMax);
	m_fYScaleMin			= pProps->GetReal("YScaleMin", m_fYScaleMin);
	m_fYScaleMax			= pProps->GetReal("YScaleMax", m_fYScaleMax);
	m_fXScaleDuration		= pProps->GetReal("XScaleDuration", m_fXScaleDuration);
	m_fYScaleDuration		= pProps->GetReal("YScaleDuration", m_fYScaleDuration);
	m_fXPan					= pProps->GetReal("XPan", m_fXPan);
	m_fYPan					= pProps->GetReal("YPan", m_fYPan);
	m_fAlpha				= pProps->GetReal("Alpha", m_fAlpha);
	m_fTimeScale			= pProps->GetReal("TimeScale", m_fTimeScale);
	m_fDampenScale			= pProps->GetReal("DampenScale", m_fDampenScale);
	m_fSpringCoeff			= pProps->GetReal("SpringCoeff", m_fSpringCoeff);
	m_fModelDisplace		= pProps->GetReal("ModelDisplace", m_fModelDisplace);
	m_fMinFrameRate			= pProps->GetReal("MinFrameRate", m_fMinFrameRate);
	m_fMinResolutionScale	= LTCLAMP(pProps->GetReal("MinResolutionScale", m_fMinResolutionScale), 0.0f, 1.0f);
	m_bRenderSolid			= pProps->GetBool("RenderSolid", m_bRenderSolid);
	m_dwNumPoliesX			= (uint32)pProps->GetLongInt("NumPoliesX", 16);
	m_dwNumPoliesY			= (uint32)pProps->GetLongInt("NumPoliesY", 16);
	m_nNumStartupFrames		= (uint32)pProps->GetLongInt("NumStartupFrames", 0);
	m_bEnableCollisions		= pProps->GetBool("EnableCollisions", m_bEnableCollisions);

	m_bReflectionMap			= pProps->GetBool("ReflectionMap", m_bReflectionMap);
	m_sReflectionMapLOD			= pProps->GetString("ReflectionMapLOD", "");
	m_sReflectionRTGroup		= pProps->GetString("ReflectionMapRTGroup", "");

	m_bRefractionMap			= pProps->GetBool("RefractionMap", m_bRefractionMap);
	m_sRefractionMapLOD			= pProps->GetString("RefractionMapLOD", "");
	m_sRefractionRTGroup		= pProps->GetString("RefractionMapRTGroup", "");

	//read in all the modifier names
	for(uint32 nCurrModifier = 0; nCurrModifier < MAX_MODIFIERS; nCurrModifier++)
	{
		char pszPropName[64];
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Modifier%d", nCurrModifier + 1);

		m_sModifierName[nCurrModifier] = pProps->GetString(pszPropName, "");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void PolyGrid::PostReadProp(ObjectCreateStruct *pStruct)
{
	pStruct->m_Flags = FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::SetupModifiers
//
//	PURPOSE:	with all the object names filled out, it will search for the
//				objects and take information from them, filling out its internal
//				information
//
// ----------------------------------------------------------------------- //
void PolyGrid::SetupModifiers()
{
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		//first off see if there is a name
		if(!m_sModifierName[nCurrMod].size( ))
			continue;

		//now grab the object that the name refers to
		ObjArray<HOBJECT, 1> ObjList;
		uint32 nNumFound;

		g_pLTServer->FindNamedObjects(m_sModifierName[nCurrMod].c_str( ), ObjList, &nNumFound);

		//grab the first object if applicable
		HOBJECT hObj = NULL;

		if(nNumFound > 0)
			hObj = ObjList.GetObject(0);

		//and now see if we found an object
		if(hObj == NULL)
			continue;

		//make sure that the object is the correct class
		if(g_pLTServer->GetObjectClass(hObj) != g_pLTServer->GetClass("PolyGridModifier"))
			continue;

		//we now have the object, we need to find the intersection with the polygrid
		PolyGridModifier* pMod = (PolyGridModifier*)g_pLTServer->HandleToObject(hObj);

		//alright, now we need to figure out the area we are projecting onto
		LTVector vPos, vModPos;
		g_pLTServer->GetObjectPos(hObj, &vModPos);
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		//ok, now we need to find the orienation of the polygrid
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		//now get the basis vectors of the object space
		LTVector vRight		= rRot.Right();
		LTVector vForward	= rRot.Forward();

		//now find the point of the object projected onto the plane relative to the
		//object position
		float fX = vRight.Dot(vModPos - vPos) + m_vDims.x;
		float fY = vForward.Dot(vModPos - vPos) + m_vDims.z;

		//now convert this to actual integer ranges
		float fXMin = LTCLAMP((fX - pMod->m_vDims.x) / (m_vDims.x * 2.0f), 0.0f, 1.0f);
		float fYMin = LTCLAMP((fY - pMod->m_vDims.z) / (m_vDims.z * 2.0f), 0.0f, 1.0f);
		float fXMax = LTCLAMP((fX + pMod->m_vDims.x) / (m_vDims.x * 2.0f), 0.0f, 1.0f);
		float fYMax = LTCLAMP((fY + pMod->m_vDims.z) / (m_vDims.z * 2.0f), 0.0f, 1.0f);
		
		//see if we actually have any area!
		if((fXMin >= fXMax) || (fYMin >= fYMax))
			continue;

		//alright, we actually have all the data we need, so we can setup this
		//modifier

		//see if this should be enabled or not
		if(pMod->m_bStartEnabled)
			m_nActiveModifiers |= (1 << nCurrMod);

		//copy over the data
		m_nNumAccelPoints[nCurrMod]		= pMod->m_nNumAccelPoints;
		m_fAccelAmount[nCurrMod]		= pMod->m_fAccelAmount;

		//save the rectangle
		m_fXMin[nCurrMod] = fXMin;
		m_fYMin[nCurrMod] = fYMin;
		m_fXMax[nCurrMod] = fXMax;
		m_fYMax[nCurrMod] = fYMax;

		//move onto our next one
	}

	//we can now free the modifier strings
	FreeModifierStrings();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::FreeModifierStrings
//
//	PURPOSE:	Clear the modifier strings.
//
// ----------------------------------------------------------------------- //
void PolyGrid::FreeModifierStrings()
{
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_sModifierName[nCurrMod].erase( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::UpdateClients
//
//	PURPOSE:	Sends the current TextureFX status to the clients
//
// ----------------------------------------------------------------------- //

void PolyGrid::UpdateClients()
{
	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_POLYGRID_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(m_nActiveModifiers);

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_POLYGRID_ID);
		CreateSFXMessage(cMsg);

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void PolyGrid::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nModifier = (uint32)atoi(crParsedMsg.GetArg(1)) - 1;

	//keep track of the original active list
	uint8 nPrevActive = m_nActiveModifiers;

	if(nModifier < MAX_MODIFIERS)
	{
		m_nActiveModifiers |= (1 << nModifier);
	}

	// Tell the clients about the change
	if(nPrevActive != m_nActiveModifiers)
		UpdateClients();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void PolyGrid::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nModifier = (uint32)atoi(crParsedMsg.GetArg(1)) - 1;

	//keep track of the original active list
	uint8 nPrevActive = m_nActiveModifiers;

	if(nModifier < MAX_MODIFIERS)
	{
		m_nActiveModifiers &= ~(1 << nModifier);
	}

	// Tell the clients about the change
	if(nPrevActive != m_nActiveModifiers)
		UpdateClients();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PolyGrid::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) 
		return;

	CreateSFXMessage(*pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PolyGrid::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_vDims = cMsg.ReadLTVector();
	
	uint16 wColor = cMsg.Readuint16();
	Color255WordToVector(wColor, &(m_vColor));
	
	m_fXScaleMin = cMsg.Readfloat();
	m_fXScaleMax = cMsg.Readfloat();
	m_fYScaleMin = cMsg.Readfloat();
	m_fYScaleMax = cMsg.Readfloat();
	m_fXScaleDuration = cMsg.Readfloat();
	m_fYScaleDuration = cMsg.Readfloat();
	m_fXPan = cMsg.Readfloat();
	m_fYPan = cMsg.Readfloat();
	m_fAlpha = cMsg.Readfloat();
	m_fTimeScale = cMsg.Readfloat();
	m_fDampenScale = cMsg.Readfloat();
	m_fSpringCoeff = cMsg.Readfloat();
	m_fModelDisplace = cMsg.Readfloat();
	m_fMinFrameRate = cMsg.Readfloat();
	m_fMinResolutionScale = cMsg.Readfloat();

	char szString[256];

	cMsg.ReadString( szString, ARRAY_LEN( szString ));
	m_sMaterial = szString;

    m_dwNumPoliesX = cMsg.Readuint16();
	m_dwNumPoliesY = cMsg.Readuint16();
	m_nNumStartupFrames = cMsg.Readuint16();
	m_bRenderSolid = cMsg.Readbool();
	
	m_bEnableCollisions = cMsg.Readbool();

	//write out our modifier data
	m_nActiveModifiers = cMsg.Readuint8();
	for(uint32 nCurrMod = 0; nCurrMod < MAX_MODIFIERS; nCurrMod++)
	{
		m_fAccelAmount[nCurrMod] = cMsg.Readfloat();
		m_nNumAccelPoints[nCurrMod] = cMsg.Readuint16();
		m_fXMin[nCurrMod] = cMsg.Readfloat();
		m_fYMin[nCurrMod] = cMsg.Readfloat();
		m_fXMax[nCurrMod] = cMsg.Readfloat();
		m_fYMax[nCurrMod] = cMsg.Readfloat();
	}
}

LTRESULT PolyGrid_Plugin::PreHook_EditStringList(
										const char* /*szRezPath*/,				
										const char* szPropName,				
										char** aszStrings,					
										uint32* pcStrings,	
										const uint32 cMaxStrings,		
										const uint32 cMaxStringLength)		
{
	//handle setting up any LOD properties on the light
	if(	LTStrEquals(szPropName, "RefractionMapLOD") ||
		LTStrEquals(szPropName, "ReflectionMapLOD"))
	{
		return CEngineLODPropUtil::AddLODStrings(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}
	else if( LTStrEquals( szPropName, "CollisionProperty" ))
	{
		if( CategoryPlugin::Instance().PopulateStringList( DATABASE_CATEGORY( CollisionProperty ).GetCategory( ), 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
