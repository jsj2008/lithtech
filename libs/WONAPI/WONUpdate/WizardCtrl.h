//----------------------------------------------------------------------------------
// WizardCtrl.h
//
// Base Control for all of the Wizard screens.  The common functionality is 
// captured here and passed to the appropriate (active) control as needed.
//----------------------------------------------------------------------------------
#ifndef __WizardCtrl_H__
#define __WizardCtrl_H__

#include "WONGUI/GUICompat.h"
#include "WONGUI/Container.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const int SYS_BTN_WIDTH          = 18; // Width of a System Button (upper right corner).
const int SYS_BTN_HEIGHT         = 16; // Height of a System Button (upper right corner).
const int SYS_BTN_HORIZ_MARGIN   =  2; // Space between a System Button and the right frame.
const int SYS_BTN_VERT_MARGIN    =  2; // Space between a System Button and the top frame.
const int SYS_BTN_SPACING        =  2; // Space between System Buttons.
const int SEP_BAR_HEIGHT         =  2; // Height of the Separator Bar.
const int SEP_BAR_HORIZ_MARGIN   =  6; // Space between Separator Bar and the frame.
const int SEP_BAR_VERT_MARGIN    =  6; // Space between Separator Bar and components.
const int NAV_BTN_HORIZ_MARGIN   = 10; // Space between Navigation Buttons and the right frame.
const int NAV_BTN_VERT_MARGIN    = 10; // Space between Navigation Buttons and the bottom frame.
const int NAV_BTN_SPACING        = 10; // Space between Navigation Buttons.
const int STD_HORIZ_MARGIN       = 10; // Standard vertical space between controls.
const int STD_VERT_MARGIN        = 10; // Standard horizontal space between controls.
const int INPUT_HORIZ_SPACING    =  6; // Space between a label and an input field.
const int INPUT_VERT_OFFSET      =  3; // Delta between an input and its corresponding label to get the text to line up.
const int INPUT_VERT_SPACING     =  4; // Space beween similar input boxes.
const int CHECKBOX_EXTRA_SPACING =  1; // Extra space the Checkbox and it's text (starts w/ 4 pixels).
const int TEXT_AREA_LEFT_PAD     =  2; // White space inside Text Areas.
const int TEXT_AREA_TOP_PAD      = -1; // White space inside Text Areas.
const int TEXT_AREA_RIGHT_PAD    =  1; // White space inside Text Areas.
const int TEXT_AREA_BOTTOM_PAD   =  0; // White space inside Text Areas.
const int PROG_BAR_FRAME_WD      =  2; // Width of the Progress Bar Frame (aplies to each side).


//----------------------------------------------------------------------------------
// WizardCtrl
//----------------------------------------------------------------------------------
class WizardCtrl : public Container
{
public:
	WizardCtrl(void);

//	virtual void AddControls(void);
};

typedef SmartPtr<WizardCtrl> WizardCtrlPtr;

};

#endif
