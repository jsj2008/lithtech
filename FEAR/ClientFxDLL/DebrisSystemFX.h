// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisSystemFX.h
//
// PURPOSE : The DebrisSystemFX object
//
// CREATED : 12/18/03
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRISSYSTEMFX_H__
#define __DEBRISSYSTEMFX_H__

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __DEBRISSYSTEMPROPS_H__
#	include "DebrisSystemProps.h"
#endif

#ifndef __ILTPHYSICSSIM_H__
#	include "iltphysicssim.h"
#endif

class CDebrisSystemFX : 
	public CBaseFX
{
public:
	
	CDebrisSystemFX();
	~CDebrisSystemFX(); 

	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	virtual bool	Update( float tmFrameTime );
	virtual void	Term( void );

protected:

	//---------------------------------------
	// Debris Creation utilities

	//called to create a single emission of debris
	void		CreateDebrisEmission();

	//called to create a single piece of debris
	bool		CreateDebrisPiece(const LTRigidTransform& tObjTransform, uint32 nSystem, uint32 nType);

	//this will randomly generate an object space position for the starting of a particle based upon
	//the current properties of this effect
	LTVector	GenerateObjectSpaceDebrisPos(	const LTVector& vEmissionOffset, const LTVector& vEmissionDims,
												float fMinRadius, float fMaxRadius);

	//this will randomly generate an object space velocity for the starting of a particle based upon
	//the current properties of this effect
	LTVector	GenerateObjectSpaceDebrisVel(const LTVector& vObjSpacePos, const LTVector& vMinVel, const LTVector& vMaxVel);

	//this will generate the orientation to use for a piece of debris
	LTRotation	GenerateObjectSpaceDebrisRot(const LTVector& vObjSpacePos, const LTVector& vObjSpaceVel);

	const CDebrisSystemProps*	GetProps()		{ return (const CDebrisSystemProps*)m_pProps; }

	//information that is stored for each debris piece
	struct SPiece
	{
		//the physics rigid body that is simulating the piece
		HPHYSICSRIGIDBODY			m_hRigidBody;

		//the effect that provides the visual representation of the piece

		//the collision listener for this piece
		HPHYSICSCOLLISIONNOTIFIER	m_hCollisionNotifier;
	};

	//the number of rigid bodies that we have
	typedef std::vector<SPiece> TPieceList;
	TPieceList	m_Pieces;
};

#endif
