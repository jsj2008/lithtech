 // ----------------------------------------------------------------------- //
//
// MODULE  : ScatterFX.h
//
// PURPOSE : Scatter special fx class - Definition
//
// CREATED : 4/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCATTER_FX_H__
#define __SCATTER_FX_H__

#include "SpecialFX.h"
#include "iltcustomrender.h"


#define SCATTER_BLINDOBJECTID		0x73f53a84	// ID for scatter blind object data
#define SCATTERFX_LODPERCENT		0.8f		// percentage of max draw dist to begin to fade a subvolume at (range is 0 to 1)
#define SCATTERFX_TABLESIZE			256			// size of tables used for particle generation
#define SCATTERFX_COSOFFSET			64			// offset into sin table to get cos (sin table is actually table size + this offset)


class CScatterFX;
class CBlindObjectScatterParticle;

class CScatterFXSubVolume
{
public:
	CScatterFXSubVolume();
	~CScatterFXSubVolume();

	// initialize the subvolume
	void Init(	CScatterFX* parent, const LTVector& vMin, const LTVector& vMax, uint32 numParticles, 
				const CBlindObjectScatterParticle* pBlindObject );

	// returns the LOD this subvolume should be at
	float CalculateLOD( const LTVector& camPos ) const;

	// returns the current state of this subvolume
	bool IsActive() const { return m_Active; }

	// this subvolume should start rendering particles (just updates it's LOD if already active)
	bool Activate( float detail );

	// this subvolume should stop rendering particles
	bool Deactivate();

	// fill in a vertex buffer with procedurally generated particles
	void CustomRenderCallback( ILTCustomRenderCallback* pCustomRender );

private:
	bool m_Active;									// true if this subvolume is currently active (visible and containing particles)
	uint32 m_NumParticles;							// number of full LOD particles in this subvolume
	float m_LOD;									// level of detail for this subvolume (0 is invisible, 1 is fully visible)
	CScatterFX* m_Parent;							// the ScatterFX object containing this subvolume
	LTVector m_MinBounds;							// min bounds for this subvolume
	LTVector m_MaxBounds;							// max bounds for this subvolume
	const CBlindObjectScatterParticle* m_pBlindObject;	// unsorted array of particles
	
	// the following members are only valid for active subvolumes
	HOBJECT m_Effect;							// volume effect object for this subvolume

	static bool tableInit;
	static float sinTable[SCATTERFX_TABLESIZE+SCATTERFX_COSOFFSET];
	static float* cosTable;
	static uint32 rndTable[SCATTERFX_TABLESIZE];
};


struct SCATTERCREATESTRUCT : public SFXCREATESTRUCT
{
	SCATTERCREATESTRUCT();

	LTVector		vDims;
	uint32			nBlindDataIndex;
	float			fHeight;
	float			fWidth;
	float			fMaxScale;
	float			fWaveRate;
	float			fWaveDist;
	float			fMaxDrawDist;
	std::string		sMaterialName;
	bool			bTranslucent;
	bool			bTranslucentLight;
	bool			bBackFaces;
	uint32			nBaseColor;
	uint32			nNumImages;
};


inline SCATTERCREATESTRUCT::SCATTERCREATESTRUCT()
:	SFXCREATESTRUCT		( ),
	vDims				( 0.0f, 0.0f, 0.0f ),
	nBlindDataIndex		( 0xffffffff ),
	fHeight				( 0.0f ),
	fWidth				( 0.0f ),
	fMaxScale			( 0.0f ),
	fWaveRate			( 0.0f ),
	fWaveDist			( 0.0f ),
	fMaxDrawDist		( 0.0f ),
	sMaterialName		( ),
	bTranslucent		( true ),
	bTranslucentLight	( false ),
	bBackFaces			( false ),
	nBaseColor			( SETRGBA(0xFF, 0xFF, 0xFF, 0xFF) ),
	nNumImages			( 1 )
{
}


class CScatterFX : public CSpecialFX
{
public:
	CScatterFX();
	~CScatterFX();

	virtual bool Init( SFXCREATESTRUCT* psfxCreateStruct );
	virtual bool Update();
	virtual bool CreateObject( ILTClient* pClientDE );
	virtual uint32 GetSFXID() { return SFX_SCATTER_ID; }

protected:
	bool OnServerMessage( ILTMessage_Read* pMsg );

private:
	LTVector		m_vDims;
	uint32			m_nBlindDataIndex;
	float			m_fHeight;
	float			m_fWidth;
	float			m_fMaxScale;
	float			m_fWaveRate;
	float			m_fWaveDist;
	float			m_fMaxDrawDist;
	float			m_fMaxDrawDistSq;
	std::string		m_sMaterialName;
	bool			m_bTranslucent;
	bool			m_bTranslucentLight;
	bool			m_bBackFaces;
	uint32			m_nBaseColor;
	uint32			m_nNumImages;
	HVERTEXDECL		m_hVertexDecl;

	LTVector m_vMinBounds;								// bounding box min value
	LTVector m_vMaxBounds;								// bounding box max value

	bool m_bEnabled;									// if false, don't update (all subvolumes will be inactive)

	uint32 m_nInternalTime;								// current time value in internal units (10000s of a second)

	uint32 m_nNumSubVolumes;							// number of subvolumes in this volume
	CScatterFXSubVolume* m_pSubVolumes;					// array of subvolumes within this scatter volume

	//our transform for this object
	LTRigidTransform	m_tTransform;

	//the inverse transform in matrix form to allow for fast transform of the camera
	LTMatrix3x4			m_mInvTransform;

	typedef std::list<HOBJECT, LTAllocator<HOBJECT, LT_MEM_TYPE_GAMECODE> > TFreeEffectList;
	typedef std::set<CScatterFXSubVolume*, std::less<CScatterFXSubVolume*>, LTAllocator<CScatterFXSubVolume*, LT_MEM_TYPE_GAMECODE> > TSubVolumeSet;
	static TFreeEffectList freeEffects;				// shared list of volumeeffect objects that have been allocated, but aren't currently used
	TSubVolumeSet activeSubVolumes;	// collection of subvolumes that are currently active

	bool UpdateSubVolumes(const LTVector& vCamPos);

	// take bounds that assume point sized particles, and adjust for particle size, wave, etc.
	void AdjustBounds( LTVector& vMin, LTVector& vMax );

	friend CScatterFXSubVolume;
};


#endif // __SCATTER_FX_H__
