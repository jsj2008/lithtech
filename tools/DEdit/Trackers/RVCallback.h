//////////////////////////////////////////////////////////////////////
// RVCallback.h - Header for the RegionView Callback tracker 

#ifndef __RVCALLBACK_H__
#define __RVCALLBACK_H__

#include "rvcommand.h"

class CRVCallback : public CRegionViewCommand
{
public:
	typedef BOOL (CRegionView::*PCallbackFn)(CUITracker *);

protected:
	PCallbackFn m_fCallback;

public:

	CRVCallback(LPCTSTR pName = "", CRegionView *pView = NULL, PCallbackFn fCallback = NULL) : CRegionViewCommand(pName, pView, TRUE), m_fCallback(fCallback) {};
	virtual ~CRVCallback() {};

	// Member variable access
	virtual PCallbackFn GetCallback() const { return m_fCallback; };
	virtual void SetCallback(PCallbackFn pCallback) { m_fCallback = pCallback; };

	// The actual callback
	virtual BOOL OnCommand() { return (m_pView->*m_fCallback)(this); };
};

// Callback class for calling non-parameter, non-return functions
class CRVCallbackSimple : public CRegionViewCommand
{
public:
	typedef void (CRegionView::*PCallbackFn)();

protected:
	PCallbackFn m_fCallback;
public:

	CRVCallbackSimple(LPCTSTR pName = "", CRegionView *pView = NULL, PCallbackFn fCallback = NULL) : CRegionViewCommand(pName, pView, TRUE), m_fCallback(fCallback) {};
	virtual ~CRVCallbackSimple() {};

	// Member variable access
	virtual PCallbackFn GetCallback() const { return m_fCallback; };
	virtual void SetCallback(PCallbackFn pCallback) { m_fCallback = pCallback; };

	// The actual callback
	virtual BOOL OnCommand() { (m_pView->*m_fCallback)(); return TRUE; };
};

#endif //__RVCALLBACK_H__