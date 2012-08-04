#ifndef __FONT28_H
#define __FONT28_H

#include "bitmapfont.h"

class CFont28 : public CBitmapFont
{
public:

	virtual char* GetClassName()	{ return "CFont28"; }

protected:

	virtual void InitCharWidths();
};

inline void CFont28::InitCharWidths()
{
	m_nCharWidth[32] =  12;
	m_nCharWidth[33] =  8;
	m_nCharWidth[34] =  12;
	m_nCharWidth[35] =  30;
	m_nCharWidth[36] =  28;
	m_nCharWidth[37] =  36;
	m_nCharWidth[38] =  29;
	m_nCharWidth[39] =  7;
	m_nCharWidth[40] =  11;
	m_nCharWidth[41] =  12;
	m_nCharWidth[42] =  16;
	m_nCharWidth[43] =  24;
	m_nCharWidth[44] =  8;
	m_nCharWidth[45] =  15;
	m_nCharWidth[46] =  7;
	m_nCharWidth[47] =  18;
	m_nCharWidth[48] =  30;
	m_nCharWidth[49] =  8;
	m_nCharWidth[50] =  26;
	m_nCharWidth[51] =  24;
	m_nCharWidth[52] =  29;
	m_nCharWidth[53] =  26;
	m_nCharWidth[54] =  28;
	m_nCharWidth[55] =  26;
	m_nCharWidth[56] =  29;
	m_nCharWidth[57] =  28;
	m_nCharWidth[58] =  7;
	m_nCharWidth[59] =  7;
	m_nCharWidth[60] =  24;
	m_nCharWidth[61] =  25;
	m_nCharWidth[62] =  24;
	m_nCharWidth[63] =  26;
	m_nCharWidth[64] =  37;
	m_nCharWidth[65] =  31;
	m_nCharWidth[66] =  27;
	m_nCharWidth[67] =  24;
	m_nCharWidth[68] =  27;
	m_nCharWidth[69] =  25;
	m_nCharWidth[70] =  24;
	m_nCharWidth[71] =  28;
	m_nCharWidth[72] =  24;
	m_nCharWidth[73] =  8;
	m_nCharWidth[74] =  22;
	m_nCharWidth[75] =  25;
	m_nCharWidth[76] =  21;
	m_nCharWidth[77] =  33;
	m_nCharWidth[78] =  26;
	m_nCharWidth[79] =  29;
	m_nCharWidth[80] =  26;
	m_nCharWidth[81] =  31;
	m_nCharWidth[82] =  27;
	m_nCharWidth[83] =  28;
	m_nCharWidth[84] =  25;
	m_nCharWidth[85] =  26;
	m_nCharWidth[86] =  31;
	m_nCharWidth[87] =  44;
	m_nCharWidth[88] =  27;
	m_nCharWidth[89] =  29;
	m_nCharWidth[90] =  26;
	m_nCharWidth[91] =  12;
	m_nCharWidth[92] =  18;
	m_nCharWidth[93] =  12;
	m_nCharWidth[94] =  29;
	m_nCharWidth[95] =  21;
	m_nCharWidth[96] =  11;
	m_nCharWidth[97] =  22;
	m_nCharWidth[98] =  23;
	m_nCharWidth[99] =  20;
	m_nCharWidth[100] = 22;
	m_nCharWidth[101] = 20;
	m_nCharWidth[102] = 17;
	m_nCharWidth[103] = 23;
	m_nCharWidth[104] = 21;
	m_nCharWidth[105] = 8;
	m_nCharWidth[106] = 12;
	m_nCharWidth[107] = 23;
	m_nCharWidth[108] = 8;
	m_nCharWidth[109] = 31;
	m_nCharWidth[110] = 20;
	m_nCharWidth[111] = 24;
	m_nCharWidth[112] = 23;
	m_nCharWidth[113] = 23;
	m_nCharWidth[114] = 17;
	m_nCharWidth[115] = 22;
	m_nCharWidth[116] = 17;
	m_nCharWidth[117] = 20;
	m_nCharWidth[118] = 25;
	m_nCharWidth[119] = 36;
	m_nCharWidth[120] = 23;
	m_nCharWidth[121] = 21;
	m_nCharWidth[122] = 21;
	m_nCharWidth[123] = 16;
	m_nCharWidth[124] = 6;
	m_nCharWidth[125] = 15;
	m_nCharWidth[126] = 25;
};

#endif