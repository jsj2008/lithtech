#ifndef __PACKERPROPERTY_H__
#define __PACKERPROPERTY_H__

//for the integer types
#include "ltinteger.h"

#include <stdlib.h>		//for NULL

//different property types
#define PROPERTY_BOOL				0
#define PROPERTY_STRING				1
#define PROPERTY_ENUM				2
#define PROPERTY_REAL				3
#define PROPERTY_INTERFACE			4


//------------------------------------------------------------
// Base Property
class CPackerProperty
{
public:

	CPackerProperty(const char* pszName = NULL, const char* pszHelp = NULL);

	virtual ~CPackerProperty();

	//sets the name of this property
	bool				SetName(const char* pszHelp);

	//gets the name of this property
	const char*			GetName() const;

	//sets the help string associated with this property
	bool				SetHelp(const char* pszHelp);

	//gets the help string associated with this property
	const char*			GetHelp() const;

	//gets the type of property
	virtual uint32		GetType() const = 0;

	//determines if this property is enabled or not
	bool				IsEnabled() const;

	//enables/disables this property
	void				SetEnabled(bool bEnable);

	//used to build up a string in order to save. It should
	//fill out the buffer with the string equivialant of its values
	//Note that the length should be adhered to
	virtual bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const = 0;

	//used to load the parameters. It is passed in a list of strings that
	//are space delimited and from this the property must load its information
	virtual bool		LoadValue(char** ppszBuffers, uint32 nNumBuffers) = 0;

private:

	//the name of this item
	char*				m_pszName;

	//the help text
	char*				m_pszHelp;

	//if this is enabled or not
	bool				m_bEnabled;
};

//------------------------------------------------------------
// Interface Property
// The interface property simply allows for better control over
// the look and feel in graphical UI's, and provides a means
// for breaking apart groups, and other things
class CPackerInterfaceProperty :
	public CPackerProperty
{
public:

	//different types
	enum	{	BLANK,			// just a blank area
				TEXT_LEFT,		// left justified text
				TEXT_CENTER,	// centered text
				TEXT_RIGHT,		// right aligned text
				SEPARATOR		// a separator line
			};

	//constructors
	CPackerInterfaceProperty();
	CPackerInterfaceProperty(const char* pszName, uint32 nType, const char* pszHelp = NULL);
	CPackerInterfaceProperty(const CPackerInterfaceProperty& rhs);

	CPackerInterfaceProperty& operator=(const CPackerInterfaceProperty& rhs);\

	uint32		GetType() const;
	bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const;
	bool		LoadValue(char** ppszBuffer, uint32 nNumBuffers);


	//type information
	uint32		GetInterfaceType() const;
	void		SetInterfaceType(uint32 nType);

private:

	uint32		m_nInterfaceType;
};


//------------------------------------------------------------
// Bool Property
class CPackerBoolProperty : public CPackerProperty
{
public:

	CPackerBoolProperty();
	CPackerBoolProperty(const char* pszName, bool bVal, const char* pszHelp = NULL);
	CPackerBoolProperty(const CPackerBoolProperty& rhs);

	CPackerBoolProperty& operator=(const CPackerBoolProperty& rhs);

	uint32		GetType() const;
	bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const;
	bool		LoadValue(char** ppszBuffer, uint32 nNumBuffers);

	bool		GetValue() const;
	void		SetValue(bool bVal);

private:

	bool		m_bVal;
};

//------------------------------------------------------------
// String Property
class CPackerStringProperty : public CPackerProperty
{
public:

	CPackerStringProperty();
	CPackerStringProperty(	const char* pszName, const char* pszValue, 
							const char* pszHelp = NULL);

	CPackerStringProperty(	const char* pszName, const char* pszValue, bool bLoad, 
							bool bStripFilename, const char* pszFileFilter, 
							const char* pszHelp = NULL);

	CPackerStringProperty(const CPackerStringProperty& rhs);

	~CPackerStringProperty();


	CPackerStringProperty& operator=(const CPackerStringProperty& rhs);

	uint32		GetType() const;
	bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const;
	bool		LoadValue(char** ppszBuffer, uint32 nNumBuffers);

	const char*	GetValue() const;
	void		SetValue(const char* pszVal);

	//file information
	const char*	GetFileFilter() const;
	bool		IsFilename() const;
	bool		ShouldStripFilename() const;

	//returns if this file is one that is intended to be loaded. true if it is,
	//false if it is meant to be saved
	bool		IsFileLoad() const;
	bool		SetAsFilename(bool bFileLoad, bool bStripFilename, const char* pszFilter);
	bool		ClearAsFilename();

private:

	//file information
	bool		m_bFilename;

	//specifies if the file is a load or a save (true for load)
	bool		m_bFileLoad;
	bool		m_bStripFilename;
	char*		m_pszFilter;

	//the actual string value
	char*		m_pszVal;
};

//------------------------------------------------------------
// Enum Property
class CPackerEnumProperty : public CPackerProperty
{
public:

	//maximum length an enumertion item may be in characters
	enum	{	MAX_ITEM_LEN	=	256	};

	CPackerEnumProperty();
	CPackerEnumProperty(const char* pszName, const char* pszItems, uint32 nSel, 
						const char* pszHelp = NULL);
	CPackerEnumProperty(const CPackerEnumProperty& rhs);

	~CPackerEnumProperty();

	CPackerEnumProperty& operator=(const CPackerEnumProperty& rhs);

	uint32		GetType() const;
	bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const;
	bool		LoadValue(char** ppszBuffer, uint32 nNumBuffers);

	//sets up the items. Should be \n delimited
	bool		SetItems(const char* pszItems);

	//sets up the items based upon a given list
	bool		SetItems(char *const* pszList, uint32 nNumItems);

	//gets the number of items
	uint32		GetNumItems() const;

	//gets a specific item
	const char*	GetItem(uint32 nItem) const;

	//finds the ID of a string. Returns -1 if not found
	int32		FindItem(const char* pszVal) const;

	//get selected item
	uint32		GetSelection() const;

	//get the text of the selected item
	const char*	GetSelectionText() const;
	
	//set the selected item
	void		SetSelection(uint32 nItem);

private:

	void		Free();

	//the item information
	uint32		m_nNumItems;
	char**		m_ppszItems;

	uint32		m_nSelected;
};

//------------------------------------------------------------
// Real Property
class CPackerRealProperty : public CPackerProperty
{
public:

	CPackerRealProperty();
	CPackerRealProperty(const char* pszName, float fVal, bool bInteger, const char* pszHelp = NULL);
	CPackerRealProperty(const char* pszName, float fVal, bool bInteger, float fMin, float fMax, const char* pszHelp = NULL);
	CPackerRealProperty(const CPackerRealProperty& rhs);

	CPackerRealProperty& operator=(const CPackerRealProperty& rhs);

	uint32		GetType() const;
	bool		SaveValue(char* pszBuffer, uint32 nBufferLen) const;
	bool		LoadValue(char** ppszBuffer, uint32 nNumBuffers);

	//gets the rounded integer version
	int32		GetIntValue() const;

	//get the floating point version
	float		GetValue() const;
	void		SetValue(float fVal);

	//range information
	bool		IsRanged() const;
	void		SetRanged(float fMin, float fMax);
	void		ClearRanged();

	float		GetMin() const;
	float		GetMax() const;

	//integer information
	bool		IsInteger() const;
	void		SetInteger(bool bInteger);

private:

	//range data
	bool		m_bRanged;
	float		m_fMin;
	float		m_fMax;

	//specify if this is integer or not
	bool		m_bInteger;

	//the actual value
	float		m_fVal;
};

#endif
