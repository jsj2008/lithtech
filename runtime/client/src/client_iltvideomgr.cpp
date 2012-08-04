#include "bdefs.h"

#include "iltvideomgr.h"
#include "clientmgr.h"
#include "sysvideo.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);



//
//Helper Macros
//
#define CHECK_VIDEOMGR(fnName) \
    if (!g_pClientMgr->m_pVideoMgr) { \
        RETURN_ERROR(2, fnName, LT_NOTINITIALIZED); }

#define CHECK_VIDEOMGR_AND_HVIDEO(fnName) \
    if (!g_pClientMgr->m_pVideoMgr) { \
        RETURN_ERROR(2, fnName, LT_NOTINITIALIZED); }\
    else if (!hVideo || g_pClientMgr->m_pVideoMgr->m_Videos.FindElement(hVideo) == BAD_INDEX) {\
        RETURN_ERROR(2, fnName, LT_INVALIDPARAMS); }

//
//Our implementation class for the ILTVideoMgr game interface.
//
class CLTVideoMgr : public ILTVideoMgr {
public:
    declare_interface(CLTVideoMgr);

    LTRESULT StartOnScreenVideo(const char *pFilename, uint32 flags, HVIDEO &hVideo);
    LTRESULT UpdateVideo(HVIDEO hVideo);
    LTRESULT GetVideoStatus(HVIDEO hVideo);
    LTRESULT StopVideo(HVIDEO hVideo);
};

//instantiate our implementation class
define_interface(CLTVideoMgr, ILTVideoMgr);

LTRESULT CLTVideoMgr::StartOnScreenVideo(const char *pFilename, uint32 flags, HVIDEO &hVideo) {
    CHECK_VIDEOMGR(CLTVideoMgr::StartOnScreenVideo);
    CHECK_PARAMS(pFilename, CLTVideoMgr::StartOnScreenVideo);

    return g_pClientMgr->m_pVideoMgr->CreateScreenVideo(pFilename, flags, hVideo);
}

LTRESULT CLTVideoMgr::UpdateVideo(HVIDEO hVideo) {
    CHECK_VIDEOMGR_AND_HVIDEO(CLTVideoMgr::UpdateVideo);

    return hVideo->DrawVideo();
}

LTRESULT CLTVideoMgr::GetVideoStatus(HVIDEO hVideo) {
    CHECK_VIDEOMGR_AND_HVIDEO(CLTVideoMgr::UpdateVideo);

    return hVideo->GetVideoStatus();
}

LTRESULT CLTVideoMgr::StopVideo(HVIDEO hVideo) {
    CHECK_VIDEOMGR_AND_HVIDEO(CLTVideoMgr::UpdateVideo);

    hVideo->Release();
    return LT_OK;
}

