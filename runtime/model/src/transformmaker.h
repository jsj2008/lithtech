#ifndef __TRANSFORMMAKER_H__
#define __TRANSFORMMAKER_H__

struct LTRotation;

#ifndef __LTANIMTRACKER_H__
#include "ltanimtracker.h"
#endif

#ifndef __MODEL_H__
#include "model.h"
#endif

class Model;
class ModelAnim;

// ----------------------------------------------------------------
//  Calculate all transforms for current animation(s)
// ----------------------------------------------------------------
class TransformMaker
{
	friend class ModelInstance ;
private:
//public:
					TransformMaker()
					{
						m_pStartMat = LTNULL;
						m_pOutput	= LTNULL;
						m_pInstance = LTNULL;
						m_nAnims	= 0;
						m_iMoveHintNode = 0xFFFFFFFF;
					}

	// Copies m_Anims, m_nAnims, and sets m_pStarbtMat to GVPStruct::m_BaseTransform.

	bool			IsValid();

	bool			SetupTransforms();

	bool			SetupTransformsWithPath();

	bool			GetNodeTransform(uint32 iNode);


// Set all these up before calling SetupTransforms or GetNodeTransform.

private:

// Used internally.


	bool			SetupCall();

	void			InitTransform(uint32 iAnim, uint32 iNode, LTRotation &outQuat, LTVector &outVec);
	void			InitTransformAdditive(uint32 iAnim, uint32 iNode, LTRotation &outQuat, LTVector &outVec);

	void			BlendTransform(uint32 iAnim, uint32 iNode);

	void			Recurse(uint32 iNode, LTMatrix *pParentT);

	void			RecurseWithPath(uint32 iNode, LTMatrix *pParentT);
	

	// All the animations.
	AnimTimeRef		m_Anims[MAX_GVP_ANIMS];
	uint32			m_nAnims;

	LTMatrix		*m_pStartMat;	// If null, then identity is used.
	LTMatrix		*m_pOutput;		// Output list.. if null, then Model::m_Transforms is used.
	
	ModelInstance	*m_pInstance;

	// If this is set, Recurse() uses this to determine which nodes it recurses into.
	// It starts at m_iCurPath and goes backwards until it reaches index 0.
	uint32			*m_pRecursePath;
	uint32			m_iCurPath;

	uint32			m_iMoveHintNode ; 

	LTMatrix		m_mTemp;

	LTRotation		m_Quat;
	LTVector		m_vTrans;

	Model			*m_pModel;

	// Local cache for the weight sets/anim when blending
	WeightSet		*m_WeightSets[MAX_GVP_ANIMS];

	// Local cache for the timeref prev/cur anims
	ModelAnim		*m_pAnimPrev[MAX_GVP_ANIMS];
	ModelAnim		*m_pAnimCur[MAX_GVP_ANIMS];
};


#endif

