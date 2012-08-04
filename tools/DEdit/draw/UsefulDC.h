//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : UsefulDC.h
//
//	PURPOSE	  : Defines the CUsefulDC class.
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

#ifndef __USEFUL_DC_H__
	#define __USEFUL_DC_H__


	// Includes....


	// Defines....
	#define MAX_SELECTORS			100
	class CSelector
	{
		public:
			
						CSelector()
						{
							m_pPen = m_pOldPen = NULL;
							m_pBrush = m_pOldBrush = NULL;
						}
			
			CPen		*m_pPen, *m_pOldPen;
			CBrush		*m_pBrush, *m_pOldBrush;
			int			m_OldRopMode;

	};



	class CUsefulDC
	{
		public:

							CUsefulDC()
							{
								m_pDC = NULL;
								m_nSelectors = 0;
							}

							CUsefulDC( CDC *pDC )
							{
								m_pDC = pDC;
								m_nSelectors = 0;
							}


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
		
		
		// Accessors.
		public:
		
			CDC				*m_pDC;
				
		
		protected:


			CSelector		m_Selectors[MAX_SELECTORS];
			DWORD			m_nSelectors;

			CPen			m_SelectorPens[MAX_SELECTORS];
			CBrush			m_SelectorBrushes[MAX_SELECTORS];

	};



#endif  // __USEFUL_DC_H__


