#ifndef __CLIENTSERVERSHARED_H
#define __CLIENTSERVERSHARED_H

// Shared client/server enumerations...

// Player states
enum PlayerState { PS_UNKNOWN=0, PS_ALIVE, PS_DEAD, PS_DYING };

// Powerup types
enum TimedPowerupType { None, Damage, Shield, Stealth, Reflect, NightVision, Infrared, Silencer, };


// object user flags

#define USRFLG_VISIBLE				(1<<0)
#define USRFLG_PLAYER				(1<<1)
#define USRFLG_IGNORE_PROJECTILES	(1<<2)
#define USRFLG_SAVEABLE				(1<<3)

#define USRFLG_MOVEABLE				(1<<4)
#define USRFLG_PICKUPOBJ_ROTATE		(1<<4)

#define USRFLG_SINGULARITY_ATTRACT	(1<<5)

#define USRFLG_KICKABLE				(1<<6)

#define USRFLG_GLOW					(1<<7)
#define USRFLG_ANGERGLOW			(1<<8)
#define USRFLG_WILLPOWERGLOW		(1<<9)
#define USERFLG_NIGHTGOGGLESGLOW	(1<<10)
#define USRFLG_INVISIBLE			(1<<11)

// Camera types

#define CAMTYPE_FULLSCREEN			0
#define CAMTYPE_LETTERBOX			1
#define CAMTYPE_FISHEYE				2


// Exit trigger types

#define ENDWORLD_ENDOFWORLD			0
#define ENDWORLD_ENDOFSUBWORLD		1
#define ENDWORLD_ENDOFEPISODE		2
#define ENDWORLD_RESTART			3


// Light waveform types

enum WaveTypes {
	WAVE_NONE,
	WAVE_SQUARE,
	WAVE_SAW,
	WAVE_RAMPUP,
	WAVE_RAMPDOWN,
	WAVE_SINE,
	WAVE_FLICKER1,
	WAVE_FLICKER2,
	WAVE_FLICKER3,
	WAVE_FLICKER4,
	WAVE_STROBE,
	WAVE_SEARCH
};


// Utilities..
//-------------------------------------------------------------------------------------------
// Color255VectorToWord
//
// Converts a color in vector format to a word in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		pVal - Color vector
// Return:
//		D_WORD - converted color.
//-------------------------------------------------------------------------------------------
inline D_WORD Color255VectorToWord( DVector *pVal )
{
	D_WORD wColor;

	// For red, multiply by 5 bits and divide by 8, which is a net division of 3 bits.  Then shift it
	// to the left 11 bits to fit into result, which is a net shift of 8 to left.
	wColor = ( D_WORD )(((( DDWORD )pVal->x & 0xFF ) << 8 ) & 0xF800 );

	// For green, multiply by 6 bits and divide by 8, which is a net division of 2 bits.  Then shift it
	// to the left 5 bits to fit into result, which is a net shift of 3 to left.
	wColor |= ( D_WORD )(((( DDWORD )pVal->y & 0xFF ) << 3 ) & 0x07E0 );

	// For blue, multiply by 5 bits and divide by 8 = divide by 3.
	wColor |= ( D_WORD )((( DDWORD )pVal->z & 0xFF ) >> 3 );

	return wColor;
}


//-------------------------------------------------------------------------------------------
// Color255WordToVector
//
// Converts a color in word format to a vector in 5-6-5 format.  Color ranges are 0-255.
// Arguments:
//		wVal - color word
//		pVal - Color vector
// Return:
//		void
//-------------------------------------------------------------------------------------------
inline void Color255WordToVector( D_WORD wVal, DVector *pVal )
{
	// For red, divide by 11 bits then multiply by 8 bits and divide by 5 bits = divide by 8 bits...
	pVal->x = ( DFLOAT )(( wVal & 0xF800 ) >> 8 );

	// For green, divide by 5 bits, multiply by 8 bits, divide by 6 bits = divide by 3 bits.
	pVal->y = ( DFLOAT )(( wVal & 0x07E0 ) >> 3 );

	// For blue, divide by 5 bits, multiply by 8 bits = multiply by 3 bits
	pVal->z = ( DFLOAT )(( wVal & 0x001F ) << 3 );
}



#endif
