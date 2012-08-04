
#ifndef __TRANSFORMMAKER_H__
#define __TRANSFORMMAKER_H__


    #ifndef __LTBASEDEFS_H__
	#include "ltbasedefs.h"
    #endif

    #ifndef __MODEL_H__
	#include "model.h"
    #endif


	class TransformMaker
	{
	public:

						TransformMaker()
						{
							m_pStartMat = LTNULL;
							m_pOutput = LTNULL;
							m_NC = LTNULL;
							m_pNCUser = LTNULL;
							m_hNCObject = LTNULL;
							m_nAnims = 0;
						}

		// Copies m_Anims, m_nAnims, and sets m_pStartMat to GVPStruct::m_BaseTransform.
		void			SetupFromGVPStruct(GVPStruct *pStruct);

		bool			IsValid();
		void			CopyTimes(TransformMaker &other);

		bool			SetupTransforms();
		bool			GetNodeTransform(uint32 iNode);

		bool			SetupMovementEncoding();


	// Set all these up before calling SetupTransforms or GetNodeTransform.
	public:

		// All the animations.
		AnimTimeRef		m_Anims[MAX_GVP_ANIMS];
		uint32			m_nAnims;

		LTMatrix		*m_pStartMat;	// If null, then identity is used.
		LTMatrix		*m_pOutput;		// Output list.. if null, then Model::m_Transforms is used.
		
		NodeControlFn	m_NC;			// Can be null.
		void			*m_pNCUser;
		HOBJECT			m_hNCObject;

		// If this is set, Recurse() uses this to determine which nodes it recurses into.
		// It starts at m_iCurPath and goes backwards until it reaches index 0.
		uint32			*m_pRecursePath;
		uint32			m_iCurPath;


	// Used internally.
	protected:

		bool			SetupCall();

		void			InitTransform(uint32 iAnim, uint32 iNode, LTRotation &outQuat, LTVector &outVec);
		void			InitTransformAdditive(uint32 iAnim, uint32 iNode, LTRotation &outQuat, LTVector &outVec);

		void			BlendTransform(uint32 iAnim, uint32 iNode);

		void			Recurse(uint32 iNode, LTMatrix *pParentT);


	protected:

		// Used as local variables so they don't add to the stack.
		CIRelation		*m_pCurRelation;
		LTMatrix		m_mCurRelation;
		CMoArray<CIRelation>	*m_pNodeRelation;

		LTMatrix		m_mTemp;
		
		LTRotation		m_Quat;
		LTVector		m_vTrans;

		Model			*m_pModel;

		LTMatrix		m_mIdent;

	};


#endif

