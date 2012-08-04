// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMoveMgr.h
//
// PURPOSE : Manage the player's interaction with SpecialMoves
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIALMOVEMGR_H__
#define __SPECIALMOVEMGR_H__

class CSpecialMoveFX;

class SpecialMoveMgr
{
private: // Singelton...

	SpecialMoveMgr();
	~SpecialMoveMgr();

	SpecialMoveMgr(	const SpecialMoveMgr &other );
	SpecialMoveMgr& operator=( const SpecialMoveMgr &other );

public: // Singelton...

	__declspec(noinline) static SpecialMoveMgr& Instance( )	{ static SpecialMoveMgr sSpecialMoveMgr; return sSpecialMoveMgr; }

	void	Init();
	void	Update();

	void	Reset();

	bool	CanReach(CSpecialMoveFX* pSpecialMove) const;
	bool	CanLookAt(CSpecialMoveFX* pSpecialMove) const;
	void	HandleLookedAt(CSpecialMoveFX* pSpecialMove);
	bool	Activate(CSpecialMoveFX* pSpecialMove);
	void	Release();

	bool	IsActive() const { return (m_pSpecialMove != NULL); }

	CSpecialMoveFX* GetObject() const { return m_pSpecialMove; }

private:

	CSpecialMoveFX* m_pSpecialMove;
	LTVector m_vDesiredPos;
	bool m_bInterpolate;
	bool m_bAnimStarted;

	CSpecialMoveFX* m_pLastLookedAt;
};

#endif  // __SPECIALMOVEMGR_H__
