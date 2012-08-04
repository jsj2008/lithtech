#ifndef __FONT08_H
#define __FONT08_H

#include "bitmapfont.h"

class CFont08 : public CBitmapFont
{
public:

	virtual char* GetClassName()	{ return "CFont08"; }

protected:

	virtual void InitCharWidths();
};

inline void CFont08::InitCharWidths()
{
	m_nCharWidth[32] =  4;
	m_nCharWidth[33] =  4;
	m_nCharWidth[34] =  4;
	m_nCharWidth[35] =  9;
	m_nCharWidth[36] =  9;
	m_nCharWidth[37] =  11;
	m_nCharWidth[38] =  9;
	m_nCharWidth[39] =  3;
	m_nCharWidth[40] =  4;
	m_nCharWidth[41] =  4;
	m_nCharWidth[42] =  6;
	m_nCharWidth[43] =  8;
	m_nCharWidth[44] =  3;
	m_nCharWidth[45] =  5;
	m_nCharWidth[46] =  3;
	m_nCharWidth[47] =  7;
	m_nCharWidth[48] =  10;
	m_nCharWidth[49] =  4;
	m_nCharWidth[50] =  9;
	m_nCharWidth[51] =  8;
	m_nCharWidth[52] =  10;
	m_nCharWidth[53] =  9;
	m_nCharWidth[54] =  9;
	m_nCharWidth[55] =  9;
	m_nCharWidth[56] =  10;
	m_nCharWidth[57] =  9;
	m_nCharWidth[58] =  3;
	m_nCharWidth[59] =  3;
	m_nCharWidth[60] =  8;
	m_nCharWidth[61] =  9;
	m_nCharWidth[62] =  8;
	m_nCharWidth[63] =  9;
	m_nCharWidth[64] =  12;
	m_nCharWidth[65] =  10;
	m_nCharWidth[66] =  9;
	m_nCharWidth[67] =  8;
	m_nCharWidth[68] =  9;
	m_nCharWidth[69] =  9;
	m_nCharWidth[70] =  8;
	m_nCharWidth[71] =  9;
	m_nCharWidth[72] =  8;
	m_nCharWidth[73] =  4;
	m_nCharWidth[74] =  7;
	m_nCharWidth[75] =  9;
	m_nCharWidth[76] =  7;
	m_nCharWidth[77] =  10;
	m_nCharWidth[78] =  9;
	m_nCharWidth[79] =  9;
	m_nCharWidth[80] =  9;
	m_nCharWidth[81] =  10;
	m_nCharWidth[82] =  9;
	m_nCharWidth[83] =  9;
	m_nCharWidth[84] =  9;
	m_nCharWidth[85] =  9;
	m_nCharWidth[86] =  10;
	m_nCharWidth[87] =  14;
	m_nCharWidth[88] =  9;
	m_nCharWidth[89] =  9;
	m_nCharWidth[90] =  9;
	m_nCharWidth[91] =  4;
	m_nCharWidth[92] =  6;
	m_nCharWidth[93] =  5;
	m_nCharWidth[94] =  10;
	m_nCharWidth[95] =  7;
	m_nCharWidth[96] =  4;
	m_nCharWidth[97] =  8;
	m_nCharWidth[98] =  7;
	m_nCharWidth[99] =  7;
	m_nCharWidth[100] = 7;
	m_nCharWidth[101] = 7;
	m_nCharWidth[102] = 6;
	m_nCharWidth[103] = 8;
	m_nCharWidth[104] = 7;
	m_nCharWidth[105] = 3;
	m_nCharWidth[106] = 5;
	m_nCharWidth[107] = 7;
	m_nCharWidth[108] = 4;
	m_nCharWidth[109] = 10;
	m_nCharWidth[110] = 7;
	m_nCharWidth[111] = 8;
	m_nCharWidth[112] = 8;
	m_nCharWidth[113] = 8;
	m_nCharWidth[114] = 6;
	m_nCharWidth[115] = 7;
	m_nCharWidth[116] = 6;
	m_nCharWidth[117] = 7;
	m_nCharWidth[118] = 8;
	m_nCharWidth[119] = 11;
	m_nCharWidth[120] = 8;
	m_nCharWidth[121] = 7;
	m_nCharWidth[122] = 7;
	m_nCharWidth[123] = 6;
	m_nCharWidth[124] = 3;
	m_nCharWidth[125] = 6;
	m_nCharWidth[126] = 9;
};

#endif