#include "bdefs.h"

#include "iltcursor.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);


//
//Internal implementation class for ILTCursorInst.
//

class CLTCursorInst : public ILTCursorInst {
public:

    CLTCursorInst() { m_hCursor = LTNULL; };
    ~CLTCursorInst() {};

    virtual LTRESULT IsValid() { return (m_hCursor ? LT_YES : LT_NO); };
    HCURSOR GetCursor() { return m_hCursor; };
    virtual void SetData(const void *pData) { m_hCursor = (HCURSOR)(pData); };
    virtual void *GetData() { return (void *)m_hCursor; };

    HCURSOR m_hCursor;
};


//
//Our implementation class for the ILTCursor interface.
//

class CLTCursor : public ILTCursor {
public:
    declare_interface(CLTCursor);

    CLTCursor() {m_hCurrentCursor = LTNULL;  m_eCursorMode = CM_None; }

    // Enable/disable hardware cursor.
    virtual LTRESULT    SetCursorMode(CursorMode cMode, bool bForce = false);

    // Get current cursor mode.  Always returns LT_OK and always fills in cMode.
    virtual LTRESULT    GetCursorMode(CursorMode &cMode);

    // Returns LT_YES if a hardware cursor can be used, LT_NO otherwise.
    // Since we can't detect this, just return LT_YES for now.
    virtual LTRESULT IsCursorModeAvailable(CursorMode cMode);

    // Set the current hardware cursor bitmap.  The bitmap comes from cshell.dll.
    virtual LTRESULT LoadCursorBitmapResource(const char *pName, HLTCURSOR &hCursor);

    // Free a cursor.
    virtual LTRESULT FreeCursor(const HLTCURSOR hCursor);

    // Set the current cursor.
    virtual LTRESULT SetCursor(HLTCURSOR hCursor);

    // Check if an HLTCURSOR is a valid one; returns LT_YES or LT_NO
    virtual LTRESULT IsValidCursor(HLTCURSOR hCursor);

    // Refresh the cursor
    virtual LTRESULT RefreshCursor();

protected:

    LTRESULT        PreSetMode(CursorMode eNewMode);
    LTRESULT        PostSetMode(CursorMode eOldMode);

    CursorMode      m_eCursorMode;
    HLTCURSOR       m_hCurrentCursor;
};

//instantiate our implementation class.
define_interface(CLTCursor, ILTCursor);



LTRESULT CLTCursor::PreSetMode(CursorMode eNewMode)
{
    int temp;

    temp = 0;

    switch(eNewMode)
    {
        case CM_Hardware:
            do {
                temp = ShowCursor(LTTRUE);
            } while (temp < 0);
            break;
        case CM_None:
            do {
                temp = ShowCursor(LTFALSE);
            } while (temp >= 0);
            break;
        default:
            break;
    }

    return LT_OK;
}

LTRESULT CLTCursor::PostSetMode(CursorMode eOldMode)
{
    switch(eOldMode)
    {
        case CM_Hardware:
            break;
        case CM_None:
            break;
        default:
            break;
    }

    return LT_OK;
}

LTRESULT CLTCursor::SetCursorMode(CursorMode cMode, bool bForce)
{
    CursorMode eOldMode;

    // Saaaaaanity check
    if (!IsCursorModeAvailable(cMode)) {
        return LT_UNSUPPORTED;
    }

    // If we're already in this mode, let's just save some cycles, shall we?
    if (cMode == m_eCursorMode && !bForce) {
        return LT_OK;
    }

    /* In the future, as more things (such as software cursor support) are added,
     * this routine will become more expensive, likely. */

    PreSetMode(cMode);
    eOldMode = m_eCursorMode;
    m_eCursorMode = cMode;
    PostSetMode(eOldMode);

    return LT_OK;
}

LTRESULT CLTCursor::IsValidCursor(HLTCURSOR hCursor)
{
    return hCursor->IsValid();
}

LTRESULT CLTCursor::SetCursor(HLTCURSOR hCursor)
{
    if (!hCursor->IsValid())
        return LT_INVALIDPARAMS;

    ::SetCursor((HCURSOR)hCursor->GetData());

    return LT_OK;
}

LTRESULT CLTCursor::GetCursorMode(CursorMode &cMode)
{
    cMode = m_eCursorMode;

    return LT_OK;
}

LTRESULT CLTCursor::FreeCursor(const HLTCURSOR hCursor)
{
    /* We don't really do anything here, since we're not presently creating the cursor
     * from scratch. */

    delete ((CLTCursorInst*)hCursor);

    return LT_OK;
}

LTRESULT CLTCursor::IsCursorModeAvailable(CursorMode cMode)
{
    /* Eventually there may be more checks here */

    return LT_YES;
}

LTRESULT CLTCursor::LoadCursorBitmapResource(const char *pName, HLTCURSOR &hCursor)
{
    HINSTANCE hInst;
    LTRESULT dRes;
    HCURSOR hWinCursor;
    HLTCURSOR hNew;

    if ((dRes = ilt_client->GetEngineHook("cres_hinstance",(void **)&hInst)) != LT_OK)
        return dRes;

    hWinCursor = (HCURSOR)(::LoadImage(hInst, pName, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR));

    if (!hWinCursor)
        return LT_MISSINGCURSORRESOURCE;

    LT_MEM_TRACK_ALLOC(hNew = new CLTCursorInst(),LT_MEM_TYPE_MISC);

    hNew->SetData((void *)hWinCursor);
    hCursor = hNew;

    return LT_OK;
}

LTRESULT CLTCursor::RefreshCursor()
{
    int temp;

    temp = 0;

    switch(m_eCursorMode)
    {
        case CM_Hardware:
            do {
                temp = ShowCursor(LTTRUE);
            } while (temp < 0);
            break;
        case CM_None:
            do {
                temp = ShowCursor(LTFALSE);
            } while (temp >= 0);
            break;
        default:
            break;
    }

    return LT_OK;
}
