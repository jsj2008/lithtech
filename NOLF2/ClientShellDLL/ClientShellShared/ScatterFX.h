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


#define SCATTER_BLINDOBJECTID		0x73f53a84	// ID for scatter blind object data
#define SCATTERFX_SUBVOLUMEDIMS		512.0f		// maximum length of a side of a scatter subvolume
#define SCATTERFX_LODPERCENT		0.8f		// percentage of max draw dist to begin to fade a subvolume at (range is 0 to 1)
#define SCATTERFX_TABLESIZE			256			// size of tables used for particle generation
#define SCATTERFX_COSOFFSET			64			// offset into sin table to get cos (sin table is actually table size + this offset)


class CScatterFX;


struct CScatterFXParticle
{
	LTVector pos;
	uint32 color;
	float scale;
	float xoffset;
	float yoffset;
	uint8 waveRot;
	uint8 waveStart;
};


class CScatterFXSubVolume
{
public:
	CScatterFXSubVolume();
	~CScatterFXSubVolume();

	// initialize the subvolume
	void Init( CScatterFX* parent, const LTVector& pos, const LTVector& dims, uint32 numParticles, CScatterFXParticle* particles );

	// returns the LOD this subvolume should be at
	float CalculateLOD( const LTVector& camPos );

	// returns the current state of this subvolume
	bool IsActive() { return m_Active; }

	// this subvolume should start rendering particles (just updates it's LOD if already active)
	bool Activate( float detail );

	// this subvolume should stop rendering particles
	bool Deactivate();

	// fill in a vertex buffer with procedurally generated particles
	bool FillVertexBuffer( void* pVertexBufferData, void* pLightingData, uint32 nVBNumVerts, uint32 nPrevFilledVerts, uint32& nNumFilledVerts );

private:
	bool m_Active;								// true if this subvolume is currently active (visible and containing particles)
	uint32 m_NumParticles;						// number of full LOD particles in this subvolume
	float m_LOD;								// level of detail for this subvolume (0 is invisible, 1 is fully visible)
	CScatterFX* m_Parent;						// the ScatterFX object containing this subvolume
	LTVector m_Pos;								// center of this subvolume
	LTVector m_Dims;							// dimensions of this subvolume
	LTVector m_MinBounds;						// min bounds for this subvolume
	LTVector m_MaxBounds;						// max bounds for this subvolume
	CScatterFXParticle* m_Particles;			// unsorted array of particles

	// the following members are only valid for active subvolumes
	HOBJECT m_Effect;							// volume effect object for this subvolume

	static bool tableInit;
	static float sinTable[SCATTERFX_TABLESIZE+SCATTERFX_COSOFFSET];
	static uint32 rndTable[SCATTERFX_TABLESIZE];
};


struct SCATTERCREATESTRUCT : public SFXCREATESTRUCT
{
	SCATTERCREATESTRUCT();

	LTVector vDims;
	uint32 nBlindDataIndex;
	float fHeight;
	float fWidth;
	float fMaxScale;
	float fTilt;
	float fWaveRate;
	float fWaveDist;
	float fMaxDrawDist;
	HSTRING hstrTextureName;
	bool bUseSaturate;
};


inline SCATTERCREATESTRUCT::SCATTERCREATESTRUCT()
{
	vDims.Init();
	nBlindDataIndex = 0xffffffff;
	fHeight = 0.0f;
	fWidth = 0.0f;
	fMaxScale = 0.0f;
	fTilt = 0.0f;
	fWaveRate = 0.0f;
	fWaveDist = 0.0f;
	fMaxDrawDist = 0.0f;
	hstrTextureName = LTNULL;
	bUseSaturate = false;
}


class CScatterFX : public CSpecialFX
{
public:
	CScatterFX();
	~CScatterFX();

	virtual LTBOOL Init( SFXCREATESTRUCT* psfxCreateStruct );
	virtual LTBOOL Update();
	virtual LTBOOL CreateObject( ILTClient* pClientDE );
	virtual uint32 GetSFXID() { return SFX_SCATTER_ID; }

protected:
	LTBOOL OnServerMessage( ILTMessage_Read* pMsg );

private:
	LTVector m_vDims;
	uint32 m_nBlindDataIndex;
	float m_fHeight;
	float m_fWidth;
	float m_fMaxScale;
	float m_fTilt;
	float m_fWaveRate;
	float m_fWaveDist;
	float m_fMaxDrawDist;
	float m_fMaxDrawDistSq;
	HSTRING m_hstrTextureName;
	bool m_bUseSaturate;

	LTVector m_vMinBounds;								// bounding box min value
	LTVector m_vMaxBounds;								// bounding box max value

	bool m_bEnabled;									// if false, don't update (all subvolumes will be inactive)

	uint32 m_nInternalTime;								// current time value in internal units (10000s of a second)
	LTVector m_vCamPos;									// current camera position

	uint32 m_nNumSubVolumes;							// number of subvolumes in this volume
	CScatterFXSubVolume* m_pSubVolumes;					// array of subvolumes within this scatter volume

	static std::list<HOBJECT> freeEffects;				// shared list of volumeeffect objects that have been allocated, but aren't currently used
	std::set<CScatterFXSubVolume*> activeSubVolumes;	// collection of subvolumes that are currently active

	bool UpdateSubVolumes();

	// take bounds that assume point sized particles, and adjust for particle size, wave, etc.
	void AdjustBounds( LTVector& pos, LTVector& dims );

	friend CScatterFXSubVolume;
};


#endif // __SCATTER_FX_H__
