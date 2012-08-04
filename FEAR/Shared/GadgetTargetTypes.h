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

inline bool IsValidGadgetTargetType( int GT )
{
	return ( GT > eINVALID && GT < eMAX_GADGET_TARGET_TYPES );
}


struct GTINFO
{
	GTINFO( GadgetTargetType e = eINVALID, const char *pN = "INVALID", bool bSTB = true, bool bRT = true,
			 bool bCS = false, bool bCLP = false, bool bCW = false, bool bCCB = false )
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
	const bool		m_bShowTimeBar;
	const bool		m_bResetTime;
	const bool		m_bCanShoot;
	const bool		m_bCanLockPick;
	const bool		m_bCanWeld;
	const bool		m_bCanCodeBreak;
};


// Hard table of Gadget Target Types...

static GTINFO GTInfoArray[] =
{			
	//		TYPE			NAME			 BAR	  TIME	   SHOOT	LOCKPICK   WELD	  CODE
	GTINFO( eINVALID ),
	GTINFO( eKeyPad,		"KeyPad",		 true,  true,  false, false, false, true ),
	GTINFO( eCardReader,	"CardReader",	 true,  true,  false, false, false, true ),
	GTINFO( eCodedText,		"CodedText",	 true, true,  false, false, false, true ),
	GTINFO( eComboLock,		"ComboLock",	 true,  false, false, false, true,  false ),
	GTINFO( ePadLock,		"PadLock",		 true,  true,  true,  true,  false, false ),
	GTINFO( eDoorKnob,		"DoorKnob",		 true,  true,  false, true,  false, false ),
	GTINFO( eTelephone,		"Telephone",	 false, true,  false, false, false, false ),
	GTINFO( ePhotographable,"Photographable",false, true,  false, false, false, false ),
	GTINFO( eBombable,		"Bombable",		 false, true,  false, false, false, false ),
	GTINFO( eInvisibleInk,	"InvisibleInk",	 true,  true,  false, false, false, false ),
};



#endif // __GADGET_TARGET_TYPES_H__