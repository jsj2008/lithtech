// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMgr.h
//
// PURPOSE : Special FX Mgr	- Definition
//
// CREATED : 10/24/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MGR_H__
#define __SFX_MGR_H__

#include "iltclient.h"
#include "SpecialFXList.h"
#include "SFXMsgIds.h"


#define DYN_ARRAY_SIZE			SFX_TOTAL_NUMBER
#define	CAMERA_LIST_SIZE		20
#define	MAX_AIM_MAGNETS			20
#define	MAX_NAV_MARKERS			32

#ifndef _FINAL
	#define	MAX_DEBUG_LINE_SYSTEMS	2000
#else
	#define	MAX_DEBUG_LINE_SYSTEMS	0
#endif

class CCharacterFX;
class CLadderFX;
class CTurretFX;
class CSpecialMoveFX;

class CSFXMgr
{
	public :

        CSFXMgr()  {}
		~CSFXMgr() {}

        bool   Init(ILTClient* pClientDE);

        CSpecialFX* FindSpecialFX(uint8 nType, HLOCALOBJ hObj);

		CSpecialFXList* GetCameraList() { return &m_cameraSFXList; }

		void	RemoveSpecialFX(HLOCALOBJ hObj);
		void	UpdateSpecialFX();
		void	RenderFX(HOBJECT hCamera);
		CSpecialFX* HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read *pMsg);

		//this should be called within a begin3d/end3d block to run through and update all render targets
		//that need to be updated
		void	UpdateRenderTargets(const LTRigidTransform& tCamera, const LTVector2& vCameraFOV);

		//calling this will dirty all render targets that were visible. This is primarily needed
		//for rendering multiple frames within a single update
		void	DirtyVisibleRenderTargets();

		void	RemoveAll();

        CSpecialFX* CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct,
            ILTMessage_Read *pMsg=NULL, HOBJECT hServerObj=NULL);

		static void	DeleteSFX(CSpecialFX *pSFX);

		inline CSpecialFXList* GetFXList(uint8 nType) 
		{ 
			if (nType < 0 || nType >= DYN_ARRAY_SIZE) return NULL;

			return &m_dynSFXLists[nType]; 
		}

		void OnObjectRotate( HOBJECT hObj, bool bTeleport, LTRotation *pNewRot );
		void OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID hTrackerID);
		void OnSFXMessage(ILTMessage_Read *pMsg);
		void OnSFXMessageOverride(ILTMessage_Read *pMsg);

		CCharacterFX* GetCharacterFX(HOBJECT hObject);
		CCharacterFX* GetCharacterFromHitBox(HOBJECT hHitBox);
		CCharacterFX* GetCharacterFromClientID(uint32 nClientId);

		CLadderFX* GetLadderFX(HOBJECT hObject);
		CTurretFX* GetTurretFX( HOBJECT hObject );
		CSpecialMoveFX* GetSpecialMoveFX(HOBJECT hObject);

	private :

        bool    AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

        int             GetDynArrayIndex(uint8 nFXId);
        unsigned int    GetDynArrayMaxNum(uint8 nArrayIndex);

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE]; // Lists of dynamic special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx
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
inline bool BoxesIntersect( const LTVector &vBox1Min, const LTVector &vBox1Max, const LTVector &vBox2Min, const LTVector &vBox2Max )
{
	if(	vBox1Min.x - vBox2Max.x >= 0.0f || vBox1Max.x - vBox2Min.x <= 0.0f ||
		vBox1Min.y - vBox2Max.y >= 0.0f || vBox1Max.y - vBox2Min.y <= 0.0f ||
		vBox1Min.z - vBox2Max.z >= 0.0f || vBox1Max.z - vBox2Min.z <= 0.0f )
		return false;

	return true;
}

#endif // __SFX_MGR_H__