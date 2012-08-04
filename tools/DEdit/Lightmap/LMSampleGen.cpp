#include "bdefs.h"
#include "editobjects.h"
#include "LMSampleGen.h"
#include "LMLight.h"
#include "geomroutines.h"


//determines if the passed in object is a light or not
static bool IsObjectLight(CBaseEditObj* pObj)
{
	if( (strcmp(pObj->GetClassName(), "DirLight") == 0) ||
		(strcmp(pObj->GetClassName(), "Light") == 0))
	{
		return true;
	}

	//no go
	return false;
}




CLMSampleGen::CLMSampleGen() :
	m_pHead(NULL),
	m_bLambertian(false),
	m_bShadows(false),
	m_vAmbient(0, 0, 0)
{
}

CLMSampleGen::~CLMSampleGen()
{
	ClearLightList();
}

//inserts a light into the list of lights that can contribute to the lightmaps
bool CLMSampleGen::UpdateLightList(CMoArray<CBaseEditObj*> *pObjList)
{
	//remove any old lights
	ClearLightList();

	//now build up the data structures
	for(uint32 nCurrObj = 0; nCurrObj < pObjList->GetSize(); nCurrObj++)
	{
		bool bIsLight = false;
		bool bIsOmni;

		CBaseEditObj* pObj = (*pObjList)[nCurrObj];

		//check for the different light types
		if(strcmp(pObj->GetClassName(), "Light") == 0)
		{
			bIsLight	= true;
			bIsOmni		= true;
		}
		else if(strcmp(pObj->GetClassName(), "DirLight") == 0)
		{
			bIsLight	= true;
			bIsOmni		= false;
		}

		//see if we hit a light
		if(bIsLight)
		{
			CLMLight* pNewLight = new CLMLight;

			if(pNewLight)
			{
				//now read in the properties
				ConvertLight(pObj, pNewLight, bIsOmni);
				AddLight(pNewLight);
			}
		}
	}

	//success
	return true;
}

static inline void CalcLightContribution(CLMLight* pLight, const LTVector& vPos, 
										const LTVector& vNormal, LTVector& vColor,
										bool bLambertian, bool bShadows, CShadowCalc* pCalc)
{
	ASSERT(pLight);

	LTVector	vRayDir;
	CReal		fDot, fAnglePercent, fDistPercent, fFovDotLimit;
	CReal		fAttenuation, fDist, fDistSqr;


	//find a unit vector from the light to the sample position
	vRayDir = vPos - pLight->m_vPos;
	
	//get the distance squared
	fDistSqr = vRayDir.MagSqr();

	//distance cull
	if(fDistSqr >= pLight->m_fRadiusSqr)
		return;

	//find the normal distance
	fDist = (PReal)sqrt(fDistSqr);
	vRayDir /= fDist; //this normalizes. Assumes distSqr is accurate

	
	// lambert falloff factor:
	// 1.0 = full bright
	// 0.0 = unlit
	if (bLambertian)
	{
		if(pLight->m_bIsOmni)
		{
			fAttenuation = (-vNormal).Dot(vRayDir);
		}
		else
		{
			fAttenuation = (-vNormal).Dot(pLight->m_vDir);
		}

		if(fAttenuation < 0.0f)
		{
			return;
		}
	}
	else
	{
		fAttenuation = 1.0f;
	}

	//calculate the distance attenuation
	fDistPercent = 1.0f / (pLight->m_fCoA + pLight->m_fCoB * fDist + pLight->m_fCoC * fDistSqr); 

	//check to see if we have an omni light
	if(pLight->m_bIsOmni)
	{
		//all possible early outs have been done...see if we are in shadows
		if(bShadows)
		{
			if((vRayDir.Dot(-vNormal) < 0.0f) || pCalc->IsSegmentBlocked(vPos, -vRayDir, fDist, pLight))
				return;
		}

		//we have an omni directional light
		vColor +=  (pLight->m_vInnerColor * fDistPercent  			
					+ pLight->m_vOuterColor * (1.0f - fDistPercent))
					* fAttenuation
					* pLight->m_fBrightScale;

		return;
	}

	//we have a directional light
	fDot = vRayDir.Dot(pLight->m_vDir);
	
	fFovDotLimit = pLight->m_fCosHalfAngle;
	if(fDot < fFovDotLimit)
	{
		//not in the field of view of the light
		return;
	}

	//all possible early outs have been done...see if we are in shadows
	if(bShadows)
	{
		if((vRayDir.Dot(-vNormal) < 0.0f) || pCalc->IsSegmentBlocked(vPos, -vRayDir, fDist, pLight))
			return;
	}

	fAnglePercent = (fDot - fFovDotLimit) / (1.0f - fFovDotLimit);

	vColor +=  (pLight->m_vInnerColor * fAnglePercent + pLight->m_vOuterColor * (1.0f - fAnglePercent))
				* fDistPercent 
				* fAttenuation
				* pLight->m_fBrightScale;
}




