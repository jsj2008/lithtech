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

enum CharacterClass   	{ UNKNOWN=0, CMC, SHOGO, FALLEN, CRONIAN, UCA, UCA_BAD, BYSTANDER, ROGUE, STRAGGLER, COTHINEAL };
enum CharacterAlignment { LIKE=0, TOLERATE, HATE };


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAlignement
//
//	PURPOSE:	Determine how c1 feels about c2
//
// ----------------------------------------------------------------------- //

inline CharacterAlignment GetAlignement(CharacterClass c1, CharacterClass c2)
{
	CharacterAlignment ca = HATE;

	switch(c1)
	{
		case CMC:
		{
			switch(c2)
			{
				case CMC:
					ca = LIKE;
				break;
				case BYSTANDER:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case SHOGO:
		{
			switch(c2)
			{
				case SHOGO:
					ca = LIKE;
				break;
				case BYSTANDER:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case CRONIAN:
		case FALLEN:
		{
			switch(c2)
			{
				case CRONIAN:
				case FALLEN:
				case COTHINEAL:
					ca = LIKE;
				break;
				case BYSTANDER:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case UCA:
		{
			switch(c2)
			{
				case UCA:
					ca = LIKE;
				break;
				case BYSTANDER:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case UCA_BAD:
		{
			switch(c2)
			{
				case UCA_BAD:
				case SHOGO:
					ca = LIKE;
				break;
				case BYSTANDER:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case BYSTANDER:
		{
			switch(c2)
			{
				case UCA:
				default : 
					ca = LIKE;	
				break;
			}
		}
		break;

		case COTHINEAL:
		{
			switch(c2)
			{
				case COTHINEAL:
					ca = LIKE;
				break;
				case CRONIAN:
				case FALLEN:
					ca = TOLERATE;
				break;
				default : break;
			}
		}
		break;

		case STRAGGLER:
		{
			switch(c2)
			{
				case STRAGGLER:
					ca = LIKE;
				break;
				default : break;
			}
		}
		break;
	}

	return ca;
}

#endif // __CHARACTER_ALIGNEMENT_H__