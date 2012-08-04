#ifndef __WON_DIRTYPES_H__
#define __WON_DIRTYPES_H__
#include "WONShared.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct DirDataObject
{
	std::string mDataType;
	ByteBufferPtr mData;

	DirDataObject() { }
	DirDataObject(const std::string &theDataType, const ByteBuffer* theData) : 
		mDataType(theDataType), mData(theData) {}
};

typedef std::list<DirDataObject> DirDataObjectList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::set<std::string> DirDataTypeSet;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct DirACL
{
	char mType; // 'o' = owner, 'r' = read, 'w' = write, 'd' = delete
	unsigned long mUser;
	unsigned long mCommunity;
	unsigned short mTrust;

	DirACL() : mType(0), mUser(0), mCommunity(0), mTrust(0) {}

	DirACL(char theType, unsigned long theUser, unsigned long theCommunity, unsigned short theTrust) :
		mType(theType), mUser(theUser), mCommunity(theCommunity), mTrust(theTrust) {}
};

typedef std::list<DirACL> DirACLList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum DirGetFlags
{
	// Bits 0-15 are for decomposition and common flags

	// Decomposition Flags - apply these decompositions for Directories
	DIR_GF_DECOMPROOT      = 0x00000001,  // Add the dir itself 
	DIR_GF_DECOMPSERVICES  = 0x00000002,  // Add dir services
	DIR_GF_DECOMPSUBDIRS   = 0x00000004,  // Add dir subdirs
	DIR_GF_DECOMPRECURSIVE = 0x00000008,  // Recursive into dir subdirs

	// Common flags - include these attributes for all entities
	DIR_GF_ADDTYPE         = 0x00000010,  // Add entity types
	DIR_GF_ADDDISPLAYNAME  = 0x00000020,  // Add display names
	DIR_GF_ADDCREATED      = 0x00000040,  // Add creation date/time
	DIR_GF_ADDTOUCHED      = 0x00000080,  // Add touched date/time
	DIR_GF_ADDLIFESPAN     = 0x00000100,  // Add lifespan
	DIR_GF_ADDDOTYPE       = 0x00000200,  // Add DataObject types
	DIR_GF_ADDDODATA       = 0x00000400,  // Add DataObject data
	DIR_GF_ADDDATAOBJECTS  = 0x00000800,  // Add all DataObjects
	DIR_GF_ADDACLS         = 0x00001000,  // Add ACLs
	DIR_GF_ADDCRC          = 0x00002000,  // Add entity CRC
	DIR_GF_ADDUIDS         = 0x00004000,  // Add create and touch user ids
	DIR_GF_ADDORIGIP       = 0x00008000,  // Add originating IP address (admins only)

	// Bits 16-23 are for Directory only fields

	// Directory Flags - include these attributes for directories
	DIR_GF_DIRADDPATH      = 0x00010000,  // Add dir paths (from root)
	DIR_GF_DIRADDNAME      = 0x00020000,  // Add service names
	DIR_GF_DIRADDREQUNIQUE = 0x00040000,  // Add directory unqiue display name flag

	// Bits 24-31 are for Service only fields

	// Service Flags - include these attributes for services
	DIR_GF_SERVADDPATH     = 0x01000000,  // Add dir paths (from root)
	DIR_GF_SERVADDNAME     = 0x02000000,  // Add service names
	DIR_GF_SERVADDNETADDR  = 0x04000000,  // Add service net addresses
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Entity Flags (byte) - Control attributes in add/change requests
enum DirUpdateFlags
{
	DIR_UF_UNIQUEDISPLAYNAME = 0x01,  // Display name must be unique
	DIR_UF_DIRNOTUNIQUE      = 0x02,  // Directory allows duplicate display names
	DIR_UF_DIRREQUNIQUE      = 0x04,  // Directory requires unique display names
	DIR_UF_OVERWRITE         = 0x08,  // Overwrite existing entities

	// These 2 flags SHARE the 0x10 value (by design)
	DIR_UF_SERVRETURNADDR    = 0x10,  // Return service net address in reply
	DIR_UF_DIRNOACLINHERIT   = 0x10,  // Do not inherit parent ACLs for AddDirs

	DIR_UF_SERVGENNETADDR    = 0x20,  // Generate NetAddr from connection (AddService(Ex) only)
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// DataObjectSetMode (byte) - Mode for setting data objects on a DirEntity
enum DirDataObjectSetMode
{
	DIR_DO_ADDREPLACE    = 0,  // Add on not exist, replace on exist
	DIR_DO_ADDIGNORE     = 1,  // Add on not exist, ignore on exist
	DIR_DO_ADDONLY       = 2,  // Add on not exist, error on exist
	DIR_DO_REPLACEIGNORE = 3,  // Replace on exist, ignore on not exist
	DIR_DO_REPLACEONLY   = 4,  // Replace on exist, error on not exist
	DIR_DO_RESETDELETE   = 5,  // Clear existing set first, then add all.
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// FindMatchMode (byte) - Mode for find queries
enum DirFindMatchMode
{
	DIR_FMM_EXACT   = 0,  // Compared value must equal search value
	DIR_FMM_BEGIN   = 1,  // Compared value must begin with search value
	DIR_FMM_END     = 2,  // Compared value must end with search value
	DIR_FMM_CONTAIN = 3   // Compared value must contain search value
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// FindFlags (byte) - Control flags for find queries
enum DirFindFlags
{
	DIR_FF_MATCHALL  = 0x01,  // Return all valid matches
	DIR_FF_FULLKEY   = 0x02,  // Match only if all search field match
	DIR_FF_RECURSIVE = 0x04   // Search directories recursively for matches
};


};  // namespace WONAPI


#endif
