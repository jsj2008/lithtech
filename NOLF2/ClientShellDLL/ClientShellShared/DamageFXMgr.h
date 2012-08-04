// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.h
//
// PURPOSE : Damage FX Manager class - Definition
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_FX_MGR_H__
#define __DAMAGE_FX_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"
	#include "sprinklesfx.h"
	#include "VarTrack.h"
	#include "Timer.h"
	#include "TemplateList.h"
	#include "ClientFXMgr.h"
//
// Defins...
//

	#define DMGFXMGR_DEFAULT_FILE			"Attributes\\DamageFX.txt"
	#define DMGFXMGR_MAX_NAME_LEN	32
	#define DMGFXMGR_MAX_PATH		64
	#define DMGFXMGR_INVALID_ID		-1

	#define DAMAGEFX_TINT_OFF		0
	#define	DAMAGEFX_TINT_RAMPUP	1
	#define	DAMAGEFX_TINT_RAMPDOWN	2
	#define DAMAGEFX_TINT_ON		3

//
// Forwards...
//
	
	class CDamageFXMgr;

//
// Globals...
//

	extern CDamageFXMgr	*g_pDamageFXMgr;


struct DAMAGEFX
{
	DAMAGEFX();
	~DAMAGEFX();

	LTBOOL		Init( CButeMgr &ButeMgr, char *aTagName );
	void		Start( );
	void		Stop( LTBOOL bFade = LTTRUE );
	void		StartSoundAndVisuals();
	void		StopSoundAndVisuals( LTBOOL bFade = LTTRUE );
	void		Update( float fElapsedTime );
	LTVector	UpdateTintColor( );
	LTVector	UpdateLightScale( );

	void		CreateSprinkles();
	void		DestroySprinkles();

	inline LTBOOL IsActive( ) const { return m_bActive || m_bFade; }
	
	uint32		m_nID;
	DamageFlags	m_nDamageFlag;

	char		*m_szName;
	char		*m_szIcon;
	char		*m_szStartSound;
	char		*m_szLoopSound;
	char		*m_szFXName;
	char		*m_szTakingHealthFXName;
	char		*m_szTakingArmorFXName;
	char		*m_sz3rdPersonFXName;
	char		*m_sz3rdPersonInstFXName;
	char		*m_sz3rdPersonDeathFXName;
	char		*m_szBodyFXName;
	char		*m_aszSprinkleName[MAX_SPRINKLE_TYPES];
	char		m_szVarTrackName[64];

	LTVector	m_vMaxTintColor;	// The max tint color
	LTVector	m_vCurTintColor;	// Our current tint color
	uint8		m_TintDir;			// What state is the tint in
	LTFLOAT		m_fTintRampUpTm;	// How long does it take to get to full color
	LTFLOAT		m_fTintRampDownTm;	// How long does it take to get to no color
	LTFLOAT		m_fTintOffTm;		// How long is the tint at no color
	LTFLOAT		m_fTintOnTm;		// How long is the tint at full color

	LTFLOAT		m_fRotSpeed;		// Rate at which we roll around the cameras forward
	LTFLOAT		m_fRotMax;			// The maximum angle we can roll
	LTFLOAT		m_fRotDir;			// Which direction are we rolling
    LTFLOAT		m_fOffsetRot;		
    LTFLOAT		m_fMaxRot;
    LTFLOAT		m_fMinRot;
    LTFLOAT		m_fMoveMult;
	LTFLOAT		m_fMinFXPercent;

	LTFLOAT		m_fFOVXSpeed;
	LTFLOAT		m_fFOVYSpeed;
	LTFLOAT		m_fFOVMax;
	LTFLOAT		m_fFOVXOffset;
    LTFLOAT		m_fFOVYOffset;
    LTFLOAT		m_fFOVXDir;
    LTFLOAT		m_fFOVYDir;

	LTFLOAT		m_fLightScale;		// How much the light is scaled 0-1
	LTFLOAT		m_fLightScaleSpeed; // How fast the light gets scaled
	LTVector	m_vLightScaleColor;	// Color we are scaling to
	
	LTBOOL		m_bActive;			// Are we active?
	LTBOOL		m_bFade;			// Are we fading?
	LTFLOAT		m_fFadeTm;			// How long does it take to fade this fx

	bool		m_bAllowMovement;
	bool		m_bAllowInput;
	bool		m_bAllowWeapons;

	LTBOOL		m_bJumpRequested;	// Did the player tap jump?
	int8		m_nNumJumps;		// How many times the player jumped so far
	int8		m_nJumpsToEscape;	// How many jumps it takes to free the player

