#ifndef __TEXTUREDIMS_H__
#define __TEXTUREDIMS_H__

#include "streamsim.h"
#include "dtxmgr.h"
#include "spritefile.h"

//reads in the texture dimensions
inline bool GetTextureDims(const char* pszTextureName, uint32& nWidth, uint32& nHeight)
{
	char pszFileName[MAX_PATH + 1];

	//make sure that the path has a trailing slash on it
	uint32 nLen = strlen(g_pGlobs->m_ProjectDir);

	if(nLen && (g_pGlobs->m_ProjectDir[nLen - 1] != '\\'))
	{
		sprintf(pszFileName, "%s\\%s", g_pGlobs->m_ProjectDir, pszTextureName);
	}
	else
	{
		sprintf(pszFileName, "%s%s", g_pGlobs->m_ProjectDir, pszTextureName);
	}

	//see if this is a sprite
	if(CSpriteFile::IsSprite(pszFileName))
	{
		//this is a sprite, so what we need to do is open it, grab the first frame,
		//and use that as the texture name
		CSpriteFile Sprite;
		if(!Sprite.Load(pszFileName))
			return false;

		//grab the first frame if available
		if(Sprite.GetNumFrames() == 0)
			return false;

		const char* pszFrame = Sprite.GetFrame(0);

		if(strlen(pszFrame) && (pszFrame[nLen - 1] != '\\'))
		{
			sprintf(pszFileName, "%s\\%s", g_pGlobs->m_ProjectDir, pszFrame);
		}
		else
		{
			sprintf(pszFileName, "%s%s", g_pGlobs->m_ProjectDir, pszFrame);
		}
	}
			
	ILTStream* pStream = streamsim_Open(pszFileName, "rb");
	if(pStream)
	{
		TextureData		*pData;

		if(dtx_Create(pStream, &pData, TRUE, TRUE) == DE_OK)
		{
			nWidth  = pData->m_Header.m_BaseWidth;
			nHeight = pData->m_Header.m_BaseHeight;
			dtx_Destroy(pData);
			pStream->Release();
			return true;
		}			

		pStream->Release();
	}

	return false;
}

#endif
