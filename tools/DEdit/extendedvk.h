#ifndef __EXTENDEDVK_H__
#define __EXTENDEDVK_H__

//keycodes for some of the extended characters from Yuki. There could be
//potential issues on bizarre keyboards, and this is a sort of workaround
//(this and related changes YF 10/20)
enum ExtendedVirtualKeyCode
{
	EXTVK_SEMICOLON				= 186,
	EXTVK_EQUALS				= 187,
	EXTVK_COMMA					= 188,
	EXTVK_MINUS					= 189,
	EXTVK_PERIOD				= 190,
	EXTVK_SLASH					= 191,
	EXTVK_BACKQUOTE 			= 192,
	EXTVK_OPENBRACKET			= 219,
	EXTVK_BACKSLASH				= 220,
	EXTVK_CLOSEBRACKET			= 221,
	EXTVK_SINGLEQUOTE			= 222,
};
			
#endif