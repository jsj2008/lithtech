// JoystickAxis.h: interface for the  joystick axis classes
//
//////////////////////////////////////////////////////////////////////

#ifndef USE_JOYSTICKAXIS
#define USE_JOYSTICKAXIS

#define JOYSTICK_MAX_AXIS	101

#define JOYSTICK_AXIS_NONE	0

class CBaseFolder;

/**************************************************************************/
// Joystick Axis info class
/**************************************************************************/

class CJoystickAxisInfo
{
public:
    void    Init(char* sName, uint32 nType, float fRangeLow, float fRangeHigh);

	char*	GetName() { return m_sName; };
    uint32  GetType() { return m_nType; };
	float	GetRangeLow() { return m_fRangeLow; };
	float	GetRangeHigh() { return m_fRangeHigh; };

private:
	char	m_sName[INPUTNAME_LEN];
    uint32  m_nType;
	float	m_fRangeLow;
	float	m_fRangeHigh;
};

/**************************************************************************/
// Joystick Axis base class
/**************************************************************************/

class CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisBase();

	// Build the  items
    virtual void Build(ILTClient *pClientDE, CBaseFolder *pDestFolder);

	// Updates the enable/disable status of each control.
	// bJoystickOn indicates if the joystick is enabled in the s.
    virtual void        UpdateEnable(LTBOOL bJoystickOn);

	// Load/Save to and from the console
    virtual void        LoadFromConsole(ILTClient *pClientDE);
    virtual void        SaveToConsole(ILTClient *pClientDE);

	LTBOOL	IsAxisBound();

	CCycleCtrl*	GetAxisCtrl() {return m_pAxisCtrl;}
	int		GetAxis() {return m_nAxis;}
	void	SetAxis(int newAxis) {m_nAxis = newAxis;}


protected:
	// Build the  option that allows the user to select the axis
    CCycleCtrl  *BuildAxisOption(ILTClient *pClientDE, CBaseFolder *pDestFolder, int nMessageID, int *pnDestValue);

protected:
	// Set the joystick device name
    BOOL    SetDeviceName(ILTClient *pClientDE);

	// Creates all the data in the axis array
    void    CreateAxisArray(ILTClient *pClientDE);

	// Save out the new actions that might not be in the autoexec.cfg
    void    SaveNewActions(ILTClient *pClientDE);

	// get a binding for a joystick trigger
    void    GetJoystickBindingInfo(ILTClient *pClientDE);

	// Send out a rangebind to the console
    void    DoRangeBind(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh);

	// Clear a rangebind to the console
    void    ClearRangeBind(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName);

	// Send out a rangescale to the console
    void    DoRangeScale(ILTClient *pClientDE, char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset);

	// Set a consolve var to a float
    void    SetConsoleVar(ILTClient *pClientDE, char *lpszName, float nVal);

	// find the index into the axis array for the given axis name (return 0 if not found)
    int     GetAxisIndex(ILTClient *pClientDE, char* sAxisName);

	// scales a value from one range to another in a linear fashion
	float	ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax);

	// Save an analog binding to the consle
    void SaveToConsoleAnalog(ILTClient *pClientDE);

	// Save a digital binding to the consle
    void SaveToConsoleDigital(ILTClient *pClientDE);

protected:
	// names of possible actions
	char	m_sActionDigitalLow[MAX_ACTIONNAME_LEN];
	char	m_sActionDigitalHigh[MAX_ACTIONNAME_LEN];

	char	m_sActionAnalog[MAX_ACTIONNAME_LEN];
	char	m_sDeadZoneConsoleVar[MAX_ACTIONNAME_LEN+16];

	// numbers used to make the scale from the slider
	float	m_fScaleCenter;
	float	m_fScaleRangeDiv2;

	// rangebind and rangescale data
	char	m_sDeviceName[INPUTNAME_LEN];
	char	m_sTriggerName[INPUTNAME_LEN];
	float	m_fScale;
	float	m_fRangeScaleMin;
	float	m_fRangeScaleMax;
	float	m_fRangeScalePreCenterOffset;
	char	m_sActionName[MAX_ACTIONNAME_LEN];
	float	m_fRangeLow;
	float	m_fRangeHigh;
	char	m_sActionName2[MAX_ACTIONNAME_LEN];
	float	m_fRangeLow2;
	float	m_fRangeHigh2;
	float	m_fDeadZone;

	// set to TRUE if bindings were read in successfully
    LTBOOL               m_bBindingsRead;

    LTBOOL               m_bAddDigitalBindingsToAnalog;

	// Data members
	int					m_nAxis;
    LTBOOL               m_bInvertAxis;
	int					m_nDeadZone;
    LTBOOL               m_bAnalog;
	int					m_nSensitivity;
    LTBOOL               m_bCenterOffset;

	// Orig Data members
	int					m_nOrigAxis;
	int					m_nOrigSensitivity;
	int					m_nOrigDeadZone;
    LTBOOL               m_bOrigAnalog;
    LTBOOL               m_bOrigInvertAxis;
    LTBOOL               m_bOrigCenterOffset;

	// number of joystick axis
	int					m_nNumJoystickAxis;

	// Information about each joystick axis
	CJoystickAxisInfo m_aryAxisInfo[JOYSTICK_MAX_AXIS];

	// Message ID for each control
	int					m_nAxisID;
	int					m_nInvertAxisID;
	int					m_nDeadZoneID;
	int					m_nAnalogID;
	int					m_nSensitivityID;
	int					m_nCenterOffsetID;

	// GUI controls
	CCycleCtrl		*m_pAxisCtrl;
	CToggleCtrl		*m_pInvertAxisCtrl;
	CSliderCtrl			*m_pDeadZoneCtrl;
	CToggleCtrl		*m_pAnalogCtrl;
	CSliderCtrl			*m_pSensitivityCtrl;
	CToggleCtrl		*m_pCenterOffsetCtrl;
};

/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

class CJoystickAxisLook : public CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisLook();

	// Build the  items
    virtual void	Build(ILTClient *pClientDE, CBaseFolder *pDestFolder);

	// Updates the enable/disable status of each control.
	// bJoystickOn indicates if the joystick is enabled in the s.
    virtual void        UpdateEnable(LTBOOL bJoystickOn);

	// Load/Save to and from the console
    virtual void        LoadFromConsole(ILTClient *pClientDE);
    virtual void        SaveToConsole(ILTClient *pClientDE);

protected:
	// Members
    LTBOOL               m_bFixedPosition;

	// String IDs
	int					m_nFixedPositionID;

	// Controls
	CToggleCtrl		*m_pFixedPositionCtrl;
};

/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/

class CJoystickAxisTurn : public CJoystickAxisBase
{
public:
	// Load/Save to and from the console
    virtual void        LoadFromConsole(ILTClient *pClientDE);
    virtual void        SaveToConsole(ILTClient *pClientDE);
};

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

class CJoystickAxisMove : public CJoystickAxisBase
{
public:
	// Constructor
	CJoystickAxisMove();

	// Load/Save to and from the console
    virtual void        LoadFromConsole(ILTClient *pClientDE);
    virtual void        SaveToConsole(ILTClient *pClientDE);
};

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/

class CJoystickAxisStrafe : public CJoystickAxisMove
{
public:
	// Load/Save to and from the console
    virtual void        LoadFromConsole(ILTClient *pClientDE);
    virtual void        SaveToConsole(ILTClient *pClientDE);
};

#endif