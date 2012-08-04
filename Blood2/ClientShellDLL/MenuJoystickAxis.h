// MenuJoystickAxis.h: interface for the menu joystick axis classes
//
//////////////////////////////////////////////////////////////////////

#ifndef USE_MENUJOYSTICKAXIS
#define USE_MENUJOYSTICKAXIS

#define MENUJOYSTICK_MAX_AXIS	101

#define MENUJOYSTICK_AXIS_NONE	0


/**************************************************************************/
// Joystick Axis info class
/**************************************************************************/

class CMenuJoystickAxisInfo
{
public:
	void	Init(char* sName, DDWORD nType, float fRangeLow, float fRangeHigh);

	char*	GetName() { return m_sName; };
	DDWORD	GetType() { return m_nType; };
	float	GetRangeLow() { return m_fRangeLow; };
	float	GetRangeHigh() { return m_fRangeHigh; };

private:
	char	m_sName[INPUTNAME_LEN];
	DDWORD	m_nType;
	float	m_fRangeLow;
	float	m_fRangeHigh;
};

/**************************************************************************/
// Joystick Axis base class
/**************************************************************************/

class CMenuJoystickAxisBase
{
public:
	// Constructor
	CMenuJoystickAxisBase();

	// Build the menu items
	virtual void		Build(CClientDE *pClientDE, CMenuBase *pDestMenu);

	// Updates the enable/disable status of each control.
	// bJoystickOn indicates if the joystick is enabled in the menus.
	virtual void		UpdateEnable(DBOOL bJoystickOn);

	// Load/Save to and from the console
	virtual void		LoadFromConsole(CClientDE *pClientDE);
	virtual void		SaveToConsole(CClientDE *pClientDE);	

	// return the joystick device name
	char*				GetDeviceName() { return m_sDeviceName; };

protected:
	// Build the menu option that allows the user to select the axis
	CLTGUITextItemCtrl	*BuildAxisOption(CClientDE *pClientDE, CMenuBase *pDestMenu, int nMessageID, int *pnDestValue);

protected:
	// Set the joystick device name
	BOOL	SetDeviceName(CClientDE *pClientDE);

	// Creates all the data in the axis array
	void	CreateAxisArray(CClientDE *pClientDE);

	// Save out the new actions that might not be in the autoexec.cfg
	void	SaveNewActions(CClientDE *pClientDE);

	// get a binding for a joystick trigger
	void	GetJoystickBindingInfo(CClientDE *pClientDE);

	// Send out a rangebind to the console
	void	DoRangeBind(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName, char *lpszActionName, float nRangeLow, float nRangeHigh);

	// Clear a rangebind to the console
	void	ClearRangeBind(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName);

	// Send out a rangescale to the console
	void	DoRangeScale(CClientDE *pClientDE, char *lpszDevice, char *lpszControlName, float nScale, float nRangeScaleMin, float nRangeScaleMax, float nRangeScalePreCenterOffset);

	// Set a consolve var to a float
	void	SetConsoleVar(CClientDE *pClientDE, char *lpszName, float nVal);

	// find the index into the axis array for the given axis name (return 0 if not found)
	int		GetAxisIndex(CClientDE *pClientDE, char* sAxisName);

	// scales a value from one range to another in a linear fashion
	float	ScaleToRange(float fFromVal, float fFromMin, float fFromMax, float fToMin, float fToMax);

	// Save an analog binding to the consle
	void SaveToConsoleAnalog(CClientDE *pClientDE);

	// Save a digital binding to the consle
	void SaveToConsoleDigital(CClientDE *pClientDE);

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
	DBOOL	m_bBindingsRead;

	// set to TRUE to write out digital bindings as well as analog (set for ForwardBackward and LeftRight axis
	DBOOL	m_bAddDigitalBindingsToAnalog;

	// Data members
	int					m_nAxis;
	DBOOL				m_bInvertAxis;	
	int					m_nDeadZone;
	DBOOL				m_bAnalog;
	int					m_nSensitivity;		
	DBOOL				m_bCenterOffset;

	// Orig Data members
	int					m_nOrigAxis;
	int					m_nOrigSensitivity;
	int					m_nOrigDeadZone;
	DBOOL				m_bOrigAnalog;
	DBOOL				m_bOrigInvertAxis;
	DBOOL				m_bOrigCenterOffset;

	// number of joystick axis
	int					m_nNumJoystickAxis;

	// Information about each joystick axis
	CMenuJoystickAxisInfo m_aryAxisInfo[MENUJOYSTICK_MAX_AXIS];

	// Message ID for each control
	int					m_nAxisID;
	int					m_nInvertAxisID;
	int					m_nDeadZoneID;
	int					m_nAnalogID;
	int					m_nSensitivityID;	
	int					m_nCenterOffsetID;

	// GUI controls
	CLTGUITextItemCtrl	*m_pAxisCtrl;
	CLTGUIOnOffCtrl		*m_pInvertAxisCtrl;
	CLTGUISliderCtrl	*m_pDeadZoneCtrl;
	CLTGUIOnOffCtrl		*m_pAnalogCtrl;
	CLTGUISliderCtrl	*m_pSensitivityCtrl;
	CLTGUIOnOffCtrl		*m_pCenterOffsetCtrl;	
};

/**************************************************************************/
// Joystick Axis look class
/**************************************************************************/

class CMenuJoystickAxisLook : public CMenuJoystickAxisBase
{
public:
	// Constructor
	CMenuJoystickAxisLook();

	// Build the menu items
	virtual void		Build(CClientDE *pClientDE, CMenuBase *pDestMenu);

	// Updates the enable/disable status of each control.
	// bJoystickOn indicates if the joystick is enabled in the menus.
	virtual void		UpdateEnable(DBOOL bJoystickOn);

	// Load/Save to and from the console
	virtual void		LoadFromConsole(CClientDE *pClientDE);
	virtual void		SaveToConsole(CClientDE *pClientDE);

protected:
	// Members
	DBOOL				m_bFixedPosition;

	// String IDs
	int					m_nFixedPositionID;

	// Controls
	CLTGUIOnOffCtrl		*m_pFixedPositionCtrl;
};

/**************************************************************************/
// Joystick Axis turn class
/**************************************************************************/

class CMenuJoystickAxisTurn : public CMenuJoystickAxisBase
{
public:
	// Load/Save to and from the console
	virtual void		LoadFromConsole(CClientDE *pClientDE);
	virtual void		SaveToConsole(CClientDE *pClientDE);
};

/**************************************************************************/
// Joystick Axis move class
/**************************************************************************/

class CMenuJoystickAxisMove : public CMenuJoystickAxisBase
{
public:
	// Constructor
	CMenuJoystickAxisMove();
	
	// Load/Save to and from the console
	virtual void		LoadFromConsole(CClientDE *pClientDE);
	virtual void		SaveToConsole(CClientDE *pClientDE);
};

/**************************************************************************/
// Joystick Axis stafe class
/**************************************************************************/

class CMenuJoystickAxisStrafe : public CMenuJoystickAxisMove
{
public:
	// Load/Save to and from the console
	virtual void		LoadFromConsole(CClientDE *pClientDE);
	virtual void		SaveToConsole(CClientDE *pClientDE);
};

#endif