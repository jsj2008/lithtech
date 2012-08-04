//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __COLORSELECTDLG_H__
#define __COLORSELECTDLG_H__


	#include "resource.h"
	#include "dibmgr.h"


	// Callback for when the color changes..
	typedef void (*ColorChangeCB)(COLORREF newColor, void *pUser);


	class ColorChangeNotify
	{
	public:
		BOOL			operator==(ColorChangeNotify other)
		{
			return m_CB == other.m_CB && m_pUser == other.m_pUser;
		}

		ColorChangeCB	m_CB;
		void			*m_pUser;
	};
	

	class CColorSelectDlg : public CDialog
	{
	// Construction
	public:
		CColorSelectDlg(CWnd* pParent = NULL);   // standard constructor
		~CColorSelectDlg();

	// Dialog Data
		//{{AFX_DATA(CColorSelectDlg)
		enum { IDD = IDD_COLORSELECT_DLG };
			// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

		
		COLORREF			GetCurColor()		{ return m_CurColor; }
		void				SetCurColor(COLORREF color, BOOL bNotify=FALSE);

		// Register for callbacks...
		BOOL				AddCallback(ColorChangeCB cb, void *pUser);
		void				RemoveCallback(ColorChangeCB cb, void *pUser);


	private:

		CMoArray<ColorChangeNotify>	m_Callbacks;

		CDibMgr				m_DibMgr;
		CDib				*m_pDib;
		
		CRect				m_DrawRect;
		BOOL				m_bLButtonDown;
		CPoint				m_LastPt;
		CVector				m_CurSelPt;
		DWORD				m_SelBoxRadius;

		//determines if this dialog has been initialized yet
		BOOL				m_bInitialized;


		COLORREF			m_CurColor;


		void				SetColorSel();
		COLORREF			GetColorFromPoint( CVector point );
		
		void				DrawColors();
		void				DrawColorStrip( CReal inR, CReal inG, CReal inB, WORD *pBuf, DWORD halfWidth );

		void				HSV_To_RGB( CReal h, CReal l, CReal s, CVector &color );
		void				RGB_To_HSV(DVector color, float &h, float &s, float &v);

		void				NotifyCallbacks();



	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CColorSelectDlg)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:

		// Generated message map functions
		//{{AFX_MSG(CColorSelectDlg)
		virtual BOOL OnInitDialog();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnPaint();
		afx_msg void OnSize(UINT nType, int cx, int cy);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};


#endif  // __COLORSELECTDLG_H__

