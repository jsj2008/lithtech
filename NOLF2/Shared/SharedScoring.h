// ----------------------------------------------------------------------- //
//
// MODULE  : SharedScoring.h
//
// PURPOSE : SharedScoring - shared mission summary stuff
//
// CREATED : 10/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_SCORE_H__
#define __SHARED_SCORE_H__

#include "ltbasetypes.h"
#include "ClientServerShared.h"

class CPlayerScore
{
public:
	CPlayerScore();

    void	Init(uint32 nClientID);
    void    WriteData(ILTMessage_Write *pMsg);
    void    ReadData(ILTMessage_Read *pMsg);

#ifndef _CLIENTBUILD
	void	AddFrag();
	void	RemoveFrag();
	void	AddTag();
	void	RemoveTag();
	void	AddBonus(int nBonus);
	void	UpdateClients(HCLIENT hClients = LTNULL);
#endif

	int32	GetScore() {return m_nScore;}
	int32	GetFrags() {return m_nFrags;}
	int32	GetTags() {return m_nTags;}

private:
	uint32	m_nClientID;
	int32	m_nScore;
	int32	m_nFrags;
	int32	m_nTags;

	bool	m_bInitted;
};

#endif // __SHARED_SCORE_H__