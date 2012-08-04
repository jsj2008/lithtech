/****************************************************************************
;
;	MODULE:		LTRWindowlessSite (.CPP)
;
;	PURPOSE:	Support class for RealVideoMgr
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrwindowlesssite.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTWindowlessSite member functions
//-----------------------------------------------------------------------------
CLTWindowlessSite::CLTWindowlessSite(IUnknown* pContext, 
					     IUnknown* pUnkOuter, CLTRealVideoPlayer* pPlayer)
    : m_lRefCount(0)
    , m_pUser(NULL)
    , m_pParentSite(NULL)
    , m_pUnkOuter(pUnkOuter)
    , m_pWatcher(NULL)
    , m_pContext(pContext)
    , m_pVideoSurface(NULL)
    , m_bIsVisible(TRUE)
    , m_pValues(NULL)
    , m_lZOrder(0)
    , m_bInDestructor(FALSE)
    , m_pCCF(NULL)
    , m_bDamaged(FALSE)
    , m_bInRedraw(FALSE)
{
    // addref the context 
    m_pContext->AddRef();

    // get the CCF 
    m_pContext->QueryInterface(IID_IRMACommonClassFactory,
			       (void**)&m_pCCF);

    // addref CCF
    m_pCCF->CreateInstance(CLSID_IRMAValues,(void**)&m_pValues);

    /* If we are not being aggregated, then point our outer
     * unknown to our own implementation of a non-delegating
     * unknown... this is like aggregating ourselves<g>
     */
    if (pUnkOuter == NULL)
    {
	m_pUnkOuter = (IUnknown*)(NonDelegatingUnknown*) this;
    }

    m_size.cx = 0;
    m_size.cy = 0;
    m_position.x = 0;
    m_position.y = 0;

    // create video surface
    LT_MEM_TRACK_ALLOC(m_pVideoSurface = new CLTRealVideoSurface,LT_MEM_TYPE_MISC);
	m_pVideoSurface->Init(m_pContext, this, pPlayer);
    m_pVideoSurface->AddRef();

    m_bIsChildWindow = TRUE;
}


//-----------------------------------------------------------------------------
CLTWindowlessSite::~CLTWindowlessSite()
{
    m_bInDestructor = TRUE;

    // Clean up all child sites

    int n = 0;
    int count = m_Children.Count();
    for (n=0; n<count; n++)
    {
 	ExSiteInfo* pSiteInfo = (ExSiteInfo*) m_Children.GetFirst();
	DestroyChild(pSiteInfo->m_pSite);
	delete pSiteInfo;
	m_Children.RemoveHead();
    }

    // clean up passive site watchers 

    count = m_PassiveSiteWatchers.Count();
    for (n=0; n<count; n++)
    {
	IRMAPassiveSiteWatcher* pWatcher =
	    (IRMAPassiveSiteWatcher*) m_PassiveSiteWatchers.GetFirst();
	PN_RELEASE(pWatcher);
	m_PassiveSiteWatchers.RemoveHead();
    }

    // release intefaces
    PN_RELEASE(m_pValues);
    PN_RELEASE(m_pCCF);
    PN_RELEASE(m_pContext);
}

