//----------------------------------------------------------------------------------------
//
// MODULE  : UserNotificationMgr.h
//
// PURPOSE : Handles a list of notifications that when activated will display
//			 an icon on the screen. This is commonly used for notifying the
//			 user of content related issues.
//
// CREATED : 5/29/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
//----------------------------------------------------------------------------------------

#ifndef __USERNOTIFICATIONMGR_H__
#define __USERNOTIFICATIONMGR_H__

//the severity of a notification
enum ENotificationSeverity
{
	eNotificationSeverity_High,
	eNotificationSeverity_Medium,
	eNotificationSeverity_Low,
	eNotificationSeverity_Disabled,
};

//the callback function for each notfication object. This should return the severity of the notification
//at the current time
typedef ENotificationSeverity (*TNotificationCallbackFn)(void* pUserData);

class CUserNotificationMgr
{
public:

	// lifetime operations
	CUserNotificationMgr();
	~CUserNotificationMgr();

	//called to shut down the system and free any associated resources
	void		Term();

	//operations to render a notification handler. This handler will be called once each update
	//and each time it can return true to indicate that the notification should be provided
	bool		RegisterNotification(TNotificationCallbackFn pfnCallback, void* pUser, const char* pszIcon, float fActivationTime);

	//called to update. Existing triggered notifications will be aged by the specified time frame, and
	//then each notification checked to see if they should be considered active
	void		UpdateNotifications(float fFrameTime);

	//called to render the active notification icons to the screen. This must be called within a begin/end3d
	//block.
	void		RenderActiveNotifications();

private:

	//internal structure used to represent a notification
	class CNotification
	{
	public:

		CNotification();
		~CNotification();

		//the callback function for this notification
		TNotificationCallbackFn			m_pfnCallback;

		//the user data associated with the callback
		void*							m_pUserData;

		//the severity of the notification
		ENotificationSeverity			m_eSeverity;

		//the lifetime remaining for this icon to be displayed (<= 0 means don't display)
		float							m_fDisplayTimeLeft;

		//the amount of time that this icon should be displayed for after it is activated
		float							m_fActiveTime;

		//the texture used for the icon
		TextureReference						m_hIcon;

		//the total amount of time that this notification has been active over the run. Note
		//that this does not count the time that it is rendered, only each successive notification
		//callback that reports that the notification is valid
		float							m_fTotalTimeActive;
	};

	//out listing of all the notifications that we have
	typedef std::vector<CNotification, LTAllocator<CNotification, LT_MEM_TYPE_CLIENTSHELL> >	TNotificationList;
	TNotificationList			m_Notifications;

	//the amount of time that we have been active
	float						m_fTotalTimeActive;
};

//this is called to install the notifiers that are found in UserNotifiers.cpp into the specified
//notification manager.
void	InstallUserNotifiers(CUserNotificationMgr& NotificationMgr);

#endif
