// ----------------------------------------------------------------------- //
//
// MODULE  : AimMgr.h
//
// PURPOSE : definition of class to manage player aim mode
//
// CREATED : 05/18/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AIMMGR_H__
#define __AIMMGR_H__

class AimMgr
{
private: // Singelton...

	AimMgr();
	~AimMgr();

	AimMgr(	const AimMgr &other );
	AimMgr& operator=( const AimMgr &other );

public: // Singelton...

	__declspec(noinline) static AimMgr& Instance( )	{ static AimMgr sAccuracyMgr; return sAccuracyMgr; }

	void	Init();
	void	Update();

	bool	CanAim() const;
	void	BeginAim();
	void	EndAim();

	bool	IsAiming() const {return m_bAiming;}

	bool	OnCommandOn(int command);
	bool	OnCommandOff(int command);

	void	SetCanAim(bool bCanAim);

private:



	bool	m_bAiming;
	bool	m_bCanAim;

};

#endif  // __AIMMGR_H__
