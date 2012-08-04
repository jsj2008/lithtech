// ----------------------------------------------------------------------- //
//
// MODULE  : ModelDecalMgr.cpp
//
// PURPOSE : Implementation of class used to manage model decals
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModelDecalMgr.h"
#include "iltrenderer.h"
#include "VarTrack.h"
#include <malloc.h>
#include <algorithm>

#include "ILTRenderer.h"
extern ILTRenderer *g_pLTRenderer;

// Console variable for overriding the model decal performance level for convenience.
// Note: Code should use the class interface instead of updating this variable!!!
VarTrack g_CV_ModelDecalPerformanceLevel;

CGameModelDecalMgr* g_pModelDecalMgr = NULL;

// Value indicating that a decal isn't currently fading
#define FADETIME_NOT_FADING (-FLT_MAX)

CGameModelDecalMgr::CGameModelDecalMgr()
{
	LTASSERT(g_pModelDecalMgr == NULL, "Multiple model decal managers instantiated!");
	g_pModelDecalMgr = this;
	
	g_CV_ModelDecalPerformanceLevel.Init(g_pLTClient, "ModelDecalPerformanceLevel", "1", 1.0f);
	
	LoadDecalTypes();
	
	m_fLastDecalTime = -1.0f;
	
	// Set the restriction settings to unrestricted to begin with
	m_fMaxDecals = -1.0f;
	m_fDecalsPerModel = -1.0f;
	m_fDecalsPerSecond = -1.0f;
	
	// Initialize the performance settings category
	if (!DATABASE_CATEGORY(ModelDecalSettings).Init())
	{
		LTERROR("Error initializing model decal settings database");
	}

	// Load first performance settings record by default valid values of
	// m_nPerformancelevel are 1, 2 and 3
	m_nPerformanceLevel = 1;
	LoadPerformanceSettings();
	
	m_hDeletingDecal = NULL;
	// Register the deletion callback
#ifdef _DEBUG
	ModelDecalDeleteFn pOldCallback = NULL;
	void *pOldData;
	g_pLTRenderer->GetModelDecalDeleteFn(pOldCallback, pOldData);
	LTASSERT(pOldCallback == NULL, "Model decal deletion notification function already set!");
#endif // _DEBUG
	g_pLTRenderer->SetModelDecalDeleteFn(DecalDeleteCallback, this);
}

CGameModelDecalMgr::~CGameModelDecalMgr()
{
	g_pModelDecalMgr = NULL;
	
	// Can't clean up if the renderer's already terminated...
	if (!g_pLTRenderer)	
		return;
		
	// Release the materials
	TDecalTypeList::iterator iCurType = m_aDecalTypes.begin();
	for (; iCurType != m_aDecalTypes.end(); ++iCurType)
	{
		TMaterialList::iterator iCurMaterial = iCurType->m_aMaterials.begin();
		for (; iCurMaterial != iCurType->m_aMaterials.end(); ++iCurMaterial)
		{
			g_pLTRenderer->ReleaseMaterialInstance(*iCurMaterial);
		}
	}

	// Un-register the deletion callback
#ifdef _DEBUG
	ModelDecalDeleteFn pOldCallback = NULL;
	void *pOldData = NULL;
	g_pLTRenderer->GetModelDecalDeleteFn(pOldCallback, pOldData);
	LTASSERT(pOldData == this, "Model decal deletion notification function changed before shutdown!");
#endif // _DEBUG
	g_pLTRenderer->SetModelDecalDeleteFn(NULL, NULL);
}


