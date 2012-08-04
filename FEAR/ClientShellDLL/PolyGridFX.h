 // ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.h
//
// PURPOSE : Polygrid special fx class - Definition
//
// CREATED : 10/13/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLYGRIDFX_H__
#define __POLYGRIDFX_H__

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

#include "SpecialFX.h"
#include "PolyGridBuffer.h"

#define PG_MAX_MODIFIERS			4

struct PGCREATESTRUCT : public SFXCREATESTRUCT
{
    PGCREATESTRUCT();

	virtual void Read(ILTMessage_Read *pMsg);

    LTVector vDims;
    LTVector vColor;
    float fXScaleMin;
    float fXScaleMax;
    float fYScaleMin;
    float fYScaleMax;
    float fXScaleDuration;
    float fYScaleDuration;
    float fXPan;
    float fYPan;
	std::string sMaterial;
    float fAlpha;
    uint32 dwNumPoliesX;
	uint32 dwNumPoliesY;
	uint32 nNumStartupFrames;
    bool bSolid;
    bool bEnableCollisions;
	float fDampenScale;
	float fTimeScale;
	float fSpringCoeff;
	float fModelDisplace;
	float fMinFrameRate;
	float fMinResolutionScale;
	uint8 nActiveModifiers;
	float fXMin[PG_MAX_MODIFIERS];
	float fYMin[PG_MAX_MODIFIERS];
	float fXMax[PG_MAX_MODIFIERS];
	float fYMax[PG_MAX_MODIFIERS];
	uint16 nNumAccelPoints[PG_MAX_MODIFIERS];
	float fAccelAmount[PG_MAX_MODIFIERS];
};

inline PGCREATESTRUCT::PGCREATESTRUCT()
{
	vDims.Init();
	vColor.Init();
	fXScaleMin			= 0.0f;
	fXScaleMax			= 0.0f;
	fYScaleMin			= 0.0f;
	fYScaleMax			= 0.0f;
	fXScaleDuration		= 0.0f;
	fYScaleDuration		= 0.0f;
	fXPan				= 0.0f;
	fYPan				= 0.0f;
	fAlpha				= 1.0f;
	dwNumPoliesX		= 0;
	dwNumPoliesY		= 0;
	nNumStartupFrames	= 0;
    bSolid		        = false;
    bEnableCollisions	= true;

	fDampenScale		= 0.8f;
	fTimeScale			= 1.0f;
	fSpringCoeff		= 80.0f;
	fModelDisplace		= 10.0f;
	fMinFrameRate		= 10.0f;
	nActiveModifiers	= 0;

	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		fXMin[nCurrMod]				= 0.0f;
		fYMin[nCurrMod]				= 0.0f;
		fXMax[nCurrMod]				= 0.0f;
		fYMax[nCurrMod]				= 0.0f;
		nNumAccelPoints[nCurrMod]	= 0;
		fAccelAmount[nCurrMod]		= 0.0f;
	}
}

inline void PGCREATESTRUCT::Read(ILTMessage_Read *pMsg)
{
	vDims = pMsg->ReadLTVector();
	uint16 wColor = pMsg->Readuint16();
	Color255WordToVector(wColor, &(vColor));
	fXScaleMin = pMsg->Readfloat();
	fXScaleMax = pMsg->Readfloat();
	fYScaleMin = pMsg->Readfloat();
	fYScaleMax = pMsg->Readfloat();
	fXScaleDuration = pMsg->Readfloat();
	fYScaleDuration = pMsg->Readfloat();
	fXPan = pMsg->Readfloat();
	fYPan = pMsg->Readfloat();
	fAlpha = pMsg->Readfloat();
	fTimeScale = pMsg->Readfloat();
	fDampenScale = pMsg->Readfloat();
	fSpringCoeff	= pMsg->Readfloat();
	fModelDisplace = pMsg->Readfloat();
	fMinFrameRate = pMsg->Readfloat();
	fMinResolutionScale = pMsg->Readfloat();

	char szString[256];
	pMsg->ReadString( szString, ARRAY_LEN( szString ));
	sMaterial = szString;

	dwNumPoliesX = (uint32)pMsg->Readuint16();
	dwNumPoliesY = (uint32)pMsg->Readuint16();
	nNumStartupFrames = (uint32)pMsg->Readuint16();
	bSolid = pMsg->Readbool();
	bEnableCollisions = pMsg->Readbool();

	//read in our modifier data
	nActiveModifiers = pMsg->Readuint8();
	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		fAccelAmount[nCurrMod] = pMsg->Readfloat();
		nNumAccelPoints[nCurrMod] = pMsg->Readuint16();				
		fXMin[nCurrMod] = pMsg->Readfloat();
		fYMin[nCurrMod] = pMsg->Readfloat();
		fXMax[nCurrMod] = pMsg->Readfloat();
		fYMax[nCurrMod] = pMsg->Readfloat();
	}
}

