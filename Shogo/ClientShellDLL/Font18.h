#ifndef __FONT18_H
#define __FONT18_H

#include "bitmapfont.h"

class CFont18 : public CBitmapFont
{
public:

	virtual char* GetClassName()	{ return "CFont18"; }

protected:

	virtual void InitCharWidths();
};

inline void CFont18::InitCharWidths()
{
	m_nCharWidth[32] =  9;
	m_nCharWidth[33] =  7;
	m_nCharWidth[34] =  9;
	m_nCharWidth[35] =  20;
	m_nCharWidth[36] =  18;
	m_nCharWidth[37] =  24;
	m_nCharWidth[38] =  20;
	m_nCharWidth[39] =  6;
	m_nCharWidth[40] =  9;
	m_nCharWidth[41] =  9;
	m_nCharWidth[42] =  11;
	m_nCharWidth[43] =  17;
	m_nCharWidth[44] =  6;
	m_nCharWidth[45] =  11;
	m_nCharWidth[46] =  6;
	m_nCharWidth[47] =  13;
	m_nCharWidth[48] =  20;
	m_nCharWidth[49] =  6;
	m_nCharWidth[50] =  18;
	m_nCharWidth[51] =  17;
	m_nCharWidth[52] =  20;
	m_nCharWidth[53] =  18;
	m_nCharWidth[54] =  19;
	m_nCharWidth[55] =  18;
	m_nCharWidth[56] =  19;
	m_nCharWidth[57] =  19;
	m_nCharWidth[58] =  6;
	m_nCharWidth[59] =  6;
	m_nCharWidth[60] =  17;
	m_nCharWidth[61] =  17;
	m_nCharWidth[62] =  17;
	m_nCharWidth[63] =  18;
	m_nCharWidth[64] =  25;
	m_nCharWidth[65] =  21;
	m_nCharWidth[66] =  18;
	m_nCharWidth[67] =  17;
	m_nCharWidth[68] =  19;
	m_nCharWidth[69] =  17;
	m_nCharWidth[70] =  16;
	m_nCharWidth[71] =  19;
	m_nCharWidth[72] =  16;
	m_nCharWidth[73] =  7;
	m_nCharWidth[74] =  15;
	m_nCharWidth[75] =  18;
	m_nCharWidth[76] =  15;
	m_nCharWidth[77] =  22;
	m_nCharWidth[78] =  18;
	m_nCharWidth[79] =  20;
	m_nCharWidth[80] =  18;
	m_nCharWidth[81] =  21;
	m_nCharWidth[82] =  19;
	m_nCharWidth[83] =  20;
	m_nCharWidth[84] =  17;
	m_nCharWidth[85] =  18;
	m_nCharWidth[86] =  21;
	m_nCharWidth[87] =  29;
	m_nCharWidth[88] =  19;
	m_nCharWidth[89] =  19;
	m_nCharWidth[90] =  17;
	m_nCharWidth[91] =  9;
	m_nCharWidth[92] =  13;
	m_nCharWidth[93] =  9;
	m_nCharWidth[94] =  20;
	m_nCharWidth[95] =  15;
	m_nCharWidth[96] =  9;
	m_nCharWidth[97] =  16;
	m_nCharWidth[98] =  15;
	m_nCharWidth[99] =  14;
	m_nCharWidth[100] = 16;
	m_nCharWidth[101] = 14;
	m_nCharWidth[102] = 12;
	m_nCharWidth[103] = 16;
	m_nCharWidth[104] = 14;
	m_nCharWidth[105] = 6;
	m_nCharWidth[106] = 9;
	m_nCharWidth[107] = 16;
	m_nCharWidth[108] = 7;
	m_nCharWidth[109] = 21;
	m_nCharWidth[110] = 14;
	m_nCharWidth[111] = 17;
	m_nCharWidth[112] = 15;
	m_nCharWidth[113] = 15;
	m_nCharWidth[114] = 12;
	m_nCharWidth[115] = 15;
	m_nCharWidth[116] = 12;
	m_nCharWidth[117] = 15;
	m_nCharWidth[118] = 17;
	m_nCharWidth[119] = 24;
	m_nCharWidth[120] = 16;
	m_nCharWidth[121] = 14;
	m_nCharWidth[122] = 15;
	m_nCharWidth[123] = 11;
	m_nCharWidth[124] = 5;
	m_nCharWidth[125] = 12;
	m_nCharWidth[126] = 17;
};

#endif