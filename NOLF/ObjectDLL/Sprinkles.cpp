
#include "stdafx.h"
#include "sprinkles.h"
#include "sfxmsgids.h"


#define ADD_SPRINKLE_PROP(num, defFilename) \
	ADD_STRINGPROP_FLAG(Filename##num##, defFilename, PF_FILENAME)\
	ADD_STRINGPROP_FLAG(SkinName##num##, "", PF_FILENAME)\
	ADD_LONGINTPROP(Count##num##, 16)\
	ADD_REALPROP(Speed##num##, 50.0f)\
	ADD_REALPROP(Size##num##, 10.0f)\
	ADD_REALPROP_FLAG(SpawnRadius##num##, 500.0f, PF_RADIUS)\
	ADD_COLORPROP(ColorMin##num##, 255.0f, 255.0f, 255.0f)\
	ADD_COLORPROP(ColorMax##num##, 255.0f, 255.0f, 255.0f)\
	ADD_VECTORPROP_VAL(AnglesVel##num##, 1.0f, 1.0f, 1.0f)



BEGIN_CLASS(Sprinkles)
	ADD_SPRINKLE_PROP(0, "SFX\\Snow\\SprTex\\Snow1.dtx")
	ADD_SPRINKLE_PROP(1, "")
	ADD_SPRINKLE_PROP(2, "")
	ADD_SPRINKLE_PROP(3, "")

	ADD_SPRINKLE_PROP(4, "")
	ADD_SPRINKLE_PROP(5, "")
	ADD_SPRINKLE_PROP(6, "")
	ADD_SPRINKLE_PROP(7, "")
END_CLASS_DEFAULT(Sprinkles, BaseClass, NULL, NULL)



// --------------------------------------------------------------------------------- //
// Sprinkles
// --------------------------------------------------------------------------------- //

Sprinkles::Sprinkles() : BaseClass(OT_NORMAL)
{
}


Sprinkles::~Sprinkles()
{
    if(g_pLTServer)
    {
        for(uint32 i = 0; i < MAX_SPRINKLE_TYPES; i++)
        {
            if (m_SprinkleStruct.m_Types[i].m_hFilename)
            {
                g_pLTServer->FreeString(m_SprinkleStruct.m_Types[i].m_hFilename);
            }

            if (m_SprinkleStruct.m_Types[i].m_hSkinName)
            {
                g_pLTServer->FreeString(m_SprinkleStruct.m_Types[i].m_hSkinName);
            }
        }
    }
}


uint32 Sprinkles::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			OnPreCreate((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate();
		}
		break;

		case MID_UPDATE:
		{
			OnUpdate();
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


void Sprinkles::OnPreCreate(ObjectCreateStruct *pStruct)
{
	char propName[64];
	GenericProp gProp;
	SPRINKLETYPECREATESTRUCT *pType;
    uint32 i;

	pStruct->m_Flags = FLAG_FORCECLIENTUPDATE;

	m_SprinkleStruct.m_nTypes = 0;
	for(i=0; i < MAX_SPRINKLE_TYPES; i++)
	{
		pType = &(m_SprinkleStruct.m_Types[i]);

		// FilenameX
		sprintf(propName, "Filename%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) != LT_OK)
			break;

		if(gProp.m_String[0] == 0)
			break;

        pType->m_hFilename = g_pLTServer->CreateString(gProp.m_String);
		if(!pType->m_hFilename)
			break;

		// SkinNameX
		sprintf(propName, "SkinName%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) != LT_OK)
			break;

        pType->m_hSkinName = g_pLTServer->CreateString(gProp.m_String);

		// CountX
		sprintf(propName, "Count%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
            pType->m_Count = (uint32)gProp.m_Long;
			if(pType->m_Count > 255)
			{
                g_pLTServer->CPrint("Warning: SprinklesFX count > 255, clamping");
				pType->m_Count = 255;
			}
		}

		// SpeedX
		sprintf(propName, "Speed%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_Speed = gProp.m_Float;
		}

		// SizeX
		sprintf(propName, "Size%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_Size = gProp.m_Float;
		}

		// SpawnRadiusX
		sprintf(propName, "SpawnRadius%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_SpawnRadius = gProp.m_Float;
		}

		// AnglesVelX
		sprintf(propName, "AnglesVel%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_AnglesVel = gProp.m_Vec;
		}

		// ColorMinX
		sprintf(propName, "ColorMin%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_ColorMin = gProp.m_Color;
		}

		// ColorMaxX
		sprintf(propName, "ColorMax%d", i);
        if(g_pLTServer->GetPropGeneric(propName, &gProp) == LT_OK)
		{
			pType->m_ColorMax = gProp.m_Color;
		}

		++m_SprinkleStruct.m_nTypes;
	}
}


void Sprinkles::OnInitialUpdate()
{
    ILTMessage *pMsg;

    if(g_pLTServer->Common()->CreateMessage(pMsg) == LT_OK)
	{
		pMsg->WriteByte(SFX_SPRINKLES_ID);

        m_SprinkleStruct.Write(g_pLTServer, pMsg);

        g_pLTServer->SetObjectSFXMessage(m_hObject, *pMsg);
		pMsg->Release();
	}

}


void Sprinkles::OnUpdate()
{
}