//calculates a light value for the specified point and normal
void CLMSampleGen::CalcSample(	const LTVector& vPos, const LTVector& vNormal, 
								const CLightHolderOptions& Options,
								uint8& nR, uint8& nG, uint8& nB)
{
	//the inital color. The ambient color of the holder
	LTVector vColor(Options.m_nAmbientR, Options.m_nAmbientG, Options.m_nAmbientB);

	//see if we need to receive light
	if(Options.m_bReceiveLight)
	{
		vColor += m_vAmbient;

		//just run through and calculate light contributions
		CLMLight* pCurr = m_pHead;
		for(; pCurr != NULL; pCurr = pCurr->m_pNext)
		{
			CalcLightContribution(	pCurr, vPos, vNormal, vColor, 
									m_bLambertian, 
									(m_bShadows && Options.m_bReceiveShadows), 
									&m_ShadowCalc);
		}
	}

	//clamp and assign
	nR = (int)(LTMIN(255, LTMAX(0, vColor.x)));
	nG = (int)(LTMIN(255, LTMAX(0, vColor.y)));
	nB = (int)(LTMIN(255, LTMAX(0, vColor.z)));
}

//gets the specified LMLight given the object light
CLMLight* CLMSampleGen::GetLight(CBaseEditObj* pLight)
{
	CLMLight* pCurr = m_pHead;

	for(; pCurr; pCurr = pCurr->m_pNext)
	{
		if(pCurr->m_pSrcObject == pLight)
		{
			//found a match
			return pCurr;
		}
	}

	//no match
	return NULL;
}

