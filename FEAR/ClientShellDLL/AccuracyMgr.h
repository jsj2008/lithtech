// ----------------------------------------------------------------------- //
//
// MODULE  : AccuracyMgr.h
//
// PURPOSE : definition of class to handle accuracy penalties
//
// CREATED : 05/10/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ACCURACYMGR_H__
#define __ACCURACYMGR_H__


class CAccuracyMgr
{
	DECLARE_SINGLETON( CAccuracyMgr );
	
public: 

	void	Init();
	void	Update();

	void	Reset();

	//return value in 0..1 range
	float	GetCurrentPerturb() const {return m_fCurrentPerturb;}

	//return value in terms of actual perturb for the current weapon
	float   GetCurrentWeaponPerturb() const;

	static VarTrack s_vtFastTurnRate;
	static VarTrack s_vtDebugPerturb;
	static VarTrack s_vtDebugPerturbPercent;

private:

	float	CalculateMovementPerturb();
	float	CalculateFiringPerturb();
	float	CalculateTurningPerturb();

	float		m_fCurrentPerturb;
	float		m_fTargetPerturb;
	LTVector	m_vLastForward;

	HWEAPONDATA m_hWpnData;
};

#endif  // __ACCURACYMGR_H__
