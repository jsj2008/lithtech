// OptionsAdvancedSelect.cpp: implementation of the COptionsAdvancedSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "optionsadvancedselect.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsAdvancedSelect::COptionsAdvancedSelect()
{
	m_bUseNameField=FALSE;		// True if the name field is to be used
	m_bMatchWholeName=FALSE;	// True if the whole name should be matched (false for partial string)
	m_sNameField="";			// The contents of the name field

	m_bUseClassField=FALSE;		// True if the class field should be used
	m_sClassField="";			// The class field

	m_sPropName="";				// The name of the property
	m_nPropType=0;				// The index of the property type in the combobox
	m_sPropValue="true/false";	// The value string for the property
	m_bNodesWithProperty=false;	// True if the property field should be used
	m_bMatchValue=false;		// True if we want to require a specific value for the property

	m_nSelectionOperation=0;	// The selection operation (0-select 1-deselect)

	m_bShowResults=FALSE;		// Show the results of the operation?
}

COptionsAdvancedSelect::~COptionsAdvancedSelect()
{

}

// Loads the advanced select options from the registry
BOOL COptionsAdvancedSelect::Load()
{
	m_bUseNameField=GetBoolValue("UseNameField", FALSE);
	m_bMatchWholeName=GetBoolValue("MatchWholeName", FALSE);
	m_sNameField=GetStringValue("NameField");

	m_bUseClassField=GetBoolValue("UseClassField", FALSE);
	m_sClassField=GetStringValue("ClassField");

	m_sPropName=GetStringValue("PropertyName");
	m_nPropType=GetDWordValue("PropertyType", 0);			
	m_sPropValue=GetStringValue("PropertyValue");		
	m_bNodesWithProperty=GetBoolValue("UsePropertyField", FALSE);	
	m_bMatchValue=GetBoolValue("MatchValue", FALSE);		

	m_nSelectionOperation=GetDWordValue("SelectionOperation", 0);
	m_bShowResults=GetBoolValue("ShowResults",FALSE);

	return TRUE;
}

// Saves the advanced select options to the registry
BOOL COptionsAdvancedSelect::Save()
{
	SetBoolValue("UseNameField", m_bUseNameField);
	SetBoolValue("MatchWholeName", m_bMatchWholeName);
	SetStringValue("NameField", m_sNameField);

	SetBoolValue("UseClassField", m_bUseClassField);
	SetStringValue("ClassField", m_sClassField);

	SetStringValue("PropertyName", m_sPropName);
	SetDWordValue("PropertyType", m_nPropType);			
	SetStringValue("PropertyValue", m_sPropValue);		
	SetBoolValue("UsePropertyField", m_bNodesWithProperty);	
	SetBoolValue("MatchValue", m_bMatchValue);

	SetDWordValue("SelectionOperation", m_nSelectionOperation);
	SetBoolValue("ShowResults", m_bShowResults);

	return TRUE;
}
