// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAlignment.h
//
// PURPOSE : Character alignment
//
// CREATED : 12/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_ALIGNEMENT_H__
#define __CHARACTER_ALIGNEMENT_H__

enum CharacterClass   	{ UNKNOWN=0, GOOD, BAD, NEUTRAL };
enum CharacterAlignment { LIKE=0, TOLERATE, HATE };
enum CharacterSide { CS_ENEMY = 0, CS_FRIEND, CS_NEUTRAL };

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAlignement
//
//	PURPOSE:	Determine how c1 feels about c2
//
// ----------------------------------------------------------------------- //

inline CharacterAlignment GetAlignement(CharacterClass c1, CharacterClass c2)
{
	switch(c1)
	{
		case GOOD:
		{
			switch(c2)
			{
				case BAD:
					return HATE;
				case GOOD:
					return LIKE;
				case NEUTRAL:
					return LIKE;
			}
		}
		break;

		case BAD:
		{
			switch(c2)
			{
				case BAD:
					return LIKE;
				case GOOD:
					return HATE;
				case NEUTRAL:
					return LIKE;
			}
		}
		break;

		case NEUTRAL:
		{
			switch(c2)
			{
				case BAD:
					return TOLERATE;
				case GOOD:
					return TOLERATE;
				case NEUTRAL:
					return LIKE;
			}
		}
		break;
	}

	return TOLERATE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSide
//
//	PURPOSE:	Determine whether c1 is on the same side as c2
//
// ----------------------------------------------------------------------- //

inline CharacterSide GetSide(CharacterClass c1, CharacterClass c2)
{
	CharacterSide cs = CS_NEUTRAL;
	if (c1 == GOOD)
	{
		if (c2 == GOOD)
			cs = CS_FRIEND;
		if (c2 == BAD)
			cs = CS_ENEMY;
	}
	else if (c1 == BAD)
	{
		if (c2 == GOOD)
			cs = CS_ENEMY;
		if (c2 == BAD)
			cs = CS_FRIEND;
	}
	return cs;
}

#endif // __CHARACTER_ALIGNEMENT_H__