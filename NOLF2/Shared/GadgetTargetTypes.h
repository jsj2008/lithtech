// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetTypes.h
//
// PURPOSE : Shared types so the disabler knows what to do
//
// CREATED : 8/31/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef __GADGET_TARGET_TYPES_H__
#define __GADGET_TARGET_TYPES_H__

enum GadgetTargetType
{
	eINVALID = 0,
	eKeyPad,
	eCardReader,
	eCodedText,
	eComboLock,
	ePadLock,
	eDoorKnob,
	eTelephone,
	ePhotographable,
	eBombable,
	eInvisibleInk,

	eMAX_GADGET_TARGET_TYPES,
};

inline LTBOOL IsValidGadgetTargetType( int GT )
{
	return ( GT > eINVALID && GT < eMAX_GADGET_TARGET_TYPES );
}


struct GTINFO
{
	GTINFO( GadgetTargetType e = eINVALID, const char *pN = "INVALID", LTBOOL bSTB = LTTRUE, LTBOOL bRT = LTTRUE,
			 LTBOOL bCS = LTFALSE, LTBOOL bCLP = LTFALSE, LTBOOL bCW = LTFALSE, LTBOOL bCCB = LTFALSE )
			 :	m_eTargetType	( e ),
				m_pName			( pN ),
				m_bShowTimeBar	( bSTB ),
				m_bResetTime	( bRT ),
				m_bCanShoot		( bCS ),
				m_bCanLockPick	( bCLP ),
				m_bCanWeld		( bCW ),
				m_bCanCodeBreak	( bCCB )
	{

	}

	// Dont let anybody mess with us!...

	const GadgetTargetType	m_eTargetType;
	
	const char			*m_pName;
	const LTBOOL		m_bShowTimeBar;
	const LTBOOL		m_bResetTime;
	const LTBOOL		m_bCanShoot;
	const LTBOOL		m_bCanLockPick;
	const LTBOOL		m_bCanWeld;
	const LTBOOL		m_bCanCodeBreak;
};


// Hard table of Gadget Target Types...

static GTINFO GTInfoArray[] =
{			
	//		TYPE			NAME			 BAR	  TIME	   SHOOT	LOCKPICK   WELD	  CODE
	GTINFO( eINVALID ),
	GTINFO( eKeyPad,		"KeyPad",		 LTTRUE,  LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTTRUE ),
	GTINFO( eCardReader,	"CardReader",	 LTTRUE,  LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTTRUE ),
	GTINFO( eCodedText,		"CodedText",	 LTTRUE, LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTTRUE ),
	GTINFO( eComboLock,		"ComboLock",	 LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTTRUE,  LTFALSE ),
	GTINFO( ePadLock,		"PadLock",		 LTTRUE,  LTTRUE,  LTTRUE,  LTTRUE,  LTFALSE, LTFALSE ),
	GTINFO( eDoorKnob,		"DoorKnob",		 LTTRUE,  LTTRUE,  LTFALSE, LTTRUE,  LTFALSE, LTFALSE ),
	GTINFO( eTelephone,		"Telephone",	 LTFALSE, LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTFALSE ),
	GTINFO( ePhotographable,"Photographable",LTFALSE, LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTFALSE ),
	GTINFO( eBombable,		"Bombable",		 LTFALSE, LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTFALSE ),
	GTINFO( eInvisibleInk,	"InvisibleInk",	 LTTRUE,  LTTRUE,  LTFALSE, LTFALSE, LTFALSE, LTFALSE ),
};



#endif // __GADGET_TARGET_TYPES_H__