void CGameModelDecalMgr::Save(ILTMessage_Write* pMsg, SaveDataState eSaveDataState)
{
	// Save the decal types
	pMsg->Writeuint32(m_aDecalTypes.size());
	for (uint32 nDecalNameLoop = 0; nDecalNameLoop < m_aDecalTypes.size(); ++nDecalNameLoop)
	{
		HRECORD hRecord = DATABASE_CATEGORY(ModelDecal).GetRecordByIndex(nDecalNameLoop);
		pMsg->WriteString(DATABASE_CATEGORY(ModelDecal).GetRecordName(hRecord));
	}
	
	// Make sure our decal list is clean
	CleanDecalList();
	
	// Save the decals
	pMsg->Writeuint32(m_aDecals.size());
	TDecalList::iterator iCurDecal = m_aDecals.begin();
	for (; iCurDecal != m_aDecals.end(); ++iCurDecal)
	{
		pMsg->Writeuint32(iCurDecal->m_nType);
		LTMatrix3x4 mProjection;
		g_pLTRenderer->GetModelDecalProjection(iCurDecal->m_hDecal, mProjection);
		pMsg->WriteType(mProjection);
		// Write out the dims and position of the object, so we can try to figure out
		// what object we were referring to when we load
		// Note : This must happen because we can't save object references from the client!
		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(iCurDecal->m_hObject, &vDims);
		LTVector vPos;
		g_pLTClient->GetObjectPos(iCurDecal->m_hObject, &vPos);
		pMsg->WriteLTVector(vDims);
		pMsg->WriteLTVector(vPos);
	}
}

void CGameModelDecalMgr::Load(ILTMessage_Read* pMsg, SaveDataState eLoadDataState)
{
	// Fill the decal type translation table
	uint32 nNumDecalTypes = pMsg->Readuint32();
	Tuint32List aTranslationTable(nNumDecalTypes, k_nInvalidDecalType);
	Tuint32List::iterator iCurType = aTranslationTable.begin();
	char aDecalNameBuffer[256];
	for (; iCurType != aTranslationTable.end(); ++iCurType)
	{
		pMsg->ReadString(aDecalNameBuffer, LTARRAYSIZE(aDecalNameBuffer));
		*iCurType = GetDecalType(aDecalNameBuffer);
	}

	// Clear any decals we already have
	while (!m_aDecals.empty())
	{
		RemoveDecal(m_aDecals.size() - 1);
	}
	
	// Load the decals	
	uint32 nNumDecals = pMsg->Readuint32();
	m_aLoadRecords.reserve(nNumDecals);
	for (uint32 nCurDecal = 0; nCurDecal < nNumDecals; ++nCurDecal)
	{
		// Translate the decal type
		uint32 nTableIndex = pMsg->Readuint32();
		uint32 nDecalType = nTableIndex < aTranslationTable.size() ? aTranslationTable[nTableIndex] : k_nInvalidDecalType;
		// Filter out decals of an unknown type
		if (nDecalType == k_nInvalidDecalType)
		{
			// Skip the rest of the record
			pMsg->ReadLTVector();
			pMsg->ReadLTVector();
			continue;
		}
		// Add a loading record, so the decal can be created on the next update
		// Note : This must happen because we can't save object references from the client!
		m_aLoadRecords.push_back(SLoadRecord());
		SLoadRecord &sCurLoadRecord = m_aLoadRecords.back();
		sCurLoadRecord.m_nDecalType = nDecalType;
		pMsg->ReadType(&sCurLoadRecord.m_mProjection);
		sCurLoadRecord.m_vDims = pMsg->ReadLTVector();
		sCurLoadRecord.m_vPos = pMsg->ReadLTVector();
	}
}

void CGameModelDecalMgr::FindObjectAfterLoadFilterFn(HOBJECT hObj, void* pUserData)
{
	SFindObjectsAfterLoadInfo* pInfo = (SFindObjectsAfterLoadInfo*)pUserData;
	
	// Test the object dims & position against our load record
	LTVector vObjPos, vObjDims;
	g_pLTClient->GetObjectPos(hObj, &vObjPos);
	g_pPhysicsLT->GetObjectDims(hObj, &vObjDims);
	if (vObjPos.NearlyEquals(pInfo->m_pLoadRecord->m_vPos, 1.0f) && 
		vObjDims.NearlyEquals(pInfo->m_pLoadRecord->m_vDims, 1.0f))
	{
		// We found a match.
		pInfo->m_hResult = hObj;
	}
}

