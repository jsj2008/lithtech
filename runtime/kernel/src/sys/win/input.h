//------------------------------------------------------------------
//
//  FILE      : Input.h
//
//  PURPOSE   : This module handles the input for in-game stuff.
//
//  CREATED   : May 17 1997
//
//  COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __INPUT_H__
#define __INPUT_H__

#ifndef __LTCODES_H__
#include "ltcodes.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __CONCOMMAND_H__
#include "concommand.h"
#endif


// --------------------------------------------------------------------- //
// Functions.
// --------------------------------------------------------------------- //

class InputMgr
{
public:
    bool  (*Init)(InputMgr *pMgr, ConsoleState *pState);
    void    (*Term)(InputMgr *pMgr);
    bool  (*IsInitted)(InputMgr *pMgr);


    // Shows all the devices in the console.
    void (*ListDevices)(InputMgr *pMgr);

    // Play a force-feedback effect
    long (*PlayJoystickEffect)(InputMgr *pMgr, const char *strEffectName, float x, float y);

    // This is the main routine that reads input from all devices and sets
    // any actions that are on.
    void (*ReadInput)(InputMgr *pMgr, BYTE *pActionsOn, float axisOffsets[3]);

    // Flushes the DirectInput buffers.
    bool (*FlushInputBuffers)(InputMgr *pMgr);

    // Clear all input.. forces everything to the off state.
    LTRESULT (*ClearInput)();

    // You want to add action defs first so there will be something to bind to!
    // Note:  action codes -1, -2, and -3 are for axis offsets!
    void (*AddAction)(InputMgr *pMgr, const char *pActionName, int actionCode);

    // Enables input from a particular device.
    bool (*EnableDevice)(InputMgr *pMgr, const char *pDeviceName);

    // Clear bindings for a device.
    bool (*ClearBindings)(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName);

    // Add a binding for a device (set ranges to 0 to not use ranges).
    bool (*AddBinding)(InputMgr *pMgr,
        const char *pDeviceName, const char *pTriggerName, const char *pActionName,
        float rangeLow, float rangeHigh);

    // Sets the trigger's scale.
    bool (*ScaleTrigger)(InputMgr *pMgr, const char *pDeviceName, const char *pTriggerName,
		float scale, float fRangeScaleMin, float fRangeScaleMax, float fRangeScalePreCenterOffset);

    // Returns a list of trigger bindings for the given device
    DeviceBinding *(*GetDeviceBindings)(uint32 nDevice);

    // Frees list of trigger bindings
    void (*FreeDeviceBindings)(DeviceBinding *pBindings);

    // Start tracking input devices
    bool (*StartDeviceTrack)(InputMgr *pMgr, uint32 nDevices, uint32 nBufferSize);

    // Track the input devices
    bool (*TrackDevice)(DeviceInput *pInputAttay, uint32 *pInOut);

    // End device tracking
    bool (*EndDeviceTrack)();

    // Retrieve a list of objects associated with one or more devices
    DeviceObject *(*GetDeviceObjects)(uint32 nDeviceFlags);

    // Free the list returned from GetDeviceObjects()
    void (*FreeDeviceObjects)(DeviceObject *pList);

    // Gets the name of the first device of the given type
    bool (*GetDeviceName)(uint32 nDeviceType, char *pStrBuffer, uint32 nBufferSize);

    // Gets the name of the device object given objectid.
    bool (*GetDeviceObjectName)( char const* pszDeviceName, uint32 nDeviceObjectId, char* pszDeviceObjectName, uint32 nDeviceObjectNameLen );

    // Finds out if specified device is enabled
    bool (*IsDeviceEnabled)(const char *pDeviceName);

    // displays the available objects for the specified input device
    bool (*ShowDeviceObjects)(const char *sDeviceName);

    // displays the available objects for the specified input device
    bool (*ShowInputDevices)();

};

// Get an input manager.
LTRESULT input_GetManager(InputMgr **pMgr);

// Saves the state of all the input bindings.
void input_SaveBindings(FILE *fp);

#endif  // __INPUT_H__




