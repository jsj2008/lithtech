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


#define DYN_ARRAY_SIZE		(SFX_TOTAL_NUMBER + 1)
#define	CAMERA_LIST_SIZE	20

class CCharacterFX;

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
		void	HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage);

		void	RemoveAll();

        CSpecialFX* CreateSFX(uint8 nId, SFXCREATESTRUCT *psfxCreateStruct,
            HMESSAGEREAD hMessage=LTNULL, HOBJECT hServerObj=LTNULL);

		static void	DeleteSFX(CSpecialFX *pSFX);

		inline CSpecialFXList* GetFXList(uint8 nType) 
		{ 
			if (nType < 0 || nType >= DYN_ARRAY_SIZE) return LTNULL;

			return &m_dynSFXLists[nType]; 
		}

		void OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void OnModelKey(HLOCALOBJ hObj, ArgList *pArgs);
		void OnSFXMessage(HMESSAGEREAD hMessage);

		CCharacterFX* GetCharacterFX(HOBJECT hObject);

	private :

        void    AddDynamicSpecialFX(CSpecialFX* pSFX, uint8 nId);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

        int             GetDynArrayIndex(uint8 nFXId);
        unsigned int    GetDynArrayMaxNum(uint8 nArrayIndex);

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE]; // Lists of dynamic special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx
};

#endif // __SFX_MGR_H__