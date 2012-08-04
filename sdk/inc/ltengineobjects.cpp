

/*!

 MODULE  : ltengineobjects.cpp

 PURPOSE : C++ LT engine objects

 CREATED : 9/17/97

*/

#include "iltmessage.h"
#include "ltengineobjects.h"
#include <stdio.h>
#include "iltsoundmgr.h"
#include "iltcommon.h"
#include "ltserverobj.h"
#include "iobjectplugin.h"
#include "ltobjectcreate.h"


//the ILTServer game interface
#include "iltserver.h"
static ILTServer *ilt_server;
define_holder(ILTServer, ilt_server);


//ILTCommon interface
#include "iltcommon.h"
static ILTCommon *ilt_common_server;
define_holder_to_instance(ILTCommon, ilt_common_server, Server);

// Define all the game classes and the gameservershell.
DEFINE_CLASSES()



/*!

 BaseClass.

*/
BEGIN_CLASS(BaseClass)
	ADD_STRINGPROP(Name, "noname")
	ADD_VECTORPROP_FLAG(Pos, PF_DISTANCE)
	ADD_ROTATIONPROP(Rotation)
	ADD_LONGINTPROP(RenderGroup, 0)
END_CLASS_DEFAULT_NOPARENT(BaseClass, 0, 0)

BaseClass::~BaseClass()
{
}

