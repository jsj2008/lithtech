//-------------------------------------------------------------------------
//
// MODULE  : ArcCtrl.h
//
// PURPOSE : GUI element for interacting with a segmented arc of buttons
//
// CREATED : 12/31/01 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __ARC_CTRL_H
#define __ARC_CTRL_H

#include "iltdrawprim.h"
#include "ScreenSpriteMgr.h"
#include "BaseScaleFX.h"
#include "TronLayoutMgr.h"
#include "SubroutineMgr.h"

typedef enum 
{
	SEG_TYPE_NONE = 0,

	SEG_LIBRARY_HIGHLIGHT,
	SEG_LIBRARY_ALPHA,
	SEG_LIBRARY_BETA,
	SEG_LIBRARY_GOLD,
	SEG_LIBRARY_FOREIGN,
	SEG_LIBRARY_GHOSTED,
	SEG_LIBRARY_CORRUPTED,

	SEG_SYSTEM_HIGHLIGHT,
	SEG_SYSTEM_CONFLICT,
	SEG_SYSTEM_BADBLOCK,
	SEG_SYSTEM_BASECODE,
	SEG_SYSTEM_BASECODE_LEFT,
	SEG_SYSTEM_BASECODE_MIDDLE,
	SEG_SYSTEM_BASECODE_RIGHT,
	SEG_SYSTEM_ALPHA,
	SEG_SYSTEM_BETA,
	SEG_SYSTEM_GOLD,

	// Creepin' corruption.
	SEG_SYSTEM_ALPHA_CORRUPT_0,
	SEG_SYSTEM_ALPHA_CORRUPT_1L,
	SEG_SYSTEM_ALPHA_CORRUPT_1R,
	SEG_SYSTEM_ALPHA_CORRUPT_2L,
	SEG_SYSTEM_ALPHA_CORRUPT_2R,
	SEG_SYSTEM_ALPHA_CORRUPT_3L,
	SEG_SYSTEM_ALPHA_CORRUPT_3R,
	SEG_SYSTEM_ALPHA_CORRUPT_4L,
	SEG_SYSTEM_ALPHA_CORRUPT_4R,
	SEG_SYSTEM_BETA_CORRUPT_0L,
	SEG_SYSTEM_BETA_CORRUPT_0R,
	SEG_SYSTEM_BETA_CORRUPT_1L,
	SEG_SYSTEM_BETA_CORRUPT_1R,
	SEG_SYSTEM_BETA_CORRUPT_2L,
	SEG_SYSTEM_BETA_CORRUPT_2R,
	SEG_SYSTEM_GOLD_CORRUPT_0,
	SEG_SYSTEM_GOLD_CORRUPT_1L,
	SEG_SYSTEM_GOLD_CORRUPT_1R,

	SEG_TYPE_LAST,
} SegmentType;

typedef enum
{
	ARC_TYPE_UNDEFINED = 0,
	ARC_TYPE_SYSTEM_MEMORY,
	ARC_TYPE_LIBRARY,
} ArcType;

class CArcSector
{
public:
	CArcSector();
	~CArcSector();

	void Build(int iNum, LTIntPt pt, float r1, float r2, float theta1, float theta2, ArcType eArcType);
	void Term();

	void SetSub(PlayerSubroutine * pSubPtr)	{m_pSubPtr = pSubPtr;}
	PlayerSubroutine * GetSubObject()		{return m_pSubPtr;}

	void GetRange(int &iFirst, int &iLast) {iFirst = m_iFirst; iLast = m_iLast;}

	void ShowSprites(bool bShow);
	void ShowIcon(bool bShow);

	void Render(HOBJECT hObj, char * szSocketName);

	void Occupy(int iFirst, int iLast, PlayerSubroutine * pSubPtr);
	void ClearFX();

	void Ignore(bool bIgnore) {m_bIgnore = bIgnore;}
	bool ShouldIgnore() {return m_bIgnore;}

	bool IsOccupied() {return m_bOccupied;}

	void AddHighlighting(SegmentType eSegType);
	void RemoveHighlighting();

	void SetCondition(SegmentType eType, int iLevel = 0);

	CBaseScaleFX * CreateModel(SegmentType eSegType);
	
private:
	ArcType			m_eArcType;
	int				m_iSectorNumber;
	int				m_iAnimNumber;
	bool			m_bOccupied;			// used for the 'drop' cursor
	bool			m_bIgnore;			// used for the 'pickup' and 'procedural' cursor
	uint32			m_iColor;
	int				m_iFirst;
	int				m_iLast;
	bool			m_bShowSprites;
	bool			m_bShowIcon;

	CScreenSprite * m_pSprite;
	CScreenSprite * m_pRingSprite;
	CScreenSprite * m_pConditionSprite;

