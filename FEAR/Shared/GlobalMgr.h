// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.h
//
// PURPOSE : Definition of global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_MGR_H__
#define __GLOBAL_MGR_H__

#include "SoundMgr.h"
#include "SurfaceDefs.h"


class CGlobalMgr
{
	public :

		class CErrorHandler
		{
		public:
			virtual void ShutdownWithError(char* pMgrName, const char* pButeFilePath) const = 0;
		};

	public :

		CGlobalMgr();
		virtual ~CGlobalMgr();

        static bool Init(const CErrorHandler &cError);
		static void Term();

	private :

		bool Internal_Init(const CErrorHandler &cError);
		void Internal_Term();

		static CGlobalMgr *GetSingleton() { return s_pSingleton; }
		static void SetSingleton(CGlobalMgr *pGlobalMgr) { s_pSingleton = pGlobalMgr; }

		static CGlobalMgr *s_pSingleton;

		uint32			m_nRefCount;

	private:

		PREVENT_OBJECT_COPYING( CGlobalMgr );
};

#endif // __GLOBAL_MGR_H__
