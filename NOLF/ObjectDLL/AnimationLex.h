// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONLEX_H__
#define __ANIMATIONLEX_H__

namespace Animation
{
	class CAnimationLex
	{
		public :
		
			// Ctors/Dtors/Etc
			
			CAnimationLex();
			~CAnimationLex();

			// Methods

			LTBOOL Lex(const char* szFile);

			// Handlers

			void HandleString();
			void HandleComment();
			void HandleNewline();

			// Simple accessors

			uint32 GetLine() { return m_nLine; }

		private :

			uint32	m_nLine;
	};
};

extern Animation::CAnimationLex g_Lex;
extern FILE *yyanimationin;
extern int yyanimationlex();

#endif
