//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : UsefulDC.h
//
//	PURPOSE	  : Defines the CUsefulDib class.
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

#ifndef __USEFUL_DIB_H__
	#define __USEFUL_DIB_H__


	// Includes....


	// Defines....
	#define MAX_DIB_SELECTORS			100
	class CDib;

	 #define COLOR24_TO_16(x)	(WORD)( (((x>>3)&31)<<10) | (((x>>11)&31)<<5) | ((x>>19)&31) )


	class CDibSelector
	{
		public:
			
						CDibSelector()
						{
							m_PenColor = 0;
							m_BrushColor = 0;
						}
			
			
			WORD		m_PenColor;
			WORD		m_BrushColor;
			int			m_RopMode;

	};



	class CUsefulDib
	{
		public:

							CUsefulDib();


		// Functionality.
		public:

			// You MUST call End() when you're done with what you've setup Start() for.
			// This stuff goes on a stack, so you can call Start() many times before calling End().
			void			StartPen( int penStyle, int penWidth, COLORREF penColor );
			void			StartBrush( COLORREF brushColor );
			void			StartPenBrush( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor );
			void			StartPenBrushROP( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor, int ropMode );
			void			End( int count=1 );
			
			void			DrawLine( CPoint &pt1, CPoint &pt2 );
			void			DrawLine( CVector &pt1, CVector &pt2 );
			
			void			DrawEllipse( int x1, int y1, int x2, int y2 );
			
			void			DrawRectangle( int x1, int y1, int x2, int y2 );
			void			DrawRectangle( LPRECT r )	{ DrawRectangle(r->left, r->top, r->right, r->bottom); }
		
		
		public:
		
			CDib			*m_pDib;
				
		
		protected:


			CDibSelector	m_Selectors[MAX_DIB_SELECTORS];
			DWORD			m_nSelectors;

			WORD			m_CurPenColor;
			WORD			m_CurBrushColor;
			int				m_CurRopMode;

	};



#endif  // __USEFUL_DC_H__


