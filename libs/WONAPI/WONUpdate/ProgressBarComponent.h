//----------------------------------------------------------------------------------
// ProgressBarComponent.h
//----------------------------------------------------------------------------------
#ifndef __ProgressBarComponent_H__
#define __ProgressBarComponent_H__

#include "WONGUI/Component.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// ProgressBarComponent
//----------------------------------------------------------------------------------
class ProgressBarComponent: public Component
{
protected:
	// Attributes.
	int      mStartPos;
	int      mEndPos;
	int      mCurPos;
	DWORD    mColor;
	DWORD    mColorBackground;
	ImagePtr mImage;
	ImagePtr mImageBackground;

	// Restricted Operations.
	inline void CheckBounds(void)           
	{
		if (mCurPos > mEndPos) 
			mCurPos = mEndPos;
		if (mCurPos < mStartPos) 
			mCurPos = mStartPos;
	}
	void Init(void);

public:
	// Operations.
	inline int  GetPosition(void) const               { return mCurPos; }
	inline void SetPosition(int Pos)                  { mCurPos = Pos; CheckBounds(); Invalidate(); }
	inline void IncrementPosition(int Pos = 1)        { mCurPos += Pos; CheckBounds(); Invalidate(); }
	inline void SetRange(int Start, int End)          { mStartPos = Start; mEndPos = End; CheckBounds(); Invalidate(); }
	inline void GetRange(int& Start, int& End) const  { Start = mStartPos; End = mEndPos; }
	inline int  GetStart(void) const                  { return mStartPos; }
	inline int  GetEnd(void) const                    { return mEndPos; }

	inline void SetColor(DWORD theColor)              { mColor = theColor; Invalidate(); }
	inline void SetBackgroundColor(DWORD theColor)    { mColorBackground = theColor; Invalidate(); }

	inline void SetImage(ImagePtr theImage)           { mImage = theImage; Invalidate(); }
	inline void SetBackgroundImage(ImagePtr theImage) { mImageBackground = theImage; Invalidate(); }

	// Overrides.
	virtual void Paint(Graphics &g);

	// Constructor.
	inline ProgressBarComponent(void)                 { Init(); }
	inline ~ProgressBarComponent(void)                {}

};

typedef SmartPtr<ProgressBarComponent> ProgressBarComponentPtr;

}

#endif
