// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGrid.cpp
//
// PURPOSE : PolyGrid - Implementation
//
// CREATED : 10/20/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyGrid.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "CommonUtilities.h"

BEGIN_CLASS(PolyGrid)
	ADD_VECTORPROP_FLAG(Dims, PF_DIMS | PF_LOCALDIMS)
    ADD_COLORPROP(Color1, 255.0f, 255.0f, 255.0f)
    ADD_COLORPROP(Color2, 255.0f, 255.0f, 255.0f)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "Tex\\Water\\Wa01\\Wa0001.dtx", PF_FILENAME)
    ADD_REALPROP(XScaleMin, 15.0f)
    ADD_REALPROP(XScaleMax, 15.0f)
    ADD_REALPROP(YScaleMin, 15.0f)
    ADD_REALPROP(YScaleMax, 15.0f)
    ADD_REALPROP(XScaleDuration, 10.0f)
    ADD_REALPROP(YScaleDuration, 10.0f)
    ADD_REALPROP(XPan, 10.0f)
    ADD_REALPROP(YPan, 10.0f)
	ADD_REALPROP(Alpha, 0.7f)
    ADD_BOOLPROP(Additive, LTFALSE)
    ADD_BOOLPROP(Multiply, LTFALSE)
	ADD_LONGINTPROP(NumPolies, 160)
	PROP_DEFINEGROUP(PlasmaInfo, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(PlasmaType, 1, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Ring1Rate, 50, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Ring2Rate, 10, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Ring3Rate, 30, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Ring4Rate, 20, PF_GROUP1)

END_CLASS_DEFAULT(PolyGrid, CClientSFX, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::PolyGrid
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

PolyGrid::PolyGrid() : CClientSFX()
{
	VEC_SET(m_vDims, 32.0f, 32.0f, 0.0f);
	VEC_SET(m_vColor1, 50.0f, 50.0f, 200.0f);
	VEC_SET(m_vColor2, 0.0f, 0.0f, 220.0f);

	m_fXScaleMin = 15.0f;
	m_fXScaleMax = 15.0f;
	m_fYScaleMin = 15.0f;
	m_fYScaleMax = 15.0f;
	m_fXScaleDuration = 10.0f;
	m_fYScaleDuration = 10.0f;
	m_fXPan = 10.0f;
	m_fYPan = 10.0f;
    m_hstrSurfaceSprite = LTNULL;
	m_fAlpha = 0.7f;
    m_bAdditive = LTFALSE;
    m_bMultiply = LTFALSE;

	m_nPlasmaType = 1;

    m_bSetup = LTFALSE;

	m_dwNumPolies = 160;

	m_nRingRate[0] = 50;
	m_nRingRate[1] = 10;
	m_nRingRate[2] = 30;
	m_nRingRate[3] = 20;
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
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSurfaceSprite)
	{
		pServerDE->FreeString(m_hstrSurfaceSprite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::Setup
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void PolyGrid:: Setup(LTVector* pvDims, LTVector* pvColor1, LTVector* pvColor2,
                      HSTRING hstrSurfaceSprite, LTFLOAT fXScaleMin, LTFLOAT fXScaleMax,
                      LTFLOAT fYScaleMin, LTFLOAT fYScaleMax, LTFLOAT fXScaleDuration,
                      LTFLOAT fYScaleDuration, LTFLOAT fXPan, LTFLOAT fYPan,
                      LTFLOAT fAlpha, uint32 dwNumPolies, LTBOOL bAdditive,
                      LTBOOL bMultiply)
{
    uint16 wColor1, wColor2;

    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !pvDims || !pvColor1 || !pvColor2 || dwNumPolies < 4) return;

	VEC_COPY(m_vDims, *pvDims);
	pServerDE->SetObjectDims(m_hObject, pvDims);
	VEC_COPY(m_vColor1, *pvColor1);
	VEC_COPY(m_vColor2, *pvColor2);

	wColor1 = Color255VectorToWord( pvColor1 );
	wColor2 = Color255VectorToWord( pvColor2 );

	// Limit the number of polies to 32k...
	if ( dwNumPolies > 0x7FFF )
		dwNumPolies = 0x7FFF;

	HSTRING hstrSprite = hstrSurfaceSprite ? hstrSurfaceSprite : pServerDE->CreateString("");

	// Tell the clients about the polygrid...
	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_POLYGRID_ID);
	pServerDE->WriteToMessageVector(hMessage, &m_vDims);
	pServerDE->WriteToMessageWord(hMessage, wColor1);
	pServerDE->WriteToMessageWord(hMessage, wColor2);
	pServerDE->WriteToMessageFloat(hMessage, fXScaleMin);
	pServerDE->WriteToMessageFloat(hMessage, fXScaleMax);
	pServerDE->WriteToMessageFloat(hMessage, fYScaleMin);
	pServerDE->WriteToMessageFloat(hMessage, fYScaleMax);
	pServerDE->WriteToMessageFloat(hMessage, fXScaleDuration);
	pServerDE->WriteToMessageFloat(hMessage, fYScaleDuration);
	pServerDE->WriteToMessageFloat(hMessage, fXPan);
	pServerDE->WriteToMessageFloat(hMessage, fYPan);
	pServerDE->WriteToMessageFloat(hMessage, fAlpha);
	pServerDE->WriteToMessageHString(hMessage, hstrSprite);
    pServerDE->WriteToMessageWord(hMessage, (uint16)dwNumPolies);
	pServerDE->WriteToMessageByte(hMessage, bAdditive);
	pServerDE->WriteToMessageByte(hMessage, bMultiply);
	pServerDE->WriteToMessageByte(hMessage, m_nPlasmaType);
	pServerDE->WriteToMessageByte(hMessage, m_nRingRate[0]);
	pServerDE->WriteToMessageByte(hMessage, m_nRingRate[1]);
	pServerDE->WriteToMessageByte(hMessage, m_nRingRate[2]);
	pServerDE->WriteToMessageByte(hMessage, m_nRingRate[3]);
	pServerDE->EndMessage(hMessage);

	if (!hstrSurfaceSprite)
	{
		pServerDE->FreeString(hstrSprite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PolyGrid::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				ReadProp((ObjectCreateStruct *)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				pServerDE->SetObjectUserFlags(m_hObject, USRFLG_VISIBLE);

				if (m_bSetup)
				{
					Setup(&m_vDims, &m_vColor1, &m_vColor2, m_hstrSurfaceSprite,
						  m_fXScaleMin, m_fXScaleMax, m_fYScaleMin, m_fYScaleMax,
						  m_fXScaleDuration, m_fYScaleDuration, m_fXPan, m_fYPan,
						  m_fAlpha, m_dwNumPolies, m_bAdditive, m_bMultiply);
				}
			}

			CacheFiles();
			break;
		}

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void PolyGrid::ReadProp(ObjectCreateStruct * pStruct)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pStruct->m_Flags = FLAG_VISIBLE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("SpriteSurfaceName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSurfaceSprite = pServerDE->CreateString(buf);

    if (pServerDE->GetPropVector("Dims", &m_vDims) == LT_OK)
	{
        m_bSetup = LTTRUE;
	}

	pServerDE->GetPropVector("Color1", &m_vColor1);
	pServerDE->GetPropVector("Color2", &m_vColor2);
	pServerDE->GetPropReal("XScaleMin", &m_fXScaleMin);
	pServerDE->GetPropReal("XScaleMax", &m_fXScaleMax);
	pServerDE->GetPropReal("YScaleMin", &m_fYScaleMin);
	pServerDE->GetPropReal("YScaleMax", &m_fYScaleMax);
	pServerDE->GetPropReal("XScaleDuration", &m_fXScaleDuration);
	pServerDE->GetPropReal("YScaleDuration", &m_fYScaleDuration);
	pServerDE->GetPropReal("XPan", &m_fXPan);
	pServerDE->GetPropReal("YPan", &m_fYPan);
	pServerDE->GetPropReal("Alpha", &m_fAlpha);
	pServerDE->GetPropBool("Additive", &m_bAdditive);
	pServerDE->GetPropBool("Multiply", &m_bMultiply);

	long nLongVal;
    if (pServerDE->GetPropLongInt("NumPolies", &nLongVal) == LT_OK)
	{
        m_dwNumPolies = (uint32)nLongVal;
	}

	pServerDE->GetPropLongInt("PlasmaType", &nLongVal);
    m_nPlasmaType = (uint8)nLongVal;

	pServerDE->GetPropLongInt("Ring1Rate", &nLongVal);
    m_nRingRate[0] = (uint8)nLongVal;

	pServerDE->GetPropLongInt("Ring2Rate", &nLongVal);
    m_nRingRate[1] = (uint8)nLongVal;

	pServerDE->GetPropLongInt("Ring3Rate", &nLongVal);
    m_nRingRate[2] = (uint8)nLongVal;

	pServerDE->GetPropLongInt("Ring4Rate", &nLongVal);
    m_nRingRate[3] = (uint8)nLongVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PolyGrid::CacheFiles
//
//	PURPOSE:	Cache the resources associated with this object...
//
// ----------------------------------------------------------------------- //

void PolyGrid::CacheFiles()
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSurfaceSprite)
	{
		char* pFile = pServerDE->GetStringData(m_hstrSurfaceSprite);
		if (pFile && pFile[0])
		{
			pServerDE->CacheFile(FT_SPRITE, pFile);
		}
	}
}