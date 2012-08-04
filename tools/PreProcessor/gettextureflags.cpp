
#include "bdefs.h"
#include "processing.h"
#include "preworld.h"
#include "gettextureinfo.h"


// Applies texture flags to all the world's surfaces.  Returns the number
// of surfaces it applied flags to.
uint32 ApplyTextureFlagsToWorld(CGLinkedList<TInfo*> &theList, CPreMainWorld *pMainWorld)
{
	uint32		nFlagsGotten = 0;

	for(uint32 i=0; i < pMainWorld->m_WorldModels; i++)
	{
		CPreWorld *pWorld = pMainWorld->m_WorldModels[i];
		for(GPOS pos = pWorld->m_Surfaces; pos; )
		{
			CPreSurface *pSurface = pWorld->m_Surfaces.GetNext(pos);

			for(uint32 nCurrTex = 0; nCurrTex < CPreSurface::NUM_TEXTURES; nCurrTex++)
			{
				CPreTexture& Tex = pSurface->m_Texture[nCurrTex];

				if(!Tex.IsValid())
					continue;

				TInfo *pInfo = FindTInfo(theList, Tex.m_pTextureName);
				if(pInfo)
				{	
					Tex.m_TextureFlags = pInfo->m_Flags;
					Tex.m_TextureWidth = pInfo->m_Width;
					Tex.m_TextureHeight = pInfo->m_Height;
					++nFlagsGotten;
				}
			}
		}
	}

	return nFlagsGotten;
}


uint32 GetTextureFlags(CPreMainWorld *pWorld)
{
	CGLinkedList<TInfo*>	theList;
	if(GetTextureInfo(pWorld, theList) == 0)
	{
		DrawStatusText(eST_Error, "Couldn't find any textures. Make sure that the project path is properly set");
		return 0;
	}

	uint32 nFlagsGotten = ApplyTextureFlagsToWorld(theList, pWorld);
	
	GDeleteAndRemoveElements(theList);
	DrawStatusText(eST_Normal, "Imported %d texture flags", nFlagsGotten);

	return nFlagsGotten;
}




