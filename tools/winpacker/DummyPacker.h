#ifndef __DUMMYPACKER_H__
#define __DUMMYPACKER_H__

#ifndef __IPACKERIMPL_H__
#	include "IPackerImpl.h"
#endif

class CDummyPacker : public IPackerImpl
{
public:

	//---------------------
	// General Information

	//this is called after the packer has been selected, and is used to get a unique
	//name for this packer. This name is used in the user interface and also for
	//saving settings. It should be human readable. The name must be copied into the
	//passed in buffer, and must not exceed buffer size (including ending \0)
	virtual bool	GetPackerName(char *pszBuffer, int nBufferSize);


	// Called by the application when it needs to retrieve a list
	// of options that the user can set
	virtual bool	RequestUserOptions(IPackerUI* pUI);

	// Called when a property is changed by the user. This is where
	// different options can be validated/invalidated, etc. Note: This function
	// should not add any extra options to through the interface, merely enable
	// or disable
	virtual bool	PropertyChanged(CPackerProperty* pProperty, 
									CPackerPropList* pPropList, IPackerUI* pUI);

	//---------------------
	// Execution

	// Called when the user has chosen to begin processing the level. The output
	// object is passed in so that all messages can be directed to it as they
	// arise. In addition, the property list is passed for the preprocessor
	// to retrieve its settings from
	virtual bool	Process(const char* pszFilename, CPackerPropList* pPropList, 
							IPackerOutput* pOutput);

private:

};

#endif
