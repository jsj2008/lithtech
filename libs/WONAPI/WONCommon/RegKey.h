#ifndef __WON_REGKEY_H__
#define __WON_REGKEY_H__
#include "WONShared.h"
#include "ByteBuffer.h"

#ifdef WIN32_NOT_XBOX
#include "RegKey_Windows.h"
#else

namespace WONAPI
{

enum RegKeyRoot
{
	REGKEY_CLASSES_ROOT,          
	REGKEY_CURRENT_USER,         
	REGKEY_LOCAL_MACHINE,         
	REGKEY_USERS,                 
	REGKEY_PERFORMANCE_DATA, 
	REGKEY_CURRENT_CONFIG,     
	REGKEY_DYN_DATA       
};

class RegKey
{
public:
	// Types
	enum GetResult { Ok, NotFound, BadType, NoMore, NoSize };
	enum DataType { Long, String, Binary, Other };

	// Constructors/Destructor
	RegKey(RegKeyRoot theRoot, const std::string& theKey, bool createKey = false, bool nonVolatile = true) {}
	RegKey() {}
	virtual ~RegKey() {}

	bool IsOpen() const { return false; }
	bool OpenNewKey(RegKeyRoot theRoot, const std::string& theKey, bool createKey = false, bool nonVolatile = true) { return false; }
	bool OpenSubKey(const std::string& theSubKey, bool createSubKey = false, bool nonVolatile = true) { return false; }
	bool GetSubKey(const std::string& theSubKeyName, RegKey& theSubKey, bool createSubKey = false, bool nonVolatile = true) const { return false; }

	GetResult GetFirstSubKey(RegKey& theSubKey) const { return NotFound; }
	GetResult GetNextSubKey(RegKey& theSubKey) const { return NotFound; }

	GetResult GetFirstValueName(std::string& theName, DataType& theType) const { return NotFound; }
	GetResult GetNextValueName(std::string& theName, DataType& theType) const { return NotFound; }

	GetResult GetValue(const std::string& theName, std::string& theValR) const { return NotFound; }
	GetResult GetValue(const std::string& theName, unsigned long& theValR) const { return NotFound; }
	GetResult GetValue(const std::string& theName, ByteBufferPtr &theValR) const { return NotFound; }

	bool SetValue(const std::string& theName, const std::string& theValue) { return false; }
	bool SetValue(const std::string& theName, unsigned long theValue) { return false; }
	bool SetValue(const std::string& theName, const void* theValueP, unsigned long theLength) { return false; }

	bool DeleteValue(const std::string& theName) { return false; }

	virtual bool DeleteSubKey(const std::string& theSubKey) const { return false; }
	virtual bool DeleteSubKey(RegKey& theSubKey) const { return false; }

	const std::string& GetLeafName(void) const { return mLeafName; }

	void CloseKey(void) {}

private:
	std::string mLeafName;  // Name of key off of root

	// Disallow these methods
	RegKey(const RegKey& theKeyR);
	RegKey& operator=(const RegKey& theKeyR);
};

}; // namespace WONAPI
#endif

#endif // _REGKEY_H
