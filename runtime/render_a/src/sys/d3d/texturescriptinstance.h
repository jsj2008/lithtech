#ifndef __TEXTURESCRIPTINSTANCE_H__
#define __TEXTURESCRIPTINSTANCE_H__

#ifndef __TEXTURESCRIPTVARMGR_H__
#	include "texturescriptvarmgr.h"
#endif

//forward declarations
class ITextureScriptEvaluator;

//the different channels that the transforms can be installed on
enum ETextureScriptChannel {	TSChannel_Null,
								TSChannel_Base,
								TSChannel_Detail,
								TSChannel_EnvMap,
								TSChannel_LightMap,
								TSChannel_DualTexture,

								//must come last
								TSChannel_Count
							};

//an object that represents a single stage of a texture transform
class CTextureScriptInstanceStage
{
public:
	
	CTextureScriptInstanceStage() :
		m_bValid(false),
		m_pOverride(NULL),
		m_pEvaluator(NULL)
	{
	}

	//the ID of this group for the variables
	uint32						m_nID;

	//the texture channel that this matrix is intended to go into
	ETextureScriptChannel		m_eChannel;

	//the actual matrix that was calculated
	LTMatrix					m_mTransform;

	//the render variables that were used when it was last calculated
	float						m_fOldVars[CTextureScriptVarMgr::NUM_VARS];

	//the actual evaluator object
	ITextureScriptEvaluator*	m_pEvaluator;

	//determines if this stage is valid
	bool						m_bValid;

	//determines if this stage should simply be overridden by another stage
	CTextureScriptInstanceStage* m_pOverride;
};


class CTextureScriptInstance
{
public:

	enum {	NUM_STAGES = 2 };

	//called to update the matrix transforms. If force is not set, it will
	//attempt to determine if the matrices are dirty, and not set it if it isn't
	bool	Evaluate(bool bForce = false);

	//Installs the matrices into the specified channels with the appropriate flags.
	//If evaluate is true it will (unforcefully) evaluate the matrices. The other
	//parameters indicate how many channels are being used, and the variable list
	//is N elements, each describing the type of texture that the channel represents
	// (the ETextureScriptChannel enum)
	bool	Install(uint32 nNumChannels, ...);

	//disables all transforms
	bool	Uninstall(uint32 nNumChannels, ...);

	//reference counting functionality
	void	AddRef()			{ m_nRefCount++; }
	uint32	Release()			{ ASSERT(m_nRefCount > 0); return --m_nRefCount; }

private:

	//only allow the script manager to create instances and manipulate it. This avoids
	//exposing too much to outside callees
	friend class CTextureScriptMgr;

	CTextureScriptInstance();
	~CTextureScriptInstance();

	//sets up a texture transform as a unique evaluator
	bool	SetupStage(uint32 nStage, uint32 nVarID, ETextureScriptChannel eChannel, ITextureScriptEvaluator* pEvaluator);

	//sets up a texture transform as a reference to another transform (it will piggy back the
	//transform from there)
	bool	SetupStageAsReference(uint32 nStage, ETextureScriptChannel eChannel, uint32 nReferTo);

	//the various stages
	CTextureScriptInstanceStage	m_Stages[NUM_STAGES];

	//the time that this was last evaluated
	float						m_fOldTime;

	//the frame code of when the matrix was calculated
	uint32						m_nOldFrameCode;

	//flag indicating whether or not this is the first update
	bool						m_bFirstUpdate;

	//the reference count
	uint32						m_nRefCount;
};


#endif