void CGameModelDecalMgr::Update()
{
	// Allow console override access to the performance level
	if ((uint32)g_CV_ModelDecalPerformanceLevel.GetFloat() != GetPerformanceLevel())
		SetPerformanceLevel((uint32)g_CV_ModelDecalPerformanceLevel.GetFloat());
		
	if (m_bCleanOnNextUpdate)
	{
		CleanDecalList();
		m_bCleanOnNextUpdate = false;
	}

	// Process the load records
	if (!m_aLoadRecords.empty())
	{
		m_aDecals.reserve(m_aDecals.size() + m_aLoadRecords.size());
		do 
		{
			// Get the next load record
			SLoadRecord& sCurLoadRecord = m_aLoadRecords.back();
			// Set up the record for the query function
			SFindObjectsAfterLoadInfo sInfo;
			sInfo.m_pLoadRecord = &sCurLoadRecord;
			sInfo.m_hResult = INVALID_HOBJECT;
			// Find the object of our affection
			LTVector vMin, vMax;
			g_pLTClient->FindObjectsInBox(sCurLoadRecord.m_vPos, sCurLoadRecord.m_vDims + LTVector(1.0f, 1.0f, 1.0f), 
							FindObjectAfterLoadFilterFn, &sInfo);
			// Add a decal if we found something				
			if (sInfo.m_hResult != INVALID_HOBJECT)
			{
				// Don't worry about time restrictions
				m_fLastDecalTime = -1;
				AddDecal(sInfo.m_hResult, sCurLoadRecord.m_nDecalType, sCurLoadRecord.m_mProjection);
			}
			// Remove it from the list
			m_aLoadRecords.pop_back();
		} while (!m_aLoadRecords.empty());
	}
	
	UpdateFading();
}

void CGameModelDecalMgr::AddDecal(HOBJECT hObject, HMODELNODE hNode, uint32 nDecalType, const LTVector& vOrigin, const LTVector& vDirection)
{
	// Filter invalid decal requests
	if (nDecalType >= m_aDecalTypes.size())
	{
		return;
	}

	// Look up the decal type
	const SDecalType &sDecalType = m_aDecalTypes[nDecalType];

	// Get the inverse transform for the node
	LTTransform tInvNodeTransform;
	if (LT_OK != g_pModelLT->GetNodeTransform(hObject, hNode, tInvNodeTransform, true))
	{
		LTERROR("Failed to get node transform for model to project decal.  Verify that this is expected behavior.");
		return;
	}

	tInvNodeTransform.Inverse();
	
	// Move the ray into the node's space
	LTVector vRelOrigin, vRelDirection;
	tInvNodeTransform.Transform(vOrigin, vRelOrigin);
	vRelDirection = tInvNodeTransform.m_rRot.RotateVector(vDirection);
	
	// Get the binding pose transform for the node
	LTRigidTransform tBindingSpace;
	if (LT_OK != g_pModelLT->GetBindPoseNodeTransform(hObject, hNode, tBindingSpace))
	{
		LTERROR("Failed to get bind pose for model to project decal.  Verify that this is expected behavior.");
		return;
	}
	
	// Move the ray into object binding space
	LTVector vBindOrigin = tBindingSpace * vRelOrigin;
	LTVector vBindDirection = tBindingSpace.m_rRot * vRelDirection;
	
	// Build the basis space for the ray
	LTVector vBindUp = vBindDirection.BuildOrthonormal();
	
	// Get the radius based on decal type
	float fRadius = sDecalType.m_fRadius + GetRandom(-sDecalType.m_fRadiusVariance, sDecalType.m_fRadiusVariance);

	// Set up the projection matrix	
	LTTransform tBindBasis(vBindOrigin / fRadius, LTRotation(vBindDirection, vBindUp), fRadius);
	// Add in a random rotation
	tBindBasis.m_rRot.Rotate(tBindBasis.m_rRot.Forward(), GetRandom(0.0f, MATH_TWOPI));
	// Get the inverse for the projection
	LTMatrix3x4 mProjection;
	tBindBasis.GetInverse().ToMatrix(mProjection);
	// Scale by 1/2, invert Y and offset by 1/2 to transform into texture space
	LTMatrix3x4 mTemp(	0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, -0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 1.0f, 0.0f);

	mProjection = mTemp * mProjection;

	// Add the decal
	AddDecal(hObject, nDecalType, mProjection);
}
	
