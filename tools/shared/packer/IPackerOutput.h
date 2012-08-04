//---------------------------------------------------------------
// IPackerOutput.h
//
// Provides the definition for IPackerOutput, which provides a 
// mechanism for Packer implementations to display output
// in an organized manner.
//
// Created: 4/16/01
// Author: John O'Rorke
// Modification History:
//
//---------------------------------------------------------------
#ifndef __IPACKEROUTPUT_H__
#define __IPACKEROUTPUT_H__

//these are different status of messages
enum EMsgType {	MSG_CRITICAL,
				MSG_ERROR,
				MSG_WARNING,
				MSG_NORMAL,
				MSG_VERBOSE,
				MSG_DEBUG
			};

class IPackerOutput
{
public:

	//---------------------
	// Constructors

	IPackerOutput()				{}
	virtual ~IPackerOutput()	{}

	//---------------------
	// Task Management

	// This will create a task and add it to the task queue. Tasks
	// can be associated with messages as well as a progress indicator.
	virtual bool	CreateTask(const char* pszTaskName)						= 0;

	// This function allows for a task to be activated, meaning that
	// all messages sent will be associated with this task, as will
	// all progress messages
	virtual bool	ActivateTask(const char* pszTaskName)					= 0;

	// This function allows for a sub task to be started. This does not create
	// a new task or redirect messages, only changes possible UI for a sub
	// task that is being performed as part of a larger task. When you switch
	// to a task, that task also becomes the active subtask
	virtual bool	ActivateSubTask(const char* pszSubTaskName)				= 0;

	// This will update the progress of the currently active task. The
	// value can range from 0 to 100, where 0 is starting, and 100 is
	// completed
	virtual bool	UpdateProgress(float fProgress)							= 0;

	//---------------------
	// Messaging

	// This will log a message. It will be associated with the current
	// task.
	virtual bool	LogMessage(	EMsgType nStatus, const char* pszMsg)		= 0;

	// This will log a message. It will be associated with the current
	// task. This version allows the inclusion of a help string so that
	// the user can get more information about the message
	virtual bool	LogMessageH(EMsgType nStatus, const char* pszHelpMsg,
								const char* pszMsg)							= 0;


private:
};


#endif