class CPolyGridFX : 
	public CSpecialFX
{
public :

	CPolyGridFX();
	~CPolyGridFX();

    virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
    virtual bool Update();
    virtual bool CreateObject(ILTClient* pClientDE);

	virtual uint32 GetSFXID() { return SFX_POLYGRID_ID; }

	//Given a point and a direction to look in, this function will find out where it intersects
	//the polygrid, and determine the displacement from the polygrid at that point. It will
	//return false if it doesn't intersect. This assumes an axis aligned polygrid.
	bool GetOrientedIntersectionHeight(const LTVector& vPos, const LTVector& vUnitDir, float& fDisplacement, LTVector& vIntersection);

	//given a point in world space, this will create a disturbance at the designated location
	//with the specified force
	void	CreateDisturbance(const LTVector& vPtWS, float fStrength);

	//given a point in world space, this will create a disturbance at the designated location
	//with the specified force. This version takes an additional radius and applies a downward cone
	//of force over the sphere on the polygrid centered around the specified point, with the center
	//having the specified strength and falling off towards zero at the edges
	void	CreateDisturbance(const LTVector& vPtWS, float fRadius, float fStrength);

	LTVector GetDims() const { return m_vDims; }

	HPHYSICSRIGIDBODY GetRigidBody( ) const { return m_hRigidBody; }

	// Finds a polygrid at a point for doing splashes.
	static CPolyGridFX* FindSplashInPolyGrid( HOBJECT hObject, LTVector const& vSplashPos );
	// Creates a splash in a polygrid.  The splash is in the xz of the vSplashPos, but at the top of the polygrid.
	void DoPolyGridSplash( HOBJECT hSplasher, LTVector const& vSplashPos, float fImpulse );

	typedef std::vector<CPolyGridFX*, LTAllocator<CPolyGridFX*, LT_MEM_TYPE_CLIENTSHELL> > InstanceList;
	static InstanceList& GetInstanceList() { return m_lstInstances; }

protected:

	void UpdateSurface();

	//different updaters for the different surface styles
	void PrecalculatePlasma();
	void UpdatePlasma();
	void UpdateFourRingPlasma();

	bool OnServerMessage(ILTMessage_Read *pMsg);

	//wave propagation methods
	void InitWaveProp();
	void UpdateWaveProp(float fFrameTime);
	void CreateModelWaves(uint32 nBuffer, float fFrameTime);

	//runs through several iterations of updating specified in NumStartupFrames so that
	//the water won't be completely calm when starting
	void HandleStartupFrames();

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the custom rendering
	void RenderPolyGrid(ILTCustomRenderCallback* pInterface);

	
private :

	//called to create the physics objects associated with this polygrid
	bool			CreatePhysicsObjects(const LTRigidTransform& tTransform, const LTVector& vHalfDims);

	//called to free any physics objcts that this polygrid is holding onto
	void			FreePhysicsObjects();

	//utility function for getting a buffer at the specified row and height in triangles, not
	//the actual buffer. This will automatically adjust for the top and left buffers. If this
	//is used to span rows, make sure to accomodate for the buffers on the right and left sides.
	float*			GetBufferAt(uint32 nBufferIndex, uint32 nQuadX, uint32 nQuadY);

	//static function that the collision notifier calls into. This will call into the polygrid's
	//collision handler
	static void		CollisionHandlerCB(	HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
										const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
										float fVelocity, bool& bIgnoreCollision, void* pUser);

	//called in response to a physical collision to generate a wave accordingly
	void			HandleCollision(const LTVector& vCollisionPt, float fVelocity);

	//for controlling the scaling of the surface (should be a sin wave eventually)
	bool			m_bScalingXUp;
	bool			m_bScalingYUp;
	HVERTEXDECL		m_hVertexDecl;

	//the current UV offset and scale
	float			m_fUOffset;
	float			m_fVOffset;
	float			m_fUScale;
	float			m_fVScale;

	//Wave propagation buffers. These will have padding on each side equal to the kernal size.
	//this padding will be filled with 0's and should never be modified.
	CPolyGridBuffer<float>	m_WaveBuffer[2];
	uint32					m_nCurrWaveBuffer;
	float					m_fPrevFrameTime;

	//maximum number of models in the water that we can keep track of
	enum { MAX_MODELS_TO_TRACK = 32	};

	//the handles to each tracked model
	HLOCALOBJ		m_hTrackedModels[MAX_MODELS_TO_TRACK];

	//the last position of the models we are tracking
	LTVector		m_vTrackedModelsPos[MAX_MODELS_TO_TRACK];

	//wave prop data
	float			m_fDampenScale;
	float			m_fTimeScale;
	float			m_fSpringCoeff;
	float			m_fModelDisplace;
	float			m_fMinFrameRate;
	float			m_fMinResolutionScale;


	//wave prop modifiers
	uint8			m_nActiveModifiers;
	uint16			m_nXMin[PG_MAX_MODIFIERS];
	uint16			m_nYMin[PG_MAX_MODIFIERS];
	uint16			m_nXMax[PG_MAX_MODIFIERS];
	uint16			m_nYMax[PG_MAX_MODIFIERS];
	uint16			m_nNumAccelPoints[PG_MAX_MODIFIERS];
	float			m_fAccelAmount[PG_MAX_MODIFIERS];

	//information about the previous unfinished impact
	float			m_fPrevImpactAmountLeft[PG_MAX_MODIFIERS];
	uint32			m_nPrevImpactX[PG_MAX_MODIFIERS];
	uint32			m_nPrevImpactY[PG_MAX_MODIFIERS];

    LTVector		m_vDims;
    LTVector		m_vColor;
    float			m_fXScaleMin;
    float			m_fXScaleMax;
    float			m_fYScaleMin;
    float			m_fYScaleMax;
    float			m_fXScaleDuration;
    float			m_fYScaleDuration;
    float			m_fXPan;
    float			m_fYPan;
	std::string		m_sMaterial;
    float			m_fAlpha;
    uint32			m_dwNumPoliesX;
	uint32			m_dwNumPoliesY;
	uint32			m_nNumStartupFrames;

    bool			m_bSolid;
    bool			m_bEnableCollisions;

	//this is a flag that is set each time a polygrid is rendered, and cleared each time it
	//is updated. This allows us to only update polygrids that are visible
	bool			m_bWasDrawn;

	//the physical objects that represent our polygrid in the physics simulation
	HPHYSICSRIGIDBODY			m_hRigidBody;
	HPHYSICSCOLLISIONNOTIFIER	m_hCollisionNotifier;

	static InstanceList m_lstInstances;
};

#endif // __POLYGRIDFX_H__