void CGameModelDecalMgr::AddDecal(HOBJECT hObject, uint32 nDecalType, const LTMatrix3x4 &mProjection)
{
	// Filter invalid decal requests
	if (nDecalType >= m_aDecalTypes.size())
	{
		return;
	}
	
	// Intermediate result variable
	LTRESULT nResult;
	// Current simulation time
	double fCurTime = SimulationTimer::Instance().GetTimerAccumulatedS();

	// Total
	if ((m_fMaxDecals >= 0.0f) && (m_aDecals.size() >= (uint32)m_fMaxDecals))
		return;	

	// Over time
	float fSecondsSinceLastDecal = (float)(fCurTime - m_fLastDecalTime);
	if ((m_fDecalsPerSecond > 0.0f) && (m_fLastDecalTime >= 0.0f) && (fSecondsSinceLastDecal < (1.0f / m_fDecalsPerSecond)))
		return;

	// Per-model
	uint32 nNumModelDecals;
	nResult = g_pLTRenderer->GetModelDecals(hObject, NULL, nNumModelDecals);
	if (nResult != LT_OK)
		return;
	if ((m_fDecalsPerModel >= 0.0f) && (nNumModelDecals >= (uint32)m_fDecalsPerModel))
		return;

	// Look up the decal type
	SDecalType &sDecalType = m_aDecalTypes[nDecalType];

	// Get the material based on decal type
	const TMaterialList &aMaterials = sDecalType.m_aMaterials;
	if (aMaterials.empty())
		return;
	HMATERIAL hMaterial = aMaterials[GetRandom(0, aMaterials.size() - 1)];
	
	// Create the decal
	HMODELDECAL hDecal;
	nResult = g_pLTRenderer->CreateModelDecal(hObject, mProjection, hMaterial, false, 0, hDecal);
	if (nResult != LT_OK)
		return;
	
	// Add it to the list
	m_aDecals.push_back(SDecal());
	SDecal &sNewDecal = m_aDecals.back();
	sNewDecal.m_hDecal = hDecal;
	sNewDecal.m_hObject = hObject;
	// Register for deletion notification
	sNewDecal.m_hObject.SetReceiver(*this);
	sNewDecal.m_nType = nDecalType;
	// Add it to the fading list, if appropriate for this decal type
	if (sDecalType.m_fFadeDelay >= 0.0f)
	{
		sNewDecal.m_fFadeStartTime = fCurTime + sDecalType.m_fFadeDelay;
		m_aFadingDecals.push_back(m_aDecals.size() - 1);
	}
	else
	{
		sNewDecal.m_fFadeStartTime = FADETIME_NOT_FADING;
	}

	// Remember now as the last decal creation time	
	m_fLastDecalTime = fCurTime;
}

uint32 CGameModelDecalMgr::GetDecalType(const char* pDecalName)
{
	return GetDecalType(DATABASE_CATEGORY(ModelDecal).GetRecordByName(pDecalName));
}

uint32 CGameModelDecalMgr::GetDecalType(HRECORD hRecord)
{
	if (hRecord == NULL)
		return k_nInvalidDecalType;

	return DATABASE_CATEGORY(ModelDecal).GetRecordIndex(hRecord);
}

