#ifndef __VK__DEFS__H__
#define __VK__DEFS__H__

//----------------------------------------------------------
//
// MODULE  : VKDefs.h
//
// PURPOSE : Keyboard definitions and utilities (Windows-specific)
//
// CREATED : 6/27/97 7:26:40 PM
//
//----------------------------------------------------------

char VKToASCII (int nKey);

#ifdef DEFINE_WINDOWS_VIRTUAL_KEYS

#define VK_LBUTTON		1
#define VK_RBUTTON		2
#define VK_CANCEL		3
#define VK_MBUTTON		4

#define VK_BACK			8
#define VK_TAB			9

#define VK_CLEAR		12
#define VK_RETURN		13

#define VK_SHIFT		16
#define VK_CONTROL		17
#define VK_MENU			18
#define VK_PAUSE		19
#define VK_CAPITAL		20


#define VK_ESCAPE		27

#define VK_SPACE		32
#define VK_PRIOR		33
#define VK_NEXT			34
#define VK_END			35
#define VK_HOME			36
#define VK_LEFT			37
#define VK_UP			38
#define VK_RIGHT		39
#define VK_DOWN			40
#define VK_SELECT		41
#define VK_PRINT		42
#define VK_EXECUTE		43
#define VK_SNAPSHOT		44
#define VK_INSERT		45
#define VK_DELETE		46
#define VK_HELP			47

#define VK_LWIN			91
#define VK_RWIN			92
#define VK_APPS			93

#define VK_NUMPAD0		96
#define VK_NUMPAD1		97
#define VK_NUMPAD2		98
#define VK_NUMPAD3		99
#define VK_NUMPAD4		100
#define VK_NUMPAD5		101
#define VK_NUMPAD6		102
#define VK_NUMPAD7		103
#define VK_NUMPAD8		104
#define VK_NUMPAD9		105
#define VK_MULTIPLY		106
#define VK_ADD			107
#define VK_SEPARATOR	108
#define VK_SUBTRACT		109
#define VK_DECIMAL		110
#define VK_DIVIDE		111
#define VK_F1			112
#define VK_F2			113
#define VK_F3			114
#define VK_F4			115
#define VK_F5			116
#define VK_F6			117
#define VK_F7			118
#define VK_F8			119
#define VK_F9			120
#define VK_F10			121
#define VK_F11			122
#define VK_F12			123
#define VK_F13			124
#define VK_F14			125
#define VK_F15			126
#define VK_F16			127
#define VK_F17			128
#define VK_F18			129
#define VK_F19			130
#define VK_F20			131
#define VK_F21			132
#define VK_F22			133
#define VK_F23			134
#define VK_F24			135

#define VK_NUMLOCK		144
#define VK_SCROLL		145

#define VK_LSHIFT		160
#define VK_RSHIFT		161
#define VK_LCONTROL		162
#define VK_RCONTROL		163
#define VK_LMENU		164
#define VK_RMENU		165

#define VK_PROCESSKEY	229

#define VK_ATTN			246
#define VK_CRSEL		247
#define VK_EXSEL		248
#define VK_EREOF		249
#define VK_PLAY			250
#define VK_ZOOM			251
#define VK_NONAME		252
#define VK_PA1			253
#define VK_OEM_CLEAR	254

#endif // DEFINE_WINDOWS_VIRTUAL_KEYS


#define VK_TILDE		192

#define VK_0			48
#define VK_1			49
#define VK_2			50
#define VK_3			51
#define VK_4			52
#define VK_5			53
#define VK_6			54
#define VK_7			55
#define VK_8			56
#define VK_9			57

#define VK_A			65
#define VK_B			66
#define VK_C			67
#define VK_D			68
#define VK_E			69
#define VK_F			70
#define VK_G			71
#define VK_H			72
#define VK_I			73
#define VK_J			74
#define VK_K			75
#define VK_L			76
#define VK_M			77
#define VK_N			78
#define VK_O			79
#define VK_P			80
#define VK_Q			81
#define VK_R			82
#define VK_S			83
#define VK_T			84
#define VK_U			85
#define VK_V			86
#define VK_W			87
#define VK_X			88
#define VK_Y			89
#define VK_Z			90



#endif // __VK__DEFS__H__