// LithTech Removable Storage Interface

#ifndef __ILTREMOVABLESTORAGEMGR_H__
#define __ILTREMOVABLESTORAGEMGR_H__

//lithtech module header
#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

// Platform independent interface
// Zero based identifier for removable drives
typedef int32		LTDRIVEID;

// Invalid drive id constant
#define LTDRIVE_INVALID_ID		(-1)
#define LTDRIVE_MAX_STREAMS		(1)
#define LTDRIVE_MAX_DRIVES		(1)
#define LTDRIVE_MAX_FILENAME	(256)

// Attribute flags for files
enum
{
    LTS_FILE_READABLE         = (1<<0),	// Is the file readable?
    LTS_FILE_WRITEABLE        = (1<<1),	// Is the file writable?
    LTS_FILE_EXECUTABLE       = (1<<2),	// Is the file executable?
    LTS_FILE_DIRECTORY        = (1<<3),	// Is this a directory?
    LTS_FILE_PROTECTED        = (1<<4),	// Is the file copy protected?
    LTS_FILE_DAMAGED          = (1<<5)	// Is the file damaged (eg. an incomplete write)?
};

typedef enum ELTStorageError
{
	LTS_ERROR_NONE			= 0,	// No error
	LTS_ERROR_GENERIC		= -1,	// Generic error, such as bad params, low level errors etc.
	LTS_ERROR_DRIVECHANGED	= -2,	// One or more drives have changed since the last Poll
	LTS_ERROR_DRIVEINVALID	= -3,	// The drive passed in was invalid
	LTS_ERROR_COULDNOTFORMAT= -4,	// Could not format the specified drive
	LTS_ERROR_NOTFORMATTED	= -5,	// The specified drive was not formatted
	LTS_ERROR_TOOMANYDIRS	= -6,	// Too many directories exist, unable to create another
	LTS_ERROR_TOOMANYFILES	= -7,	// Too many directories exist, unable to create another
	LTS_ERROR_FILESEXIST	= -8,	// Could not delete a directory as files exist in it
	LTS_ERROR_BADFILENAME	= -9,	// The filename specified was invalid
	LTS_ERROR_NOTAFILE		= -10,	// An attempt was made to open a directory
	LTS_ERROR_NOSUCHFILE	= -11,	// The specified file could not be opened
	LTS_ERROR_NOSUCHDIR		= -12,	// The specified directory did not exist
	LTS_ERROR_NOPERMISSIONS	= -13,	// No permissions to access this file!
	LTS_ERROR_NOSPACE		= -14,	// There is not enough space available to perform a write or create a file.
	LTS_ERROR_NOTOPENED		= -15,	// The underlying low-level file had not been opened
	LTS_ERROR_MAXSTREAMS	= -16	// Maximum open streams has been reached
} ELTStorageError;

typedef struct SLTTimestamp
{
	uint8			m_Pad;			// unused
	uint8			m_Secs;			// Seconds
	uint8			m_Mins;			// Minutes
	uint8			m_Hours;		// Hours
	uint8			m_Day;			// Day
	uint8			m_Month;		// Month
	uint16			m_Year;			// Year
} SLTTimestamp; // 8 bytes

typedef struct SLTStorageFileInfo
{
	SLTTimestamp	m_Create;		// Creation time
	SLTTimestamp	m_Modify;		// Modification time
	uint32			m_FileSize;		// Size of this file (0 for directory)
	uint32			m_Attributes;	// Attributes for this file
	char			m_FileName[LTDRIVE_MAX_FILENAME]; // Name of this file
} SLTStorageFileInfo;

//module interface class
class ILTRemovableStorageMgr : public IBase {
public:
    interface_version(ILTRemovableStorageMgr, 0);
    
	// Initialises the memory card subsystem
	// Note: This is performed by the engine and is not required in game code
	virtual LTRESULT	Init() = 0;
	// Checks for changes to the memory cards detected at startup
	// Note: 1) This should be called before all other functions (except Init),
	//       as the drives may have changed since the last Poll
	//       2) Returns LT_OK if no information has changed, LT_ERROR otherwise
	virtual LTRESULT	Poll() = 0;
	// Returns the last error that occurred
	virtual ELTStorageError	GetLastError() = 0;

	// Drive access methods
	// Returns the number of active, valid drives
	virtual uint32 		GetNumRemovableDrives() = 0;
	// Fills in the array passed in with the valid drive ids for the machine
	// Returns the number of drives which were enumerated
	virtual uint32		EnumRemovableDrives(uint32 maxDrives, LTDRIVEID* pDriveIds) = 0;
	// Returns the number of available bytes on this drive
	virtual uint32 		GetAvailableSpace(LTDRIVEID driveId) = 0;
	// Returns true if the drive has been formatted
	virtual bool 		IsFormatted(LTDRIVEID driveId) = 0;
	// Formats a drive - all files will be destroyed
	virtual LTRESULT	Format(LTDRIVEID driveId) = 0;

	// Directory access methods
	// Creates a directory on the drive
	virtual LTRESULT	MakeDir(LTDRIVEID driveId, const char *pDirName) = 0;
	// Deletes a directory from the drive, if bDeleteFiles is true then all the files in
	// the directory will be removed, otherwise if there are files in the directory then
	// they will NOT be removed and the directory will NOT be deleted
	virtual LTRESULT	DeleteDir(LTDRIVEID driveId, const char *pDirName, bool bDeleteFiles) = 0;
	// Changes the drive's current directory
	virtual LTRESULT	ChangeDir(LTDRIVEID driveId, const char *pDirName) = 0;
	// Gets the drive's current directory
	// Note: This may return NULL if the drive was invalid
	virtual const char*	GetDir(LTDRIVEID driveId) = 0;

	// File access methods
	// Fills in up to maxFiles elemenets of the array passed in with the files in the current directory
	// Returns the number of files found, if this is < 0, then an error occurred
	// Notes: If bFirstCall is specified, then the first files in the directory
	// will be retrieved, if the number of files retrieved is equal to maxFiles, then this
	// function can be called again with bFirstCall set to false. This will then retrieve the next
	// set of files in the current directory.
	virtual int32		GetFileList(LTDRIVEID driveId, const char *pFilter, bool bFirstCall, uint32 maxFiles, SLTStorageFileInfo *pFiles) = 0;
	// Opens the file specified by pFileName and returns an ILTStream which can then be
	// used to read or write to the drive.
	// If bCreate is specified, then the file will be created if it does not already exist.
	// Notes: 1) The stream must be freed using the ILTStream->Release() function.
	//        2) This function WILL NOT create a subdirectory if one is specified in
	//           the filename and the subdirectory does not exist! MakeDirectory must
	//           be called first.
	//        3) This will return NULL if a directory name is specified or if the
	//           user does not have permissions to access this file
	virtual ILTStream* 	OpenFile(LTDRIVEID driveId, const char *pFileName, bool bCreate) = 0;
	// Deletes the named file from drive
	virtual LTRESULT	DeleteFile(LTDRIVEID driveId, const char *pFileName) = 0;
};

#endif  // __ILTREMOVABLESTORAGEMGR_H__