//converts a light object to a lightmap light object
void CLMSampleGen::ConvertLight(CBaseEditObj* pSrc, CLMLight *pDest, bool bIsOmni)
{
	//sanity check
	ASSERT(pSrc);
	ASSERT(pDest);

	pDest->m_vPos			= pSrc->GetPos();
	pDest->m_bIsOmni		= bIsOmni;
	pDest->m_pSrcObject		= pSrc;

	//setup the radius
	CRealProp* pRadius		= (CRealProp*)pSrc->GetPropertyList()->GetProp("LightRadius");
	
	pDest->m_fRadius		= (pRadius) ? pRadius->m_Value : 0.0f;
	pDest->m_fRadiusSqr		= pDest->m_fRadius * pDest->m_fRadius;	

	//read in the outer color
	if(CVectorProp* pOuterColor = (CVectorProp*)pSrc->GetPropertyList()->GetProp("OuterColor"))
	{
		pDest->m_vOuterColor = pOuterColor->m_Vector;
	}

	//read in the brightscale
	if(CRealProp* pBrightScale = (CRealProp*)pSrc->GetPropertyList()->GetProp("BrightScale"))
	{
		pDest->m_fBrightScale = pBrightScale->m_Value;
	}


	//need to calculate the ABC values of the light
	CVectorProp* pCoef		= (CVectorProp*)pSrc->GetPropertyList()->GetProp("AttCoefs");
	CVectorProp* pExponent	= (CVectorProp*)pSrc->GetPropertyList()->GetProp("AttExps");

	if(pCoef && pExponent)
	{
		// a, b, and c must be [0,+inf)
		pDest->m_fCoA = (pCoef->m_Vector.x < 0.0f) ? 0.0f : pCoef->m_Vector.x;
		pDest->m_fCoB = (pCoef->m_Vector.y < 0.0f) ? 0.0f : pCoef->m_Vector.y;
		pDest->m_fCoC = (pCoef->m_Vector.z < 0.0f) ? 0.0f : pCoef->m_Vector.z;

		// at least one of a, b, or c must be non-zero
		if(pDest->m_fCoA + pDest->m_fCoB + pDest->m_fCoC == 0.0f )
			pDest->m_fCoA = 1.0f;

		pDest->m_fCoA *= (float)pow( pDest->m_fRadius, pExponent->m_Vector.x );
		pDest->m_fCoB *= (float)pow( pDest->m_fRadius, pExponent->m_Vector.y );
		pDest->m_fCoC *= (float)pow( pDest->m_fRadius, pExponent->m_Vector.z );
	}

	if(!bIsOmni)
	{
		//read in the inner color
		if(CVectorProp* pInnerColor = (CVectorProp*)pSrc->GetPropertyList()->GetProp("InnerColor"))
		{
			pDest->m_vInnerColor = pInnerColor->m_Vector;
		}

		//we need to calculate the direction and the field of view angle
		CRealProp* pFOV = (CRealProp*)pSrc->GetPropertyList()->GetProp("FOV");

		//read in the half angle
		pDest->m_fCosHalfAngle = (pFOV) ? pFOV->m_Value / 2.0f : 0.0f;

		float fAngleRad = pDest->m_fCosHalfAngle * MATH_PI / 180.0f;

		//convert it to the cosine
		pDest->m_fCosHalfAngle = (float)cos(fAngleRad);

		//calculate the tangent of the half angle
		if(fabs(pDest->m_fCosHalfAngle) > 0.01f)
		{
			//directional light's tangent of the half angle (for culling purposes)
			pDest->m_fTanHalfAngle = (float)sin(fAngleRad) / pDest->m_fCosHalfAngle;
		}
		else
		{
			pDest->m_fTanHalfAngle = 0.0f;
		}

		//now we need to read in the direction of this light
		if(CRotationProp* pRot = (CRotationProp*)pSrc->GetPropertyList()->GetProp("Rotation"))
		{
			LTVector vRight, vUp, vForward;

			gr_GetEulerVectors(
				pRot->GetEulerAngles(),
				vRight,
				vUp,
				vForward);

			pDest->m_vDir = vForward;
		}
	}
	else
	{
		//read in the inner color
		if(CVectorProp* pInnerColor = (CVectorProp*)pSrc->GetPropertyList()->GetProp("LightColor"))
		{
			pDest->m_vInnerColor = pInnerColor->m_Vector;
		}
	}

}


//Adds a light onto the list. The list will be in charge of deleting and maintaining
//the light
void CLMSampleGen::AddLight(CLMLight* pLight)
{
	if(pLight)
	{
		//we have a light. Just set up its links
		pLight->m_pNext = m_pHead;
		pLight->m_pPrev = NULL;

		if(m_pHead)
		{
			m_pHead->m_pPrev = pLight;
		}

		m_pHead = pLight;
	}
}

//removes a light from the list. This will also delete the light
void CLMSampleGen::DeleteLight(CLMLight* pLight)
{
	
	if(pLight)
	{
		//first fix up our links
		if(m_pHead == pLight)
		{
			m_pHead = pLight->m_pNext;
		}

		//now fix up the lights links
		if(pLight->m_pNext)
		{
			pLight->m_pNext->m_pPrev = pLight->m_pPrev;
		}
		if(pLight->m_pPrev)
		{
			pLight->m_pPrev->m_pNext = pLight->m_pNext;
		}

		//now free the light
		delete pLight;
	}
}

//clears the entire light list
void CLMSampleGen::ClearLightList()
{
	CLMLight* pCurr = m_pHead;
	CLMLight* pNext;

	//delete all the lights
	while(pCurr)
	{
		pNext = pCurr->m_pNext;
		delete pCurr;
		pCurr = pNext;
	}

	//now clean up our list
	m_pHead = NULL;
}


