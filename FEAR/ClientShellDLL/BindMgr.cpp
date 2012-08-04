// ----------------------------------------------------------------------- //
//
// MODULE  : BindMgr.cpp
//
// PURPOSE : Manages bindings between input devices and commands
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "BindMgr.h"
#include "iltinput.h"
#include "GameClientShell.h" // For OnCommandOn/Off

static ILTInput *g_pLTInput;
define_holder(ILTInput, g_pLTInput);

CBindMgr::CBindMgr() :
	m_bInputEnabled(true),
	m_bCallbacksEnabled(true)
{
}

CBindMgr &CBindMgr::GetSingleton()
{ 
	static CBindMgr cMgr; 
	return cMgr; 
}

void CBindMgr::SetBinding(const SBinding &cBinding, bool bOverwrite)
{
	if (bOverwrite)
		ClearDeviceBindings(cBinding);
	m_aBindings.push_back(cBinding);

	if (m_aCommandStates.size() <= cBinding.m_nCommand)
	{
		m_aCommandStates.resize(cBinding.m_nCommand + 1, false);
		m_aLastCommandStates.resize(m_aCommandStates.size(), false);
		m_aClearCommandStates.resize(m_aCommandStates.size(), false);
	}
}

bool CBindMgr::GetDeviceBinding(uint32 nDevice, uint32 nObject, uint32 nIndex, SBinding *pBinding) const
{
	uint32 nCurMatch = 0;
	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if ((iCurBinding->m_nDevice == nDevice) &&
			(iCurBinding->m_nObject == nObject))
		{
			if (nCurMatch == nIndex)
			{
				*pBinding = *iCurBinding;
				return true;
			}
			++nCurMatch;
		}
	}

	return false;
}

void CBindMgr::GetCommandBindings(uint32 nCommand, TBindingList *pBindings) const
{
	pBindings->resize(0);

	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if (iCurBinding->m_nCommand == nCommand)
		{
			pBindings->push_back(*iCurBinding);
		}
	}
}

void CBindMgr::ClearDeviceBindings(uint32 nDevice, uint32 nObject)
{
	for (TBindingList::iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if ((iCurBinding->m_nDevice == nDevice) &&
			(iCurBinding->m_nObject == nObject))
		{
			m_aBindings.erase(iCurBinding);
			break;
		}
	}
}

void CBindMgr::ClearDeviceBindings(const SBinding &cBinding)
{
	for (TBindingList::iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if ((iCurBinding->m_nDevice == cBinding.m_nDevice) &&
			(iCurBinding->m_nObject == cBinding.m_nObject) &&
			(iCurBinding->m_fCommandMax >= cBinding.m_fCommandMin) &&
			(iCurBinding->m_fCommandMin <= cBinding.m_fCommandMax))
		{
			m_aBindings.erase(iCurBinding);
			break;
		}
	}
}

void CBindMgr::ClearCommandBindings(uint32 nCommand)
{
	for (TBindingList::iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		)
	{
		if (iCurBinding->m_nCommand == nCommand)
		{
			m_aBindings.erase(iCurBinding);
		}
		else
		{
			++iCurBinding;
		}
	}
}

void CBindMgr::ClearBindings()
{
	m_aBindings.resize(0);
}

void CBindMgr::GetCommandValues(uint32 nCommand, TfloatList *pValues) const
{
	pValues->resize(0);

	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if (iCurBinding->m_nCommand == nCommand)
			pValues->push_back(GetBindingValue(*iCurBinding));
	}
}

float CBindMgr::GetMaxCommandValue(uint32 nCommand) const
{
	float fResult = -FLT_MAX;

	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if (iCurBinding->m_nCommand == nCommand)
		{
			fResult = LTMAX(fResult, GetBindingValue(*iCurBinding));
		}
	}

	return fResult;
}

float CBindMgr::GetMinCommandValue(uint32 nCommand) const
{
	float fResult = FLT_MAX;

	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if (iCurBinding->m_nCommand == nCommand)
		{
			fResult = LTMIN(fResult, GetBindingValue(*iCurBinding));
		}
	}

	return fResult;
}

float CBindMgr::GetExtremalCommandValue(uint32 nCommand) const
{
	float fResult = 0.0f;

	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end();
		++iCurBinding)
	{
		if (iCurBinding->m_nCommand == nCommand)
		{
			float fCommandValue = GetBindingValue(*iCurBinding);
			if (fabsf(fCommandValue) > fabsf(fResult))
				fResult = fCommandValue;
		}
	}

	return fResult;
}

