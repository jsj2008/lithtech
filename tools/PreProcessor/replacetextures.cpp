
#include "bdefs.h"
#include "editregion.h"
#include "editpoly.h"
#include "preworld.h"
#include "findworldmodel.h"


class CTempSurface
{
	public:
		
		CEditBrush *m_pBrush;
		CEditPoly *m_pPoly;
		CPrePlane m_Plane;
};


BOOL IsPolyInsideSurface(CTempSurface *pSurface, CPrePoly *pPoly)
{
	DWORD i, j;
	CEditPoly *pEditPoly;
	CPrePlane edgePlane;
	PVector vTemp, testPt;
	
	pEditPoly = pSurface->m_pPoly;
	for(i=0; i < pEditPoly->NumVerts(); i++)
	{
		VEC_SUB(vTemp, pEditPoly->NextPt(i), pEditPoly->Pt(i));
		VEC_CROSS(edgePlane.m_Normal, vTemp, pEditPoly->Normal());
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = VEC_DOT(edgePlane.m_Normal, pEditPoly->Pt(i));

		for(j=0; j < pPoly->NumVerts(); j++)
		{
			testPt = pPoly->Pt(j);

			if(DIST_TO_PLANE(testPt, edgePlane) < -0.1f)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL MatchSurface(CTempSurface *pSurface, CPrePoly *pPoly)
{
	CPrePlane *pPlane, diffPlane;

	pPlane = pPoly->m_pSurface->m_pPlane;

	// Are the planes the same?
	diffPlane.m_Normal.x = (PReal)fabs(pPlane->m_Normal.x - pSurface->m_Plane.m_Normal.x);
	diffPlane.m_Normal.y = (PReal)fabs(pPlane->m_Normal.y - pSurface->m_Plane.m_Normal.y);
	diffPlane.m_Normal.z = (PReal)fabs(pPlane->m_Normal.z - pSurface->m_Plane.m_Normal.z);
	diffPlane.m_Dist = (PReal)fabs(pPlane->m_Dist - pSurface->m_Plane.m_Dist);
	
	if(diffPlane.m_Normal.x < 0.01f && diffPlane.m_Normal.y < 0.01f && 
		diffPlane.m_Normal.z < 0.01f && diffPlane.m_Dist < 0.01f)
	{
		return IsPolyInsideSurface(pSurface, pPoly);
	}
	else
	{
		return FALSE;
	}
}


CTempSurface* MatchSurface(CMoArray<CTempSurface*> &surfaces, CPrePoly *pPoly)
{
	DWORD i;

	for(i=0; i < surfaces.GetSize(); i++)
	{
		if(MatchSurface(surfaces[i], pPoly))
			return surfaces[i];
	}

	return NULL;
}


int ReplaceTexturesInWorld(CMoArray<CTempSurface*> &surfaces, CPreWorld *pWorld)
{
	int nReplaced;
	GPOS pos;
	CPrePoly *pPoly;
	CTempSurface *pTempSurface;


	nReplaced = 0;

	// Go thru each poly in the world.
	for(pos=pWorld->m_Polies; pos; )
	{
		pPoly = pWorld->m_Polies.GetNext(pos);
		pTempSurface = MatchSurface(surfaces, pPoly);
		if(pTempSurface)
		{
			assert(CEditPoly::NUM_TEXTURES == CPreSurface::NUM_TEXTURES);

			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				CPreTexture& Tex = pPoly->m_pSurface->m_Texture[nCurrTex];

				if(strcmp(pTempSurface->m_pPoly->GetTexture(nCurrTex).m_pTextureName, Tex.m_pTextureName) != 0)
				{
					Tex.m_pTextureName = pWorld->m_StringHolder.AddString(pTempSurface->m_pPoly->GetTexture(nCurrTex).m_pTextureName);
					pPoly->m_pSurface->m_Flags = (WORD)GetBrushType(pTempSurface->m_pBrush);
					
					if(stricmp(Tex.m_pTextureName, "textures\\invisible.dtx") == 0)
						pPoly->m_pSurface->m_Flags |= SURF_INVISIBLE;

					++nReplaced;
				}
			}
		}
	}

	return nReplaced;
}


int ReplaceTextures(CEditRegion *pRegion, CPreMainWorld *pWorld)
{
	int nReplaced;
	DWORD i, iPoly;
	CEditBrush *pBrush;
	CMoArray<CTempSurface*> surfaces;
	CTempSurface *pTempSurface;
	LPOS brushPos;


	// Build the surface list.
	surfaces.SetCacheSize(3000);
	for(brushPos=pRegion->m_Brushes; brushPos; )
	{
		pBrush = pRegion->m_Brushes.GetNext(brushPos);
		
		for(iPoly=0; iPoly < pBrush->m_Polies; iPoly++)
		{
			pTempSurface = new CTempSurface;
			pTempSurface->m_pPoly = pBrush->m_Polies[iPoly];
			pTempSurface->m_pBrush = pBrush;
			PLANE_COPY(pTempSurface->m_Plane, pBrush->m_Polies[iPoly]->m_Plane);

			surfaces.Append(pTempSurface);
		}
	}

	nReplaced = 0;
	for(i=0; i < pWorld->m_WorldModels; i++)
	{
		nReplaced += ReplaceTexturesInWorld(surfaces, pWorld->m_WorldModels[i]);
	}

	DeleteAndClearArray(surfaces);
	return nReplaced;
}






