#ifndef _LTMULTIWND_H_
#define _LTMULTIWND_H_

// Includes
#include "LTMaskedWnd.h"

class CLTMultiWnd : public CLTMaskedWnd
{
	public:
		// Constructors, destructors, and intialization functions
		CLTMultiWnd();
		virtual ~CLTMultiWnd() { Term(); }

		virtual BOOL	Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTSurfaceArray *pcollSurfs,
						int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);

		virtual BOOL	InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd,
						CStringArray *pcollBitmaps, int xPos = 0, int yPos = 0,
						DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);

		virtual void	Term();
		virtual void	FreeAllSurfaces(CLTSurfaceArray* pcollSurfs = NULL);
		virtual BOOL	CreateSurfaces(CStringArray *pcollBitmaps, CLTSurfaceArray* pcollSurfs);
		virtual void	SetImage(int nImage) { if(nImage >= m_collSurfaces.GetSize()) return; SetSurface(m_collSurfaces[nImage]); }

	protected:

		CLTSurfaceArray	m_collSurfaces;
};

// Inlines
inline CLTMultiWnd::CLTMultiWnd()
{
}

inline void CLTMultiWnd::Term()
{
	CLTMaskedWnd::Term();
}

#endif