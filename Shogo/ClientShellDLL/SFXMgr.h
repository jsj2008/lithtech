// ----------------------------------------------------------------------- //
//
// MODULE  : SFXMgr.h
//
// PURPOSE : Special FX Mgr	- Definition
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#ifndef __SFX_MGR_H__
#define __SFX_MGR_H__

#include "clientheaders.h"
#include "SpecialFXList.h"
#include "SFXMsgIds.h"


#define DYN_ARRAY_SIZE		(SFX_TOTAL_NUMBER + 1)
#define	CAMERA_LIST_SIZE	20

class CSFXMgr
{
	public :

		CSFXMgr()  { m_pClientDE = LTNULL; }
		~CSFXMgr() {}

		LTBOOL	Init(ILTClient* pClientDE);

		CSpecialFX* FindSpecialFX(uint8 nType, HLOCALOBJ hObj);
		CSpecialFX* FindProjectileFX(HLOCALOBJ hObj);

		CSpecialFXList* GetCameraList() { return &m_cameraSFXList; }

		void	RemoveSpecialFX(HLOCALOBJ hObj);
		void	UpdateSpecialFX();
		void	HandleSFXMsg(HLOCALOBJ hObj, ILTMessage_Read* hMessage);

		void	RemoveAll();

		CSpecialFX*	CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct);
		CSpecialFX* CreateAutoSFX(HOBJECT hServerObj, ILTMessage_Read* hMessage);

		CSpecialFXList* GetBulletHoleList() { return &m_dynSFXLists[1]; }

		void OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, LTFLOAT forceMag);

	private :

		LTBOOL	AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

		int				GetDynArrayIndex(uint8 nFXId);
		unsigned int	GetDynArrayMaxNum(uint8 nArrayIndex);

		ILTClient*		m_pClientDE;

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE];  // Lists of dynamic special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx
};

#endif // __SFX_MGR_H__