#ifndef __ILTDIAGNOSTICINSTANCEINFO_H__
#define __ILTDIAGNOSTICINSTANCEINFO_H__


//------------------------------------------------------------
//Base class for instance info
//------------------------------------------------------------

// (specific implementations of this for specific systems
// will determine what data goes inside and gets output by these methods)
/*!


*/
class ILTDiagnosticInstanceInfo
{
public:
	typedef uint8 InfoLevel;
	enum { FULL_INFO = 0xFF };

	// append the info string to pString (up to stringSize characters)
	// return the number of characters added
	/*!
	\param level
	\param pString
	\param stringSize
	\return 

	\see 

	Used for: Diagnostics.
	*/
	virtual size_t AddInfoString(const InfoLevel& level,
								 char* pString,
								 size_t stringSize) const = 0;

	/* E.g. implementation:
	strncpy(pString, "My info string for level", stringSize);
	return strlen("My info string for level");
	*/
};

#endif
