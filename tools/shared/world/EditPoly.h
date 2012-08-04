//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditPoly.h
//
//	PURPOSE	  : Defines the CEditPoly class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITPOLY_H__
	#define __EDITPOLY_H__


	// Includes....
	#include "basepoly.h"
	#include "editregion.h"
	#include "texturedplane.h"


	// Defines....
	class CRegionView;
	class CPrePoly;
	class CLTANode;
	class CLTAFile;


	class CEditPoly : public CBasePoly
	{
		public:

			enum	{ NUM_TEXTURES	= 2 };

								CEditPoly();
								CEditPoly( CEditPoly *pCopyFrom );
								CEditPoly( CEditBrush *pBrush );
								~CEditPoly();
		
			void				Term();

			CTexturedPlane&			GetTexture(uint32 nTexture)			{ assert(nTexture < NUM_TEXTURES); return m_Textures[nTexture]; }
			const CTexturedPlane&	GetTexture(uint32 nTexture) const	{ assert(nTexture < NUM_TEXTURES); return m_Textures[nTexture]; }

		private:

			void				CommonConstructor( );

		// Loading/saving.
		public:

			BOOL				LoadEditPolyLTA( CLTANode* pNode, CStringHolder *pStringHolder );
			void				SaveEditPolyLTA( CLTAFile* pFile, uint32 level );

			bool				LoadEditPolyTBW( CAbstractIO& InFile, CStringHolder *pStringHolder );
			void				SaveEditPolyTBW( CAbstractIO& OutFile );

		// Functionality.
		public:

			void				CopyAttributes( CBasePoly *pOther, CStringHolder *pStringHolder=NULL );
			
			// Decrements all indices >= index.
			void				DecrementPoints( uint32 index );

			// Tells if and where the ray intersects the poly.
			BOOL				IntersectRay( CEditRay &ray, CReal &t, BOOL bBackface );

			BOOL				CopyEditPoly( CEditPoly *pPoly, BOOL bCopyIndices=TRUE, CStringHolder *pStringHolder=NULL );
			
			void				Flip();
			
			void				SetupBaseTextureSpace(uint32 nTex);

			void				SetTextureSpace(uint32 nTex, const LTVector& newO, const LTVector& newP, const LTVector& newQ);

			// sets up OPQs based on uv coordinates of the first 3 verts
			bool				SetUVTextureSpace(uint32 nTex, const float* coords, const int texWidth, const int texHeight );

			//determines if this polygon is concave or not
			bool				IsConcave();

			//determines if this polygon lies entirely within a single plane
			bool				IsCoplanar();

			//determines the surface area of the polygon
			CReal				GetSurfaceArea();

			//retrieves the normal that should be used for various texture operations
			LTVector			GetTextureNormal()			{return Normal();}

		private:

			//the textures
			CTexturedPlane			m_Textures[NUM_TEXTURES];
			

#ifndef DIRECTEDITOR_BUILD
		//Preprocessor specific data
		public:

			// User data pointer .. used by the preprocessor to point to its surface.
			void					*m_pUser1;
#endif


	};


	typedef CMoArray<CEditPoly*> CEditPolyArray;


#endif  // __EDITPOLY_H__



