
#ifndef __SPRITECONTROL_H__
#define __SPRITECONTROL_H__


	// Sprite control flags.  Default flags for a sprite are SC_PLAY|SC_LOOP.
	#define SC_PLAY		(1<<0)
	#define SC_LOOP		(1<<1)


	class SpriteControl
	{
	public:

		virtual DRESULT		GetNumAnims(DDWORD &nAnims)=0;
		virtual DRESULT		GetNumFrames(DDWORD iAnim, DDWORD &nFrames)=0;
		
		virtual DRESULT		GetCurPos(DDWORD &iAnim, DDWORD &iFrame)=0;
		virtual DRESULT		SetCurPos(DDWORD iAnim, DDWORD iFrame)=0;
		
		virtual DRESULT		GetFlags(DDWORD &flags)=0;
		virtual DRESULT		SetFlags(DDWORD flags)=0;
	};


#endif




