#ifndef __FONT12_H
#define __FONT12_H

#include "bitmapfont.h"

class CFont12 : public CBitmapFont
{
public:

	virtual char* GetClassName()	{ return "CFont12"; }

protected:

	virtual void InitCharWidths();
};

inline void CFont12::InitCharWidths()
{
	m_nCharWidth[32] =  7;
	m_nCharWidth[33] =  5;
	m_nCharWidth[34] =  7;
	m_nCharWidth[35] =  14;
	m_nCharWidth[36] =  13;
	m_nCharWidth[37] =  17;
	m_nCharWidth[38] =  15;
	m_nCharWidth[39] =  5;
	m_nCharWidth[40] =  7;
	m_nCharWidth[41] =  7;
	m_nCharWidth[42] =  9;
	m_nCharWidth[43] =  12;
	m_nCharWidth[44] =  5;
	m_nCharWidth[45] =  8;
	m_nCharWidth[46] =  5;
	m_nCharWidth[47] =  9;
	m_nCharWidth[48] =  15;
	m_nCharWidth[49] =  5;
	m_nCharWidth[50] =  13;
	m_nCharWidth[51] =  12;
	m_nCharWidth[52] =  15;
	m_nCharWidth[53] =  13;
	m_nCharWidth[54] =  14;
	m_nCharWidth[55] =  13;
	m_nCharWidth[56] =  14;
	m_nCharWidth[57] =  14;
	m_nCharWidth[58] =  5;
	m_nCharWidth[59] =  5;
	m_nCharWidth[60] =  12;
	m_nCharWidth[61] =  12;
	m_nCharWidth[62] =  12;
	m_nCharWidth[63] =  13;
	m_nCharWidth[64] =  17;
	m_nCharWidth[65] =  15;
	m_nCharWidth[66] =  13;
	m_nCharWidth[67] =  13;
	m_nCharWidth[68] =  14;
	m_nCharWidth[69] =  12;
	m_nCharWidth[70] =  12;
	m_nCharWidth[71] =  13;
	m_nCharWidth[72] =  12;
	m_nCharWidth[73] =  6;
	m_nCharWidth[74] =  11;
	m_nCharWidth[75] =  12;
	m_nCharWidth[76] =  11;
	m_nCharWidth[77] =  16;
	m_nCharWidth[78] =  13;
	m_nCharWidth[79] =  14;
	m_nCharWidth[80] =  13;
	m_nCharWidth[81] =  15;
	m_nCharWidth[82] =  13;
	m_nCharWidth[83] =  14;
	m_nCharWidth[84] =  13;
	m_nCharWidth[85] =  13;
	m_nCharWidth[86] =  15;
	m_nCharWidth[87] =  21;
	m_nCharWidth[88] =  14;
	m_nCharWidth[89] =  14;
	m_nCharWidth[90] =  12;
	m_nCharWidth[91] =  7;
	m_nCharWidth[92] =  10;
	m_nCharWidth[93] =  6;
	m_nCharWidth[94] =  15;
	m_nCharWidth[95] =  11;
	m_nCharWidth[96] =  6;
	m_nCharWidth[97] =  11;
	m_nCharWidth[98] =  12;
	m_nCharWidth[99] =  11;
	m_nCharWidth[100] = 12;
	m_nCharWidth[101] = 10;
	m_nCharWidth[102] = 9;
	m_nCharWidth[103] = 11;
	m_nCharWidth[104] = 11;
	m_nCharWidth[105] = 5;
	m_nCharWidth[106] = 7;
	m_nCharWidth[107] = 11;
	m_nCharWidth[108] = 5;
	m_nCharWidth[109] = 15;
	m_nCharWidth[110] = 11;
	m_nCharWidth[111] = 11;
	m_nCharWidth[112] = 11;
	m_nCharWidth[113] = 11;
	m_nCharWidth[114] = 9;
	m_nCharWidth[115] = 11;
	m_nCharWidth[116] = 9;
	m_nCharWidth[117] = 11;
	m_nCharWidth[118] = 13;
	m_nCharWidth[119] = 18;
	m_nCharWidth[120] = 12;
	m_nCharWidth[121] = 10;
	m_nCharWidth[122] = 11;
	m_nCharWidth[123] = 8;
	m_nCharWidth[124] = 5;
	m_nCharWidth[125] = 8;
	m_nCharWidth[126] = 12;
};

#endif