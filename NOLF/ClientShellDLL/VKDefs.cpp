//----------------------------------------------------------
//
// MODULE  : VKDefs.cpp
//
// PURPOSE : Keyboard definitions and utilities (Windows-specific)
//
// CREATED : 12/29/97
//
//----------------------------------------------------------

#include "stdafx.h"
#include "iltclient.h"

char VKToASCII (int nKey)
{
    LTBOOL bShiftDown = !!(GetKeyState (VK_SHIFT) & 0x8000);
    LTBOOL bCapsLockOn = !!(GetKeyState (VK_CAPITAL) & 0x01);
    LTBOOL bNumLockOn = !!(GetKeyState (VK_NUMLOCK) & 0x01);
    LTBOOL bUpperCase = (bCapsLockOn && !bShiftDown) || (!bCapsLockOn && bShiftDown);

	if (nKey >= 'A' && nKey <= 'Z' && !bUpperCase)
	{
		nKey += 32;
	}
	else if ((nKey >= 48 && nKey <= 57) && bShiftDown)
	{
		// we are not checking explicitly for some of
		// the values included in the ranges above since
		// because they are characters only derived with
		// a shift combination, they should not ever be
		// encountered

		switch (nKey)
		{
			case '0': nKey = ')'; break;
			case '1': nKey = '!'; break;
			case '2': nKey = '@'; break;
			case '3': nKey = '#'; break;
			case '4': nKey = '$'; break;
			case '5': nKey = '%'; break;
			case '6': nKey = '^'; break;
			case '7': nKey = '&'; break;
			case '8': nKey = '*'; break;
			case '9': nKey = '('; break;
		}
	}
	else if ((nKey >= 186 && nKey <= 191) || (nKey >= 219 && nKey <= 222))
	{
		switch (nKey)
		{
			case 186: nKey = bShiftDown ? ':' : ';'; break;
			case 187: nKey = bShiftDown ? '+' : '='; break;
			case 188: nKey = bShiftDown ? '<' : ','; break;
			case 189: nKey = bShiftDown ? '_' : '-'; break;
			case 190: nKey = bShiftDown ? '>' : '.'; break;
			case 191: nKey = bShiftDown ? '?' : '/'; break;

			case 219: nKey = bShiftDown ? '{' : '['; break;
			case 220: nKey = bShiftDown ? '|' : '\\'; break;
			case 221: nKey = bShiftDown ? '}' : ']'; break;
			case 222: nKey = bShiftDown ? '\"' : '\''; break;
		}
	}
	else if (nKey >= 96 && nKey <= 105 && bNumLockOn)
	{
		switch (nKey)
		{
			case VK_NUMPAD0: nKey = '0'; break;
			case VK_NUMPAD1: nKey = '1'; break;
			case VK_NUMPAD2: nKey = '2'; break;
			case VK_NUMPAD3: nKey = '3'; break;
			case VK_NUMPAD4: nKey = '4'; break;
			case VK_NUMPAD5: nKey = '5'; break;
			case VK_NUMPAD6: nKey = '6'; break;
			case VK_NUMPAD7: nKey = '7'; break;
			case VK_NUMPAD8: nKey = '8'; break;
			case VK_NUMPAD9: nKey = '9'; break;
		}
	}
	else if (nKey >= 106 && nKey <= 111)
	{
		switch (nKey)
		{
			case VK_MULTIPLY:	nKey = '*'; break;
			case VK_ADD:		nKey = '+'; break;
			case VK_SUBTRACT:	nKey = '-'; break;
			case VK_DECIMAL:	nKey = '.'; break;
			case VK_DIVIDE:		nKey = '/'; break;
		}
	}

	return nKey;
}