	PlayerSubroutine * m_pSubPtr;

	// The scaleFX
	CBaseScaleFX *	m_pSubTypeFX;
	HMODELSOCKET	m_Socket;
	CBaseScaleFX *	m_pHotFX;
	typedef std::vector<CBaseScaleFX *> FXArray;
	FXArray			m_ConditionFXArray;

	// Visual appearance parameters
	LTIntPt			m_Position;
	float			m_fInnerRadius;
	float			m_fOuterRadius;
	float			m_fTheta1;
	float			m_fTheta2;
};

class CArcCtrl
{
public:
	CArcCtrl();
	virtual ~CArcCtrl();

	LTBOOL		Render();

	LTBOOL		Init();
	LTBOOL		Init(LTIntPt pos, float fInner, float fOuter, int nNumSegments);
	void		SetArcType(ArcType eType) {m_eArcType = eType;}

	void		Term();

	// Parameters for controlling appearance and input
	void		SetArc(float fArcStart, float fArcEnd);	// default is full circle
	void		SetRadii(float fInnerRadius, float fOuterRadius, float fThreshold = 10.0f);
	void		SetPosition(LTIntPt pos);
	void		SetNumSegments(int num);
	void		SetNumHighlightSegments(int num);		// essentially, cursor size
	void		ShowInactive(LTBOOL bShowInactive);
	void		ShowOutlines(LTBOOL bShowOutlines);

	// Color-code overrides
	void		SetHighlightColor(uint32 color);
	void		SetEmptyColor(uint32 color);
	void		SetOutlineColor(uint32 color);
	void		SetConflictColor(uint32 color);

	int			GetActiveSegment() { return m_CurrentSegment; }

	PlayerSubroutine * GetHotSub();
	PlayerSubroutine * GetSegmentObject(int iSeg);

	void		AssociateHObject(HOBJECT hObj, char * szSocketName);
	HOBJECT		GetHObject() {return m_hObj;}

	void		SetLibraryState(int i) {m_iLibraryState = i;}
	int			GetLibraryState() {return m_iLibraryState;}

	void		ShowSprites(bool bShow);
	void		ShowIcon(int iSeg, bool bShow);

	// Menu interaction with the elements
	void		ShiftClockwise(PlayerSubroutine * pNewSub);
	void		ShiftCounterClockwise(PlayerSubroutine * pNewSub);

	void		OccupySegment(int segment, PlayerSubroutine * pSub);

	void		ClearSegment(int segment);

	void		IgnoreSegment(int segment, bool bIgnore);

	// User input
	LTBOOL		OnMouseMove(int mx, int my);
	void		Update(); // it's a mouse move without a mouse move

	void		RemoveHighlighting();
	void		AddHighlighting();

	LTBOOL		IsOnMe(int mx, int my);
	bool		IsHot() {return m_bIsHot;}

	void		Build();					// given the current parameters, rebuild the control

protected:
	ArcType		m_eArcType;

	int			m_iLibraryState;

	void		ComputeOptimalTesselation();

	float		VectorToAngle(float x, float y);

	int			m_iTesselation;

	// most recent mouse cursor coordinates
	int			m_mouseX;
	int			m_mouseY;

	int			m_CurrentSegment;
	float		m_fSegmentTheta;
	float		m_fMouseAngle;
	LTBOOL		m_bShowInactive;
	LTBOOL		m_bShowOutlines;

	uint32		m_HighlightColor;
	uint32		m_EmptyColor;
	uint32		m_OutlineColor;
	uint32		m_ConflictColor;

	LTIntPt		m_Position;
	float		m_fArcStart;
	float		m_fArcEnd;
	float		m_fInnerRadius;
	float		m_fOuterRadius;
	float		m_fThreshold;
	int			m_nNumSegments;
	int			m_nNumHighlightSegments;	// 1-3, 0 = ignore, -1 = pickup

	LTBOOL		m_bHasChangedAppearance;
	bool		m_bIsHot;

	// For a rotation
	float		m_fAnimTime;			// remaining time for an animated rotation
	float		m_fTotalAnimTime;

	BSCREATESTRUCT m_bcs;				// used for the creation/recreation of all the models used

	HOBJECT		m_hObj;					// attached model
	char		m_szSocketName[32];		// socket to use

	// various arrays
	int			m_nSegmentPrims;
	LT_POLYF4	*m_pSegmentPrims;
	int			m_nOutlinePrims;
	LT_POLYF4	*m_pOutlinePrims;
	int			m_nSeparatorPrims;
	LT_POLYF4	*m_pSeparatorPrims;

	CArcSector	*m_SectorArray;
};

#endif // __ARC_CTRL_H
