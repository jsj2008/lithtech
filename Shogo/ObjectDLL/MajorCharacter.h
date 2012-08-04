// ----------------------------------------------------------------------- //
//
// MODULE  : MajorCharacter.h
//
// PURPOSE : MajorCharacter - Definition - Wrapper for major characters
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __MAJOR_CHARACTER_H__
#define __MAJOR_CHARACTER_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"

class MajorCharacter : public BaseAI
{
	public :

 		MajorCharacter();
 		~MajorCharacter();

		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected:

		virtual DBOOL	ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);
		virtual void	KillDlgSnd();
		virtual void    PlayDialogSound(char* pSound, CharacterSoundType eType=CST_DIALOG, DBOOL bAtObjectPos=DFALSE);

};

#endif // __MAJOR_CHARACTER_H__