	HLTSOUND	m_hLoopSound;		

	//the timer that is used to time transitions between states.
	float		m_fElapsedTime;		// the amount of time that has already elapsed
	float		m_fEndTime;			// The time that the current state ends at
	VarTrack	m_vtTestFX;			// Used for testing the specific DamageFX

	SprinklesFX*		m_pSprinkles;
	uint8				m_nNumSprinkles;
	
	LTBOOL		m_bInstantEffect;	// Does this effect go away immediately instead of looping?

	CLIENTFX_LINK	m_DamageFXInstance;	// A link to an instance of a FxED created FX

	LTBOOL		m_bAttachCameraToAni;	// Should we attach the camera to the head of the animation
	LTBOOL		m_bShowClientModel;		// Should make the local client model visible

	LTBOOL		m_bAnimationControlsFX;		// Does the animation contain keys for turning certain fx on and off
	LTBOOL		m_bUpdateSoundAndVisuals;	// Should we update the sound and visual fx... can be controlled by anis
};

typedef CTList<DAMAGEFX*> DamageFxList;
typedef	CTList<DAMAGEFX*> ActiveDamageFxList;

class CDamageFXMgr : public CGameButeMgr
{
	friend struct DAMAGEFX;

	public: // Methods...

		CDamageFXMgr();
		~CDamageFXMgr();

		LTBOOL		Init( const char* szAttributeFile = DMGFXMGR_DEFAULT_FILE );
		void		Term();

		LTBOOL		WriteFile() { return m_buteMgr.Save(); }
		void		Reload()    { Term(); m_buteMgr.Term(); Init(); }

		DAMAGEFX	*GetDamageFX( uint32 nID );
		DAMAGEFX	*GetDamageFX( char *pName );
		
		int			GetNumDamageFX() const { return m_lstDamageFx.GetLength(); }

		DAMAGEFX	*GetFirstDamageFX() { return ( m_lstDamageFx.GetLength() ? *m_lstDamageFx.GetItem( TLIT_FIRST ) : LTNULL ); }
		DAMAGEFX	*GetNextDamageFX() { return ( m_lstDamageFx.GetLength() ? 
													(m_lstDamageFx.GetItem( TLIT_NEXT ) ? 
														*m_lstDamageFx.GetItem( TLIT_CURRENT ) : LTNULL) : LTNULL ); }

		DAMAGEFX	*GetFirstActiveFX() { return ( m_lstActiveDmgFx.GetLength() ? *m_lstActiveDmgFx.GetItem( TLIT_FIRST ) : LTNULL ); }
		DAMAGEFX	*GetNextActiveFX() { return ( m_lstActiveDmgFx.GetLength() ?
														(m_lstActiveDmgFx.GetItem( TLIT_NEXT ) ?
															*m_lstActiveDmgFx.GetItem( TLIT_CURRENT ) : LTNULL) : LTNULL); }

		LTBOOL		IsDamageActive( DamageFlags nDmgFlag );

		bool		AllowMovement() const { return m_bAllowMovement; }
		bool		AllowInput() const { return m_bAllowInput; }
		bool		AllowWeapons( ) const { return ( m_nDisableWeaponCounts == 0 ); }
		bool		IsFOVAffected();

		void		SetDamageFXTintColor( LTVector &vCol, bool bForce );
		void		SetDamageFXLightScale( LTVector &vCol, bool bForce );
		void		SetDamageFXMovementAndInput( bool bMove, bool bInput, bool bAlloWeapons );
		
		void		Update();
		void		Clear();

		LTBOOL		GetBool(char *pTag,char *pAttribute);
		LTFLOAT		GetFloat(char *pTag,char *pAttribute);
		int			GetInt(char *pTag,char *pAttribute);
		LTIntPt		GetPoint(char *pTag,char *pAttribute);
		uint32		GetDWord(char *pTag,char *pAttribute);
		void		GetString(char *pTag,char *pAttribute, char *pBuf, int nBufLen);
		LTVector	GetVector(char *pTag,char *pAttribute);

		bool		OnModelKey( HLOCALOBJ hObj, ArgList *pArgs );


	private :	// Members...

		DamageFxList		m_lstDamageFx;
		ActiveDamageFxList	m_lstActiveDmgFx;
		LTVector			m_vDamageFXTintColor;
		LTBOOL				m_bTintColorChanged;
		LTVector			m_vDamageFXLightScaleColor;
		LTBOOL				m_bLightScaleColorChanged;
		bool				m_bAllowMovement;
		bool				m_bAllowInput;
		uint32				m_nDisableWeaponCounts;

};

#endif //__DAMAGE_FX_MGR_H__