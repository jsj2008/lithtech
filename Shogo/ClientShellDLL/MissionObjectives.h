// ----------------------------------------------------------------------- //
//
// MODULE  : MissionObjectives.h
//
// PURPOSE : Riot's Mission Objective system - Definition
//
// CREATED : 3/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __MISSIONOBJECTIVES_H
#define __MISSIONOBJECTIVES_H

class ILTClient;
class CRiotClientShell;

struct OBJECTIVE
{
	OBJECTIVE()		{ nID = 0; hSurface = LTNULL; bCompleted = LTFALSE; pPrev = LTNULL; pNext = LTNULL; }

	uint32		nID;
	LTBOOL		bCompleted;
	HSURFACE	hSurface;

	OBJECTIVE*	pPrev;
	OBJECTIVE*	pNext;
};

class CMissionObjectives
{
public:

	CMissionObjectives();
	~CMissionObjectives();

	void	Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);

	void	AddObjective (uint32 nID, LTBOOL bCompleted=LTFALSE);
	void	RemoveObjective (uint32 nID);
	void	CompleteObjective (uint32 nID);


	void	ScrollUp();
	void	ScrollDown();

	void	Reset();
	void	ResetTop();
	void	SetLevelName (char* strLevelName);
	void	StartOpenAnimation();
	void	StartCloseAnimation();
	void	Draw();

	LTBOOL	IsClosing()		{ return m_bCloseAnimating; }

	void		Save(ILTMessage_Write* hWrite);
	void		Load(ILTMessage_Read* hRead);

protected:

	void	DrawObjective (HSURFACE hScreen, OBJECTIVE* pObjective, LTRect* rcSrc, int x, int y);

protected:
	
	ILTClient*	m_pClientDE;
	CRiotClientShell* m_pClientShell;
	OBJECTIVE*	m_pObjectives;
	OBJECTIVE*	m_pTopObjective;
	HSURFACE	m_hDisplay;
	HSURFACE	m_hSeparator;
	uint32		m_cxSeparator;
	uint32		m_cySeparator;
	LTBOOL		m_bScrollable;
	
	LTBOOL		m_bOpenAnimating;
	LTBOOL		m_bCloseAnimating;
	LTRect		m_rcTop;
	LTRect		m_rcBottom;

	LTBOOL		m_bScrollingUp;
	LTBOOL		m_bScrollingDown;
	LTFLOAT		m_fScrollOffset;
	LTFLOAT		m_fScrollOffsetTarget;
};

#endif