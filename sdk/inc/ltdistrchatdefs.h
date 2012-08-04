/*!

 MODULE  : ltdistrchatdefs.h

 PURPOSE : Declaration of miscellaneous utility structs, constants, etc.
			used by the distributed chat service
		   
 CREATED : 5/8/01

*/

#ifndef __LTDISTRCHATDEFS_H__
#define __LTDISTRCHATDEFS_H__

#ifndef __SET__
#include <set>
#define __SET__
#endif

#ifndef __QUEUE__
#include <queue>
#define __QUEUE__
#endif

#ifndef __LIST__
#include <list>
#define __LIST__
#endif

/*!
\name Miscellaneous chat constants

Used For: DistributedServices.
*/
enum { 
	//! Maximum number of connected chat users.  (Essentially arbitrary)
	MAX_CHAT_USERS	= 200,	
	//! Maximum length of a chat user's nickname.
	MAX_CHAT_LEN_NAME = 26
};

typedef int32 ChatUserID; //! The type used for chat user ids.  (These are actually the same as the RemoteCommunicationsContext unique id value.)


/*!
Simple struct to wrap a fixed length string that represents a user's nickname.

Used For: DistributedServices.
*/
struct ChatNickname
{
	char	Name[MAX_CHAT_LEN_NAME]; 	//! The nickname string itself.  You may access this directly.

	/*! Conversion operator for const char pointer; returns the Name string.
	Useful in casting operations, for example:
	\code printf("My name is %s\n", (const char*)MyNickname); // MyNickname is a ChatNickname
	*/
	operator const char*() const
	{
		return Name;
	}

	operator char*() //! Conversion operator for char pointer; returns the Name string.

	{
		return Name;
	}

	/*! Copy constructor for const char pointer; allows constructs such as 
	\code ChatNickname newName = "Supreme Overlord";
	*/
	ChatNickname(const char* p)
	{
		strncpy(Name, p, MAX_CHAT_LEN_NAME);
	}

	/*! Copy constructor for const \b ChatNickname; allows constructs such as 
	\code ChatNickname newName = oldName; // oldName is another ChatNickname
	*/
	ChatNickname(const ChatNickname& n)
	{
		strncpy(Name, n.Name, MAX_CHAT_LEN_NAME);
	}

	/*! Assignment operator for const char pointer; allows constructs such as 
	\code ChatNickname newName;
		  newName = "Supreme Overlord";
	*/
	ChatNickname& operator=(const char* p)
	{
		strncpy(Name, p, MAX_CHAT_LEN_NAME);
		return *this;
	}

	/*! Assignment operator for \b ChatNickname; allows constructs such as 
	\code ChatNickname newName;
		  newName = oldName; // oldName is another ChatNickname;
	*/
	ChatNickname& operator=(const ChatNickname& n)
	{
		if( &n != this )	// guard against self assignment
			strncpy(Name, n.Name, MAX_CHAT_LEN_NAME);
		return *this;
	}
 
	ChatNickname() 	//! Constructor.  Blanks the Name string.
	{
		Name[0] = 0;
	}


};


const char DEFAULT_NICK_PREFIX[] = "Player_"; //! Default chat client nicknames are built from this + UserID.  These prefix is reserved for the default nicknames.

/*!
Struct to wrap a fixed length message string, routing information, and chat-system internal state.

Used For: DistributedServices.
*/
struct ChatMessage
{
/*!
\name ChatMessage constants

Used For: DistributedServices.
*/
	enum { 
		//! Maximum length of the message string (without trailing null)
		MAX_MESSAGE_LEN = 255,
		//! Used internally to indicate a broadcast message.
		CHAT_BROADCAST_MSG = MAX_CHAT_USERS + 1,
		//! Indicates a message generated on the server (rather than by a user)
		CHAT_SERVER_MSG	// a message from the server
	};

	typedef uint8 LengthByte; 	//! Used internally during distribution.