void CGameModelDecalMgr::FadeDecals(HOBJECT hModel)
{
	// Shortcut-out if this model doesn't have any decals (the engine can check in constant time)
	uint32 nNumDecals = 0;
	g_pLTRenderer->GetModelDecals(hModel, NULL, nNumDecals);
	if (nNumDecals == 0)
		return;
		
	// Current simulation time
	double fCurTime = SimulationTimer::Instance().GetTimerAccumulatedS();
	
	// Go through all the active decals
	TDecalList::iterator iCurDecal = m_aDecals.begin();
	for (; iCurDecal != m_aDecals.end(); ++iCurDecal)
	{
		// Skip decals that aren't attached to this model
		if (iCurDecal->m_hObject != hModel)
			continue;
		// Get the fading delay time for this decal
		double fTypeDelay = m_aDecalTypes[iCurDecal->m_nType].m_fFadeDelay;
		// Skip decals that are already fading
		if (iCurDecal->m_fFadeStartTime != FADETIME_NOT_FADING)
		{
			// Update the fade start time if the fade delay of the decal type hasn't elapsed yet
			if ((fTypeDelay > 0.0f) && ((fCurTime - iCurDecal->m_fFadeStartTime) < 0.0f))
			{
				iCurDecal->m_fFadeStartTime = fCurTime - fTypeDelay;
			}
			continue;
		}
		// Set the fade start time, offset by the delay time
		iCurDecal->m_fFadeStartTime = fCurTime + LTMAX(fTypeDelay, 0.0f);
		// Start it fading, and add it to the fading list
		m_aFadingDecals.push_back(iCurDecal - m_aDecals.begin());
	}
}

void CGameModelDecalMgr::TransferDecals(HOBJECT hSource, HOBJECT hDest)
{
	// Find out how many decals there are
	uint32 nNumDecals = 0;
	g_pLTRenderer->GetModelDecals(hSource, NULL, nNumDecals);
	if (nNumDecals == NULL)
		return;
	// Get the decal list
	HMODELDECAL *pDecalList = (HMODELDECAL*)alloca(sizeof(HMODELDECAL*) * nNumDecals);
	g_pLTRenderer->GetModelDecals(hSource, pDecalList, nNumDecals);

	// Transfer over the decals
	HMODELDECAL *pCurDecal = pDecalList;
	HMODELDECAL *pEndDecal = pDecalList + nNumDecals;
	for (; pCurDecal != pEndDecal; ++pCurDecal)
	{
		g_pLTRenderer->SetModelDecalObject(*pCurDecal, hDest);
	}
	
	// Update the decal references to match
	TDecalList::iterator iCurDecal = m_aDecals.begin();
	for (; iCurDecal != m_aDecals.end(); ++iCurDecal)
	{
		if (iCurDecal->m_hObject == hSource)
			iCurDecal->m_hObject = hDest;
	}
}

void CGameModelDecalMgr::LoadDecalTypes()
{
	// Initialize the database
	if (!DATABASE_CATEGORY(ModelDecal).Init())
		return;
		
	// Load from the GDB
	uint32 nNumRecords = DATABASE_CATEGORY(ModelDecal).GetNumRecords();
	
	m_aDecalTypes.resize(nNumRecords);
	for (uint32 nDecalTypeLoop = 0; nDecalTypeLoop < m_aDecalTypes.size(); ++nDecalTypeLoop)
	{
		SDecalType &sCurDecalType = m_aDecalTypes[nDecalTypeLoop];

		// What record are we reading?
		HRECORD hRecord = DATABASE_CATEGORY(ModelDecal).GetRecordByIndex(nDecalTypeLoop);
		
		// Read the material list
		HATTRIBUTE hMaterialAttribute = DATABASE_CATEGORY(ModelDecal).GetAttribute(hRecord, "Material");
		uint32 nNumMaterials = g_pLTDatabase->GetNumValues(hMaterialAttribute);
		sCurDecalType.m_aMaterials.reserve(nNumMaterials);
		for (uint32 nCurMaterial = 0; nCurMaterial < nNumMaterials; ++nCurMaterial)
		{
			const char *pMaterialFile = g_pLTDatabase->GetString(hMaterialAttribute, nCurMaterial, NULL);
			if (pMaterialFile != NULL)
			{
				HMATERIAL hMaterial = g_pLTRenderer->CreateMaterialInstance(pMaterialFile);
				if (hMaterial != INVALID_MATERIAL)
					sCurDecalType.m_aMaterials.push_back(hMaterial);
			}
		}
		
		// Read the other parameters, which are much simpler
		sCurDecalType.m_fRadius = GETCATRECORDATTRIB(ModelDecal, hRecord, Radius);
		sCurDecalType.m_fRadiusVariance = GETCATRECORDATTRIB(ModelDecal, hRecord, RadiusVariance);
		sCurDecalType.m_fFadeDelay = GETCATRECORDATTRIB(ModelDecal, hRecord, FadeDelay);
		sCurDecalType.m_fFadeDuration = LTMAX(GETCATRECORDATTRIB(ModelDecal, hRecord, FadeDuration), 0.001f);
	}
}

