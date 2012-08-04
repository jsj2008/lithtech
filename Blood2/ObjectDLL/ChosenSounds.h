// ----------------------------------------------------------------------- //
//
// MODULE  : ChosenSounds.h
//
// PURPOSE : List of files to cache.  These are files created by 
//
// CREATED : 6/21/98
//
// ----------------------------------------------------------------------- //

#ifndef __CHOSENSOUNDS_H__
#define __CHOSENSOUNDS_H__


/*
	CHARACTER_CALEB			0
	CHARACTER_OPHELIA		1
	CHARACTER_ISHMAEL		2
	CHARACTER_GABREILLA		3
*/

#define MAX_TAUNT_SOUNDS 14


/*
	Taunt sounds..
*/
char* g_pTauntSounds[4][MAX_TAUNT_SOUNDS] = 
{
	// Caleb
	{
		"tnt1_cal",
		"tnt2_cal",
		"tnt3_cal",
		"tnt4_cal",
		"tnt5_cal",
		"tnt6_cal",
		"tnt7_cal",
		"tnt8_cal",
		"tnt9_cal",
		"tnt10_cal",
		"tnt11_cal",
		"tnt12_cal",
		"tnt13_cal",
		"tnt14_cal"
	},
	// Ophelia
	{
		"tnt1_oph",
		"tnt2_oph",
		"tnt3_oph",
		"tnt4_oph",
		"tnt5_oph",
		"tnt6_oph",
		"tnt7_oph",
		"tnt8_oph",
		"tnt9_oph",
		"tnt10_oph",
		DNULL,
		DNULL,
		DNULL,
		DNULL
	},
	// Ishmael
	{
		"b2tnt1_ish",
		"b2tnt2_ish",
		"b2tnt3_ish",
		"b2tnt4_ish",
		"b2tnt5_ish",
		"b2tnt6_ish",
		"b2tnt7_ish",
		"b2tnt8_ish",
		"b2tnt9_ish",
		"b2tnt10ish",
		"b2tnt11ish",
		"b2tnt12ish",
		"b2tnt13ish",
		DNULL
	},
	// Gabriella
	{
		"b2tnt1_gab",
		"b2tnt2_gab",
		"b2tnt3_gab",
		"b2tnt4_gab",
		"b2tnt5_gab",
		"b2tnt6_gab",
		"b2tnt7_gab",
		"b2tnt8_gab",
		"b2tnt9_gab",
		"b2tnt10gab",
		"b2tnt11gab",
		DNULL,
		DNULL,
		DNULL
	}
};
	


#endif // __CHOSENSOUNDS_H__