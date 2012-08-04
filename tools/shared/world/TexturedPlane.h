#ifndef __TEXTUREDPLANE_H__
#define __TEXTUREDPLANE_H__

#include "editregion.h"

class CLTANode;
class CLTAFile;


class CTexturedPlane
{
	public:

		CTexturedPlane();
		CTexturedPlane(const CTexturedPlane& rhs);
		virtual ~CTexturedPlane();

		void				CopyTextureAttributes(const CTexturedPlane *pOther, CStringHolder *pStringHolder );
					
		void				GetTextureSpace(CVector &outO, CVector &outP, CVector &outQ)
		{
			outO = O;
			outP = P;
			outQ = Q;
		}

		void				SetTextureSpace(const LTVector& vNormal, const LTVector& newO, const LTVector& newP, const LTVector& newQ);

	// Accessors.
	public:

		const LTVector&		GetO() const				{ return O; }
		const LTVector&		GetP() const 				{ return P; }
		const LTVector&		GetQ() const				{ return Q; }

		void				SetO(const LTVector& tempO)	{ O = tempO; }
		void				SetP(const LTVector& tempP)	{ P = tempP; }
		void				SetQ(const LTVector& tempQ)	{ Q = tempQ; }

	//for streaming too and from an LTA
	public:

		void				SaveTextureInfoLTA( CLTAFile* pFile, uint32 level );
		void				SavePhysicsMaterialLTA( CLTAFile* pFile, uint32 level );
		void				SaveFlagsLTA(CLTAFile* pFile, uint32 level);
		void				SaveShadeLTA(CLTAFile* pFile, uint32 level);

		void				LoadTextureInfoLTA( CLTANode* pNode, CStringHolder* pStringHolder);

		void				LoadTextureInfoTBW( CAbstractIO& InFile, CStringHolder* pStringHolder);
		void				SaveTextureInfoTBW( CAbstractIO& OutFile );

#ifdef DIRECTEDITOR_BUILD
			
	// DEdit specific data.
	public:

		//used to get the dimensions of the texture
		uint32					GetBaseTextureWidth();
		uint32					GetBaseTextureHeight();

		// NOTE: This ALWAYS points at a texture.  If m_pTextureName points at a sprite,
		// then this points at the first frame of the sprite's animation.
		struct DFileIdent_t		*m_pTextureFile;
		struct DFileIdent_t		*m_pDetailTextureFile;
		void					UpdateTextureID();

#endif

		char					*m_pTextureName;

	protected:

		LTVector				O, P, Q;

};

#endif