void CGameModelDecalMgr::CleanDecalList()
{
	// Remove all decals referring to null objects.
	// RemoveDecal swaps the current element with the last and then pops off 
	// the back to the list; we don't need to do any special handling here.

	for ( size_t i = 0; i < m_aDecals.size(); ++i )
	{
		if (m_aDecals[i].m_hObject == INVALID_HOBJECT)
		{
			RemoveDecal( i );
		}
	}
}

void CGameModelDecalMgr::OnLinkBroken(LTObjRefNotifier* pRef, HOBJECT hObj)
{
	// Remember to clean up the decal list soon
	// Note: This is done later so we don't have to worry about any interactions with
	// removing the object references while in the middle of getting notifications
	m_bCleanOnNextUpdate = true;
}

void CGameModelDecalMgr::SetPerformanceLevel(uint32 nLevel)
{
	// Parameter check (valid values are between 1 and max number of ModelDecalSettings)
	if (nLevel < 1 || nLevel > DATABASE_CATEGORY(ModelDecalSettings).GetNumRecords())
		return;

	// Set the performance level & load it up
	m_nPerformanceLevel = nLevel;
	LoadPerformanceSettings();
	
	// Update the console variable to match
	g_CV_ModelDecalPerformanceLevel.SetFloat((float)m_nPerformanceLevel);
}

const char *CGameModelDecalMgr::GetPerformanceLevelName(uint32 nLevel) const
{
	// Parameter check
	if (nLevel >= DATABASE_CATEGORY(ModelDecalSettings).GetNumRecords())
		return NULL;

	// Look-up the record
	HRECORD hRecord = DATABASE_CATEGORY(ModelDecalSettings).GetRecordByIndex(nLevel);
	if (hRecord == NULL)
	{
		LTERROR("Unable to read performance settings record");
		return NULL;
	}
	
	// Return the name
	return DATABASE_CATEGORY(ModelDecalSettings).GetRecordName(hRecord);
}

uint32 CGameModelDecalMgr::GetNumPerformanceLevels() const
{
	// Read the count from the database
	return DATABASE_CATEGORY(ModelDecalSettings).GetNumRecords();
}

void CGameModelDecalMgr::LoadPerformanceSettings()
{
	// Look-up the record
	HRECORD hRecord = DATABASE_CATEGORY(ModelDecalSettings).GetRecordByIndex(m_nPerformanceLevel - 1);
	if (hRecord == NULL)
	{
		LTERROR("Unable to read performance settings record");
		return;
	}

	// Load the attributes
	m_fMaxDecals = GETCATRECORDATTRIB(ModelDecalSettings, hRecord, Total);
	m_fDecalsPerModel = GETCATRECORDATTRIB(ModelDecalSettings, hRecord, PerModel);
	m_fDecalsPerSecond = GETCATRECORDATTRIB(ModelDecalSettings, hRecord, PerSecond);
}

