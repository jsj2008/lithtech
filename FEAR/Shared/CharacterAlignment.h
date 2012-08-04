// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAlignment.h
//
// PURPOSE : Character alignment
//
// CREATED : 12/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ALIGNMENT_H__
#define __CHARACTER_ALIGNMENT_H__


enum EnumCharacterAlignment 
{
	kCharAlignment_Invalid	= -1,
	kCharAlignment_Max		= 20,
};

enum EnumCharacterStance
{
	kCharStance_Undetermined,
	kCharStance_Like,	
	kCharStance_Hate,	
	kCharStance_Tolerate,
	kCharStance_Count,
};

typedef std::bitset<kCharStance_Count>	STANCE_BITS;

namespace CharacterAlignment
{
	extern const char* c_aszStance[];
	EnumCharacterStance ConvertNameToStance( const char* szName );
};

#endif // __CHARACTER_ALIGNMENT_H__
