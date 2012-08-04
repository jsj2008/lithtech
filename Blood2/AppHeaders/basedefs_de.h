
#ifndef __BASEDEFS_H__
#define __BASEDEFS_H__


	#include <stdlib.h>
	#include <string.h>

	#ifdef __cplusplus
		#define BEGIN_EXTERNC() extern "C" \
			{
		#define END_EXTERNC() };
	#else
		#define BEGIN_EXTERNC()
		#define END_EXTERNC()
	#endif
	

	#if !defined(__cplusplus) || defined(DIRECTENGINE_COMPILE)
		#define COMPILE_WITH_C
	#endif	
	
	#ifdef COMPILE_WITH_C
		#define LPBASECLASS struct BaseClass_t * 
		#define	LPAGGREGATE struct Aggregate_t *
	#else
		class BaseClass;
		#define LPBASECLASS BaseClass*
		class Aggregate;
		#define LPAGGREGATE Aggregate*
	#endif


	// Useful...
	#define SAFE_STRCPY(dest, src) \
	{\
		if(sizeof(dest) > 4)\
		{\
			strncpy(dest, src, sizeof(dest)-1);\
			dest[sizeof(dest)-1] = 0; \
		}\
		else\
		{\
			strcpy(dest, src);\
		}\
	}


	// Used in GetFileList/FreeFileList.
	#define TYPE_FILE		1
	#define TYPE_DIRECTORY	2
	
	typedef struct FileEntry_t
	{
		int			m_Type;				// Directory or file?
		char		*m_pBaseFilename;	// pic1.pcx
		char		*m_pFullFilename;	// interface/bitmaps/pic1.pcx
		struct FileEntry_t	*m_pNext;
	} FileEntry;


	// Used in the counter functions.
	typedef struct DCounter_t
	{
		unsigned long	m_Data[4];
	} DCounter;

	

	#define DEFINE_HANDLE_TYPE(name) \
		typedef struct {int __nothing;} ##name##_t, *name;


	#define MAX_CS_FILENAME_LEN	100

	// HMESSAGE defines

	// Maximum number of bytes in a grouped of messages of a communications message.
	#define MAX_PACKET_LEN		1024

	// Maxumum number of bytes in a message of a communications message.
	#define MAX_MESSAGE_LEN		1024

	// Maximum number of bytes a single object can save into MID_SAVEOBJECT message.
	#define MAX_SAVEOBJECT_LEN	8192


	// Model state flags.
	#define MS_PLAYDONE		1	// The (nonlooping) model has finished playing the animation.


	// HDECOLOR stuff.
	typedef unsigned long HDECOLOR;
	#define COLOR_TRANSPARENCY_MASK	0x80000000

	// r, g, and b, are 0-255 integers.
	#define SETRGB(r,g,b) \
		(((unsigned long)(r) << 16) | ((unsigned long)(g) << 8) | ((unsigned long)(b)))
		
	// r, g, and b are 0-1 floats.
	#define SETRGB_F(r,g,b) \
		(((unsigned long)((r)*255.0f) << 16) | ((unsigned long)((g)*255.0f) << 8) | ((unsigned long)((b)*255.0f)))

	// Setup a color with transparency.
	#define SETRGB_T(r,g,b)		(SETRGB((r),(g),(b)) | COLOR_TRANSPARENCY_MASK)
	#define SETRGB_FT(r,g,b)	(SETRGB_F((r),(g),(b)) | COLOR_TRANSPARENCY_MASK)

	// Gets r, g, and b as 0-255 integers.
	#define GETR(val) (((val) >> 16) & 0xFF)
	#define GETG(val) (((val) >> 8) & 0xFF)
	#define GETB(val) ((val) & 0xFF)
	#define GETRGB(val, r, g, b) \
	{\
		(r) = GETR(val);\
		(g) = GETG(val);\
		(b) = GETB(val);\
	}

	// Gets translucency value (into a 0 or 1 integer).
	#define GETT(val) (!!((val) & COLOR_TRANSPARENCY_MASK))

		
	class LMessage;
	typedef LMessage*		HMESSAGEREAD;
	typedef LMessage*		HMESSAGEWRITE;



	// Handle definitions.. it defines them in this wierd way so 
	// you can't accidentally cast between them.
	DEFINE_HANDLE_TYPE(HCLASS)
	DEFINE_HANDLE_TYPE(HSURFACE)
	DEFINE_HANDLE_TYPE(HDEFONT)
	DEFINE_HANDLE_TYPE(HSTRING)
	DEFINE_HANDLE_TYPE(HSOUNDDE)
	DEFINE_HANDLE_TYPE(HCLIENT);
	DEFINE_HANDLE_TYPE(HATTACHMENT);
	DEFINE_HANDLE_TYPE(HNETSERVICE);
	DEFINE_HANDLE_TYPE(HCONSOLEVAR);


	typedef unsigned long HMODELNODE;
	#define INVALID_MODEL_NODE ((HMODELNODE)-1)

	// Object handle definition.
	class DObject;
	typedef DObject* HOBJECT;
	typedef DObject* HLOCALOBJ;


	
	
	// ---------------------------------------------------------------- //
	// Here is the layout of all message IDs.
	// ---------------------------------------------------------------- //
	
	// Server->Client (0-255):
	// DirectEngine reserves 0-15 and defines:
	#define STC_BPRINT	0	// Used for debugging.. print a message in the console.

	// Client->Server (0-255):
	// DirectEngine reserves 0-15.

	// Object->Object (0-65k)
	// 0-1000 reserved.
	// 1000-2000 shared general messages (damage, I see you, etc..)
	// 2000-2500 Riot
	// 3500-4000 Blood 2
	// 4000-4500 Dreadon


	// This is used all over to classify which side of a polygon something's on.
	typedef enum
	{
		BackSide=0,
		FrontSide=1,
		Intersect=2
	} PolySide;


	
	// Include really base stuff.
	#include "basetypes_de.h"
	#include "de_codes.h"


	
	#define MAX_FORCEUPDATE_OBJECTS	200
	typedef struct
	{
		HOBJECT	*m_Objects; // This array is MAX_FORCEUPDATE_OBJECTS large.
		DDWORD	m_nObjects;
	} ForceUpdate;



	// This structure is used when creating objects.  When you want to
	// create an object, you call ServerDE::CreateObject with one of these.
	// The structure you pass in is passed to the object's PostPropRead function,
	// where it can override whatever it wants to.
	typedef struct ObjectCreateStruct_t
	{
		// Main info.
		unsigned short	m_ObjectType;
		DDWORD			m_Flags;
		DVector			m_Pos;
		DVector			m_Scale;
		DRotation		m_Rotation;
		D_WORD			m_ContainerCode;	// Container code if it's a container.  It's in here because
											// you can only set it at creation time.

		DDWORD			m_UserData;			// User data


		char			m_Filename[MAX_CS_FILENAME_LEN+1];	// This is the model, sound, or sprite filename.
															// It also can be the WorldModel name.
															// This can be zero-length when it's not needed.
		
		char			m_SkinName[MAX_CS_FILENAME_LEN+1];	// This can be zero-length.. if you set it for an 
															// OT_MODEL, it's the skin filename.	  

		// Server only info.
		char			m_Name[MAX_CS_FILENAME_LEN+1];		// This object's name.
		float			m_NextUpdate;						// This will be the object's starting NextUpdate.
		float			m_fDeactivationTime;				// Amount of time before object deactivates self
	} ObjectCreateStruct;

	// Initialize an ObjectCreateStruct.
	#define INIT_OBJECTCREATESTRUCT(theStruct) \
	{ \
		memset(&theStruct, 0, sizeof(theStruct));\
		VEC_SET(theStruct.m_Scale, 1, 1, 1); \
		theStruct.m_Rotation.m_Spin = 1.0f; \
	}


	// Model hook.
	#define MHF_USETEXTURE		1	// Use the texture for this model (if any).
	typedef struct ModelHookData_t
	{
		HLOCALOBJ		m_hObject;
		unsigned long	m_Flags;		// Combination of MHF_ flags above.
		unsigned long	m_ObjectFlags;	// The model's object flags.  You can modify them
										// in here without changing the model's flags
										// permanently.
		DVector			*m_LightAdd;	// RGB 0-255
	} ModelHookData;


	// Render modes are what are used to describe a video mode/video card.
	typedef struct RMode_t
	{
		DBOOL			m_bHardware;

		char			m_RenderDLL[200];		// What DLL this comes from.		
		char			m_InternalName[100];	// This is what the DLLs use to identify a card.
		char			m_Description[100];		// This is a 'friendly' string describing the card.
		
		DDWORD			m_Width, m_Height, m_BitDepth;
		struct RMode_t	*m_pNext;
	} RMode;

	// The ID of the description string contained in render DLLs.
	#define RDLL_DESCRIPTION_STRINGID	5


	// Device input for device tracking
	#define MAX_INPUT_BUFFER_SIZE	32

	#define	DEVICETYPE_KEYBOARD		1
	#define DEVICETYPE_MOUSE		2
	#define DEVICETYPE_JOYSTICK		4
	#define DEVICETYPE_UNKNOWN		8
	
	#define CONTROLTYPE_UNKNOWN		0			// unknown type
	#define CONTROLTYPE_XAXIS		1			// horizontal axis, such as left-right motion of a mouse
	#define CONTROLTYPE_YAXIS		2			// vertical axis, such as forward-backward motion of a mouse
	#define CONTROLTYPE_ZAXIS		3			// z-axis, such as a wheel on a mouse or a throttle on a joystick
	#define CONTROLTYPE_RXAXIS		4			// rotation around the x-axis
	#define CONTROLTYPE_RYAXIS		5			// rotation around the y-axis
	#define CONTROLTYPE_RZAXIS		6			// rotation around the z-axis
	#define CONTROLTYPE_SLIDER		7			// a slider axis
	#define CONTROLTYPE_BUTTON		8			// a mouse button
	#define CONTROLTYPE_KEY			9			// a key on a keyboard
	#define CONTROLTYPE_POV			10			// point-of-view indicator or "hat"

	typedef struct DeviceInput_t
	{
		DDWORD			m_DeviceType;			// type of input device (DEVICETYPE_ defines above)
		char			m_DeviceName[128];		// name of input device

		DDWORD			m_ControlType;			// type of control that changed (CONTROLTYPE_ defines above)
		char			m_ControlName[64];		// name of control that changed
		DDWORD			m_InputValue;			// input value for the control
	} DeviceInput;

	// Device Object for listing objects on a particular device
	typedef struct DeviceObject_t
	{
		DDWORD		m_DeviceType;			// type of input device (DEVICETYPE_ defines above)
		char		m_DeviceName[128];		// name of input device ("Keyboard", "Mouse", etc.)

		DDWORD		m_ObjectType;			// type of object (CONTROLTYPE_ defines above)
		char		m_ObjectName[64];		// name of object ("x-axis", "button 0", etc.)

		float		m_RangeLow;				// low end of range for this object
		float		m_RangeHigh;			// high end of range for this object

		struct DeviceObject_t*	m_pNext;
	} DeviceObject;
	
	// Device Binding info for GetDeviceBindings()

	#define MAX_ACTIONNAME_LEN 30
	#define INPUTNAME_LEN 100

	typedef struct GameAction_t
	{
		int						nActionCode;						// action number from config file
		char					strActionName[MAX_ACTIONNAME_LEN];	// name of action from config file
		float					nRangeLow;							// low range if used (zero if not used)
		float					nRangeHigh;							// high range if used (zero if not used)
		struct GameAction_t*	pNext;								// next in list

	} GameAction;

	typedef struct DeviceBinding_t
	{
		char					strDeviceName[INPUTNAME_LEN];		// name of device for this trigger
		char					strTriggerName[INPUTNAME_LEN];		// name of this trigger (device object name)
		float					nScale;								// scale to multiply input by
		float					nRangeScaleMin;						// min for range scale
		float					nRangeScaleMax;						// max for range scale
		float					nRangeScalePreCenterOffset;			// offset of cent value for input data
		struct GameAction_t*	pActionHead;						// list of actions bound to this trigger
		struct DeviceBinding_t* pNext;								// next in list

	} DeviceBinding;



	// Structure used by a driver to describe a net service (such as ipx or tcp/ip).
	#define MAX_NETSERVICENAME_LEN	128
	class NetService
	{
	public:
		HNETSERVICE	m_handle;
		DDWORD		m_dwFlags; // Combination of NETSERVICE_ flags.
		DGUID		m_guidService;
		char		m_sName[MAX_NETSERVICENAME_LEN];
		NetService	*m_pNext;
	};


	// Structure used by a driver to describe a specific net session.
	#define MAX_NETSESSIONNAME_LEN	4096
	#define MAX_NETPASSWORD_LEN		64
	#define MAX_HOSTIP_LEN			32
	#define NO_HOST_PORT			0xFFFF

	class NetSession
	{
	public:
					NetSession() {Clear();}
		virtual		~NetSession() {}

		void		Clear()
		{
			m_dwFlags = 0;
			m_dwMaxPlayers = 0;
			m_dwCurPlayers = 0;
			m_Ping = 0;
			m_sName[0] = 0;
			m_sPassword[0] = 0;
			m_HostIP[0] = 0;
			m_HostPort = NO_HOST_PORT;
			m_pNext = DNULL;
		}

		DDWORD		m_dwFlags;
		DGUID		m_guidApp;
		DGUID		m_guidInst;
		DDWORD		m_dwMaxPlayers;
		DDWORD		m_dwCurPlayers;
		DDWORD		m_Ping;	// Ping time in milliseconds.
		
		// Host info.  0 length string and NO_HOST_PORT if not on TCP/IP.
		char		m_HostIP[MAX_HOSTIP_LEN];
		DDWORD		m_HostPort;

		char		m_sName[MAX_NETSESSIONNAME_LEN];
		char		m_sPassword[MAX_NETPASSWORD_LEN];
		NetSession	*m_pNext;
	};


	// Structure used by to instruct a driver to create/host a new session.
	typedef struct NetHost_t
	{
		DDWORD		m_Port; // Port if TCP/IP.  If zero, it uses the default.
		DDWORD		m_dwFlags;
		DDWORD		m_dwMaxPlayers;
		char		m_sName[MAX_NETSESSIONNAME_LEN];
		char		m_sPassword[MAX_NETPASSWORD_LEN];

	}	NetHost;




	// ---------------------------------------------------------------------- //
	// Collision info
	// 
	// Used to determine information in GetStandingOn or GetLastCollision
	// ---------------------------------------------------------------------- //

	class CollisionInfo
	{
	public:
		// The blocking plane of whatever the object collided with.  If both object's are non-solid, then this 
		// will have a normal vector of mag 0.
		DPlane		m_Plane;

		// The object collided with.
		HOBJECT		m_hObject;

		// If the m_hObject is the world, then m_hPoly contains a handle to the polygon the object collided with.
		// Otherwise it is equal to DNULL.
		HPOLY		m_hPoly;

		// Stopping velocity.  Engine will automatically apply this velocity to stop object from continuing
		// to collide with blocker.
		DVector		m_vStopVel;
	};


	// ---------------------------------------------------------------------- //
	// Sound 3D Provider
	//
	// Used with GetSoundSW3DProviderList, GetSoundHW3DProviderList and 
	// ReleaseSound3DProviderList.
	// ---------------------------------------------------------------------- //
	
	// Sound 3D provider ID's
	#define SOUND3DPROVIDERID_NONE					0
	#define SOUND3DPROVIDERID_DS3D_HARDWARE			1
	#define SOUND3DPROVIDERID_DS3D_HARDWARE_EAX		2
	#define SOUND3DPROVIDERID_DS3D_SOFTWARE			3
	#define SOUND3DPROVIDERID_A3D					4
	#define SOUND3DPROVIDERID_INTEL_RSX				5
	#define SOUND3DPROVIDERID_MILES3D				6
	#define SOUND3DPROVIDERID_UNKNOWN				7

	// Caps bits
	#define SOUND3DPROVIDER_CAPS_REVERB			(1<<0)

	struct Sound3DProvider
	{
		DDWORD				m_dwProviderID;
		char				m_szProvider[_MAX_PATH+1];
		DDWORD				m_dwCaps;
		Sound3DProvider *	m_pNextProvider;
	};

	// ---------------------------------------------------------------------- //
	// Reverb properties
	//
	// Use this structure with SetReverb and GetReverb
	// ---------------------------------------------------------------------- //
	
	#define REVERBPARAM_VOLUME				(1<<0)		// m_fVolume field valid
	#define REVERBPARAM_ACOUSTICS			(1<<1)		// m_dwAcoustics field valid
	#define REVERBPARAM_REFLECTTIME			(1<<2)		// m_fReflectTime field valid
	#define REVERBPARAM_DECAYTIME			(1<<3)		// m_fDecayTime field valid
	#define REVERBPARAM_DAMPING				(1<<4)		// m_fDamping field valid
	#define REVERBPARAM_ALL					REVERBPARAM_VOLUME | REVERBPARAM_ACOUSTICS | REVERBPARAM_REFLECTTIME | REVERBPARAM_DECAYTIME | REVERBPARAM_DAMPING

	// These are the valid values for m_dwAcoustics field
	enum
	{
		REVERB_ACOUSTICS_GENERIC,
		REVERB_ACOUSTICS_PADDEDCELL,
		REVERB_ACOUSTICS_ROOM,
		REVERB_ACOUSTICS_BATHROOM,
		REVERB_ACOUSTICS_LIVINGROOM,
		REVERB_ACOUSTICS_STONEROOM,
		REVERB_ACOUSTICS_AUDITORIUM,
		REVERB_ACOUSTICS_CONCERTHALL,
		REVERB_ACOUSTICS_CAVE,
		REVERB_ACOUSTICS_ARENA,
		REVERB_ACOUSTICS_HANGAR,
		REVERB_ACOUSTICS_CARPETEDHALLWAY,
		REVERB_ACOUSTICS_HALLWAY,
		REVERB_ACOUSTICS_STONECORRIDOR,
		REVERB_ACOUSTICS_ALLEY,
		REVERB_ACOUSTICS_FOREST,
		REVERB_ACOUSTICS_CITY,
		REVERB_ACOUSTICS_MOUNTAINS,
		REVERB_ACOUSTICS_QUARRY,
		REVERB_ACOUSTICS_PLAIN,
		REVERB_ACOUSTICS_PARKINGLOT,
		REVERB_ACOUSTICS_SEWERPIPE,
		REVERB_ACOUSTICS_UNDERWATER,
		REVERB_ACOUSTICS_DRUGGED,
		REVERB_ACOUSTICS_DIZZY,
		REVERB_ACOUSTICS_PSYCHOTIC,

		REVERB_ACOUSTICS_COUNT           // total number of room types
	};

	struct ReverbProperties
	{
		DDWORD				m_dwParams;		// Set the params bits for which fields are valid
		float				m_fVolume;		// 0.0 - 1.0
		DDWORD				m_dwAcoustics;	// One of the REVERB_ACOUSTICS_xxx values
		float				m_fReflectTime;	// 0.0 - 5.0 Time for first reflection
		float				m_fDecayTime;	// 0.1 - 20.0 Determines how quickly reflection diminish
		float				m_fDamping;		// 0.0 - 2.0, == 1.0f is even damping, < 1.0f low frequencies dampen faster
											// > 1.0f high frequencies dampen faster
	};

	// ---------------------------------------------------------------------- //
	// Sound effects.
	// 
	// The type of sounds are:  ambient, local and 3D.  The flags controlling
	// these types are mutually exclusive.  An ambient sound will have distance
	// effects of rolloff, but no orientation effects.  A local sound will have
	// no orientation or distance effects and will be as if the sound was played
	// inside the player's head.  The 3d sound will have distance, orientation
	// and doppler effects.
	//
	// Sounds are played from the beginning once the message reaches the client.
	// If it is important that the playback be synchronized with the server's 
	// beginning time, then set PLAYSOUND_TIMESYNC.  Normally, this is not
	// that important.  The server will use it internally if a client comes in
	// after a sound has already been played.
	//
	// The server keeps a sound object if any of the following flags are set:
	// PLAYSOUND_LOOP, PLAYSOUND_ATTACHED, PLAYSOUND_GETHANDLE, PLAYSOUND_TIMESYNC,
	// PLAYSOUND_TIME. Server kept sounds will update clients that come and go.  
	// Non-server kept sounds are sent to the existing clients once, so the 
	// overhead is much less.
	// 
	// Server kept sounds with PLAYSOUND_GETHANDLE must be removed by the game.  
	// Other server kept sounds are removed if they time out, or the object they
	// are attached to is removed.  When a sound is removed from the server,
	// it tells the clients to remove their copies.
	//
	// Server tells clients about its sounds if the client object is within
	// twice the outer radius of the sound.  If local sound, then server always
	// tells all the clients.
	//
	// Sounds that have a client object in m_hObject and PLAYSOUND_CLIENTLOCAL
	// or PLAYSOUND_ATTACHED are played with PLAYSOUND_LOCAL for that particular client.
	//
	// The PLAYSOUND_CLIENT is for client initiated sounds only.  When playing
	// client side sounds, the PLAYSOUND_ATTACHED and PLAYSOUND_CLIENTLOCAL
	// flags and m_wObjectID are ignored.
	//
	// Using PLAYSOUND_FILESTREAM tells the client to stream the file from disk.
	// This prevents the loading of all of the sound data, but is slower.
	// ---------------------------------------------------------------------- //

	#define PLAYSOUND_LOCAL			0x0000	// Play sound locally (inside head)
	#define PLAYSOUND_AMBIENT		0x0001	// Play sound as ambient sound.
	#define PLAYSOUND_3D			0x0002	// Play sound as 3D sound.
	#define PLAYSOUND_LOOP			0x0004	// Loop the sound.
	#define PLAYSOUND_ATTACHED		0x0008	// Sounds position & orientation comes from object in m_hObject
	#define PLAYSOUND_GETHANDLE		0x0010	// Handle requested
	#define PLAYSOUND_TIME			0x0020	// Server must time sound
	#define PLAYSOUND_CTRL_VOL		0x0040	// Control volume m_nVolume
	#define PLAYSOUND_REVERB		0x0080	// Allow reverb
	#define PLAYSOUND_CLIENT		0x0100	// Client side sound
	#define PLAYSOUND_TIMESYNC		0x0200	// Playback synchronized with server clock
	#define PLAYSOUND_FILESTREAM	0x0400	// Stream the file
	#define PLAYSOUND_CLIENTLOCAL	0x0800	// Sound is played with PLAYSOUND_LOCAL for object in m_hObject

	typedef struct PlaySoundInfo_t
	{
		// PLAYSOUND flags.
		DDWORD		m_dwFlags;
	
		// File name of sound
		char 		m_szSoundName[_MAX_PATH+1];

		// SERVER ONLY: Object sound is attached to.
		// Only needed if PLAYSOUND_ATTACHED or PLAYSOUND_CLIENTLOCAL set
		HOBJECT		m_hObject;

		// Handle of sound
		// Filled by PlaySound if PLAYSOUND_GETHANDLE set
		HSOUNDDE	m_hSound;

		// Voice priority 
		// 0 is lowest priority
		unsigned char m_nPriority;

		// Maximum radius of sound.  No sound outside this.
		// Only needed if PLAYSOUND_3D or PLAYSOUND_AMBIENT
		float		m_fOuterRadius;
		
		// Inner radius of sound.  Sound at maximum volume inside this radius
		// Only needed if PLAYSOUND_3D or PLAYSOUND_AMBIENT
		float		m_fInnerRadius;
		
		// Volume of sound in percent [0,100].
		// Only needed if PLAYSOUND_CTRL_VOL set, otherwise defaults 100
		DBYTE		m_nVolume;

		// Position of sound.
		// Only needed if PLAYSOUND_AMBIENT and PLAYSOUND_3D set
		// Only needed if PLAYSOUND_ATTACHED cleared
		DVector		m_vPosition;

	} PlaySoundInfo;

	#define PLAYSOUNDINFO_COPY( dest, src ) \
		memcpy(( void * )&( dest ), ( void * )&( src ), sizeof( PlaySoundInfo ));

	#define PLAYSOUNDINFO_INIT(x) memset(&x, 0, sizeof(x));

	// ---------------------------------------------------------------------- //
	// InitSoundInfo
	//
	// Used to initialize the sound engine.
	// ---------------------------------------------------------------------- //

	// Maximum number of voices allowed for either sw or 3d
	#define INITSOUNDINFO_MAXVOICES			128

	// Used for m_dwFlags parameter
	#define INITSOUNDINFOFLAG_CONVERT16TO8	(1<<0)	// Convert all 16 bit buffers to 8 bit
	#define INITSOUNDINFOFLAG_RELOADSOUNDS	(1<<1)	// Reload any sounds that exist before InitSound called

	// Engine can fill these flags in the m_dwResults parameter
	#define INITSOUNDINFORESULTS_REVERB		(1<<0)	// Provider chosen supports reverb

	typedef struct InitSoundInfo_t
	{
		// Name of 3d provider to use
		char						m_sz3DProvider[_MAX_PATH+1];

		// Number of sw voices
		DBYTE						m_nNumSWVoices;

		// Number of 3D voices
		DBYTE						m_nNum3DVoices;

		// Output sound format.  Sample rate (8000, 11025, 22050 or
		// 44100 kHz), Bits per sample (8 or 16)...
		unsigned short				m_nSampleRate;
		unsigned short				m_nBitsPerSample;

		// Use INITSOUNDINFOFLAG_xxx flags
		unsigned long				m_dwFlags;

		// Engine fills in this parameter with INITSOUNDINFORESULTS_xxx after InitSound is called.
		unsigned long				m_dwResults;

		// Initial volume (0-100)
		unsigned short				m_nVolume;

		// Distance factor in meters/game unit
		float						m_fDistanceFactor;

		// Number of seconds to make streaming buffer.  Don't go less than 0.2, though.  The buffer
		// is also limited by a minimum size of 20k.
		float						m_fMinStreamTime;

	} InitSoundInfo;

	// Initialize the InitSoundInfo structure to the default values...
	#define INITSOUNDINFO_INIT( initSoundInfo ) \
		initSoundInfo.m_sz3DProvider[0]				= 0;									\
		initSoundInfo.m_nNumSWVoices				= 32;									\
		initSoundInfo.m_nNum3DVoices				= 0;									\
		initSoundInfo.m_nSampleRate					= 22050;								\
		initSoundInfo.m_nBitsPerSample				= 16;									\
		initSoundInfo.m_dwFlags						= 0;									\
		initSoundInfo.m_nVolume						= 100;									\
		initSoundInfo.m_fDistanceFactor				= 1.0f;									\
		initSoundInfo.m_fMinStreamTime				= 0.5f;

	
	// ------------------------------------------------------------------ //
	// Use this to start a game.
	// ------------------------------------------------------------------ //

	#define STARTGAME_HOST		0	// Start a world and host it (using dialogs).
	#define STARTGAME_HOSTTCP	1	// Start a world and host on TCP/IP.

	#define STARTGAME_CLIENT	2	// Connect to a server using dialogs.
	#define STARTGAME_CLIENTTCP	3	// Connect to the first TCP/IP game it can find
									// at m_pTCPAddress.
	
	#define STARTGAME_NORMAL	4	// Start a normal game.
	#define GAMEMODE_NONE		5	// (Used for GetGameMode, means we're not 
									// running a world or on a server yet).

	#define SG_LOBBY			1	// Game was lobby-launched

	#define MAX_SGR_STRINGLEN	100

	class StartGameRequest
	{
	public:
						StartGameRequest()
						{
							m_Type = STARTGAME_NORMAL;
							m_WorldName[0] = 0;
							m_TCPAddress[0] = 0;
							m_RecordFilename[0] = 0;
							m_PlaybackFilename[0] = 0;
							m_flags = 0;
							m_pGameInfo = DNULL;
							m_GameInfoLen = 0;
							m_pNetSession = DNULL;
							m_pClientData = DNULL;
							m_ClientDataLen = 0;
						}	
		
		int				m_Type;
		char			m_WorldName[MAX_SGR_STRINGLEN];
		char			m_TCPAddress[MAX_SGR_STRINGLEN];		// TCP/IP address, if any.
		
		// Filename to record into, if any (set to 0 length if you don't want to record).
		// NOTE: when this is set, the engine starts the server but doesn't run the level, you must
		// send a message to the server telling it to load the world.
		char			m_RecordFilename[MAX_SGR_STRINGLEN];	
		
		// The filename of a recorded demo.  If this is filled in, the engine starts a server
		// and fills in m_WorldName with the world that the demo record uses.  You need to
		// send a message to the server telling it to load that world.
		char			m_PlaybackFilename[MAX_SGR_STRINGLEN];
		
		NetSession		*m_pNetSession;	// This must be used with STARTGAME_CLIENT.
		NetHost			m_HostInfo;		// This must be used with STARTGAME_HOST.
		DDWORD			m_flags;		// Various flags

		void			*m_pGameInfo;	// This data is copied over and can be accessed by
		DDWORD			m_GameInfoLen;	// the server with ServerDE::GetGameInfo() (if you're
										// running a local or hosted game).
	
		// This data gets sent up and passed into OnClientEnterWorld on the server.
		void			*m_pClientData;
		DDWORD			m_ClientDataLen;
	};


	// The new console parsing thing.
	class ConParse
	{
	public:
	
				ConParse()				{m_pCommandPos = NULL;}
				ConParse(char *pBuffer)	{m_pCommandPos = pBuffer;}

		// Sets it up to parse the specified buffer.
		void	Init(char *pBuffer)		{m_pCommandPos = pBuffer;}

		// The parsed arguments.
		char	*m_Args[PARSE_MAXTOKENS];
		int		m_nArgs;

	// Used internally by the engine.
	public:

		// Parse the next set of tokens.  Returns TRUE if it parsed anything.
		DBOOL	Parse();

		// Parses until it finds tokens with pLookFor as the first one.
		// You can use this just like Parse like this:
		// while(ParseFind("AmbientLight", FALSE)) { ... }
		DBOOL	ParseFind(char *pLookFor, DBOOL bCaseSensitive, DDWORD minTokens=1);

	private:
		
		char	*m_pCommandPos;
		char	m_ArgBuffer[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
	};


	// ---------------------------------------------------------------------- //
	// Intersection stuff.
	// ---------------------------------------------------------------------- //

	// Return DTRUE to select this object and DFALSE to not select it.
	typedef DBOOL (*ObjectFilterFn)(HOBJECT hObj, void *pUserData);


	// Pass this in to the IntersectSegment routine.
	class IntersectQuery
	{
	public:
					IntersectQuery()
					{
						m_Flags = 0;
						m_FilterFn = 0;
					}

		// Start and end points.
		DVector			m_From;
		DVector			m_To;

		// Only used for CastRay.. this is the direction the ray should be cast in.
		// This doesn't need to be normalized.
		DVector			m_Direction;

		// A combination of the intersect flags (in de_codes.h).
		DDWORD			m_Flags;
		
		// If this is not NULL, then it'll call this function when it has a 
		// POSSIBLE object intersection (it doesn't know if it really intersects
		// yet when it calls this).  If you return FALSE from this function, 
		// then it will ignore the object and continue on.
		ObjectFilterFn	m_FilterFn;
		
		// Passed into pUserData of the filter function.
		void			*m_pUserData;		
	};

	
	typedef struct IntersectInfo_t
	{
		// Point of intersection.
		DVector		m_Point;

		// Plane of intersection.
		DPlane		m_Plane;

		// Object it hit.
		HOBJECT		m_hObject;

		// The polygon it hit (if it's a world poly).
		// Value is INVALID_HPOLY if it didn't hit one.
		HPOLY		m_hPoly;

		// Surface flags of what it hit (these aren't object flags, and are
		// only set if it hit the world or a WorldModel).
		DDWORD		m_SurfaceFlags;
	} IntersectInfo;

	#define ClientIntersectInfo		IntersectInfo
	#define ClientIntersectQuery	IntersectQuery


#endif  // __BASEDEFS_H__


