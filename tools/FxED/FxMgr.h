//------------------------------------------------------------------
//
//   MODULE  : FXMGR.H
//
//   PURPOSE : Defines class CFxMgr
//
//   CREATED : On 10/5/98 At 6:58:51 PM
//
//------------------------------------------------------------------

#ifndef __FXMGR__H_
	#define __FXMGR__H_

	// Includes....

	#include "basefx.h"
	#include "LinkList.h"

	// Typedefs....

	typedef int (*FX_GETNUM)();
	typedef FX_REF (*FX_GETREF)(int);

	typedef int (*FXD_GETNUMANIMS)();
	typedef char* (*FXD_GETANIMNAME)(int);

	// Structures

	struct FXD_REF
	{
		HINSTANCE							m_hInst;
		FXD_GETNUMANIMS						m_fxNumAnims;
		FXD_GETANIMNAME						m_fxGetAnimName;
	};

	// Classes....

	class CFxMgr
	{
		public :

			// Constuctor

											CFxMgr();

			// Destructor
							
											~CFxMgr();

			// Member Functions

			BOOL							Init(const char *sDll);
			void							Term();

			BOOL							LoadFxDll(const char *sName);			
			FX_REF*							FindFX(const char *sName);

			// Accessors

			CLinkList<FX_REF>*				GetFx() { return &m_collRefs; }

		protected :

			// Member Variables

			CLinkList<FX_REF>				m_collRefs;
	};

#endif