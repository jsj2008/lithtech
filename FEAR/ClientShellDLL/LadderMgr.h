// ----------------------------------------------------------------------- //
//
// MODULE  : LadderMgr.h
//
// PURPOSE : Manage the player's interaction with ladders
//
// CREATED : 06/22/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LADDERMGR_H__
#define __LADDERMGR_H__

class CLadderFX;

class LadderMgr
{
private: // Singelton...

	LadderMgr();
	~LadderMgr();

	LadderMgr(	const LadderMgr &other );
	LadderMgr& operator=( const LadderMgr &other );

public: // Singelton...

	__declspec(noinline) static LadderMgr& Instance( )	{ static LadderMgr sLadderMgr; return sLadderMgr; }

	void	Init();
	void	Update();

	void	Reset();

	bool	CanReachLadder(CLadderFX *pLadder) const;
	bool	ActivateLadder(CLadderFX *pLadder);

	bool	CanReleaseLadder() const;
	void	ReleaseLadder();


	bool	IsClimbing() const {return (m_pLadder != NULL); }
	bool	IsAtTop() const;
	bool	IsAtBottom() const;

	bool	IsDismounting() const;

	CLadderFX* GetLadder() const { return m_pLadder; }

	const LTVector2& GetPitchRange() const { return m_vPitchRange; }
	const LTVector2& GetYawRange() const { return m_vYawRange; }

private:

	void StartSliding( bool bSliding );

	CLadderFX* m_pLadder;
	bool	m_bRightHand;
	bool	m_bStartTop;
	bool	m_bSliding;
	HLTSOUND m_hSlideSound;

	float		m_fRungHeight;
	float		m_fTopExtension;
	LTVector2	m_vPitchRange;
	LTVector2	m_vYawRange;

	LTVector	m_vEntryPos;
	LTRotation	m_rEntryRot;

};

#endif  // __LADDERMGR_H__
