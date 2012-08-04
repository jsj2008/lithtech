// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMgr.h
//
// PURPOSE : Special FX Mgr	- Definition
//
// CREATED : 10/24/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MGR_H__
#define __SFX_MGR_H__

#include "iltclient.h"
#include "SpecialFXList.h"
#include "SFXMsgIds.h"
#include "LightGroupFX.h"
#include "TextureFXMgr.h"


#define DYN_ARRAY_SIZE		(SFX_TOTAL_NUMBER + 1)
#define	CAMERA_LIST_SIZE	20

class CCharacterFX;
class CBodyFX;

class CSFXMgr
{
	public :

        CSFXMgr()  {}
		~CSFXMgr() {}

        LTBOOL   Init(ILTClient* pClientDE);

        CSpecialFX* FindSpecialFX(uint8 nType, HLOCALOBJ hObj);

		CSpecialFXList* GetCameraList() { return &m_cameraSFXList; }

		void	RemoveSpecialFX(HLOCALOBJ hObj);
		void	UpdateSpecialFX();
		void	RenderFX(HOBJECT hCamera);
		void	HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read *pMsg);

		void	RemoveAll();

        CSpecialFX* CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct,
            ILTMessage_Read *pMsg=LTNULL, HOBJECT hServerObj=LTNULL);

		static void	DeleteSFX(CSpecialFX *pSFX);

		inline CSpecialFXList* GetFXList(uint8 nType) 
		{ 
			if (nType < 0 || nType >= DYN_ARRAY_SIZE) return LTNULL;

			return &m_dynSFXLists[nType]; 
		}

		void OnObjectRotate( HOBJECT hObj, bool bTeleport, LTRotation *pNewRot );
		void OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		void OnSFXMessage(ILTMessage_Read *pMsg);

		CCharacterFX* GetCharacterFX(HOBJECT hObject);
		CCharacterFX* GetCharacterFromHitBox(HOBJECT hHitBox);
		CBodyFX* GetBodyFX(HOBJECT hObject);
		CBodyFX* GetBodyFromHitBox(HOBJECT hHitBox);

#ifdef __PSX2
		// Get nearby characters
		int GetCharactersInRadius(const LTVector& vecOrigin, float fRadius, CPtrList &lstResult,
									LTBOOL bSearchGood = LTTRUE, LTBOOL bSearchNeutral = LTTRUE, 
									LTBOOL bSearchBad = LTTRUE);

		int GetCharactersInCone(const LTVector& vecOrigin, const LTVector& vecForward, float fAngle,
									float fDist, CPtrList &lstResult, LTBOOL bSearchGood = LTTRUE,
									LTBOOL bSearchNeutral = LTTRUE, LTBOOL bSearchBad = LTTRUE);
#endif

	private :

        bool    AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

        int             GetDynArrayIndex(uint8 nFXId);
        unsigned int    GetDynArrayMaxNum(uint8 nArrayIndex);

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE]; // Lists of dynamic special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx

		// Special case handler for the lightgroup fx messages
		CLightGroupFXMgr m_cLightGroupFXMgr;

		// Handler for all the texture effects messages
		CTextureFXMgr	m_cTextureFXMgr;
};

//////////////////////////////////////////////////////////////////////////////
// Functions below are used for GetCharactersInRadius
//////////////////////////////////////////////////////////////////////////////
//
// BoxesIntersect
//
// Tests if boxes intersect each other.
// Arguments
//		vBox1Min - Mininim of box 1
//		vBox1Max - Maximum of box 1
//		vBox2Min - Mininim of box 2
//		vBox2Max - Maximum of box 2
//
static bool BoxesIntersect( const LTVector &vBox1Min, const LTVector &vBox1Max, const LTVector &vBox2Min, const LTVector &vBox2Max )
{
	if(	vBox1Min.x - vBox2Max.x >= 0.0f || vBox1Max.x - vBox2Min.x <= 0.0f ||
		vBox1Min.y - vBox2Max.y >= 0.0f || vBox1Max.y - vBox2Min.y <= 0.0f ||
		vBox1Min.z - vBox2Max.z >= 0.0f || vBox1Max.z - vBox2Min.z <= 0.0f )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// SetupBoxPoints
//
// Calculates the points of a box.
// Arguments
//		vPts[8] - OUT:  Array of 8 points to fill in.
//		vDims - IN:  dims.
//
//////////////////////////////////////////////////////////////////////////////
//
// SetupBoxPoints
//
// Calculates the points of a box.
// Arguments
//		vPts[8] - OUT:  Points to fill in.
//		vDims - IN:  dims.
//
static void SetupBoxPoints( LTVector *pvPts, const LTVector &vPos, const LTVector &vDims )
{
	VEC_SET(pvPts[0], +vDims.x, +vDims.y, +vDims.z);
	pvPts[0] += vPos;
	VEC_SET(pvPts[1], +vDims.x, -vDims.y, +vDims.z);
	pvPts[1] += vPos;
	VEC_SET(pvPts[2], +vDims.x, +vDims.y, -vDims.z);
	pvPts[2] += vPos;
	VEC_SET(pvPts[3], +vDims.x, -vDims.y, -vDims.z);
	pvPts[3] += vPos;
	VEC_SET(pvPts[4], -vDims.x, +vDims.y, +vDims.z);
	pvPts[4] += vPos;
	VEC_SET(pvPts[5], -vDims.x, -vDims.y, +vDims.z);
	pvPts[5] += vPos;
	VEC_SET(pvPts[6], -vDims.x, +vDims.y, -vDims.z);
	pvPts[6] += vPos;
	VEC_SET(pvPts[7], -vDims.x, -vDims.y, -vDims.z);
	pvPts[7] += vPos;
}

#endif // __SFX_MGR_H__