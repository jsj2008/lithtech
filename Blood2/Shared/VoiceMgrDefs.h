/****************************************************************************
;
;	 MODULE:		VOICEMGRDEFS (.H)
;
;	PURPOSE:		Voice Manager Definitions
;
;	HISTORY:		10/11/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _VOICEMGRDEFS_H_
#define _VOICEMGRDEFS_H_


// Voice Manager Characters...

#define	VMC_CALEB			0
#define	VMC_OPHELIA			1
#define VMC_ISHMAEL			2
#define VMC_GABRIELLA		3

#ifdef _ADDON
#define	VMC_MALECULTIST		4
#define	VMC_FEMALECULTIST	5
#define	VMC_SOULDRUDGE		6
#define	VMC_PROPHET			7
#endif

#ifdef _ADDON
#define VMC_MAX				8
#else
#define VMC_MAX				4
#endif


// Voice Manager Events...

#define VME_IDLE			0
#define VME_BIGGIB			1
#define VME_PAIN			2
#define VME_KILL			3
#define VME_DEATH			4
#define VME_BURNING			5
#define VME_POWERUP			6
#define VME_SPAWN			7
#define VME_SUICIDE			8
#define VME_WEAPON			9
#define VME_TAUNT			10
#define VME_JUMP			11

#define VME_MAX				12


// Voice Manager Uniques...

#define VMU_HOWITZER		0
#define VMU_TESLACANNON		1
#define VMU_SNIPERRIFLE		2
#define VMU_AKIMBO			3
#define VMU_VOODOODOLL		4
#define VMU_ASSAULTRIFLE	5
#define VMU_NAPALMLAUNCHER	6
#define VMU_SINGULARITY		7
#define VMU_SHOTGUN			8

#define VMU_MAX				9


// EOF...

#endif
