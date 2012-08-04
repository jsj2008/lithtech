#include "stdafx.h"
#include "ShatterEffectMgr.h"
#include "ShatterEffect.h"

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

//unique value to ensure that we don't conflict with other blind object data
#define SHATTERINFO_BLINDOBJECTID	0x00000a85		// just a random, but constant 32-bit ID


CShatterEffectMgr::CShatterEffectMgr()
{
	//reserve a certain amount of data for our list of shatter effects
	m_ShatterList.reserve(32);
}

CShatterEffectMgr::~CShatterEffectMgr()
{
	//free up any shatter effects that we may currently have
	FreeShatterEffects();
}

//called when a new shatter effect needs to be created, and is passed all the parameters needed
//to create the new effect
bool CShatterEffectMgr::CreateShatterEffect(uint32 nShatterDataID, const LTRigidTransform& tObjTransform, 
											const LTVector& vHitPos, const LTVector& vHitDir,
											HRECORD hShatterType)
{
	//check for valid input parameters
	if(!hShatterType)
		return false;

	//we first off need to try and find the blind object data to shatter this world model
	uint8* pWMData;
	uint32 nDataSize;
	if(g_pLTClient->GetBlindObjectData(nShatterDataID, SHATTERINFO_BLINDOBJECTID, pWMData, nDataSize) != LT_OK)
	{
		//failed to find the blind object data necessary to create the shatter effect
		return false;
	}
#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	// Note: All shatter effects currently load 32-bit quantities, and only 32-bit quantities.
	// We can therefore switch the data entirely in one block. Since shatter data can be reused though,
	// make sure to only swap it once. The first value is always a poly count, so use that to determine
	// if swapping is needed
	if(*((uint32*)pWMData) > 0xFFFF)
	{
		LTASSERT((nDataSize % sizeof(uint32)) == 0, "Invalid shatter blind object data size encountered!");
		LittleEndianToNative((uint32*)pWMData, nDataSize / sizeof(uint32));
	}
#endif // PLATFORM_XENON

	//create the shatter effect object
	CShatterEffect* pNewEffect = debug_new(CShatterEffect);
	if(!pNewEffect)
		return false;

	//now actually initialize the new effect
	if(!pNewEffect->Init(pWMData, tObjTransform, vHitPos, vHitDir, hShatterType))
	{
		debug_delete(pNewEffect);
		return false;
	}

	//now add this to the list of shatter effects
	m_ShatterList.push_back(pNewEffect);

	//success
	return true;
}

//called to update all of the shatter effects
void CShatterEffectMgr::UpdateShatterEffects(float fElapsedS)
{
	//don't bother updating if no time has elapsed
	if(fElapsedS == 0.0f)
		return;

	//just run through and update all of the shatter effects, removing any objects that fail
	for(TShatterList::iterator it = m_ShatterList.begin(); it != m_ShatterList.end(); )
	{
		if(!(*it)->Update(fElapsedS))
		{
			debug_delete(*it);
			it = m_ShatterList.erase(it);
		}
		else
		{
			it++;
		}
	}
}

//called to free all of the shatter effect objects
void CShatterEffectMgr::FreeShatterEffects()
{
	//just run through and free all of the shatter effects
	for(TShatterList::iterator it = m_ShatterList.begin(); it != m_ShatterList.end(); it++)
	{
		debug_delete(*it);
	}
	m_ShatterList.resize(0);
}