	ChatUserID sendingUser; 	//! ID of the user who sent the message (\b CHAT_SERVER_MSG if sent by the server)


	ChatUserID targetUser;	//! ID of the user targeted by the mesage (\b CHAT_BROADCAST_MSG if broadcast to all users)


	char       message[MAX_MESSAGE_LEN]; //! The message string itself.  You can access this directly if you wish.
	
	uint8	   refCount;	//! Internal use by chat system.  (This will probably disappear in future revisions of the chat system)

/*! 
Constructor.  Blanks the message string and routing info.
	
Used For: DistributedServices.
*/
	ChatMessage() : sendingUser(0), targetUser(0), refCount(0)
	{
		message[0] = 0;
	}

/*!
\return \b true if a broadcast message; otherwise, returns \b false.

Determines whether a message is broadcast or targeted to a single user.

Used For: DistributedServices.
*/
	bool IsBroadcast() const			{ return targetUser == CHAT_BROADCAST_MSG; }

/*!
Sets a message to broadcast to all users.

Used For: DistributedServices.
*/
	void SetBroadcast()					{ targetUser = CHAT_BROADCAST_MSG; }

/*!
\param id User id of the chat user to target with the message.

Sets a message to be sent to the indicated user.

Used For: DistributedServices.
*/
	void SetTargetUser(ChatUserID id)	{ targetUser = id; }

	/*! Conversion operator for const char pointer; returns the message string.
	Useful in casting operations, for example:
	\code printf("New message: %s\n", (const char*)theMessage); // theMessage is a ChatMessage
	*/
	operator const char*() const
	{
		return message;
	}

	operator char*() 	//! Conversion operator for char pointer; returns the message string.
	{
		return message;
	}

	/*! Assignment operator for const char pointer; allows constructs such as 
	\code ChatMessage* pMsg = g_pChatClient->GetOutgoingChatMessage();
		   *pMsg = "Don't mess with the Supreme Overlord!";
	*/
	ChatMessage& operator=(const char* x)
	{
		strncpy(message, x, MAX_MESSAGE_LEN);
		return *this;
	}
};

typedef uint8 ChatRequest; //! Type of the ChatRequest sent from client to server, and from server to client as part of a response.

enum ChatRequestEnums //! Request enums - possible values of a \b ChatRequest.
{	
	//! Request is to change nicknames.
	SET_NICK	= 1,
	//! Request is to send a message.
 	SEND_MSG,

	//! internal use by chat system.
	REQUEST_MASK = 0xf	// maximum 4 bytes in request
};

typedef uint8 ChatRequestHandle; //! Type of the ChatRequestHandle assigned to a request to send a message, and sent from server to client as part of a response.

typedef uint8 ChatResponse; //! Type of the ChatResponse sent from server to client to indicate that a request failed.

enum ChatResponseEnums //! Response enums - possible values of a \b ChatResponse.
{
	//! general success
	CHAT_RESPONSE_OK	= 0,
	
	//! Nickname rejected because it duplicated an existing nickname. 
	NICK_DUPLICATE  = (1 << 4),
	//! Nickname rejected due to invalid syntax.
	NICK_INVALID_SYNTAX,
	//! Nickname rejected because it is reserved ("Anonymous" or starts with DEFAULT_NICK_PREFIX)
	NICK_RESERVED,
	//! Nickname rejected by the custom nickname filtering function.
	NICK_USER_CALLBACK, 

	//! Message not sent because the targeted user doesn't exist.
	MSG_NO_SUCH_USER = (1 << 4),
	//! Message not sent because it was rejected by the custom message filtering function.
	MSG_USER_FILTER_SQUELCHED,

	//! internal use by distributed chat system.
	RESPONSE_MASK = 0xf0
};

typedef std::list<ChatMessage*> MessageHistory; //! The type of the local MessageHistory; can be retrieved via \b ILTDistrChatClient::GetMessageHistory().

#endif //  __LTDISTRCHATDEFS_H__