uint32
BaseClass::EngineMessageFn(
	uint32 messageID, 
	void *pData, 
	float fData
	)
{
	ObjectCreateStruct *pStruct;
	GenericProp genProp;
	uint32 i, iSkin;
	char skinPropName[128];
	ConParse conParse;
	ILTServer *pServerLT;


	pServerLT = g_pLTServer;


/*!
	 Handle ReadProp.

*/
	if (messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;

/*!
		 If they haven't already set the type, set it to whatever the constructor set.

*/
		if(pStruct->m_ObjectType == OT_NORMAL)
		{
			pStruct->m_ObjectType = m_nType;
		}

/*!
		 Get the props.

*/
		if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
		{
			if( pServerLT->GetPropGeneric( "Name", &genProp ) == LT_OK )
			{
				SAFE_STRCPY(pStruct->m_Name, genProp.m_String);
				pStruct->m_Name[MAX_CS_FILENAME_LEN] = '\0';
			}
			if( pServerLT->GetPropGeneric( "Pos", &genProp ) == LT_OK )
			{
				pStruct->m_Pos = genProp.m_Vec;
			}
			if( pServerLT->GetPropGeneric( "Rotation", &genProp ) == LT_OK )
			{
				pStruct->m_Rotation = genProp.m_Rotation;
			}
			if( pServerLT->GetPropGeneric( "Flags", &genProp ) == LT_OK )
			{
				pStruct->m_Flags = genProp.m_Long;
			}
			if( pServerLT->GetPropGeneric( "Visible", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_VISIBLE;
				else
					pStruct->m_Flags &= ~FLAG_VISIBLE;
			}
			if( pServerLT->GetPropGeneric( "Shadow", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_SHADOW;
				else
					pStruct->m_Flags &= ~FLAG_SHADOW;
			}
			// this case is a misspelling, but is included for backwards compatibility
			if( pServerLT->GetPropGeneric( "RotateableSprite", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_ROTATABLESPRITE;
				else
					pStruct->m_Flags &= ~FLAG_ROTATABLESPRITE;
			}
			// this is the corrected spelling
			if( pServerLT->GetPropGeneric( "RotatableSprite", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_ROTATABLESPRITE;
				else
					pStruct->m_Flags &= ~FLAG_ROTATABLESPRITE;
			}


			if( pServerLT->GetPropGeneric( "Solid", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_SOLID;
				else
					pStruct->m_Flags &= ~FLAG_SOLID;
			}
			if( pServerLT->GetPropGeneric( "Gravity", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_GRAVITY;
				else
					pStruct->m_Flags &= ~FLAG_GRAVITY;
			}
			if( pServerLT->GetPropGeneric( "TouchNotify", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;
				else
					pStruct->m_Flags &= ~FLAG_TOUCH_NOTIFY;
			}
			
			if( pServerLT->GetPropGeneric( "Rayhit", &genProp ) == LT_OK )
			{
				if( genProp.m_Bool )
					pStruct->m_Flags |= FLAG_RAYHIT;
				else
					pStruct->m_Flags &= ~FLAG_RAYHIT;
			}
			
			if( pServerLT->GetPropGeneric( "Filename", &genProp ) == LT_OK )
			{
				SAFE_STRCPY(pStruct->m_Filename, genProp.m_String);
				pStruct->m_Filename[MAX_CS_FILENAME_LEN] = '\0';
			}

			if( pServerLT->GetPropGeneric( "RenderGroup", &genProp ) == LT_OK )
			{
				pStruct->m_nRenderGroup = (uint8)genProp.m_Long;
			}
			
			if( pServerLT->GetPropGeneric( "Skin", &genProp ) == LT_OK )
			{
				iSkin = 0;
				conParse.Init(genProp.m_String);
				while(pServerLT->Common()->Parse(&conParse) == LT_OK)
				{
					if(conParse.m_nArgs > 0)
					{
						SAFE_STRCPY(pStruct->m_SkinNames[iSkin], conParse.m_Args[0]);
						iSkin++;
					}

					if(iSkin >= MAX_MODEL_TEXTURES)
						break;
				}
				pStruct->m_SkinName[MAX_CS_FILENAME_LEN] = '\0';
			}
		
			for(i=0; i < MAX_MODEL_TEXTURES; i++)
			{
				sprintf(skinPropName, "Skin%d", i);

				if( pServerLT->GetPropGeneric( skinPropName, &genProp ) == LT_OK )
				{
					SAFE_STRCPY(pStruct->m_SkinNames[i], genProp.m_String);
				}
			}
		}
	}
	return ILTBaseClass::EngineMessageFn(messageID, pData, fData);
} // BaseClass::EngineMessageFn


/*!

 WorldSection.

*/
BEGIN_CLASS(WorldSection)
	ADD_BOOLPROP(VisContainer, true)
	ADD_BOOLPROP(Moveable, false)
END_CLASS_DEFAULT_FLAGS(WorldSection, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)





/*!

	Container.

*/
BEGIN_CLASS(Container)
	ADD_LONGINTPROP(Flags, (FLAG_VISIBLE | FLAG_CONTAINER))
	ADD_LONGINTPROP(ContainerCode, 0)
END_CLASS_DEFAULT_FLAGS(Container, BaseClass, NULL, NULL, CF_ALWAYSLOAD | CF_HIDDEN)

uint32 Container::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	ObjectCreateStruct *pStruct;
	int32 code;

	if(messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;
		pStruct->m_ObjectType = OT_CONTAINER;
		pStruct->m_Flags |= FLAG_CONTAINER;


/*!
		 Note: Since this object has a "Flags" property, BaseClass::EngineMessageFn
		 is just going to overwrite pStruct->m_Flags with whatever is set in the
		 Flags property in DEdit, making the change to m_Flags above pretty worthless.
		 Most likely this object should not have a "Flags" property at all, but I don't 
		 want to take it out now and break somebody's code.
		 (I did add FLAG_CONTAINER to the default at least!)

*/

		if( fData == PRECREATE_WORLDFILE )
		{
			code = 0;
			g_pLTServer->GetPropLongInt("ContainerCode", &code);
			pStruct->m_ContainerCode = (uint16)code;
			g_pLTServer->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
		}
	}



/*!
	 Pass the message down to the base class.

*/
	return BaseClass::EngineMessageFn(messageID, pData, fData);
}





/*!

 Sound.

*/
BEGIN_CLASS(Sound)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME)
	ADD_LONGINTPROP(Priority, 0.0f)
	ADD_REALPROP_FLAG(OuterRadius, 100.0f, PF_RADIUS | PF_DISTANCE)
	ADD_REALPROP_FLAG(InnerRadius, 10.0f, PF_RADIUS | PF_DISTANCE)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_BOOLPROP(Ambient, true)
END_CLASS_DEFAULT_FLAGS(Sound, BaseClass, NULL, NULL, CF_ALWAYSLOAD | CF_HIDDEN)


uint32 Sound::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	ObjectCreateStruct *pStruct;
	GenericProp genProp;

	if(messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;

		pStruct->m_ObjectType = OT_NORMAL;
		
		if( fData == PRECREATE_NORMAL )
		{
			m_fOuterRadius = 100.0f;
			m_fInnerRadius = 10.0f;
			
			m_nVolume = 100;
			
			m_nPriority = 0;
			m_bAmbient = true;
		}
		else
		{
			if( g_pLTServer->GetPropGeneric( "Filename", &genProp ) == LT_OK )
			{
				SAFE_STRCPY(m_Filename, genProp.m_String);
			}
			else
				m_Filename[0] = '\0';

			if( g_pLTServer->GetPropGeneric("OuterRadius", &genProp ) == LT_OK )
				m_fOuterRadius = genProp.m_Float;
			else
				m_fOuterRadius = 100.0f;

			if( g_pLTServer->GetPropGeneric("InnerRadius", &genProp ) == LT_OK )
				m_fInnerRadius = genProp.m_Float;
			else
				m_fInnerRadius = 10.0f;
			
			if( g_pLTServer->GetPropGeneric("Volume", &genProp) == LT_OK )
				m_nVolume = (uint8)genProp.m_Long;
			else
				m_nVolume = 100;
			
			if( g_pLTServer->GetPropGeneric("Priority", &genProp) == LT_OK )
				m_nPriority = (unsigned char)genProp.m_Long;
			else
				m_nPriority = 0;
			
			if( g_pLTServer->GetPropGeneric("Ambient", &genProp) == LT_OK )
				m_bAmbient = !!genProp.m_Bool;
			else
				m_bAmbient = true;
		}
	}
	else if(messageID == MID_OBJECTCREATED)
	{
		PlaySoundInfo playSoundInfo;
		
		PLAYSOUNDINFO_INIT( playSoundInfo );
		playSoundInfo.m_dwFlags = PLAYSOUND_LOOP;
		SAFE_STRCPY(playSoundInfo.m_szSoundName, m_Filename);
		playSoundInfo.m_nPriority = m_nPriority;
		playSoundInfo.m_fOuterRadius = m_fOuterRadius;
		playSoundInfo.m_fInnerRadius = m_fInnerRadius;
		if( m_nVolume < 100 )
		{
			playSoundInfo.m_nVolume = m_nVolume;
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		}
		
		g_pLTServer->GetObjectPos( m_hObject, &playSoundInfo.m_vPosition );
		if( m_bAmbient )
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
		}
		else
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
		}

		HLTSOUND hSound;
		g_pLTServer->SoundMgr()->PlaySound( &playSoundInfo, hSound );



/*!
		 sounds just remove themselves right away.  Since they don't do
		 anything right now, they just take up memory.

*/
		g_pLTServer->RemoveObject(m_hObject);
	}



/*!
	 Pass the message down to the base class.

*/
	return BaseClass::EngineMessageFn(messageID, pData, fData);
}




/*!
 
 MainWorld.

*/
BEGIN_CLASS(MainWorld)
END_CLASS_DEFAULT_FLAGS(MainWorld, BaseClass, NULL, NULL, CF_ALWAYSLOAD | CF_HIDDEN)


uint32 MainWorld::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	// Turn off updates for the main world
	if (messageID == MID_OBJECTCREATED)
	{
		g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}





/*!
 
 InsideDef.

*/
BEGIN_CLASS(InsideDef)
END_CLASS_DEFAULT_FLAGS(InsideDef, BaseClass, NULL, NULL, CF_NORUNTIME)





/*!

 OutsideDef.

*/
BEGIN_CLASS(OutsideDef)
END_CLASS_DEFAULT_FLAGS(OutsideDef, BaseClass, NULL, NULL, CF_NORUNTIME)





/*!

 FastApproxArea.

*/
BEGIN_CLASS(FastApproxArea)
END_CLASS_DEFAULT_FLAGS(FastApproxArea, BaseClass, NULL, NULL, CF_NORUNTIME )


/*!

 AmbientOverride

*/
BEGIN_CLASS(AmbientOverride)
	ADD_COLORPROP_FLAG(AmbientLight, 0.0f, 0.0f, 0.0f, 0)
END_CLASS_DEFAULT_FLAGS(AmbientOverride, BaseClass, NULL, NULL, CF_NORUNTIME )


/*!

 Light plug-in

*/

class CLightPlugin : public IObjectPlugin
{
public:
	CLightPlugin() {}

	struct SStringListParams
	{
		char **m_aszStrings;
		uint32 *m_pcStrings;
		uint32 m_cMaxStrings;
		uint32 m_cMaxStringLength;
	};

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength)		
	{
	   if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	   SStringListParams sParams;
	   sParams.m_aszStrings = aszStrings;
	   sParams.m_pcStrings = pcStrings;
	   sParams.m_cMaxStrings = cMaxStrings;
	   sParams.m_cMaxStringLength = cMaxStringLength;

	   if (stricmp(szPropName, "Attenuation") == 0)
	   {
		   AddString("D3D", sParams);
		   AddString("Linear", sParams);
		   AddString("Quartic", sParams);
	   }
	   else
		   return LT_UNSUPPORTED;
	   
	   return LT_OK;
	}

private:
	bool AddString(const char *pString, SStringListParams &sParams) {
		if (strlen(pString) < sParams.m_cMaxStringLength && 
			((*sParams.m_pcStrings) + 1) < sParams.m_cMaxStrings)
		{
			strcpy(sParams.m_aszStrings[(*sParams.m_pcStrings)++], pString);
			return true;
		}
		else
			return false;
	}
};

/*!

 Light.

*/
BEGIN_CLASS(Light)
	ADD_BOOLPROP(ClipLight, 1)
	ADD_BOOLPROP(LightObjects, 1)
	ADD_BOOLPROP(FastLightObjects, 1)
	ADD_BOOLPROP(CastShadows, 1)
	ADD_BOOLPROP(CastShadowMesh, 0)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS | PF_DISTANCE)
	PROP_DEFINECOMPOSITETYPE(LightAttenuation, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(AttType, "Default", PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttCoefs, 1.0f, 0.0f, 19.0f, PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttExps, 0.0f, 0.0f, -2.0f, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(Attenuation, "Quartic", PF_STATICLIST)
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f)
 	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(Size, 5.0f)
	ADD_REALPROP(ConvertToAmbient, 0.0f)
	ADD_REALPROP(ObjectBrightScale, 1.0f)
	ADD_STRINGPROP_FLAG(LightGroup, "", PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Light, BaseClass, NULL, NULL, CF_NORUNTIME, CLightPlugin)


/*!

 Engine_LightGroup.

*/
BEGIN_CLASS(Engine_LightGroup)
END_CLASS_DEFAULT_FLAGS(Engine_LightGroup, BaseClass, NULL, NULL, CF_NORUNTIME | CF_HIDDEN)




/*!

 ObjectLight.
 
*/
BEGIN_CLASS(ObjectLight)
	ADD_BOOLPROP(FastLightObjects, 1)
	ADD_BOOLPROP(CastShadows, 1)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS | PF_DISTANCE)
	PROP_DEFINECOMPOSITETYPE(LightAttenuation, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(AttType, "Default", PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttCoefs, 1.0f, 0.0f, 19.0f, PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttExps, 0.0f, 0.0f, -2.0f, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(Attenuation, "Quartic", PF_STATICLIST)
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(ConvertToAmbient, 0.0f)
	ADD_STRINGPROP_FLAG(LightGroup, "", PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(ObjectLight, BaseClass, NULL, NULL, CF_NORUNTIME, CLightPlugin)





/*!
 
 DirLight.

*/
BEGIN_CLASS(DirLight)
	ADD_BOOLPROP(ClipLight, 1)
	ADD_BOOLPROP(LightObjects, 1)
	ADD_BOOLPROP(FastLightObjects, 1)
	ADD_BOOLPROP(CastShadows, 1)
	ADD_BOOLPROP(CastShadowMesh, 0)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_FOVRADIUS | PF_DISTANCE)
	PROP_DEFINECOMPOSITETYPE(LightAttenuation, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(AttType, "Default", PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttCoefs, 1.0f, 0.0f, 19.0f, PF_GROUP(1))
		ADD_VECTORPROP_VAL_FLAG(AttExps, 0.0f, 0.0f, -2.0f, PF_GROUP(1))
	ADD_STRINGPROP_FLAG(Attenuation, "Quartic", PF_STATICLIST)
	ADD_COLORPROP(InnerColor, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP_FLAG(FOV, 90.0f, PF_FIELDOFVIEW)
	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(Size, 5.0f)
	ADD_REALPROP(ConvertToAmbient, 0.0f)
	ADD_REALPROP(ObjectBrightScale, 1.0f)
	ADD_STRINGPROP_FLAG(LightGroup, "", PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(DirLight, BaseClass, NULL, NULL, CF_NORUNTIME, CLightPlugin)





/*!

 StaticSunLight.
 
*/
BEGIN_CLASS(StaticSunLight)
	ADD_COLORPROP(InnerColor, 255.0f, 255.0f, 255.0f)
	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(ConvertToAmbient, 0.0f)
	ADD_REALPROP(ObjectBrightScale, 1.0f)
END_CLASS_DEFAULT_FLAGS(StaticSunLight, BaseClass, NULL, NULL, CF_ALWAYSLOAD)

uint32 StaticSunLight::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct;
			pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_ObjectType = OT_NORMAL;

			if( fData == PRECREATE_WORLDFILE )
			{
				if(g_pLTServer->GetPropVector("InnerColor", &m_InnerColor) != LT_OK)
					m_InnerColor.Init(255.0f, 255.0f, 255.0f);

				if(g_pLTServer->GetPropReal("ConvertToAmbient", &m_fConvertToAmbient) != LT_OK)
					m_fConvertToAmbient = 0.0f;

				if(g_pLTServer->GetPropReal("BrightScale", &m_BrightScale) != LT_OK)
					m_BrightScale = 1.0f;

				if(g_pLTServer->GetPropReal("ObjectBrightScale", &m_ObjectBrightScale) != LT_OK)
					m_BrightScale = 1.0f;
			}
			else
			{
				m_InnerColor.Init(255.0f, 255.0f, 255.0f);
				m_fConvertToAmbient = 0.0f;
				m_BrightScale		= 1.0f;
				m_ObjectBrightScale = 1.0f;
			}

			break;
		}


		case MID_OBJECTCREATED:
		{
			g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
			break;
		}

		case MID_UPDATE:
		{
			g_pLTServer->SetGlobalLightObject(m_hObject);
			break;
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

/*!

 Decal

*/

BEGIN_CLASS(Decal)
	
	ADD_VECTORPROP_VAL_FLAG(ProjectDims, 64, 64, 50, PF_ORTHOFRUSTUM)
	ADD_REALPROP_FLAG(NearClip, 0.0f, 0)
	ADD_REALPROP_FLAG(Offset, 0.3f, 0)
	ADD_BOOLPROP_FLAG(Occlude, false, 0)
	ADD_BOOLPROP_FLAG(HitWorldModels, true, 0)
	ADD_STRINGPROP_FLAG(Texture, "", PF_FILENAME)
	ADD_STRINGPROP(IgnoreBrushes, "")
	ADD_STRINGPROP_FLAG(TextureEffect, "", PF_TEXTUREEFFECT)
	ADD_REALPROP_FLAG(UOffset, 0.0f, 0)
	ADD_REALPROP_FLAG(VOffset, 0.0f, 0)

END_CLASS_DEFAULT_FLAGS( Decal, BaseClass, NULL, NULL, CF_NORUNTIME)

/*!

 RenderGroup

*/

BEGIN_CLASS(RenderGroup)
END_CLASS_DEFAULT_FLAGS( RenderGroup, BaseClass, NULL, NULL, CF_NORUNTIME)


/*!

 EdgeGenerator

*/

BEGIN_CLASS(EdgeGenerator)
	
	ADD_REALPROP_FLAG(Thickness, 0.2f, 0)
	ADD_REALPROP_FLAG(CreaseAngle, 0.0f, 0)
	ADD_REALPROP_FLAG(Extrude, 0.0f, 0)
	ADD_BOOLPROP_FLAG(ShrinkBrushes, true, 0)
	ADD_STRINGPROP_FLAG(Texture, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(TextureEffect, "", PF_TEXTUREEFFECT)
	ADD_STRINGPROP_FLAG(WorldModel, "", PF_OBJECTLINK)

END_CLASS_DEFAULT_FLAGS( EdgeGenerator, BaseClass, NULL, NULL, CF_NORUNTIME)


/*!

 Brush plug-in

*/

class CBrushPlugin : public IObjectPlugin
{
public:
	CBrushPlugin() {}

	struct SStringListParams
	{
		char **m_aszStrings;
		uint32 *m_pcStrings;
		uint32 m_cMaxStrings;
		uint32 m_cMaxStringLength;
	};

	virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength)		
	{
	   if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	   SStringListParams sParams;
	   sParams.m_aszStrings = aszStrings;
	   sParams.m_pcStrings = pcStrings;
	   sParams.m_cMaxStrings = cMaxStrings;
	   sParams.m_cMaxStringLength = cMaxStringLength;

	   if (stricmp(szPropName, "Type") == 0)
	   {
		   AddString("Normal", sParams);
		   AddString("SkyPortal", sParams);
		   AddString("Occluder", sParams);
		   AddString("RBSplitter", sParams);
		   AddString("RenderOnly", sParams);
		   AddString("Blocker", sParams);
		   AddString("NonSolid", sParams);
		   AddString("ParticleBlocker", sParams);
	   }
	   else if (stricmp(szPropName, "Lighting") == 0)
	   {
		   AddString("Flat", sParams);
		   AddString("Gouraud", sParams);
		   AddString("Lightmap", sParams);
		   AddString("ShadowMesh", sParams);
		   //AddString("SkyPan", sParams);
	   }
	   else
		   return LT_UNSUPPORTED;
	   
	   return LT_OK;
	}

private:
	bool AddString(const char *pString, SStringListParams &sParams) {
		if (strlen(pString) < sParams.m_cMaxStringLength && 
			((*sParams.m_pcStrings) + 1) < sParams.m_cMaxStrings)
		{
			strcpy(sParams.m_aszStrings[(*sParams.m_pcStrings)++], pString);
			return true;
		}
		else
			return false;
	}
};

/*!

 Brush.
 
*/
BEGIN_CLASS(Brush)
	ADD_STRINGPROP_FLAG(Type, "Normal", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(Lighting, "Gouraud", PF_STATICLIST)

	ADD_BOOLPROP(NotAStep, 0)

	ADD_BOOLPROP(Detail, 1)

	PROP_DEFINEGROUP(LightControl, PF_GROUP(1))
		ADD_COLORPROP_FLAG(AmbientLight, 0.0f, 0.0f, 0.0f, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(LMGridSize, 0, PF_GROUP(1))
		ADD_BOOLPROP_FLAG(ClipLight, 1, PF_GROUP(1))
		ADD_BOOLPROP_FLAG(CastShadowMesh, 1, PF_GROUP(1))
		ADD_BOOLPROP_FLAG(ReceiveLight, 1, PF_GROUP(1))
		ADD_BOOLPROP_FLAG(ReceiveShadows, 1, PF_GROUP(1))
		ADD_BOOLPROP_FLAG(ReceiveSunlight, 1, PF_GROUP(1))
		ADD_REALPROP_FLAG(LightPenScale, 0.0f, PF_GROUP(1))
		ADD_REALPROP_FLAG(CreaseAngle, 45.0f, PF_GROUP(1))

	ADD_STRINGPROP_FLAG(TextureEffect, "", PF_TEXTUREEFFECT)

END_CLASS_DEFAULT_FLAGS_PLUGIN(Brush, BaseClass, NULL, NULL, CF_ALWAYSLOAD, CBrushPlugin)

/*!

 DemoSkyWorldModel.

*/
BEGIN_CLASS(DemoSkyWorldModel)
	ADD_VECTORPROP_VAL_FLAG(SkyDims, 0.0f, 0.0f, 0.0f, PF_DIMS | PF_DISTANCE)
	ADD_LONGINTPROP(Flags, 1)
	ADD_LONGINTPROP(Index, 0)
	ADD_REALPROP(InnerPercentX, 0.1f)
	ADD_REALPROP(InnerPercentY, 0.1f)
	ADD_REALPROP(InnerPercentZ, 0.1f)
END_CLASS_DEFAULT_FLAGS(DemoSkyWorldModel, BaseClass, NULL, NULL, CF_ALWAYSLOAD | CF_WORLDMODEL)


uint32 DemoSkyWorldModel::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	SkyDef def;
	LTVector pos, temp;
	ObjectCreateStruct *pStruct;


	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_ObjectType = OT_WORLDMODEL;

			if( fData == PRECREATE_WORLDFILE )
			{
				g_pLTServer->GetPropVector("SkyDims", &m_SkyDims);
				g_pLTServer->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
				g_pLTServer->GetPropReal("InnerPercentX", &m_InnerPercentX);
				g_pLTServer->GetPropReal("InnerPercentY", &m_InnerPercentY);
				g_pLTServer->GetPropReal("InnerPercentZ", &m_InnerPercentZ);
				g_pLTServer->GetPropLongInt("Index", &m_Index);
			}
			else
			{
				m_SkyDims.Init();
				m_InnerPercentX = 0.1f;
				m_InnerPercentY = 0.1f;
				m_InnerPercentZ = 0.1f;
				m_Index = 0;
			}
			break;
		}

		case MID_OBJECTCREATED:
		{


/*!
			 Set the sky box?

*/
			if(m_SkyDims.x != 0.0f && 
				m_SkyDims.y != 0.0f && 
				m_SkyDims.z != 0.0f)
			{
				g_pLTServer->GetObjectPos(m_hObject, &pos);
				def.m_Min = pos - m_SkyDims;
				def.m_Max = pos + m_SkyDims;

				temp.x = m_SkyDims.x * m_InnerPercentX;
				temp.y = m_SkyDims.y * m_InnerPercentY;
				temp.z = m_SkyDims.z * m_InnerPercentZ;

				def.m_ViewMin = pos - temp;
				def.m_ViewMax = pos + temp;

				g_pLTServer->SetSkyDef(&def);
			}

			g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCEOPTIMIZEOBJECT, FLAG_FORCEOPTIMIZEOBJECT);
			g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_SKYOBJECT, FLAG2_SKYOBJECT);
			
			g_pLTServer->AddObjectToSky(m_hObject, m_Index);
			break;
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}






/*!

 SkyPointer.

*/
BEGIN_CLASS(SkyPointer)
	ADD_STRINGPROP(SkyObjectName, "")
	ADD_VECTORPROP_VAL_FLAG(SkyDims, 0.0f, 0.0f, 0.0f, PF_DIMS | PF_DISTANCE)
	ADD_LONGINTPROP(Flags, 1)
	ADD_LONGINTPROP(Index, 0)
	ADD_REALPROP(InnerPercentX, 0.1f)
	ADD_REALPROP(InnerPercentY, 0.1f)
	ADD_REALPROP(InnerPercentZ, 0.1f)
END_CLASS_DEFAULT_FLAGS(SkyPointer, BaseClass, NULL, NULL, CF_ALWAYSLOAD)


uint32 SkyPointer::EngineMessageFn(
	uint32 messageID, void *pData, float fData)
{
	SkyDef def;
	LTVector pos, temp;
	ObjectCreateStruct *pStruct;
	HOBJECT hObject;


	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_ObjectType = OT_NORMAL;

			if( fData == PRECREATE_WORLDFILE )
			{
				g_pLTServer->GetPropVector("SkyDims", &m_SkyDims);
				g_pLTServer->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
				g_pLTServer->GetPropString("SkyObjectName", m_ObjectName, sizeof(m_ObjectName)-1);
				g_pLTServer->GetPropReal("InnerPercentX", &m_InnerPercentX);
				g_pLTServer->GetPropReal("InnerPercentY", &m_InnerPercentY);
				g_pLTServer->GetPropReal("InnerPercentZ", &m_InnerPercentZ);
				g_pLTServer->GetPropLongInt("Index", &m_Index);
			}
			else
			{
				m_ObjectName[0] = 0;
				m_SkyDims.Init();
				m_InnerPercentX = 0.1f;
				m_InnerPercentY = 0.1f;
				m_InnerPercentZ = 0.1f;
				m_Index = 0;
			}
			break;
		}

		case MID_OBJECTCREATED:
		{


/*!
			 Set the sky box?

*/
			if(m_SkyDims.x != 0.0f && m_SkyDims.y != 0.0f && m_SkyDims.z != 0.0f)
			{
				g_pLTServer->GetObjectPos(m_hObject, &pos);
				def.m_Min = pos - m_SkyDims;
				def.m_Max = pos + m_SkyDims;

				temp.x = m_SkyDims.x * m_InnerPercentX;
				temp.y = m_SkyDims.y * m_InnerPercentY;
				temp.z = m_SkyDims.z * m_InnerPercentZ;

				def.m_ViewMin = pos - temp;
				def.m_ViewMax = pos + temp;

				g_pLTServer->SetSkyDef(&def);
			}

			g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
			break;
		}

		case MID_UPDATE:
		{

			ObjArray <HOBJECT, 1> objArray;

			g_pLTServer->FindNamedObjects(m_ObjectName, objArray);

			if(objArray.NumObjects())
			{
				hObject = objArray.GetObject(0);
				g_pLTServer->AddObjectToSky(hObject, m_Index);
				
				g_pLTServer->Common()->SetObjectFlags(hObject, OFT_Flags, FLAG_FORCEOPTIMIZEOBJECT, FLAG_FORCEOPTIMIZEOBJECT);
				g_pLTServer->Common()->SetObjectFlags(hObject, OFT_Flags2, FLAG2_SKYOBJECT, FLAG2_SKYOBJECT);
			}

			g_pLTServer->RemoveObject(m_hObject);
			break;
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



/*!

 GenericObject.

*/
BEGIN_CLASS(GenericObject)
	ADD_REALPROP(ObjectType, 0)
	ADD_REALPROP_FLAG(LightRadius, 300, PF_DISTANCE)
	ADD_STRINGPROP(Filename, "")
	ADD_STRINGPROP(Skin, "")
	ADD_STRINGPROP(Skin2, "")
	ADD_REALPROP(R, 255)
	ADD_REALPROP(G, 255)
	ADD_REALPROP(B, 255)
	ADD_REALPROP(A, 255)
END_CLASS_DEFAULT_FLAGS(GenericObject, BaseClass, NULL, NULL, CF_HIDDEN)


uint32 GenericObject::EngineMessageFn(
	uint32 messageID, void *pData, float fData)
{
	ObjectCreateStruct *pStruct;
	GenericProp genProp;
	ILTServer *pServerLT;


	pServerLT = g_pLTServer;
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;

			if(pServerLT->GetPropGeneric("ObjectType", &genProp) == LT_OK)
				pStruct->m_ObjectType = (uint16)genProp.m_Long;

			if(pServerLT->GetPropGeneric("LightRadius", &genProp) == LT_OK)
				m_LightRadius = genProp.m_Float;

			if(pServerLT->GetPropGeneric("R", &genProp) == LT_OK)
				m_ColorR = genProp.m_Float / 255.0f;
			if(pServerLT->GetPropGeneric("G", &genProp) == LT_OK)
				m_ColorG = genProp.m_Float / 255.0f;
			if(pServerLT->GetPropGeneric("B", &genProp) == LT_OK)
				m_ColorB = genProp.m_Float / 255.0f;
			if(pServerLT->GetPropGeneric("A", &genProp) == LT_OK)
				m_ColorA = genProp.m_Float / 255.0f;
		}
		break;

		case MID_OBJECTCREATED:
		{
			pServerLT->SetLightRadius(m_hObject, m_LightRadius);
			pServerLT->SetObjectColor(m_hObject,
				m_ColorR, m_ColorG, 
				m_ColorB, m_ColorA);
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



