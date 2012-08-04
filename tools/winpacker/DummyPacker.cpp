#include "stdafx.h" //for sleep
#include <stdio.h>
#include "DummyPacker.h"

#include "IPackerUI.h"
#include "PackerPropList.h"
#include "PackerProperty.h"
#include "IPackerOutput.h"


//---------------------
// General Information

//this is called after the packer has been selected, and is used to get a unique
//name for this packer. This name is used in the user interface and also for
//saving settings. It should be human readable. The name must be copied into the
//passed in buffer, and must not exceed buffer size (including ending \0)
bool CDummyPacker::GetPackerName(char *pszBuffer, int nBufferSize)
{
	strncpy(pszBuffer, "Dummy Packer", nBufferSize);
	return true;
}


// Called by the application when it needs to retrieve a list
// of options that the user can set
bool CDummyPacker::RequestUserOptions(IPackerUI* pUI)
{
	pUI->CreateProperty(CPackerStringProperty("TestStringName", "TestString Value", "TestString Help"), "Group1");
	pUI->CreateProperty(CPackerStringProperty("TestStringName1", "TestString Value", false, false, "All Files(*.*)|*.*||", "TestString Help"), "Group1");
	pUI->CreateProperty(CPackerEnumProperty("TestEnumName", "Item1\nItem2\nItem3", 1, "TestEnumHelp"), "Group1");
	pUI->CreateProperty(CPackerStringProperty("TestStringName2", "TestString Value", "TestString Help"), "Group1");
	pUI->CreateProperty(CPackerBoolProperty("TestBoolName", true, "TestBool Help"), "Group1");
	pUI->CreateProperty(CPackerRealProperty("TestRealName", 0, true, "TestReal Help"), "Group1");

	pUI->CreateReference("TestStringName", "Group2");

	return true;
}

// Called when a property is changed by the user. This is where
// different options can be validated/invalidated, etc. Note: This function
// should not add any extra options to through the interface, merely enable
// or disable, or change values. It should not change the form of any of the
// properties and this will be undefined. For example, switching a string to a
// filename will have unknown results. It is given the property that was 
// changed (note that this can be null) along with the list of properties 
// and the UI which the packer can modify the enabled status of properties through.
bool CDummyPacker::PropertyChanged(CPackerProperty* pProperty, CPackerPropList* pPropList, IPackerUI* pUI)
{
	pPropList->EnableProperty("TestBoolName", pPropList->GetEnum("TestEnumName", 0) == 0);

	CPackerStringProperty* pProp = (CPackerStringProperty*)pPropList->GetProperty("TestStringName1");

	if(pProp != pProperty)
	{
		pProp->SetValue(pPropList->GetString("TestStringName", "ERROR"));
	}

	return true;
}

//---------------------
// Execution

// Called when the user has chosen to begin processing the level. The output
// object is passed in so that all messages can be directed to it as they
// arise. In addition, the property list is passed for the preprocessor
// to retrieve its settings from
bool CDummyPacker::Process(const char* pszFilename, CPackerPropList* pPropList, 
						   IPackerOutput* pOutput)
{
	pOutput->CreateTask("Task 1");
	pOutput->CreateTask("Task 2");

	pOutput->ActivateTask("Task 1");

	Sleep(500);

	pOutput->LogMessage(MSG_CRITICAL, "This is the number four: 4");

	uint32 nCurr;
	for(nCurr = 0; nCurr < 100; nCurr++)
	{
		pOutput->UpdateProgress((float)nCurr);
		pOutput->LogMessage(static_cast<EMsgType>((rand() % MSG_DEBUG) + 1), "Task 1: This Progress");
		Sleep(50);
	}	

	pOutput->ActivateTask("Task 2");
	for(nCurr = 0; nCurr < 100; nCurr++)
	{
		pOutput->UpdateProgress((float)nCurr);
		pOutput->LogMessage(MSG_CRITICAL, "Task 2: This Progress");
		Sleep(50);
	}	

	return true;
}
