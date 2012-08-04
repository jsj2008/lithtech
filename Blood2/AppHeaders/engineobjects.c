
#include "serverobj_de.h"
#include "engineobjects_de.h"


void bc_AddAggregate(LPBASECLASS pObject, LPAGGREGATE pAggregate)
{
	pAggregate->m_pNextAggregate = pObject->m_pFirstAggregate;
	pObject->m_pFirstAggregate = pAggregate;
}


DDWORD bc_EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float fData)
{
	LPAGGREGATE pAggregate;
	ObjectCreateStruct *pStruct;
	GenericProp genProp;

	// Handle ReadProp.
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Get the props.
			if( fData == 1.0f || fData == 2.0f )
			{
				pStruct = (ObjectCreateStruct*)pData;

				if( g_pServerDE->GetPropGeneric( "Name", &genProp ) == DE_OK )
				{
					SAFE_STRCPY(pStruct->m_Name, genProp.m_String);
					pStruct->m_Name[MAX_CS_FILENAME_LEN] = '\0';
				}
				if( g_pServerDE->GetPropGeneric( "Pos", &genProp ) == DE_OK )
				{
					VEC_COPY( pStruct->m_Pos, genProp.m_Vec );
				}
				if( g_pServerDE->GetPropGeneric( "Rotation", &genProp ) == DE_OK )
				{
					ROT_COPY( pStruct->m_Rotation, genProp.m_Rotation );
				}
				if( g_pServerDE->GetPropGeneric( "Flags", &genProp ) == DE_OK )
				{
					pStruct->m_Flags = genProp.m_Long;
				}
				if( g_pServerDE->GetPropGeneric( "Visible", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_VISIBLE;
					else
						pStruct->m_Flags &= ~FLAG_VISIBLE;
				}
				if( g_pServerDE->GetPropGeneric( "Shadow", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_SHADOW;
					else
						pStruct->m_Flags &= ~FLAG_SHADOW;
				}
				if( g_pServerDE->GetPropGeneric( "RotateableSprite", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_ROTATEABLESPRITE;
					else
						pStruct->m_Flags &= ~FLAG_ROTATEABLESPRITE;
				}
				if( g_pServerDE->GetPropGeneric( "Chromakey", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_SPRITECHROMAKEY;
					else
						pStruct->m_Flags &= ~FLAG_SPRITECHROMAKEY;
				}
				if( g_pServerDE->GetPropGeneric( "Solid", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_SOLID;
					else
						pStruct->m_Flags &= ~FLAG_SOLID;
				}
				if( g_pServerDE->GetPropGeneric( "Gravity", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_GRAVITY;
					else
						pStruct->m_Flags &= ~FLAG_GRAVITY;
				}
				if( g_pServerDE->GetPropGeneric( "TouchNotify", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;
					else
						pStruct->m_Flags &= ~FLAG_TOUCH_NOTIFY;
				}
				if( g_pServerDE->GetPropGeneric( "Rayhit", &genProp ) == DE_OK )
				{
					if( genProp.m_Bool )
						pStruct->m_Flags |= FLAG_RAYHIT;
					else
						pStruct->m_Flags &= ~FLAG_RAYHIT;
				}
				if( g_pServerDE->GetPropGeneric( "Filename", &genProp ) == DE_OK )
				{
					SAFE_STRCPY(pStruct->m_Filename, genProp.m_String);
					pStruct->m_Filename[MAX_CS_FILENAME_LEN] = '\0';
				}
				if( g_pServerDE->GetPropGeneric( "Skin", &genProp ) == DE_OK )
				{
					SAFE_STRCPY(pStruct->m_SkinName, genProp.m_String);
					pStruct->m_SkinName[MAX_CS_FILENAME_LEN] = '\0';
				}
				break;
			}
		}
	}

	// Call the aggregates.
	pAggregate = pObject->m_pFirstAggregate;
	while(pAggregate)
	{
		pAggregate->m_EngineMessageFn(pObject, pAggregate, messageID, pData, fData);
		pAggregate = pAggregate->m_pNextAggregate;
	}

	// Default return is 1.
	return 1;
}

DDWORD bc_ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	LPAGGREGATE pAggregate;

	// Call the aggregates.
	pAggregate = pObject->m_pFirstAggregate;
	while(pAggregate)
	{
		pAggregate->m_ObjectMessageFn(pObject, pAggregate, hSender, messageID, hRead);
		pAggregate = pAggregate->m_pNextAggregate;
	}

	return 1;
}


#ifdef COMPILE_WITH_C
	BEGIN_CLASS(BaseClass)
		ADD_STRINGPROP(Name, "noname")
		ADD_VECTORPROP(Pos)
		ADD_ROTATIONPROP(Rotation)
	END_CLASS_DEFAULT_NOPARENT(BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn)
#endif



// The model class..
typedef struct
{
	BaseClass m_BaseClass;
} Model;

static DDWORD mdl_MessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	if(messageID == MID_PRECREATE)
	{
		((ObjectCreateStruct*)pData)->m_ObjectType = OT_MODEL;
	}

	// Pass the message down to the base class.
	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(Model)
	ADD_STRINGPROP(Filename, "")
	ADD_STRINGPROP(Skin, "")
	ADD_LONGINTPROP(Flags, FLAG_VISIBLE)
END_CLASS_DEFAULT_FLAGS(Model, BaseClass, mdl_MessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)



// The WorldModel class.
typedef struct
{
	BaseClass m_BaseClass;
} WorldModel;

static DDWORD wmdl_MessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	ObjectCreateStruct *pStruct;

	if(messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;
		
		pStruct->m_ObjectType = OT_WORLDMODEL;

		if( lData == 1.0f )
		{
			g_pServerDE->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
		}
	}

	// Pass the message down to the base class.
	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(WorldModel)
	ADD_LONGINTPROP(Flags, FLAG_VISIBLE)
END_CLASS_DEFAULT_FLAGS(WorldModel, BaseClass, wmdl_MessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)



// The Container class.
typedef struct
{
	BaseClass m_BaseClass;
	DDWORD m_ContainerCode;
} Container;


static DDWORD cntr_MessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	ObjectCreateStruct *pStruct;
	long code;

	if(messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;
		pStruct->m_ObjectType = OT_CONTAINER;

		if( lData == 1.0f )
		{
			code = 0;
			g_pServerDE->GetPropLongInt("ContainerCode", &code);
			pStruct->m_ContainerCode = (D_WORD)code;
			g_pServerDE->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
		}
	}

	// Pass the message down to the base class.
	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(Container)
	ADD_LONGINTPROP(Flags, FLAG_VISIBLE)
	ADD_LONGINTPROP(ContainerCode, 0)
END_CLASS_DEFAULT_FLAGS(Container, BaseClass, cntr_MessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)


// The sprite class.
typedef struct
{
	BaseClass	m_BaseClass;
	DBYTE		m_Color[4]; // RGBA
} Sprite;

static DDWORD spr_MessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	ObjectCreateStruct *pStruct;
	Sprite *pSprite;
	GenericProp prop;

	pSprite = (Sprite*)pObject;
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;
			pStruct->m_ObjectType = OT_SPRITE;
			
			g_pServerDE->GetPropGeneric("Color", &prop);
			pSprite->m_Color[0] = (DBYTE)prop.m_Color.x;
			pSprite->m_Color[1] = (DBYTE)prop.m_Color.y;
			pSprite->m_Color[2] = (DBYTE)prop.m_Color.z;

			g_pServerDE->GetPropGeneric("Scale", &prop);
			VEC_COPY(pStruct->m_Scale, prop.m_Vec);

			g_pServerDE->GetPropGeneric("Alpha", &prop);
			pSprite->m_Color[3] = (DBYTE)(prop.m_Float * 255.0f);
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pServerDE->SetObjectColor(pObject->m_hObject, 
				pSprite->m_Color[0]/255.0f, pSprite->m_Color[1]/255.0f, 
				pSprite->m_Color[1]/255.0f, pSprite->m_Color[3]/255.0f);
		}
		break;
	}

	// Pass the message down to the base class.
	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(Sprite)
	ADD_STRINGPROP(Filename, "")
	ADD_LONGINTPROP(Flags, FLAG_VISIBLE)
	ADD_VECTORPROP_VAL(Scale, 1, 1, 1)
	ADD_COLORPROP(Color, 255, 255, 255)
	ADD_REALPROP(Alpha, 1)
END_CLASS_DEFAULT_FLAGS(Sprite, BaseClass, spr_MessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)


// sound class.
typedef struct
{
	BaseClass m_BaseClass;
	char m_Filename[101];
	float m_fOuterRadius;
	float m_fInnerRadius;
	DBYTE m_nVolume;
	DBOOL m_bAmbient;
	DBOOL m_bFileStream;
	unsigned char m_nPriority;
} Sound;

static DDWORD as_MessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	ObjectCreateStruct *pStruct;
	Sound *pSound;
	GenericProp genProp;

	if(messageID == MID_PRECREATE)
	{
		pStruct = (ObjectCreateStruct*)pData;
		pSound = (Sound*)pObject;

		pStruct->m_ObjectType = OT_NORMAL;
		
		if( lData == 0.0f )
		{
			pSound->m_fOuterRadius = 100.0f;
			pSound->m_fInnerRadius = 10.0f;
			
			pSound->m_nVolume = 100;
			
			pSound->m_nPriority = 0;
			pSound->m_bAmbient = DTRUE;
			pSound->m_bFileStream = DFALSE;
		}
		else
		{
			if( g_pServerDE->GetPropGeneric( "Filename", &genProp ) == DE_OK )
			{
				SAFE_STRCPY(pSound->m_Filename, genProp.m_String);
			}
			else
				pSound->m_Filename[0] = '\0';

			if( g_pServerDE->GetPropGeneric("OuterRadius", &genProp ) == DE_OK )
				pSound->m_fOuterRadius = genProp.m_Float;
			else
				pSound->m_fOuterRadius = 100.0f;

			if( g_pServerDE->GetPropGeneric("InnerRadius", &genProp ) == DE_OK )
				pSound->m_fInnerRadius = genProp.m_Float;
			else
				pSound->m_fInnerRadius = 10.0f;
			
			if( g_pServerDE->GetPropGeneric("Volume", &genProp) == DE_OK )
				pSound->m_nVolume = (DBYTE)genProp.m_Long;
			else
				pSound->m_nVolume = 100;
			
			if( g_pServerDE->GetPropGeneric("Priority", &genProp) == DE_OK )
				pSound->m_nPriority = (unsigned char)genProp.m_Long;
			else
				pSound->m_nPriority = 0;
			
			if( g_pServerDE->GetPropGeneric("Ambient", &genProp) == DE_OK )
				pSound->m_bAmbient = genProp.m_Bool;
			else
				pSound->m_bAmbient = DTRUE;

			if( g_pServerDE->GetPropGeneric("FileStream", &genProp) == DE_OK )
				pSound->m_bFileStream = genProp.m_Bool;
			else
				pSound->m_bFileStream = DTRUE;
		}
	}
	else if(messageID == MID_INITIALUPDATE)
	{
		PlaySoundInfo playSoundInfo;
		
		PLAYSOUNDINFO_INIT( playSoundInfo );
		playSoundInfo.m_dwFlags = PLAYSOUND_LOOP;
		SAFE_STRCPY(playSoundInfo.m_szSoundName, (( Sound * )pObject )->m_Filename);
		playSoundInfo.m_nPriority = (( Sound * )pObject )->m_nPriority;
		playSoundInfo.m_fOuterRadius = (( Sound * )pObject )->m_fOuterRadius;
		playSoundInfo.m_fInnerRadius = (( Sound * )pObject )->m_fInnerRadius;
		if( (( Sound * )pObject )->m_nVolume < 100 )
		{
			playSoundInfo.m_nVolume = (( Sound * )pObject )->m_nVolume;
			playSoundInfo.m_dwFlags |= PLAYSOUND_CTRL_VOL;
		}
		g_pServerDE->GetObjectPos( pObject->m_hObject, &playSoundInfo.m_vPosition );
		if( (( Sound * )pObject )->m_bAmbient )
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_AMBIENT;
		}
		else
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_3D;
		}

		if((( Sound * )pObject )->m_bFileStream )
		{
			playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM | PLAYSOUND_TIMESYNC;
		}

		g_pServerDE->PlaySound( &playSoundInfo );

		// sounds just remove themselves right away.  Since they don't do
		// anything right now, they just take up memory.
		g_pServerDE->RemoveObject(pObject->m_hObject);
	}

	// Pass the message down to the base class.
	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(Sound)
	ADD_STRINGPROP(Filename, "")
	ADD_LONGINTPROP(Priority, 0.0f)
	ADD_REALPROP_FLAG(OuterRadius, 100.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(InnerRadius, 10.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_BOOLPROP(Ambient, 1)
	ADD_BOOLPROP(FileStream, 0)
END_CLASS_DEFAULT_FLAGS(Sound, BaseClass, as_MessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)


// Start point class.

#ifdef COMPILE_WITH_C

typedef struct
{
	BaseClass m_BaseClass;
} StartPoint;

BEGIN_CLASS(StartPoint)
END_CLASS_DEFAULT_FLAGS(StartPoint, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)

#endif // COMPILE_WITH_C



// OutsideDef class.
// These are used if you want the preprocessor to make a leak file for a level.
typedef struct
{
	BaseClass m_BaseClass;
} OutsideDef;

BEGIN_CLASS(OutsideDef)
END_CLASS_DEFAULT_FLAGS(OutsideDef, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME )



// FastApproxArea class.
// This defines an area where only fast approximation vising will occur.  The
// area is bounded by hullmakers and portals.
typedef struct
{
	BaseClass m_BaseClass;
} FastApproxArea;

BEGIN_CLASS(FastApproxArea)
END_CLASS_DEFAULT_FLAGS(FastApproxArea, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME )



// Light class.
typedef struct
{
	BaseClass m_BaseClass;
} Light;

BEGIN_CLASS(Light)
	ADD_BOOLPROP(ClipLight, 1)
	ADD_BOOLPROP(LightObjects, 1)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS)
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(OuterColor, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(Time, 0.0f)
END_CLASS_DEFAULT_FLAGS(Light, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME )


// ObjectLight class (these lights only light objects.. they don't light the world).  These
// are used for landscape areas lit by GlobalDirLights (which don't light objects).
typedef struct
{
	BaseClass m_BaseClass;
} ObjectLight;

BEGIN_CLASS(ObjectLight)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS)
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(OuterColor, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(BrightScale, 1.0f)
END_CLASS_DEFAULT_FLAGS(ObjectLight, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)



// DirLight class (just uses the Light code).
typedef struct DirLight_t
{
	BaseClass m_BaseClass;
} DirLight;

BEGIN_CLASS(DirLight)
	ADD_BOOLPROP(LightObjects, 1)
	ADD_BOOLPROP(ClipLight, 1)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS)
	ADD_COLORPROP(InnerColor, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(OuterColor, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP_FLAG(FOV, 90.0f, PF_FIELDOFVIEW)
	ADD_REALPROP(BrightScale, 1.0f)
	ADD_REALPROP(Time, 0.0f)
END_CLASS_DEFAULT_FLAGS(DirLight, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)



// LightAnimator class.
typedef struct LightAnimator_t
{
	BaseClass m_BaseClass;
} LightAnimator;

BEGIN_CLASS(LightAnimator)
	ADD_STRINGPROP(BaseLightName, "light")
END_CLASS_DEFAULT_FLAGS(LightAnimator, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)


// ProjectorSprite.
typedef struct ProjectorSprite_t
{
	BaseClass m_BaseClass;
} ProjectorSprite;

BEGIN_CLASS(ProjectorSprite)
	ADD_STRINGPROP(Filename, "sprite.spr")
END_CLASS_DEFAULT_FLAGS(ProjectorSprite, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)



// GlobalDirLight class (just uses the Light code).
typedef struct GlobalDirLight_t
{
	BaseClass m_BaseClass;
} GlobalDirLight;

BEGIN_CLASS(GlobalDirLight)
	ADD_VECTORPROP_VAL_FLAG(BigDims, 90.0f, 90.0f, 90.0f, PF_DIMS|PF_HIDDEN)
	ADD_COLORPROP(InnerColor, 255.0f, 255.0f, 255.0f)
	ADD_COLORPROP(OuterColor, 0.0f, 0.0f, 0.0f)
	ADD_REALPROP(BrightScale, 1.0f)
END_CLASS_DEFAULT_FLAGS(GlobalDirLight, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)



// World class..
typedef struct
{
	BaseClass m_BaseClass;
} World;

BEGIN_CLASS(World)
END_CLASS_DEFAULT_FLAGS(World, BaseClass, DNULL, DNULL, CF_ALWAYSLOAD)




// Brush class.
typedef struct
{
	BaseClass m_BaseClass;
} Brush;


BEGIN_CLASS(Brush)
	ADD_BOOLPROP(Solid, 1)
	ADD_BOOLPROP(Nonexistant, 0)
	ADD_BOOLPROP(Invisible, 0)
	ADD_BOOLPROP(Translucent, 0)
	ADD_BOOLPROP(SkyPortal, 0)
	ADD_BOOLPROP(FullyBright, 0)
	ADD_BOOLPROP(FlatShade, 0)
	ADD_BOOLPROP(GouraudShade, 1)
	ADD_BOOLPROP(LightMap, 1)
	ADD_BOOLPROP(Subdivide, 1)
	ADD_BOOLPROP(HullMaker, 0)
	ADD_BOOLPROP(AlwaysLightMap, 0)
	ADD_BOOLPROP(DirectionalLight, 0)
	ADD_BOOLPROP(Portal, 0)
	ADD_BOOLPROP(NoSnap, 0)
	ADD_BOOLPROP(SkyPan, 0)

	ADD_LONGINTPROP(DetailLevel, 0)
	ADD_STRINGPROP(Effect, "")
	ADD_STRINGPROP(EffectParam, "")
	ADD_REALPROP(FrictionCoefficient, 1)
END_CLASS_DEFAULT_FLAGS(Brush, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)



// DemoSkyWorldModel class.  This is a WorldModel that adds itself to the sky object
// list and has properties that level designers can edit to set the sky dimensions.
// If the sky box is set to zero, then it won't set it.  (This is so you can have
// multiple DemoSkyWorldModels in the world, and only one sets the sky box).
// The DemoSkyWorldModel uses InnerPercentX, Y, and Z as a percentage of the 
// SkyDims box for the inner box.
// Each sky world model must have a (unique) index from 0-30.  Ones with
// lower indices get drawn first.
typedef struct
{
	BaseClass	BaseClass;
	DVector		SkyDims;
	float		InnerPercentX, InnerPercentY, InnerPercentZ;
	long		Index;
} DemoSkyWorldModel;


static DDWORD DemoSky_EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	DemoSkyWorldModel *pModel;
	SkyDef def;
	DVector pos, temp;
	ObjectCreateStruct *pStruct;
	HOBJECT hObject;

	pModel = (DemoSkyWorldModel*)pObject;
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_ObjectType = OT_WORLDMODEL;

			if( lData == 1.0f )
			{
				g_pServerDE->GetPropVector("SkyDims", &pModel->SkyDims);
				g_pServerDE->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
				g_pServerDE->GetPropReal("InnerPercentX", &pModel->InnerPercentX);
				g_pServerDE->GetPropReal("InnerPercentY", &pModel->InnerPercentY);
				g_pServerDE->GetPropReal("InnerPercentZ", &pModel->InnerPercentZ);
				g_pServerDE->GetPropLongInt("Index", &pModel->Index);
			}
			else
			{
				VEC_INIT( pModel->SkyDims );
				pModel->InnerPercentX = 0.1f;
				pModel->InnerPercentY = 0.1f;
				pModel->InnerPercentZ = 0.1f;
				pModel->Index = 0;
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			// Set the sky box?
			if(pModel->SkyDims.x != 0.0f && pModel->SkyDims.y != 0.0f && pModel->SkyDims.z != 0.0f)
			{
				g_pServerDE->GetObjectPos(pModel->BaseClass.m_hObject, &pos);
				VEC_SUB(def.m_Min, pos, pModel->SkyDims);
				VEC_ADD(def.m_Max, pos, pModel->SkyDims);

				temp.x = pModel->SkyDims.x * pModel->InnerPercentX;
				temp.y = pModel->SkyDims.y * pModel->InnerPercentY;
				temp.z = pModel->SkyDims.z * pModel->InnerPercentZ;

				VEC_SUB(def.m_ViewMin, pos, temp);
				VEC_ADD(def.m_ViewMax, pos, temp);

				g_pServerDE->SetSkyDef(&def);
			}

			hObject = pModel->BaseClass.m_hObject;
			g_pServerDE->SetObjectFlags(hObject, g_pServerDE->GetObjectFlags(hObject) | (FLAG_SKYOBJECT|FLAG_FORCEOPTIMIZEOBJECT));
			g_pServerDE->AddObjectToSky(pModel->BaseClass.m_hObject, pModel->Index);
			break;
		}
	}

	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}


BEGIN_CLASS(DemoSkyWorldModel)
	ADD_VECTORPROP_VAL_FLAG(SkyDims, 0.0f, 0.0f, 0.0f, PF_DIMS)
	ADD_LONGINTPROP(Flags, 1)
	ADD_LONGINTPROP(Index, 0)
	ADD_REALPROP(InnerPercentX, 0.1f)
	ADD_REALPROP(InnerPercentY, 0.1f)
	ADD_REALPROP(InnerPercentZ, 0.1f)
END_CLASS_DEFAULT_FLAGS(DemoSkyWorldModel, BaseClass, DemoSky_EngineMessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)




// This works exactly the same as a DemoSkyWorldModel, except you identify the object by name.
typedef struct
{
	BaseClass	BaseClass;
	DVector		SkyDims;
	float		InnerPercentX, InnerPercentY, InnerPercentZ;
	long		Index;
	char		m_ObjectName[100];
	HOBJECT		m_hObject;
} SkyPointer;


static DDWORD SkyPointer_EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	SkyPointer *pModel;
	SkyDef def;
	DVector pos, temp;
	ObjectCreateStruct *pStruct;
	ObjectList *pList;
	HOBJECT hObject;

	pModel = (SkyPointer*)pObject;
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;

			pStruct->m_ObjectType = OT_NORMAL;
			pModel->m_hObject = 0;

			if( lData == 1.0f )
			{
				g_pServerDE->GetPropVector("SkyDims", &pModel->SkyDims);
				g_pServerDE->GetPropString("Name", pStruct->m_Filename, MAX_CS_FILENAME_LEN);
				g_pServerDE->GetPropString("SkyObjectName", pModel->m_ObjectName, sizeof(pModel->m_ObjectName)-1);
				g_pServerDE->GetPropReal("InnerPercentX", &pModel->InnerPercentX);
				g_pServerDE->GetPropReal("InnerPercentY", &pModel->InnerPercentY);
				g_pServerDE->GetPropReal("InnerPercentZ", &pModel->InnerPercentZ);
				g_pServerDE->GetPropLongInt("Index", &pModel->Index);
			}
			else
			{
				pModel->m_ObjectName[0] = 0;
				VEC_INIT( pModel->SkyDims );
				pModel->InnerPercentX = 0.1f;
				pModel->InnerPercentY = 0.1f;
				pModel->InnerPercentZ = 0.1f;
				pModel->Index = 0;
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			// Set the sky box?
			if(pModel->SkyDims.x != 0.0f && pModel->SkyDims.y != 0.0f && pModel->SkyDims.z != 0.0f)
			{
				g_pServerDE->GetObjectPos(pModel->BaseClass.m_hObject, &pos);
				VEC_SUB(def.m_Min, pos, pModel->SkyDims);
				VEC_ADD(def.m_Max, pos, pModel->SkyDims);

				temp.x = pModel->SkyDims.x * pModel->InnerPercentX;
				temp.y = pModel->SkyDims.y * pModel->InnerPercentY;
				temp.z = pModel->SkyDims.z * pModel->InnerPercentZ;

				VEC_SUB(def.m_ViewMin, pos, temp);
				VEC_ADD(def.m_ViewMax, pos, temp);

				g_pServerDE->SetSkyDef(&def);
			}

			g_pServerDE->SetNextUpdate(pModel->BaseClass.m_hObject, 0.001f);
			break;
		}

		case MID_UPDATE:
		{
			// Add the first object to the sky.
			pList = g_pServerDE->FindNamedObjects(pModel->m_ObjectName);
			if(pList && pList->m_pFirstLink)
			{
				hObject = pList->m_pFirstLink->m_hObject;
				g_pServerDE->AddObjectToSky(hObject, pModel->Index);
				g_pServerDE->SetObjectFlags(hObject, g_pServerDE->GetObjectFlags(hObject) | (FLAG_SKYOBJECT|FLAG_FORCEOPTIMIZEOBJECT));
				g_pServerDE->RelinquishList(pList);
			}
			
			g_pServerDE->RemoveObject(pModel->BaseClass.m_hObject);
			break;
		}
	}

	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}


