#ifndef _LTFONT_PARAMS_H_
#define _LTFONT_PARAMS_H_

class LTFontParams
{
public: 
	LTFontParams():
		Weight(400),		//FW_NORMAL
		CharSet(0),			//ANSI_CHARSET
		OutPrecision(4),	//OUT_TT_PRECIS
		ClipPrecision(0),	//CLIP_DEFAULT_PRECIS
		Quality(4),			//ANTIALIASED_QUALITY
		PitchAndFamily(0),	//DEFAULT_PITCH
		Italic(0),
		Underline(0), 
		StrikeOut(0)
	{
	}

	long			Weight;
	unsigned char	CharSet;
	unsigned char	OutPrecision;
	unsigned char	ClipPrecision;
	unsigned char	Quality;
	unsigned char	PitchAndFamily;
	unsigned char	Italic; 
	unsigned char	Underline; 
	unsigned char	StrikeOut; 
};

#endif