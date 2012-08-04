
#include "bdefs.h"
#include "de_objects.h"



LTRESULT SpriteControlImpl::GetNumAnims(uint32 &nAnims)
{
	nAnims = m_pSprite->GetSprite()->m_nAnims;
	return LT_OK;
}

LTRESULT SpriteControlImpl::GetNumFrames(uint32 iAnim, uint32 &nFrames)
{
	CHECK_PARAMS(iAnim < m_pSprite->GetSprite()->m_nAnims, SpriteControl::GetNumFrames);
	nFrames = m_pSprite->GetSprite()->m_Anims[iAnim].m_nFrames;
	return LT_OK;
}

LTRESULT SpriteControlImpl::GetCurPos(uint32 &iAnim, uint32 &iFrame)
{
	SpriteAnim *pCurAnim;

	pCurAnim = m_pSprite->m_SpriteTracker.m_pCurAnim;
	iAnim = pCurAnim - m_pSprite->GetSprite()->m_Anims;
	iFrame = m_pSprite->m_SpriteTracker.m_pCurFrame - pCurAnim->m_Frames;
	return LT_OK;
}

LTRESULT SpriteControlImpl::SetCurPos(uint32 iAnim, uint32 iFrame)
{
	SpriteAnim *pAnim;

	CHECK_PARAMS(iAnim < m_pSprite->GetSprite()->m_nAnims && 
		iFrame < m_pSprite->GetSprite()->m_Anims[iAnim].m_nFrames, SpriteControl::SetCurPos);

	pAnim = &m_pSprite->GetSprite()->m_Anims[iAnim];
	m_pSprite->m_SpriteTracker.m_pCurAnim = pAnim;
	m_pSprite->m_SpriteTracker.m_pCurFrame = &m_pSprite->m_SpriteTracker.m_pCurAnim->m_Frames[iFrame];
	m_pSprite->m_SpriteTracker.m_MsCurTime = (iFrame * 1000) / pAnim->m_MsFrameRate;
	return LT_OK;
}

LTRESULT SpriteControlImpl::GetFlags(uint32 &flags)
{
	flags = m_pSprite->m_SpriteTracker.m_Flags;
	return LT_OK;
}

LTRESULT SpriteControlImpl::SetFlags(uint32 flags)
{
	m_pSprite->m_SpriteTracker.m_Flags = flags;
	return LT_OK;
}

//------------------------------------------------------------------
//
//   FUNCTION : GetAnimLength()
//
//   PURPOSE  : Retrieves a specific animation length in milliseconds
//
//------------------------------------------------------------------

LTRESULT SpriteControlImpl::GetAnimLength(uint32 &msLen, const uint32 iAnim)
{
	if (iAnim >= m_pSprite->GetSprite()->m_nAnims)
	{
		return LT_ERROR;
	}

	SpriteAnim *pAnim = m_pSprite->GetSprite()->m_Anims + iAnim;
	
	msLen = pAnim->m_MsFrameRate * pAnim->m_nFrames;

	// Success !!

	return LT_OK;
}

//------------------------------------------------------------------
//
//   FUNCTION : GetFrameTextureHandle()
//
//   PURPOSE  : Retrieves a texture handle for a specific frame
//
//------------------------------------------------------------------

LTRESULT SpriteControlImpl::GetFrameTextureHandle(HTEXTURE &hTex, const uint32 iAnim, const uint32 iFrame)
{
	if (iAnim >= m_pSprite->GetSprite()->m_nAnims)
	{
		return LT_ERROR;
	}
	
	SpriteAnim *pAnim;
	pAnim = m_pSprite->GetSprite()->m_Anims + iAnim;

	if (pAnim)
	{
		hTex = (HTEXTURE)pAnim->m_Frames[iFrame].m_pTex;
		if (!hTex) return LT_ERROR;

		// Inc it's ref count
		hTex->SetRefCount(hTex->GetRefCount() + 1);

		// Success !!
		return LT_OK;
	}

	// Failure !!

	return LT_ERROR;
}

