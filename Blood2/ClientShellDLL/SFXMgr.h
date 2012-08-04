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

#include "cpp_client_de.h"
#include "SpecialFXList.h"
#include "SFXMsgIds.h"


#define DYN_ARRAY_SIZE		9

#define STATIC_LIST_SIZE	50
#define	CAMERA_LIST_SIZE	20

#define BULLET_LIST_INDEX	1


class CSFXMgr
{
	public :

		CSFXMgr()  { m_pClientDE = DNULL; }
		~CSFXMgr() {}

		DBOOL	Init(CClientDE* pClientDE);

		void	RemoveSpecialFX(HLOCALOBJ hObj);
		void	UpdateSpecialFX();
		void	HandleSFXMsg(HLOCALOBJ hObj, HMESSAGEREAD hMessage, DBYTE nId);
		CSpecialFX* FindStaticSpecialFX(HLOCALOBJ hObj);
		CSpecialFX* FindDynamicSpecialFX(HLOCALOBJ hObj);
		CSpecialFXList* GetCameraFXList() { return &m_cameraSFXList; }
		CSpecialFXList* GetBulletHoleFXList() { return &m_dynSFXLists[BULLET_LIST_INDEX]; }

		void	RemoveAll();

		CSpecialFX*	CreateSFX(DBYTE nId, SFXCREATESTRUCT *psfxCreateStruct, DBOOL bStatic=DFALSE, CSpecialFX* pParentFX=DNULL);

	private :

		void	AddDynamicSpecialFX(CSpecialFX* pSFX, DBYTE nId, CSpecialFX* pParent = DNULL);
		void 	UpdateDynamicSpecialFX();
		void	RemoveDynamicSpecialFX(HOBJECT hObj);
		void	RemoveAllDynamicSpecialFX();

		int				GetDynArrayIndex(DBYTE nFXId);
		unsigned int	GetDynArrayMaxNum(DBYTE nArrayIndex);

		CClientDE*		m_pClientDE;

		CSpecialFXList  m_dynSFXLists[DYN_ARRAY_SIZE]; // Lists of dynamic special fx
		CSpecialFXList	m_staticSFXList;				// List of static special fx
		CSpecialFXList	m_cameraSFXList;				// List of camera special fx
		CSpecialFXList	m_termSFXList;					// slow terminating FX
};

#endif // __SFX_MGR_H__