float CBindMgr::GetBindingValue(const SBinding &sBinding, bool bReturnDefaultOnDisabled /* = true */) const
{
	if (!m_bInputEnabled && bReturnDefaultOnDisabled)
		return sBinding.m_fDefaultValue;

	bool bReady;
	if (g_pLTInput->IsDeviceReady(sBinding.m_nDevice, &bReady) != LT_OK)
		return sBinding.m_fDefaultValue;

	if (!bReady)
		return sBinding.m_fDefaultValue;

	float fRawValue;
	if (g_pLTInput->GetDeviceObjectValue(sBinding.m_nDevice, sBinding.m_nObject, &fRawValue) != LT_OK)
		return sBinding.m_fDefaultValue;

	float fResult = (fRawValue * sBinding.m_fScale) + sBinding.m_fOffset;

	// Apply the dead zone, if it has one
	if (sBinding.m_fDeadZoneMax > sBinding.m_fDeadZoneMin)
	{
		if ((fResult > sBinding.m_fDeadZoneMin) && (fResult < sBinding.m_fDeadZoneMax))
		{
			fResult = sBinding.m_fDeadZoneValue;
		}
		else if (fResult > sBinding.m_fDeadZoneMax)
		{
			fResult -= (sBinding.m_fDeadZoneMax - sBinding.m_fDeadZoneValue);
		}
		else
		{
			fResult += (sBinding.m_fDeadZoneValue - sBinding.m_fDeadZoneMin);
		}

	}

	return fResult;
}

void CBindMgr::Update()
{
	if( !m_bInputEnabled || !m_bCallbacksEnabled )
		return;

	m_aLastCommandStates.swap(m_aCommandStates);

	// Clear the commands
	m_aCommandStates.assign(m_aCommandStates.size(), false);

	// Set the commands to "on" for any bindings whose value is above the threshold
	for (TBindingList::const_iterator iCurBinding = m_aBindings.begin();
		iCurBinding != m_aBindings.end(); 
		++iCurBinding)
	{
		// Get the current value.
		// If we are disabled, we still need the true binding value so that when we become
		// enabled we will know that the command had been on and didn't just turn on.
		float fCurValue = GetBindingValue(*iCurBinding, false);
		// The command is "on" if the binding is between the minimum and maximum values
		bool bOn = ((fCurValue >= iCurBinding->m_fCommandMin) && (fCurValue <= iCurBinding->m_fCommandMax));
		// Or this with the current state to allow multiple bindings to affect the same command
		m_aCommandStates[iCurBinding->m_nCommand] = m_aCommandStates[iCurBinding->m_nCommand] || bOn; 
	}

	// Clear any commands which are marked to be cleared.  This must be done
	// after looping over all bindings so that a command bound by 2 keys is
	// understood to be on if only the second binding is on.
	{	
		TCommandStateList::iterator iCurClear = m_aClearCommandStates.begin();
		TCommandStateList::iterator iCurCommand = m_aCommandStates.begin(); 
		for (; iCurCommand != m_aCommandStates.end(); ++iCurClear, ++iCurCommand)
		{
			// See if this command is marked for being cleared
			if (*iCurClear)
			{
				// If it's still on, turn it off
				if (*iCurCommand)
				{
					*iCurCommand = false;
				}
				else
				{
					// Otherwise remember it doesn't need to be cleared any more
					*iCurClear = false;
				}
			}
		}
	}

	if (m_bInputEnabled && m_bCallbacksEnabled)
	{
		// Send off events for anything that's changed.
		for (TCommandStateList::const_iterator iCurCommand = m_aCommandStates.begin(), iLastCommand = m_aLastCommandStates.begin();
			iCurCommand != m_aCommandStates.end();
			++iCurCommand, ++iLastCommand)
		{
			if (*iCurCommand == *iLastCommand)
				continue;

			uint32 nCommand = iCurCommand - m_aCommandStates.begin();

			if (*iCurCommand)
			{
				g_pGameClientShell->OnCommandOn(nCommand);
			}
			else
			{
				g_pGameClientShell->OnCommandOff(nCommand);
			}
		}
	}
}

void CBindMgr::ClearAllCommands()
{
	// Flag all the commands as off.  
	// As the bindmgr is updated, these flags will be reset to false as soon as the
	// command truly is not on using the current bindings.
	std::fill(m_aClearCommandStates.begin(), m_aClearCommandStates.end(), true);

	if (m_bInputEnabled && m_bCallbacksEnabled)
	{
		g_pGameClientShell->OnClearAllCommands();
	}
}
