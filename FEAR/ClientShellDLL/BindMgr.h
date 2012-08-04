// ----------------------------------------------------------------------- //
//
// MODULE  : BindMgr.h
//
// PURPOSE : Manages bindings between input devices and commands
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BINDMGR_H__
#define __BINDMGR_H__

#include "iltinput.h"
#include <float.h>

class CBindMgr
{
public:
	// Singleton access
	static CBindMgr &GetSingleton();

	// Must be called once per frame to update command status
	// Note : This will call into OnCommandOn/Off handlers if appropriate
	void Update();

	// Constant representing an invalid command
	enum { k_nInvalidCommand = -1 };

	// Defines the data for a binding
	struct SBinding
	{
		SBinding() : 
			m_nDevice(ILTInput::k_InvalidIndex), 
			m_nObject(ILTInput::k_InvalidIndex), 
			m_nCommand(CBindMgr::k_nInvalidCommand),
			m_fDefaultValue(0.0f),
			m_fOffset(0.0f),
			m_fScale(1.0f),
			m_fDeadZoneMin(FLT_MAX),
			m_fDeadZoneMax(-FLT_MAX),
			m_fDeadZoneValue(0.0f),
			m_fCommandMin(0.1f),
			m_fCommandMax(FLT_MAX)
		{}
		SBinding(const SBinding &cOther) { memcpy(this, &cOther, sizeof(*this)); }
		SBinding operator=(const SBinding &cOther) { return *(new(this) SBinding(cOther)); }

		// The device, device object, and command for this binding
		uint32 m_nDevice, m_nObject, m_nCommand;
		// Default value, returned when the device is not ready, or something similar;
		float m_fDefaultValue;
		// Value adjustment, applied as (value = (value * m_fScale) + m_fOffset)
		float m_fOffset, m_fScale;
		// Dead-zone handling, applied after value adjustment
		// Note : Dead zone will be subtracted from the final range of the value,
		// biased toward the dead zone value
		float m_fDeadZoneMin, m_fDeadZoneMax, m_fDeadZoneValue;
		// Range (inclusive) of values inside of which the binding should be considered "on" in terms of a command
		float m_fCommandMin, m_fCommandMax;
	};

	typedef std::vector<SBinding, LTAllocator<SBinding, LT_MEM_TYPE_CLIENTSHELL> > TBindingList;

	// Set a binding.  Previous bindings for the device and object will be overridden (optionally).
	void SetBinding(const SBinding &cBinding, bool bOverwrite=true);
	// Get the binding for a device/object pair
	// Returns false if no binding was found
	bool GetDeviceBinding(uint32 nDevice, uint32 nObject, uint32 nIndex, SBinding *pBinding) const;
	// Get the binding list for a command
	void GetCommandBindings(uint32 nCommand, TBindingList *pBindings) const;
	// Remove all bindings for a device/object pair
	void ClearDeviceBindings(uint32 nDevice, uint32 nObject);
	// Remove all bindings matching the device/object pair and overlapping the command range of the given binding
	void ClearDeviceBindings(const SBinding &sBinding);
	// Remove all bindings for a command
	void ClearCommandBindings(uint32 nCommand);
	// Remove all bindings
	void ClearBindings();

	// Retrieve the bound input values for a command
	void GetCommandValues(uint32 nCommand, TfloatList *pValues) const;
	// Retrieve the maximum input value for a command
	float GetMaxCommandValue(uint32 nCommand) const;
	// Retrieve the minimum input value for a command
	float GetMinCommandValue(uint32 nCommand) const;
	// Retrieve the extremal input value for a command (input value farthest away from 0, useful for axis input)
	float GetExtremalCommandValue(uint32 nCommand) const;
	// Button-style command access
	bool IsCommandOn(uint32 nCommand) const { return m_bInputEnabled && ((nCommand < m_aCommandStates.size()) ? (m_aCommandStates[nCommand] && !m_aClearCommandStates[nCommand]) : false); }

	// Clear all input.  This will turn all command results to "off" until they go off and back on.
	void ClearAllCommands();

	// Enable/disable input processing
	// Note : This is not reference counted.  Multiple calls to turn them off will be counteracted by a single call to turn them on.
	void SetEnabled(bool bOn) { m_bInputEnabled = bOn; }
	// Enable/disable callback processing
	void SetCallbacks(bool bOn) { m_bCallbacksEnabled = bOn; }
private:
	CBindMgr();

	float GetBindingValue(const SBinding &sBinding, bool bReturnDefaultOnDisabled = true ) const;

	TBindingList m_aBindings;

	// KEF - 02/22/05 - Temporary patch for working around a bug in the compiler for the 1242 XDK
#if defined(PLATFORM_XENON)
	#define COMMAND_STATE_TYPE char
#else // !PLATFORM_XENON
	#define COMMAND_STATE_TYPE bool
#endif // !PLATFORM_XENON

	typedef std::vector<COMMAND_STATE_TYPE, LTAllocator<COMMAND_STATE_TYPE, LT_MEM_TYPE_CLIENTSHELL> > TCommandStateList;
	TCommandStateList m_aCommandStates, m_aLastCommandStates, m_aClearCommandStates;

	bool m_bInputEnabled, m_bCallbacksEnabled;
};


#endif//__BINDMGR_H__

