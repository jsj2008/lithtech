 // ----------------------------------------------------------------------- //
//
// MODULE  : SnowFX.h
//
// PURPOSE : Snow special fx class - Definition
//
// CREATED : 1/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SNOW_FX_H__
#define __SNOW_FX_H__

#include "SpecialFX.h"
#include "iltcustomrender.h"


#define SNOWFX_AIRSPACEDIMS			512.0f		// maximum length of side of a snow airspace
#define SNOWFX_LODPERCENT			0.8f		// percentage of max draw dist to begin to fade an airspace at (range is 0 to 1)
#define SNOWFX_PARTICLEPOOLSIZE		16384		// size of pool of particles to allocate from (particles can be reused in other airspaces)
#define SNOWFX_TABLESIZE			256			// size of tables used for particle generation
#define SNOWFX_COSOFFSET			64			// offset into sin table to get cos (sin table is actually table size + this offset)


class CSnowFX;
class ILTCustomRenderCallback;


struct CSnowFXParticle
{
	LTVector pos;			// initial particle position relative to airspace origin
	uint32 id;				// random value used for indexing into various tables
	bool bCycled;			// has this particle been cycled, from bottom of air space to top
};


class CSnowFXParticleMgr
{
public:
	CSnowFXParticleMgr();
	~CSnowFXParticleMgr();

	// get a pointer to a block of particles
	bool AllocateParticles( uint32 numParticles, CSnowFXParticle*& particles );

	// release the allocated particles
	bool FreeParticles( CSnowFXParticle* particles );

private:
	uint32 m_NumParticles;				// number of particles in the array
	CSnowFXParticle* m_Particles;		// array containing all particles available to allocate from
};


class CSnowFXAirspace
{
public:
	CSnowFXAirspace();
	~CSnowFXAirspace();

	// initialize the airspace
	void Init( CSnowFX* parent, const LTVector& pos, const LTVector& dims );

	// returns the LOD this airspace should be at
	float CalculateLOD( const LTVector& camPos );

	// returns the current state of this airspace
	bool IsActive() { return m_Active; }

	// this airspace should start generating particles (just updates it's LOD if already active)
	bool Activate( float detail );

	// this airspace should stop generating particles
	bool Deactivate();

	// update the density of this airspace
	bool UpdateDensity();

	// fill in a vertex buffer with procedurally generated particles
	void CustomRenderCallback( ILTCustomRenderCallback* pCustomRender );

public:

	bool	m_bCycledOnce;

private:
	bool m_Active;							// true if this airspace is currently active (visible and containing particles)
	uint32 m_NumParticles;					// number of particles in this airspace (based on density and volume)
	float m_LOD;							// level of detail for this airspace (0 is invisible, 1 is fully visible)
	CSnowFX* m_Parent;						// the SnowFX object containing this airspace
	LTVector m_Pos;							// center of this airspace
	LTVector m_Dims;						// dimensions of this airspace
	LTVector m_MinBounds;					// min bounds for this airspace
	LTVector m_MaxBounds;					// max bounds for this airspace
	typedef std::vector<uint32> TBlockerList;
	TBlockerList m_Blockers;			// indices of particle blockers that affect this airspace

	// the following members are only valid for active airspaces
	CSnowFXParticle* m_Particles;			// array of particles
	float* m_MinY;							// matching array of minimum elevations for each particle
	HOBJECT m_Effect;						// volume effect object for this airspace

	static bool tableInit;
	static float sinTable[SNOWFX_TABLESIZE+SNOWFX_COSOFFSET];
	static uint32 rndTable[SNOWFX_TABLESIZE];

	bool	m_bUseCycling;
};


struct SNOWCREATESTRUCT : public SFXCREATESTRUCT
{
	SNOWCREATESTRUCT();

	LTVector		vDims;
	float			fDensity;
	float			fParticleRadius;
	float			fFallRate;
	float			fTumbleRate;
	float			fTumbleRadius;
	float			fMaxDrawDist;
	bool			bTranslucent;
	bool			bTranslucentLight;
	bool			bBackFaces;
	uint32			nBaseColor;
	std::string		sMaterialName;
	bool			bOn;
};


inline SNOWCREATESTRUCT::SNOWCREATESTRUCT()
:	SFXCREATESTRUCT		( ),
	vDims				( 0.0f, 0.0f, 0.0f ),
	fDensity			( 0.0f ),
	fParticleRadius		( 0.0f ),
	fFallRate			( 0.0f ),
	fTumbleRate			( 0.0f ),
	fTumbleRadius		( 0.0f ),
	fMaxDrawDist		( 0.0f ),
	bTranslucent		( true ),
	bTranslucentLight	( false ),
	bBackFaces			( false ),
	nBaseColor			( SETRGBA(0xFF, 0xFF, 0xFF, 0xFF) ),
	sMaterialName		( ),
	bOn					( true )
{
	
}


class CSnowFX : public CSpecialFX
{
public:
	CSnowFX();
	~CSnowFX();

	virtual bool Init( SFXCREATESTRUCT* psfxCreateStruct );
	virtual bool Update();
	virtual bool CreateObject( ILTClient* pClientDE );
	virtual uint32 GetSFXID() { return SFX_SNOW_ID; }

protected:
	bool OnServerMessage( ILTMessage_Read* pMsg );

public:

	float m_fLowestParticleY;

private:
	LTVector		m_vDims;
	float			m_fDensity;
	float			m_fParticleRadius;
	float			m_fFallRate;
	float			m_fTumbleRate;
	float			m_fTumbleRadius;
	float			m_fMaxDrawDist;
	float			m_fMaxDrawDistSq;
	bool			m_bTranslucent;
	bool			m_bTranslucentLight;
	bool			m_bBackFaces;
	uint32			m_nBaseColor;
	std::string		m_sMaterialName;
	HVERTEXDECL		m_hVertexDecl;
	bool			m_bOn;
	bool			m_bUseCycling;			// Use the particels' bCycled member to determine if the partice should be alphaed out
	

	LTVector		m_vMinBounds;			// bounding box min value
	LTVector		m_vMaxBounds;			// bounding box max value

	bool			m_bEnabled;				// if false, don't update (all airspaces will be inactive)
	float			m_fDensityScale;		// value of console variable specifying density
	float			m_fParticleSizeScale;	// value of console variable specifying snow particle scale

	uint32			m_nInternalTime;		// current time value in internal units (10000s of a second)

	uint32			m_nNumAirspaces;		// number of airspaces in this volume
	CSnowFXAirspace* m_pAirspaces;			// array of airspaces within this snow volume

	typedef std::list<HOBJECT, LTAllocator<HOBJECT, LT_MEM_TYPE_GAMECODE> > TFreeEffectList;
	typedef std::set<CSnowFXAirspace*, std::less<CSnowFXAirspace*>, LTAllocator<CSnowFXAirspace*, LT_MEM_TYPE_GAMECODE> > TSubVolumeSet;
	static TFreeEffectList freeEffects;			// shared list of volumeeffect objects that have been allocated, but aren't currently used
	TSubVolumeSet activeAirspaces;		// collection of airspaces that are currently active

	static CSnowFXParticleMgr m_ParticleMgr;		// shared block of particle information

	bool UpdateAirspaces();

	friend CSnowFXAirspace;
};


#endif // __SNOW_FX_H__