BEGIN_CLASS(SkyPointer)
	ADD_STRINGPROP(SkyObjectName, "")
	ADD_VECTORPROP_VAL_FLAG(SkyDims, 0.0f, 0.0f, 0.0f, PF_DIMS)
	ADD_LONGINTPROP(Flags, 1)
	ADD_LONGINTPROP(Index, 0)
	ADD_REALPROP(InnerPercentX, 0.1f)
	ADD_REALPROP(InnerPercentY, 0.1f)
	ADD_REALPROP(InnerPercentZ, 0.1f)
END_CLASS_DEFAULT_FLAGS(SkyPointer, BaseClass, SkyPointer_EngineMessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)



// These generate areas of fog.
typedef struct FogSphere_t
{
	BaseClass m_BaseClass;
	float m_Radius;
} FogSphere;

static DDWORD FogSphere_EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float lData)
{
	ObjectCreateStruct *pStruct;
	GenericProp prop;
	FogSphere *pSphere;


	pSphere = (FogSphere*)pObject;
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			pStruct = (ObjectCreateStruct*)pData;
			pStruct->m_ObjectType = OT_LIGHT;
			pStruct->m_Flags = FLAG_VISIBLE | FLAG_FOGLIGHT;

			g_pServerDE->GetPropGeneric("LightRadius", &prop);
			pSphere->m_Radius = prop.m_Float;			
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pServerDE->SetLightRadius(pObject->m_hObject, pSphere->m_Radius);
		}
		break;
	}

	return bc_EngineMessageFn(pObject, messageID, pData, lData);
}

BEGIN_CLASS(FogSphere)
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS)
END_CLASS_DEFAULT_FLAGS(FogSphere, BaseClass, FogSphere_EngineMessageFn, bc_ObjectMessageFn, CF_ALWAYSLOAD)



// These objects, when used with the preprocessor's -RaySelect flag, will cause the
// first brush this object's forward vector hits to be selected in the ED file.
typedef struct RaySelecter_t
{
	BaseClass	m_BaseClass;
} RaySelecter;

BEGIN_CLASS(RaySelecter)
END_CLASS_DEFAULT_FLAGS(RaySelecter, BaseClass, bc_EngineMessageFn, bc_ObjectMessageFn, CF_NORUNTIME)