//-----------------------------------------------------------------------------
void CLTWindowlessSite::SetParentSite(CLTWindowlessSite* pParentSite)
{
    m_pParentSite = pParentSite;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::NonDelegatingQueryInterface(REFIID riid, void** ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
	AddRef();
	*ppvObj = (IUnknown*)(IRMASite*)this;
	return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAValues))
    {
	AddRef();
	*ppvObj = (IRMAValues*)this;
	return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMASite))
    {
	AddRef();
	*ppvObj = (IRMASite*)this;
	return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMASite2))
    {
	AddRef();
	*ppvObj = (IRMASite2*)this;
	return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMASiteWindowless))
    {
	AddRef();
	*ppvObj = (IRMASiteWindowless*)this;
	return PNR_OK;
    }

    *ppvObj = NULL;
    return PNR_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTWindowlessSite::NonDelegatingAddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTWindowlessSite::NonDelegatingRelease()
{
    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;
    return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::QueryInterface(REFIID riid, void** ppvObj)
{
    return m_pUnkOuter->QueryInterface(riid,ppvObj);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTWindowlessSite::AddRef()
{
    return m_pUnkOuter->AddRef();
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTWindowlessSite::Release()
{
    return m_pUnkOuter->Release();
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::AttachUser(IRMASiteUser* /*IN*/ pUser)
{
    HRESULT result = PNR_FAIL;

    if (m_pUser) return PNR_UNEXPECTED;

    IRMASite* pOuterSite;
    m_pUnkOuter->QueryInterface(IID_IRMASite, (void**)&pOuterSite);
    if (pOuterSite)
    {
	result = pUser->AttachSite(pOuterSite);

	pOuterSite->Release();
    }
    
    if (PNR_OK == result)
    {
	m_pUser = pUser;
	m_pUser->AddRef();
    }

    return result;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::DetachUser()
{
    HRESULT result = PNR_OK;
    if (!m_pUser) return PNR_UNEXPECTED;

    result = m_pUser->DetachSite();
    
    if (PNR_OK == result)
    {
	m_pUser->Release();
	m_pUser = NULL;
    }

    return result;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetUser(REF(IRMASiteUser*) /*OUT*/ pUser)
{
    HRESULT result = PNR_OK;
    if (!m_pUser) return PNR_UNEXPECTED;

    pUser = m_pUser;
    pUser->AddRef();

    return result;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::CreateChild(REF(IRMASite*) /*OUT*/ pChildSite)
{
    HRESULT result = PNR_OK;


    /* Create an instance of CLTWindowlessSite, let it know it's
     * in child window mode.
     */
    CLTWindowlessSite* pChildSiteWindowless;
	LT_MEM_TRACK_ALLOC(pChildSiteWindowless = new CLTWindowlessSite(m_pContext),LT_MEM_TYPE_MISC);
    pChildSiteWindowless->AddRef();
    pChildSiteWindowless->SetParentSite(this);

    /* Get the IRMASite interface from the child to return to the
     * outside world.
     */
    pChildSiteWindowless->QueryInterface(IID_IRMASite, (void**)&pChildSite);


    /*
     * Store the CLTWindowlessSite in a list of child windows, always keep
     * a reference to it.
     * ExSiteInfo does an AddRef in constructor
     */    
     
    ExSiteInfo* pSiteInfo;
	LT_MEM_TRACK_ALLOC(pSiteInfo = new ExSiteInfo(pChildSite, (void*) pChildSiteWindowless) ,LT_MEM_TYPE_MISC);
    m_Children.Add(pSiteInfo);

    return result;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::DestroyChild(IRMASite* /*IN*/ pChildSite)
{
    //fprintf(stdout, "CLTWindowlessSite::DestroyChild()\n");

    /*
     * Verify that the site is actually a child site and find
     * its position in the list in the event that it is a child.
     */
    LTRListNodePosition prevPos;
    ExSiteInfo* pSiteInfo = NULL;
    MapSiteToSiteInfo(pChildSite, pSiteInfo, prevPos);
    if (!pSiteInfo) 
    {
	return PNR_UNEXPECTED;
    }

    /* 
     * Cleanup the list items for this child site.
     */
    if (!m_bInDestructor)
    {
        delete pSiteInfo;
	if (prevPos)
	{
	    m_Children.RemoveAfter(prevPos);
	}
	else
	{
	    m_Children.RemoveHead();
	}
    }

    return PNR_OK;
}

//-----------------------------------------------------------------------------
BOOL CLTWindowlessSite::MapSiteToSiteInfo(IRMASite* pChildSite, ExSiteInfo*& pSiteInfo, LTRListNodePosition& prevPos)
{
    BOOL res = FALSE;

    // iterate child site list 
    pSiteInfo = NULL;
    LTRListNodePosition pos = m_Children.GetHeadPosition();
    prevPos = pos;
    while (pos)
    {
        pSiteInfo = (ExSiteInfo*) m_Children.GetAt(pos);
        if (pSiteInfo->m_pSite == pChildSite)
        {
            res = TRUE;

	    if (prevPos == m_Children.GetHeadPosition())
	    {
		prevPos = NULL;
	    }
		;
	    break;
        }
	
	prevPos = pos;
	pos = m_Children.GetNextPosition(pos);	
    }

    return res;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::AttachWatcher(IRMASiteWatcher* /*IN*/ pWatcher)
{
    if (m_pWatcher) return PNR_UNEXPECTED;

    m_pWatcher = pWatcher;

    if (m_pWatcher)
    {
	m_pWatcher->AddRef();
	m_pWatcher->AttachSite(this);
    }

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::DetachWatcher()
{
    if (!m_pWatcher) return PNR_UNEXPECTED;

    m_pWatcher->DetachSite();
    PN_RELEASE(m_pWatcher);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetSize(PNxSize size)
{
    HRESULT hres = PNR_OK;

    /* before we do anything, we give the SiteWatcher a chance to 
     * influence this operation.
     */
    if (m_pWatcher)
    {
	hres = m_pWatcher->ChangingSize(m_size, size);
    }
    
    if (PNR_OK == hres && size.cx != 0 && size.cy != 0)
    {
	m_size = size;

	// iterate child site list 
	IRMAPassiveSiteWatcher* pWatcher = NULL;
	LTRListNodePosition pos = m_PassiveSiteWatchers.GetHeadPosition();
	while (pos)
	{
	    pWatcher = (IRMAPassiveSiteWatcher*) m_PassiveSiteWatchers.GetAt(pos);
	    pWatcher->SizeChanged(&m_size);

	    pos = m_PassiveSiteWatchers.GetNextPosition(pos);	
	}
    }

    return hres;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetPosition(PNxPoint position)
{
    HRESULT hres = PNR_OK;

    /*
     * Before we do anything, we give the SiteWatcher a chance to 
     * influence this operation.
     */
    if (m_pWatcher)
    {
	hres = m_pWatcher->ChangingPosition(m_position, position);
    }
    
    if (PNR_OK == hres)
    {
	/* Record the position of posterity sake */
	m_position = position;

	// iterate child site list 
	IRMAPassiveSiteWatcher* pWatcher = NULL;
	LTRListNodePosition pos = m_PassiveSiteWatchers.GetHeadPosition();
	while (pos)
	{
	    pWatcher = (IRMAPassiveSiteWatcher*) m_PassiveSiteWatchers.GetAt(pos);
	    pWatcher->PositionChanged(&m_position);

	    pos = m_PassiveSiteWatchers.GetNextPosition(pos);	
	}
    }

    return hres;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetSize(REF(PNxSize) size)
{
    size = m_size;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetPosition(REF(PNxPoint) position)
{
    position = m_position;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::DamageRect(PNxRect rect)
{
    m_bDamaged = TRUE;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::DamageRegion(PNxRegion region)
{
    m_bDamaged = TRUE;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::ForceRedraw()
{
    // make sure we have a visible window and are not re-enterering and we have damage
    if (!m_bInRedraw && m_bDamaged && m_bIsVisible)
    {
	AddRef();

	m_bInRedraw = TRUE;

	/*
	 * set up the event structure to simulate an X "Expose" event
	 */
	PNxEvent event;
	memset(&event,0,sizeof(event));

#if defined(_UNIX)
	event.event = Expose;
#elif defined(_WINDOWS)
	event.event = WM_PAINT;
#elif defined(_MACINTOSH)
	event.event = updateEvt;
#endif

	/*
	 * call our handy helper routine that takes care of everything
	 * else for us.
	 */
	ForwardUpdateEvent(&event);

	m_bInRedraw = FALSE;
	m_bDamaged = FALSE;

	Release();
    }

    return PNR_OK;
}

//-----------------------------------------------------------------------------
void CLTWindowlessSite::ForwardUpdateEvent(PNxEvent* pEvent)
{
    BOOL bHandled = FALSE;

    AddRef();

    /* 
     * send the basic updateEvt event to the user
     */
    m_pUser->HandleEvent(pEvent);

    /*
     * If the user doesn't handle the standard update event then
     * send them the cross platform RMA_SURFACE_UPDATE event don't
     * damage the original event structure
     */
    if (!pEvent->handled && m_pUser)
    {
    	PNxEvent event;
	memset(&event, 0, sizeof(PNxEvent));
	event.event = RMA_SURFACE_UPDATE;
	event.param1 = m_pVideoSurface;
	event.result = 0;
	event.handled = FALSE;

	m_pUser->HandleEvent(&event);

	bHandled = event.handled;
    }
    else
    {
    	bHandled = TRUE;
    }

    Release();
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetPropertyULONG32
(
    const char*      pPropertyName,
    ULONG32          uPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->SetPropertyULONG32
		    (
			pPropertyName,
			uPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetPropertyULONG32
(
    const char*      pPropertyName,
    REF(ULONG32)     uPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetPropertyULONG32
		    (
			pPropertyName,
			uPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetFirstPropertyULONG32
(
				    REF(const char*) pPropertyName,
				    REF(ULONG32)     uPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetPropertyULONG32
		    (
			pPropertyName,
			uPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetNextPropertyULONG32
(
				    REF(const char*) pPropertyName,
				    REF(ULONG32)     uPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetNextPropertyULONG32
		    (
			pPropertyName,
			uPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetPropertyBuffer
(
    const char*      pPropertyName,
    IRMABuffer*      pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->SetPropertyBuffer
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetPropertyBuffer
(
    const char*      pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetPropertyBuffer
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetFirstPropertyBuffer
(
    REF(const char*) pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetFirstPropertyBuffer
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetNextPropertyBuffer
(
    REF(const char*) pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetNextPropertyBuffer
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetPropertyCString
(
    const char*      pPropertyName,
    IRMABuffer*      pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->SetPropertyCString
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetPropertyCString
(
    const char*      pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetPropertyCString
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetFirstPropertyCString
(
    REF(const char*) pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetFirstPropertyCString
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetNextPropertyCString
(
    REF(const char*) pPropertyName,
    REF(IRMABuffer*) pPropertyValue
)
{
    if (!m_pValues) return PNR_UNEXPECTED;
    return m_pValues->GetNextPropertyCString
		    (
			pPropertyName,
			pPropertyValue
		    );
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::UpdateSiteWindow
(
    PNxWindow* /*IN*/ pWindow
)
{
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::ShowSite
(
    BOOL    bShow
)
{
    m_bIsVisible = bShow;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(BOOL) CLTWindowlessSite::IsSiteVisible()
{
    BOOL bIsVisible = m_bIsVisible;
    if (m_pParentSite)
    {
        bIsVisible &= m_pParentSite->IsSiteVisible();
    }
    
    return bIsVisible;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetZOrder(INT32 lZOrder)
{
    if(!m_pParentSite) return PNR_UNEXPECTED;
    m_lZOrder = lZOrder;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetZOrder(REF(INT32) lZOrder)
{
    if(!m_pParentSite) return PNR_UNEXPECTED;
    lZOrder = m_lZOrder;
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::MoveSiteToTop()
{
    if(!m_pParentSite) return PNR_UNEXPECTED;
    return PNR_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::GetVideoSurface(REF(IRMAVideoSurface*) pSurface)
{
    PN_RESULT res = PNR_FAIL;

    if (m_pVideoSurface)
    {
	res = m_pVideoSurface->QueryInterface(IID_IRMAVideoSurface, 
					   (void**)&pSurface);
    }

    return res;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTWindowlessSite::GetNumberOfChildSites()
{
    return (UINT32)m_Children.Count();
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::AddPassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher)
{
    pWatcher->AddRef();
    m_PassiveSiteWatchers.Add(pWatcher);
    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::RemovePassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher)
{
    // iterate child site list 
    IRMAPassiveSiteWatcher* pThisWatcher = NULL;
    LTRListNodePosition pos = m_PassiveSiteWatchers.GetHeadPosition();
    LTRListNodePosition prevPos = pos;
    while (pos)
    {
	pThisWatcher = (IRMAPassiveSiteWatcher*) m_PassiveSiteWatchers.GetAt(pos);
	if(pWatcher == pThisWatcher)
	{
	    PN_RELEASE(pWatcher);

	    if (prevPos == m_PassiveSiteWatchers.GetHeadPosition())
	    {
		m_PassiveSiteWatchers.RemoveHead();
	    }
	    else
	    {
		m_PassiveSiteWatchers.RemoveAfter(prevPos);
	    }

	    break;
	}

	prevPos = pos;
	pos = m_PassiveSiteWatchers.GetNextPosition(pos);	
    }

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::EventOccurred(PNxEvent* /*IN*/ pEvent)
{
    return PNR_OK;
}

//-----------------------------------------------------------------------------
PNxWindow* CLTWindowlessSite::GetParentWindow()
{
    return NULL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTWindowlessSite::SetCursor(PNxCursor cursor, REF(PNxCursor) oldCursor)
{
    return PNR_NOTIMPL;
}

//-----------------------------------------------------------------------------
ExSiteInfo::ExSiteInfo(IRMASite* pSite, void* pThisPointer)
{
    m_pSite = pSite;
    m_pSite->AddRef();

    pSite->QueryInterface(IID_IRMASiteWindowless, (void**) &m_pSiteWindowless);
    pSite->QueryInterface(IID_IRMASite2, (void**) &m_pSite2);
};

//-----------------------------------------------------------------------------
ExSiteInfo::~ExSiteInfo() 
{
    PN_RELEASE(m_pSite);
    PN_RELEASE(m_pSiteWindowless);
    PN_RELEASE(m_pSite2);
}
#endif // LITHTECH_ESD