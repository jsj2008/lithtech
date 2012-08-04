//----------------------------------------------------------------------------------
// ProgressBarComponent.cpp
//----------------------------------------------------------------------------------
#include "ProgressBarComponent.h"
#include "WONGUI/ColorScheme.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// Init: Initialize the progress bar settings.
//----------------------------------------------------------------------------------
void ProgressBarComponent::Init(void)
{
	mStartPos = 0;
	mEndPos = 0;
	mCurPos = 0;

	mColor = GetColorScheme()->GetStandardColor(StandardColor_Hilight);
	mColorBackground = 0;

	mImage = NULL;
	mImageBackground = NULL;
}


//----------------------------------------------------------------------------------
// Paint: Render the progress bar.
//----------------------------------------------------------------------------------
void ProgressBarComponent::Paint(Graphics &g)
{
	int Wd = 0;
	if (mCurPos > mStartPos)
		Wd = (int)(((double)(mCurPos - mStartPos)) * (double)Width() / 
				   (double)(mEndPos - mStartPos));

	if (mImageBackground.get() != NULL)
		g.DrawImage(mImageBackground.get(), 0,0, 0,0, Width(), Height());
	else
	{
		g.SetColor(mColorBackground);
		g.FillRect(0, 0, Wd, Height());
	}

	if (mImage.get() != NULL)
		g.DrawImage(mImage.get(), 0, 0, 0, 0, Wd, Height());
	else
	{
		g.SetColor(mColor);
		g.FillRect(0, 0, Wd, Height());
	}
}