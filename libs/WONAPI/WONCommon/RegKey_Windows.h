#ifndef __WON_REGKEY_WINDOWS_H__
#define __WON_REGKEY_WINDOWS_H__
#include "WONShared.h"

// RegKey

// Encapsulates access to a Registry Key.  Key must already exist.  Allows
// string and long values to created, read, updated, or deleted.

#include <string>
#include "ByteBuffer.h"
#include "Platform_Windows.h"

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
	RegKey(RegKeyRoot theRoot, const std::string& theKey, bool createKey = false, bool nonVolatile = true);
	RegKey();
	virtual ~RegKey();

	// Is Key open
	bool IsOpen() const { return mOpen; }

	// Open a new key (closes current key)
	bool OpenNewKey(RegKeyRoot theRoot, const std::string& theKey, bool createKey = false, bool nonVolatile = true);

	// Open a child key from the parent key, close the parent key (transforms key)
	//   Parent not closed if opening subkey fails
	bool OpenSubKey(const std::string& theSubKey, bool createSubKey = false, bool nonVolatile = true);

	// Get, and open, a child key from the parent
	bool GetSubKey(const std::string& theSubKeyName, RegKey& theSubKey, bool createSubKey = false, bool nonVolatile = true) const;

	// Enumerate through subkeys,  GetFirst primes the function and fills in the first subkey, if any
	GetResult GetFirstSubKey(RegKey& theSubKey) const;
	GetResult GetNextSubKey(RegKey& theSubKey) const;

	// Enumerate through value names,  GetFirst primes the function and fills in the first value name, if any
	GetResult GetFirstValueName(std::string& theName, DataType& theType) const;
	GetResult GetNextValueName(std::string& theName, DataType& theType) const;

	// Fetch values
	GetResult GetValue(const std::string& theName, std::string& theValR) const;
	GetResult GetValue(const std::string& theName, unsigned long& theValR) const;
	GetResult GetValue(const std::string& theName, ByteBufferPtr &theValR) const;

	// Set/Create values
	bool SetValue(const std::string& theName, const std::string& theValue);
	bool SetValue(const std::string& theName, unsigned long theValue);
	bool SetValue(const std::string& theName, const void* theValueP, unsigned long theLength);

	// Delete value
	bool DeleteValue(const std::string& theName);

	// Delete a subkey off this parent
	//   Note:  If subkey has subkeys, deletion will fail
	virtual bool DeleteSubKey(const std::string& theSubKey) const;
	virtual bool DeleteSubKey(RegKey& theSubKey) const;

	// Get name of leaf off of root
	const std::string& GetLeafName(void) const { return mLeafName; }

	// Close the current key if open
	void CloseKey(void);

protected:
	HKEY mKey;   // Open registry key
	bool mOpen;  // Is the key open?

	std::string mLeafName;  // Name of key off of root

	mutable unsigned char* mBufP;       // Buffer used to get values
	mutable unsigned long  mBufLen;     // Length of buffer
	mutable DWORD          mKeyIndex;   // Index for SubKey enumeration
	mutable DWORD          mValueIndex; // Index for Value enumeration

private:
	// Private Methods
	DWORD GetToBuf(const std::string& theName, unsigned long &theLengthR) const;
	HKEY GetRootHKey(RegKeyRoot theRoot);
	bool OpenNewKey(HKEY theRoot, const std::string& theKey, bool createKey = false, bool nonVolatile = true);

	// Disallow these methods
	RegKey(const RegKey& theKeyR);
	RegKey& operator=(const RegKey& theKeyR);
};


};  //namespace WON

#endif