void CGameModelDecalMgr::RemoveDecal(uint32 nIndex)
{
	bool bIsLast = (nIndex == (m_aDecals.size() - 1));
	bool bWasFading = (m_aDecals[nIndex].m_fFadeStartTime != FADETIME_NOT_FADING);
	// If the last element in the decal list is fading, that entry in the  fading list will need to get updated
	// with a new index because it's going to get swapped with the deleted item
	bool bIsLastFading = (m_aDecals.back().m_fFadeStartTime != FADETIME_NOT_FADING) && (!bIsLast);

	// Remove it from the object
	if (m_aDecals[nIndex].m_hObject != INVALID_HOBJECT)
	{
		m_hDeletingDecal = m_aDecals[nIndex].m_hDecal;
		g_pLTRenderer->DestroyModelDecal(m_aDecals[nIndex].m_hDecal);
	}
			
	// Remove it from the main list
	if (!bIsLast)
		m_aDecals[nIndex] = m_aDecals.back();
	m_aDecals.pop_back();
	
	if (bWasFading)
	{
		// Remove the index from the fading list
		Tuint32List::iterator iErase = std::remove(m_aFadingDecals.begin(), m_aFadingDecals.end(), nIndex);
		m_aFadingDecals.erase(iErase, m_aFadingDecals.end());
	}
	
	if (bIsLastFading)
	{	
		// Re-number the fade entries for what used to be the last decal
		Tuint32List::iterator iCurFade = m_aFadingDecals.begin();
		for (; iCurFade != m_aFadingDecals.end(); ++iCurFade)
		{
			if (*iCurFade == m_aDecals.size())
				*iCurFade = nIndex;
		}
	}
}

void CGameModelDecalMgr::UpdateFading()
{
	// Avoid doing any work if nothing's fading
	if (m_aFadingDecals.empty())
		return;
		
	// Current simulation time
	double fCurTime = SimulationTimer::Instance().GetTimerAccumulatedS();
	
	for (uint32 nCurFade = 0; nCurFade < m_aFadingDecals.size(); ++nCurFade)
	{
		uint32 nFadeDecal = m_aFadingDecals[nCurFade];
		if (nFadeDecal >= m_aDecals.size())
		{
			LTERROR("Decal fade encountered past end of list!");
			continue;
		}
		SDecal &sCurDecal = m_aDecals[nFadeDecal];
		// Skip decals that are no longer associated
		if (sCurDecal.m_hObject == NULL)
			continue;
		// Calculate the fading percentage
		float fFadePercent = (float)((fCurTime - sCurDecal.m_fFadeStartTime) / m_aDecalTypes[sCurDecal.m_nType].m_fFadeDuration);
		// Skip decals that haven't started fading yet
		if (fFadePercent < 0.0f)
			continue;
		// Remove decals that are done fading
		if (fFadePercent >= 1.0f)
		{
			// Delete the decal 
			RemoveDecal(nFadeDecal);
			// Jump out if we removed the last decal
			if (nCurFade >= m_aFadingDecals.size())
				break;
			// Back up so we don't skip anybody
			--nCurFade;
			// We're done here...
			continue;
		}
		// Get the base object's color
		float fObjectR, fObjectG, fObjectB, fObjectA;
		g_pLTBase->GetObjectColor(sCurDecal.m_hObject, &fObjectR, &fObjectG, &fObjectB, &fObjectA);

		// Update the color
		g_pLTRenderer->SetModelDecalColorOverride(sCurDecal.m_hDecal, true);
		g_pLTRenderer->SetModelDecalColor(sCurDecal.m_hDecal, fObjectR, fObjectG, fObjectB, 1.0f - fFadePercent);
	}
}

void CGameModelDecalMgr::DecalDeleteCallback(HMODELDECAL hDecal, void *pUser)
{
	((CGameModelDecalMgr*)pUser)->OnDecalDeleted(hDecal);
}

void CGameModelDecalMgr::OnDecalDeleted(HMODELDECAL hDecal)
{
	// Skip decals we already know about
	if (hDecal != m_hDeletingDecal)
	{
		// Mark matching decals for deletion from the list and clean the list on the next update
		for (TDecalList::iterator iCurDecal = m_aDecals.begin(); iCurDecal != m_aDecals.end(); ++iCurDecal)
		{
			if (iCurDecal->m_hDecal == hDecal)
			{
				iCurDecal->m_hObject = NULL;
				iCurDecal->m_hDecal = NULL;
				m_bCleanOnNextUpdate = true;
			}
		}
	}
	
	m_hDeletingDecal = NULL;
}
