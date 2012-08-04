#include "bdefs.h"

#include "soundmgr.h"
#include "de_objects.h"
#include "clientmgr.h" 
#include "clientshell.h"
#include "wave.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);


//instantiate our ILTClientSoundMgr (and ILTSoundMgr) interface implementation
define_interface(CSoundMgr, ILTClientSoundMgr);
implements_also(CSoundMgr, Default, ILTSoundMgr, Client)



#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)

//#define USETHREAD
//#define MODPROVIDER

extern int32 g_CV_ForceNoSound;

#define EAX_ENVIRONMENT_GENERIC     LS_ROOM_GENERIC
#define EAX_ENVIRONMENT_COUNT       LS_ROOM_TYPE_COUNT

#define AIL_LOCK_PROTECTION         LS_PREF_LOCK_PROTECTION
#define AIL_ENABLE_MMX_SUPPORT      LS_PREF_ENABLE_MMX
#define DIG_MIXER_CHANNELS          LS_PREF_MIXER_CHANNELS
#define DIG_USE_WAVEOUT             LS_PREF_USE_WAVEOUT
#define DIG_REVERB_BUFFER_SIZE      LS_PREF_REVERB_BUFFER_SIZE

#define HPROENUM_FIRST              LS_PROENUM_FIRST
#define M3D_NOERR                   LS_OK

#define DEFAULT_SOUND_TYPE			0

typedef LHPROENUM                   HPROENUM;

typedef LHSAMPLE                    HSAMPLE;
typedef LH3DPOBJECT                 H3DPOBJECT;
typedef LH3DSAMPLE                  H3DSAMPLE;
typedef LHDIGDRIVER                 HDIGDRIVER;
typedef LHSTREAM                    HSTREAM;

#ifdef USE_EAX20_HARDWARE_FILTERS

// filter stuff
typedef struct
{
	int FilterID;
	char* FilterName;
} FilterDesc;

FilterDesc FilterList[] =
{
	FilterReverb,		"Reverb Filter"
};

// one of each type of sound filter, we'll only need one to work
// with at any given time
static LTFILTERREVERB g_FilterReverb;

#endif


ClientGlob* GetClientGlob()
{
    extern ClientGlob g_ClientGlob;
    return &g_ClientGlob;
}


static ILTSoundSys* g_pSoundSys = LTNULL;

ILTSoundSys* SoundSys(bool bTerminate)
{
    if (bTerminate && g_pSoundSys != LTNULL)
    {
        g_pSoundSys->Term();
        g_pSoundSys = LTNULL;
    }

    else if (!bTerminate && g_pSoundSys == LTNULL)
    {
        ClientGlob* pClientGlob = GetClientGlob();

        if (pClientGlob != LTNULL && pClientGlob->m_acSoundDriverName[0] != 0)
        {
           g_pSoundSys = ILTSoundFactory::GetSoundFactory()->MakeSoundSystem(&pClientGlob->m_acSoundDriverName[0]);
           if (g_pSoundSys != LTNULL && !g_pSoundSys->Init())
           {
                g_pSoundSys->Term();
                g_pSoundSys = LTNULL;
            }
        }

		  // try default driver 
        if (g_pSoundSys == LTNULL)
        {
				// grab the default snddrv.dll NOTE: does't call Init on it. 
				g_pSoundSys = ILTSoundFactory::GetSoundFactory()->MakeSoundSystem(LTNULL);


		  }

        //ASSERT(g_pSoundSys != LTNULL);
    }

    return g_pSoundSys;
}

#define GetSoundSys SoundSys
//#define SOUND_CALL(oldName, newName)  GetSoundSys()->##newName


void Do_AIL_startup() {
#ifndef __XBOX
#ifndef USE_ABSTRACT_SOUND_INTERFACES
    AIL_startup();
#endif
#endif
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::CSoundMgr()
//
//  Constructor
//
//----------------------------------------------------------------------------------------------

//a pointer to the instance of CSoundMgr that gets allocated.
static CSoundMgr *soundmgr_impl = NULL;

CSoundMgr *GetClientILTSoundMgrImpl() {
    return soundmgr_impl;
}

CSoundMgr::CSoundMgr()
{
    soundmgr_impl = this;

    m_bInited = false;
	m_bFailedInit = false;

    m_hDigDriver = LTNULL;

    memset(&m_InitSoundInfo, 0, sizeof(InitSoundInfo));

    m_p3DProviderList = LTNULL;

    memset(&m_3DProvider, 0, sizeof(CProvider));
    m_nNum3DSamples = 0;
    m_nMax3DSamples = 0;
    m_p3DSampleList = LTNULL;
    dl_InitList(&m_3DFreeSampleList);

    m_nNumSWSamples = 0;
    m_nMaxSWSamples = 0;
    m_pSWSampleList = LTNULL;
    dl_InitList(&m_SWFreeSampleList);

    dl_InitList(&m_SoundBufferList);
    m_SoundBufferBank.Term();
    m_LocalSoundInstanceBank.Term();
    m_AmbientSoundInstanceBank.Term();
    m_3DSoundInstanceBank.Term();

    m_h3DListener = LTNULL;
    m_vListenerPosition.Init();
    m_vLastListenerPosition.Init();
    m_vListenerVelocity.Init();
    m_vListenerForward.Init(0.0f, 0.0f, 1.0f);
    m_vListenerRight.Init(1.0f, 0.0f, 0.0f);
    m_vListenerUp.Init(0.0f, 1.0f, 0.0f);

    m_bListenerInClient = true;

    m_nNumSoundsHeard = 0;

    memset(m_SoundInstanceList, 0, SOUNDMGR_MAXSOUNDINSTANCES * sizeof(CSoundInstance *));
    m_dwNumSoundInstances = 0;

    m_bConvert16to8 = false;

    m_fDistanceFactor = 1.0f;

    m_bSWReverb = false;
    m_b3DReverb = false;
    m_fReverbVolume = 0.0f;

    m_dwReverbAcoustics = 0;//EAX_ENVIRONMENT_GENERIC; -- MIKCOP

    m_fReverbReflectTime = 0.0f;
    m_fReverbDecayTime = 0.1f;
    m_fReverbDamping = 1.0f;

    m_bDigitalHandleReleased = false;
    m_bReacquireDigitalHandle = false;

    m_dwCurTime = 0;
    m_dwCommitTime = 0;
    m_bCommitChanges = false;

    m_bValid = false;
    m_bEnabled = false;

	for ( int i = 0; i <= MAX_SOUND_VOLUME_CLASSES; ++i )
	{
		m_afSoundClassMultipliers[i] = 1.0f;
		m_abUseGlobalVolume[i] = true;
	}

	m_FilterData.bUseFilter = false;
	m_FilterData.pSoundFilter = NULL;
	m_FilterData.uiFilterType = NULL_FILTER;

    memset(&m_PrimaryBufferWaveFormat, 0, sizeof(m_PrimaryBufferWaveFormat));
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::~CSoundMgr()
//
//  Destructor
//
//----------------------------------------------------------------------------------------------
CSoundMgr::~CSoundMgr()
{
    Term();
    ReleaseProviderList(m_p3DProviderList);
    m_p3DProviderList = LTNULL;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Init()
//
//  Initializes the sound system based on the CSoundInit passed in.
//
//----------------------------------------------------------------------------------------------

LTRESULT CSoundMgr::Init(InitSoundInfo &soundInit)
{
    m_SoundUpdatePacket.Clear();

    uint32 nSamples, dwIndex;
    bool bRemoveSounds;
    CSoundInstance *pSoundInstance;
    CSoundBuffer *pSoundBuffer;
    LTLink *pCur;

	// assume we're going to fail init, so if we do we clean up O.K.
	m_bFailedInit = true;

    // For NT debugging..
    if (g_CV_ForceNoSound)
    {
        return LT_OK;
    }

    // Copy the init structure in case we have to do an auto term/init later.
    m_InitSoundInfo = soundInit;
    soundInit.m_dwResults = 0;

    bRemoveSounds = (soundInit.m_dwFlags & INITSOUNDINFOFLAG_RELOADSOUNDS) ? false : true;
    if (!m_dwNumSoundInstances)
        bRemoveSounds = true;

    // Unload the sounds temporarily if we need to keep them around
    if (!bRemoveSounds)
    {

        for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
        {
            pSoundInstance = m_SoundInstanceList[dwIndex];
            ASSERT(pSoundInstance);
            if (!pSoundInstance)
                continue;

            if (pSoundInstance->Unload() != LT_OK)
            {
                RemoveInstance(*pSoundInstance);
                dwIndex--;
            }
        }

        pCur = m_SoundBufferList.m_Head.m_pNext;
        while (pCur != &m_SoundBufferList.m_Head)
        {
            pSoundBuffer = (CSoundBuffer *)pCur->m_pData;
            pCur = pCur->m_pNext;
            ASSERT(pSoundBuffer);
            if (!pSoundBuffer)
                continue;

            if (pSoundBuffer->Unload() != LT_OK)
            {
                RemoveBuffer(*pSoundBuffer);
            }
        }

    }

    // Start fresh

    if (m_bInited)
        Term(bRemoveSounds != 0);

    m_bValid = true;
    m_bEnabled = true;

    // Initialize the Sound System

	// In case of failure, be sure to return gracefully instead of bombing
	// on an access violation
	if(!GetSoundSys(false))
	{
		return LT_ERROR;
	}

    if ( g_pSoundSys->Startup() != LT_OK )
	{
        Term();
        return LT_UNABLETOINITSOUND;
	}

    // Set the maximum number of sw channels
	int nNumSWVoices = soundInit.m_nNumSWVoices;
    nNumSWVoices = LTMIN(nNumSWVoices, SOUNDMGR_MAXSOUNDINSTANCES);

#if 0
    SOUND_CALL(AIL_set_preference, SetPreference) (DIG_MIXER_CHANNELS, soundInit.m_nNumSWVoices);
#endif

	// store the primary buffer format
    m_PrimaryBufferWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    m_PrimaryBufferWaveFormat.nChannels = 2;
    m_PrimaryBufferWaveFormat.nSamplesPerSec = soundInit.m_nSampleRate;
    m_PrimaryBufferWaveFormat.wBitsPerSample = soundInit.m_nBitsPerSample;
    m_PrimaryBufferWaveFormat.nBlockAlign = (m_PrimaryBufferWaveFormat.nChannels * m_PrimaryBufferWaveFormat.wBitsPerSample) >> 3;
    m_PrimaryBufferWaveFormat.nAvgBytesPerSec = m_PrimaryBufferWaveFormat.nSamplesPerSec * m_PrimaryBufferWaveFormat.nBlockAlign;

    m_bConvert16to8 = (soundInit.m_dwFlags & INITSOUNDINFOFLAG_CONVERT16TO8) ? true : false;

    // Open the wave device
    if ( g_pSoundSys->WaveOutOpen(&m_hDigDriver, 0, WAVE_MAPPER, ( WAVEFORMAT* )&m_PrimaryBufferWaveFormat) != LS_OK )
    {
        Term();
        return LT_UNABLETOINITSOUND;
    }

    m_bDigitalHandleReleased = false;
    m_bReacquireDigitalHandle = false;

    Get3DProviderLists(m_p3DProviderList, false, soundInit.m_nNum3DVoices);

    // Choose hw and 3d providers.
    if (soundInit.m_sz3DProvider[0] && Set3DProvider(soundInit.m_sz3DProvider) != LT_OK)
    {
		// we asked for a mode card doesn't support, can happen if user 
		// has changed cards since last startup
		LTStrCpy(soundInit.m_sz3DProvider, "DirectSound Default", sizeof(soundInit.m_sz3DProvider));
		m_3DProvider.m_hProvider = NULL;
    }

    // Set the maximum number of samples for 3d
    if (m_3DProvider.m_hProvider)
    {
        g_pSoundSys->Get3DProviderAttribute(m_3DProvider.m_hProvider, "Max samples", &nSamples);

        nSamples = LTMIN(nSamples, 255);
        m_nMax3DSamples = LTMIN((uint8)nSamples, SOUNDMGR_MAXSOUNDINSTANCES);
        m_nMax3DSamples = LTMIN(m_nMax3DSamples, soundInit.m_nNum3DVoices);
		
		// reserve two samples 1 for sound filtering (needed for EAX) and primary buffer
		if ( m_nMax3DSamples > 2 )
			m_nMax3DSamples -= 2;
    }

    // Set the maximum number of samples for sw

#if 0
    nSamples = SOUND_CALL(AIL_get_preference, GetPreference) (DIG_MIXER_CHANNELS);
#endif
    nSamples = nNumSWVoices;

    if (nSamples > 4)
        nSamples -= 4;
    m_nMaxSWSamples = (uint8)LTMIN(nNumSWVoices, nSamples);
    m_nMaxSWSamples = (uint8)LTMIN(m_nMaxSWSamples, SOUNDMGR_MAXSOUNDINSTANCES - m_nMax3DSamples);

    // Precreate all the samples
    Create3DSamples();
    CreateSWSamples();

    // Create the 3d listener
    if (m_3DProvider.m_hProvider)
    {
        m_h3DListener = g_pSoundSys->Open3DListener(m_3DProvider.m_hProvider);

        if (!m_h3DListener)
        {
            Term();
            return LT_NO3DSOUNDPROVIDER;
        }

        g_pSoundSys->Set3DPosition(m_h3DListener, 0.0f, 0.0f, 0.0f);
        g_pSoundSys->Set3DVelocityVector(m_h3DListener, 0.0f, 0.0f, 0.0f);
        g_pSoundSys->Set3DOrientation(m_h3DListener, m_vListenerForward.x, m_vListenerForward.y, m_vListenerForward.z,
            m_vListenerUp.x, m_vListenerUp.y, m_vListenerUp.z);

	    g_pSoundSys->SetListenerDoppler( m_h3DListener, soundInit.m_fDopplerFactor );


        // Check for eax support
#ifdef USE_EAX20_HARDWARE_FILTERS
		if ( g_pSoundSys->SupportsEAX20Filter() )
		{
			m_b3DReverb = true;
		}
#endif
    }

    if (bRemoveSounds)
    {
        // Initialize the lists and banks
        m_SoundBufferBank.Init(32, 32);
        dl_InitList(&m_SoundBufferList);
        m_LocalSoundInstanceBank.Init(16, 16);
        m_AmbientSoundInstanceBank.Init(16, 16);
        m_3DSoundInstanceBank.Init(32, 32);

        memset(m_SoundInstanceList, 0, SOUNDMGR_MAXSOUNDINSTANCES * sizeof(CSoundInstance *));
        m_dwNumSoundInstances = 0;
    }
    // Reload the sounds
    else
    {
        pCur = m_SoundBufferList.m_Head.m_pNext;
        while (pCur != &m_SoundBufferList.m_Head)
        {
            pSoundBuffer = (CSoundBuffer *)pCur->m_pData;
            pCur = pCur->m_pNext;
            ASSERT(pSoundBuffer);
            if (!pSoundBuffer)
                continue;

            if (pSoundBuffer->Reload() != LT_OK)
                RemoveBuffer(*pSoundBuffer);
        }

        for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
        {
            pSoundInstance = m_SoundInstanceList[dwIndex];
            ASSERT(pSoundInstance);
            if (!pSoundInstance)
                continue;

            if (pSoundInstance->Reload() != LT_OK)
            {
                RemoveInstance(*pSoundInstance);
                dwIndex--;
            }
        }

    }
    m_nNumSoundsHeard = 0;

    if (soundInit.m_fDistanceFactor > 0.0f)
        m_fDistanceFactor = soundInit.m_fDistanceFactor;

    soundInit.m_nVolume = LTMIN(soundInit.m_nVolume, 100);
    SetVolume((uint8)soundInit.m_nVolume);

    m_dwCurTime = g_pSoundSys->MsCount();

	g_pSoundSys->CommitDeferred();

    m_dwCommitTime = 0;
    m_bCommitChanges = false;
    m_bInited = true;
	m_bFailedInit = false;

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Term()
//
//  Terminates the sound system.
//
//----------------------------------------------------------------------------------------------
void CSoundMgr::Term(bool bRemoveSounds)
{
    CSoundBuffer *pSoundBuffer;
    CSoundInstance *pSoundInstance;
    LTLink *pCur, *pNext;
    uint32 dwIndex;

    if (!m_bInited && !m_bFailedInit)
        return;

    if (bRemoveSounds)
    {
        // Remove all the sound instances
        for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
        {
            pSoundInstance = m_SoundInstanceList[dwIndex];
            if (pSoundInstance)
            {
                if (pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_GETHANDLE)
                {
                    if (g_DebugLevel >= 2)
                    {
                        pSoundBuffer = pSoundInstance->GetSoundBuffer();
                        if (pSoundBuffer)
                            dsi_PrintToConsole("Unfreed sound handle %s", pSoundBuffer->GetFileIdent()->m_Filename);
                        else
                            dsi_PrintToConsole("Unfreed sound handle 0x%x", pSoundInstance);
                    }
                }

                RemoveInstance(*pSoundInstance);
                dwIndex--;
            }
        }
        m_dwNumSoundInstances = 0;

        m_LocalSoundInstanceBank.Term();
        m_AmbientSoundInstanceBank.Term();
        m_3DSoundInstanceBank.Term();

        // Remove all the sound buffers
        pCur = m_SoundBufferList.m_Head.m_pNext;
        while (pCur != &m_SoundBufferList.m_Head)
        {
            pSoundBuffer = (CSoundBuffer *)pCur->m_pData;
            pNext = pCur->m_pNext;
            ASSERT(pSoundBuffer);
            if (!pSoundBuffer)
                continue;

            RemoveBuffer(*pSoundBuffer);

            pCur = pNext;
        }
        dl_InitList(&m_SoundBufferList);
        m_SoundBufferBank.Term();
    }

    // Remove the samples
    Remove3DSamples();
    RemoveSWSamples();

    // Release the 3d listener
    if (m_h3DListener && g_pSoundSys)
    {
        g_pSoundSys->Close3DListener(m_h3DListener);
        m_h3DListener = LTNULL;
    }

    // Release the providers
    if (m_3DProvider.m_hProvider && g_pSoundSys)
    {
        g_pSoundSys->Close3DProvider(m_3DProvider.m_hProvider);
        m_3DProvider.m_hProvider = LTNULL;
    }

    // Release the waveout device
    if (m_hDigDriver)
    {
        g_pSoundSys->WaveOutClose(m_hDigDriver);
        m_hDigDriver = LTNULL;
        m_bDigitalHandleReleased = false;
        m_bReacquireDigitalHandle = false;
    }

	if (g_pSoundSys)
	{
		g_pSoundSys->Shutdown();
	}
	g_pSoundSys = LTNULL;

    m_bValid = false;
    m_bEnabled = false;
    m_bInited = false;
	m_bFailedInit = false;

}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::ReleaseProviderList()
//
//  Deletes the provider list created by EnumerateProviders
//
//----------------------------------------------------------------------------------------------
void CSoundMgr::ReleaseProviderList(CProvider *pProvider)
{
    CProvider *pNextProvider;

    while (pProvider)
    {
        pNextProvider = pProvider->m_pNextProvider;
        delete pProvider;
        pProvider = pNextProvider;
    }
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::EnumerateAllProviders()
//
//  Enumerates all the providers and puts them into a list.
//
//----------------------------------------------------------------------------------------------
CProvider *CSoundMgr::EnumerateAllProviders(bool bVerifyOpens)
{

    HPROENUM next;
    LHPROVIDER hProvider;

    char *szName;
    CProvider *pProvider, *pProviderList;

    pProviderList = LTNULL;

    next = HPROENUM_FIRST;

	if (!g_pSoundSys)
	{
		return LTNULL;
	}

    while ( g_pSoundSys->Enumerate3DProviders(&next, &hProvider, &szName) )
    {
		 // got a valid provider
		 if ( hProvider != LTNULL )
		 {
	         if (bVerifyOpens)
		     {
			     // Make sure the provider works
	             if ( !g_pSoundSys->Open3DProvider(hProvider) )
		             continue;

			     g_pSoundSys->Close3DProvider(hProvider);
	         }
		
	         LT_MEM_TRACK_ALLOC(pProvider = new CProvider,LT_MEM_TYPE_SOUND);
		     pProvider->m_pNextProvider = pProviderList;
			 pProviderList = pProvider;
	         pProviderList->m_hProvider = hProvider;
		     LTStrCpy(pProviderList->m_szProviderName, szName, sizeof(pProviderList->m_szProviderName));
		 }
    }

    return pProviderList;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetProviderLists()
//
//  Enumerates all providers and puts them into a list pointers provided.  Opens and closes the provider
//  to make sure it works.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::Get3DProviderLists(CProvider *&p3DProviderList, bool bVerifyOpens, uint32 uiMax3DVoices)
{

    HPROENUM next;
	LHPROVIDER hProvider;

    char *szName;
    CProvider *pProvider;

    uint32 dwCaps;
    uint32 dwProviderID, dwResult;

    // Check if we don't already have the lists
    if (!m_p3DProviderList)
    {
        // This could be called before sound is initialized
        if (!m_bValid)
        {
			SoundSys( false );

			if (g_pSoundSys)
			{
				g_pSoundSys->Startup();
			}
			else
			{
				return LT_ERROR;
			}
        }

        next = HPROENUM_FIRST;

		g_pSoundSys->Set3DProviderMinBuffers( uiMax3DVoices );
        while ( g_pSoundSys->Enumerate3DProviders(&next, &hProvider, &szName) )
        {
			// if we got a valid provider
			if ( hProvider )
			{
				dwCaps = 0;
		        dwProviderID = hProvider;

		        if (bVerifyOpens)
			    {
				    // Make sure the provider works
					if ( !g_pSoundSys->Open3DProvider(hProvider) )
	                {
						dwProviderID = SOUND3DPROVIDERID_NONE;
		                continue;
			        }

					// Check for eax support
					g_pSoundSys->Get3DProviderAttribute(hProvider, "EAX environment selection", &dwResult);
	                if (dwResult != -1)
		                dwCaps |= SOUND3DPROVIDER_CAPS_REVERB;

			        g_pSoundSys->Close3DProvider(hProvider);
	            }

		        LT_MEM_TRACK_ALLOC(pProvider = new CProvider,LT_MEM_TYPE_SOUND);
			    pProvider->m_pNextProvider = m_p3DProviderList;
				m_p3DProviderList = pProvider;
	            m_p3DProviderList->m_dwCaps = dwCaps;
		        m_p3DProviderList->m_dwProviderID = dwProviderID;
			    pProvider->m_hProvider = hProvider;
				LTStrCpy(pProvider->m_szProviderName, szName, sizeof(pProvider->m_szProviderName));
			}
        }

        if (!m_bValid)
        {
			if (g_pSoundSys)
			{
				g_pSoundSys->Shutdown();
			}
		}
    }

    p3DProviderList = m_p3DProviderList;

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Set3DProvider()
//
//  Chooses the 3d provider.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::Set3DProvider(char *psz3DProviderName)
{
    CProvider *pProvider;

    if (!m_bValid)
        return LT_ERROR;

    m_3DProvider.m_hProvider = LTNULL;
    pProvider = m_p3DProviderList;
    while (pProvider)
    {
        // Check if found provider
        if (strcmp(pProvider->m_szProviderName, psz3DProviderName) == 0)
        {
            m_3DProvider = *pProvider;
            break;
        }

        pProvider = pProvider->m_pNextProvider;
    }

    if (!m_3DProvider.m_hProvider)
        return LT_ERROR;

	if (!g_pSoundSys)
		return LT_ERROR;

    if ( !g_pSoundSys->Open3DProvider(m_3DProvider.m_hProvider) )
    {
        m_3DProvider.m_hProvider = LTNULL;
        return LT_ERROR;
    }

    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Create3DSamples()
//
//  Creates the 3d samples.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::Create3DSamples()
{
    uint8 nSample;

    if (!m_bValid)
        return LT_ERROR;

    dl_InitList(&m_3DFreeSampleList);

    // Check if 3d is available
    if (!m_3DProvider.m_hProvider)
        return LT_ERROR;

    if (m_p3DSampleList)
    {
        Remove3DSamples();
        m_p3DSampleList = LTNULL;
    }

	if (!g_pSoundSys)
		return LT_ERROR;

    // Create an array of the 3D samples
    if (m_nMax3DSamples > 0)
    {
        LT_MEM_TRACK_ALLOC(m_p3DSampleList = new CSample[m_nMax3DSamples],LT_MEM_TYPE_SOUND);

        for (nSample = 0; nSample < m_nMax3DSamples; nSample++)
        {
            m_p3DSampleList[nSample].m_h3DSample = g_pSoundSys->Allocate3DSampleHandle(m_3DProvider.m_hProvider);
            if (!m_p3DSampleList[nSample].m_h3DSample)
                break;
            g_pSoundSys->Set3DUserData(m_p3DSampleList[nSample].m_h3DSample, SAMPLE_TYPE, SAMPLETYPE_3D);
            g_pSoundSys->Set3DUserData(m_p3DSampleList[nSample].m_h3DSample, SAMPLE_LISTITEM, (uint32)&m_p3DSampleList[nSample]);
            dl_AddHead(&m_3DFreeSampleList, &m_p3DSampleList[nSample].m_Link, &m_p3DSampleList[nSample]);
        }

        m_nNum3DSamples = nSample;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Remove3DSamples()
//
//  Removes the 3d samples.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::Remove3DSamples()
{
    CSoundInstance *pSoundInstance;
    uint8 nSample;

    // Check if 3d is available
    if (!m_3DProvider.m_hProvider || !g_pSoundSys)
        return LT_ERROR;

    // Remove the array of samples
    if (m_p3DSampleList && m_nNum3DSamples)
    {
        nSample = m_nNum3DSamples;
        while (nSample-- > 0)
        {
            // Get rid of any instance using this sample
            pSoundInstance = GetLink3DSampleSoundInstance(m_p3DSampleList[nSample].m_h3DSample);
            if (pSoundInstance)
            {
                pSoundInstance->Silence(true);
            }

            g_pSoundSys->Release3DSampleHandle(m_p3DSampleList[nSample].m_h3DSample);
        }

        delete[] m_p3DSampleList;
        m_p3DSampleList = LTNULL;
        m_nNum3DSamples = 0;
    }

    return LT_OK;
}



//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::CreateSWSamples()
//
//  Creates the software samples.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::CreateSWSamples()
{
    uint8 nSample;

    if (!m_bValid)
        return LT_ERROR;

    dl_InitList(&m_SWFreeSampleList);

    // Check if SW is available
    if (!m_hDigDriver)
        return LT_ERROR;

    if (m_pSWSampleList)
    {
        RemoveSWSamples();
        m_pSWSampleList = LTNULL;
    }

	if (!g_pSoundSys)
		return LT_ERROR;

    // Create an array of the SW samples
    if (m_nMaxSWSamples > 0)
    {
        LT_MEM_TRACK_ALLOC(m_pSWSampleList = new CSample[m_nMaxSWSamples],LT_MEM_TYPE_SOUND);

        for (nSample = 0; nSample < m_nMaxSWSamples; nSample++)
        {
            m_pSWSampleList[nSample].m_hSample = g_pSoundSys->AllocateSampleHandle(m_hDigDriver);
            if (!m_pSWSampleList[nSample].m_hSample)
                break;
            g_pSoundSys->SetSampleUserData(m_pSWSampleList[nSample].m_hSample, SAMPLE_TYPE, SAMPLETYPE_SW);
            g_pSoundSys->SetSampleUserData(m_pSWSampleList[nSample].m_hSample, SAMPLE_LISTITEM, (uint32)&m_pSWSampleList[nSample]);
            
            dl_AddHead(&m_SWFreeSampleList, &m_pSWSampleList[nSample].m_Link, &m_pSWSampleList[nSample]);
        }
        m_nNumSWSamples = nSample;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveSWSamples()
//
//  Removes the software samples.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveSWSamples()
{
    CSoundInstance *pSoundInstance;
    uint8 nSample;

    // Check if SW is available
    if (!m_hDigDriver)
        return LT_ERROR;

	if (!g_pSoundSys)
		return LT_ERROR;

    // Remove the array of samples
    if (m_pSWSampleList && m_nNumSWSamples)
    {
        nSample = m_nNumSWSamples;
        while (nSample-- > 0)
        {
            // Get rid of any instance using this sample
            pSoundInstance = GetLinkSampleSoundInstance(m_pSWSampleList[nSample].m_hSample);
            if (pSoundInstance)
            {
                pSoundInstance->Silence(true);
            }
            g_pSoundSys->ReleaseSampleHandle(m_pSWSampleList[nSample].m_hSample);
        }

        delete[] m_pSWSampleList;
        m_pSWSampleList = LTNULL;
        m_nNumSWSamples = 0;
    }

    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetDirectSound()
//
//  Gets the directsound object.
// 
//----------------------------------------------------------------------------------------------
LPDIRECTSOUND8 CSoundMgr::GetDirectSound()
{
    LPDIRECTSOUND8 lpDirectSound;
	LPDIRECTSOUNDBUFFER lpDirectSoundBuffer;

    if (!m_bValid)
        return LTNULL;

	if (!g_pSoundSys)
		return LTNULL;

    g_pSoundSys->GetDirectSoundInfo(LTNULL, (void**)&lpDirectSound, (void**)&lpDirectSoundBuffer);
    
    return lpDirectSound;
}

/*
//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetDirectMusicPerformance()
//
//  Gets the DirectMusic performance object
// 
//----------------------------------------------------------------------------------------------
IDirectMusicPerformance8* CSoundMgr::GetDirectMusicPerformance()
{
	IDirectMusicPerformance8* pPerformance;

    if (!m_bValid)
        return LTNULL;

//	GetSoundSys()->GetDirectMusicPerformance( (void**) &pPerformance );
    
    return pPerformance;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetDirectMusic()
//
//  Gets the DirectMusic object
// 
//----------------------------------------------------------------------------------------------
IDirectMusic* CSoundMgr::GetDirectMusic()
{
	IDirectMusic* pDirectMusic;

    if (!m_bValid)
        return LTNULL;

//	GetSoundSys()->GetDirectMusic( (void**) &pDirectMusic );
    
    return pDirectMusic;
}
*/

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::CreateBuffer()
//
//  Creates a sound buffer from a sound file.
// 
//----------------------------------------------------------------------------------------------
CSoundBuffer *CSoundMgr::CreateBuffer(FileIdentifier &fileIdent)
{
    CSoundBuffer *pSoundBuffer;

    if (!m_bValid)
    {
        return LTNULL;
    }

    // Check if buffer already exists
    if (fileIdent.m_pData)
    {
        // Make sure it's a sound
        if ((fileIdent.m_TypeCode == TYPECODE_SOUND) || (fileIdent.m_TypeCode == TYPECODE_UNKNOWN))
        {
            pSoundBuffer = (CSoundBuffer *)fileIdent.m_pData;
            pSoundBuffer->SetTouched(true);
            return pSoundBuffer;
        }
        else
        {
            return LTNULL;
        }
    }

    // Create a new sound buffer
    pSoundBuffer = m_SoundBufferBank.Allocate();
    if (!pSoundBuffer)
    {
        return LTNULL;
    }

    if (pSoundBuffer->Init(fileIdent) != LT_OK)
    {
        m_SoundBufferBank.Free(pSoundBuffer);
        return LTNULL;
    }

    dl_AddTail(&m_SoundBufferList, (LTLink *)pSoundBuffer->GetLink(), pSoundBuffer);

    return pSoundBuffer;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveBuffer()
//
//  Removes a sound buffer based on CSoundBuffer pointer.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveBuffer(CSoundBuffer &soundBuffer)
{
    LTLink *pCur;
    CSample *pSample;

    dl_RemoveAt(&m_SoundBufferList, (LTLink *)soundBuffer.GetLink());

    if ((soundBuffer.GetSoundBufferFlags() & SOUNDBUFFERFLAG_STREAM) == 0)
    {
        // Remove any sample-buffer links
        pCur = m_3DFreeSampleList.m_Head.m_pNext;
        while (pCur != &m_3DFreeSampleList.m_Head)
        {
            pSample = (CSample *)pCur->m_pData;
            pCur = pCur->m_pNext;

            if (!pSample || !pSample->m_h3DSample)
                continue;

            if ((CSoundBuffer*) g_pSoundSys->Get3DUserData(pSample->m_h3DSample, SAMPLE_BUFFER) == &soundBuffer)
                g_pSoundSys->Set3DUserData(pSample->m_h3DSample, SAMPLE_BUFFER, LTNULL);
        }
    }

    m_SoundBufferBank.Free(&soundBuffer);

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveBuffer()
//
//  Removes a sound buffer based on a fileident
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveBuffer(FileIdentifier &fileIdent)
{
    // Get the sound buffer
    if (!fileIdent.m_pData)
        return LT_ERROR;

    return RemoveBuffer(*(CSoundBuffer *)fileIdent.m_pData);
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveInstance()
//
//  Removes a sound instance based on CSoundInstance pointer.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveInstance(CSoundInstance &soundInstance)
{
    uint32 dwIndex;
    SoundType eSoundType;

    eSoundType = soundInstance.GetType();

    dwIndex = soundInstance.GetListIndex();
    if (m_dwNumSoundInstances > 1 && dwIndex < m_dwNumSoundInstances - 1)
    {
        m_SoundInstanceList[dwIndex] = m_SoundInstanceList[m_dwNumSoundInstances-1];
        m_SoundInstanceList[dwIndex]->SetListIndex(dwIndex);
    }
    if (m_dwNumSoundInstances)
    {
        m_SoundInstanceList[m_dwNumSoundInstances-1] = LTNULL;
        m_dwNumSoundInstances--;
    }
    soundInstance.Term();

    if (eSoundType == SOUNDTYPE_LOCAL)
    {
        m_LocalSoundInstanceBank.Free((CLocalSoundInstance *)&soundInstance);
    }
    else if (eSoundType == SOUNDTYPE_AMBIENT)
    {
        m_AmbientSoundInstanceBank.Free((CAmbientSoundInstance *)&soundInstance);
    }
    else if (eSoundType == SOUNDTYPE_3D)
    {
        m_3DSoundInstanceBank.Free((C3DSoundInstance *)&soundInstance);
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::StopAllSounds()
//
//  Stops all the sound instances from playing
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::StopAllSounds()
{
    uint32 nIndex;
    CSoundInstance *pSoundInstance;

    for (nIndex = 0; nIndex < m_dwNumSoundInstances; nIndex++)
    {
        pSoundInstance = m_SoundInstanceList[nIndex];
        if (!pSoundInstance)
            continue;

        pSoundInstance->Stop(true);
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundInstance()
//
//  Finds a sound instance based on a HLTSOUND
// 
//----------------------------------------------------------------------------------------------
CSoundInstance *CSoundMgr::FindSoundInstance(HLTSOUND hSound, bool bClientSound)
{
    CSoundInstance *pSoundInstance;
    uint32 dwIndex;

    if (bClientSound)
    {
        if (!hSound)
            return LTNULL;

        pSoundInstance = (CSoundInstance *)hSound;
        if (pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_CLIENT)
            return pSoundInstance;
    }
    else
    {
        if (hSound == (HLTSOUND)INVALID_OBJECTID)
            return LTNULL;

        // Look for sound handle in sound instance list
        for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
        {
            pSoundInstance = m_SoundInstanceList[dwIndex];
            ASSERT(pSoundInstance);
            if (!pSoundInstance)
                continue;

            if (pSoundInstance->GetHSoundDE() == hSound)
                if (!(pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_CLIENT))
                    return pSoundInstance;
        }
    }

    // Didn't find it
    return LTNULL;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Update()
//
//  Updates the sounds
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::Update()
{
    uint32 dwIndex, dwSearchIndex;
    CSoundInstance *pSoundInstance, *pSearchSoundInstance;

    HSAMPLE hSample;

    LTObject *pClientObject = LTNULL;
    LTVector vDeltaPos;
    LTVector vVelocity;
    uint32 dwFrameTime, dwCurTime;
    LTVector vTemp, vTemp2;

    if (!m_bValid || !m_bEnabled || !g_pSoundSys)
    {
        return LT_ERROR;
    }

    // Sometimes it takes a while to get the digital handle back
    if (m_bDigitalHandleReleased && m_bReacquireDigitalHandle)
        ReacquireDigitalHandle();

    // Update the timer
    dwCurTime = g_pSoundSys->MsCount();

    dwFrameTime = dwCurTime - m_dwCurTime;
    
    // See if it's time to commit all the sound changes
    if (dwCurTime > m_dwCommitTime)
    {
        m_bCommitChanges = true;
        m_dwCommitTime = dwCurTime + COMMITTIME;
    }

    // If the listener is inside the client, then use the client object's position and orientation...
    if (m_bListenerInClient)
    {
        if (g_pClientMgr->m_pCurShell)
            pClientObject = g_pClientMgr->m_pCurShell->m_pFrameClientObject;
        if (pClientObject)
        {       
            m_vListenerPosition = pClientObject->GetPos();
            quat_GetVectors((float *)&pClientObject->m_Rotation, 
                (float*)&m_vListenerRight, (float*)&m_vListenerUp, (float*)&m_vListenerForward);
        }
        else
        {
            m_vListenerPosition.Init();
            m_vListenerRight.Init(1.0f, 0.0f, 0.0f);
            m_vListenerUp.Init(0.0f, 1.0f, 0.0f);
            m_vListenerForward.Init(0.0f, 0.0f, 1.0f);
        }
    }

    // Update listener position and velocity
    if (dwFrameTime > 0)
    {
        m_vListenerVelocity = m_vListenerPosition - m_vLastListenerPosition;
        m_vListenerVelocity *= m_fDistanceFactor / (float)dwFrameTime;
		// get into meters/sec
		m_vListenerVelocity *= 1000.0f;
    }
    else
    {
        m_vListenerVelocity.Init();
    }
    m_vLastListenerPosition = m_vListenerPosition;

    // Set the listener position and velocity
    if (m_h3DListener && m_bCommitChanges)
    {

        g_pSoundSys->Get3DOrientation(m_h3DListener, &vTemp.x, &vTemp.y, &vTemp.z, &vTemp2.x, &vTemp2.y, &vTemp2.z);
        if (vTemp.DistSqr(m_vListenerForward) > 0.001f || vTemp2.DistSqr(m_vListenerUp) > 0.001f)
            g_pSoundSys->Set3DOrientation(m_h3DListener, m_vListenerForward.x, m_vListenerForward.y, m_vListenerForward.z, 
                m_vListenerUp.x, m_vListenerUp.y, m_vListenerUp.z);
		GetSoundSys()->Get3DVelocity( m_h3DListener, &vTemp.x, &vTemp.y, &vTemp.z );
		if( vTemp.DistSqr( m_vListenerVelocity ) > 0.5f )
			GetSoundSys()->Set3DVelocityVector( m_h3DListener, 
					m_vListenerVelocity.x, m_vListenerVelocity.y, m_vListenerVelocity.z );
    }

	// Clear the sound update packet so we start fresh.
	m_SoundUpdatePacket.Clear();

    // Start off with no sounds heard
    m_nNumSoundsHeard = 0;
    // Check if no sounds are waiting
    if (!m_dwNumSoundInstances)
        return LT_OK;

    // Preupdate the sounds
    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
    {
        pSoundInstance = m_SoundInstanceList[dwIndex];
        ASSERT(pSoundInstance);
        if (!pSoundInstance)
            continue;

        if (pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_DONE)
            continue;
        
        pSoundInstance->Preupdate(m_vListenerPosition);
        // Check if sound is within ear shot...
        if (!(pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_EARSHOT))
        {
            pSoundInstance->Silence();
        }
    }

    if (m_dwNumSoundInstances > 1)
    {

        qsort(m_SoundInstanceList, m_dwNumSoundInstances, sizeof(CSoundInstance *), CompareSoundInstances);

        // Reset the indices
        for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
        {
            pSoundInstance = m_SoundInstanceList[dwIndex];
            ASSERT(pSoundInstance);
            if (!pSoundInstance)
            {
                m_dwNumSoundInstances = dwIndex;
                break;
            }

            // The sort messes up the indices, so this needs to happen
            pSoundInstance->SetListIndex(dwIndex);
        }

    }

    // Give sounds channel samples based on priority
    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
    {
        pSoundInstance = m_SoundInstanceList[dwIndex];
        ASSERT(pSoundInstance);

        if (pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_DONE)
            continue;
        
        if (!(pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_EARSHOT))
            continue;

        // Check if sound already has a sample channel
        if (pSoundInstance->GetSample() || pSoundInstance->Get3DSample() || pSoundInstance->GetStream())
            continue;

        if (!pSoundInstance->GetSoundBuffer())
            continue;

        // Check if there's an initial delay.
        if (pSoundInstance->GetTimer() > pSoundInstance->GetDuration())
            continue;

        // Make sure there aren't other instances of the same buffer playing at the same start time.
        if (!pSoundInstance->GetSoundBuffer()->CanPlay(*pSoundInstance))
            continue;

        // Handle streaming sounds
        if (pSoundInstance->GetSoundBuffer()->GetSoundBufferFlags() & SOUNDBUFFERFLAG_STREAM)
        {
            if (pSoundInstance->AcquireStream() == LT_OK)
                continue;
            else
                continue;
        }

        // Check if sample needs reverb or is 3d
        if ( m_nMax3DSamples && (pSoundInstance->GetType() == SOUNDTYPE_3D) 
			|| (m_b3DReverb && pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_REVERB))
        {
            // Try to get a free 3d sample
            if (pSoundInstance->Acquire3DSample() == LT_OK)
                continue;
        }
        else if (pSoundInstance->AcquireSample() == LT_OK)
            continue;

        // Find the lowest priority sound with a sample and snag it
        for (dwSearchIndex = m_dwNumSoundInstances - 1; dwSearchIndex > dwIndex; dwSearchIndex--)
        {
            pSearchSoundInstance = m_SoundInstanceList[dwSearchIndex];

            if (m_nMax3DSamples && (pSoundInstance->GetType() == SOUNDTYPE_3D || 
                (m_b3DReverb && pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_REVERB)))
            {
                // Check if it has a 3d sample
                if ((pSearchSoundInstance->Get3DSample()) != LTNULL)
                {
                    // Check if new sound is non-streaming 3d sound
                    if (pSoundInstance->GetType() == SOUNDTYPE_3D && !(pSoundInstance->GetSoundBuffer()->GetSoundBufferFlags() & SOUNDBUFFERFLAG_STREAM))
                    {
                        pSearchSoundInstance->Silence();
                        pSoundInstance->Acquire3DSample();
                        break;
                    }
                }
            }
            // Check if it has a regular sample
            else if ((hSample = pSearchSoundInstance->GetSample()) != LTNULL)
            {
                pSearchSoundInstance->Silence();
                pSoundInstance->AcquireSample();
                break;
            }
        }
    }

	CPacket_Write cNewSoundUpdatePacket;

    // Update the instances
    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
    {
        pSoundInstance = m_SoundInstanceList[dwIndex];
        ASSERT(pSoundInstance);

        if (pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_DONE)
  			continue;

		// Only update the output on sounds with channel samples
        if (pSoundInstance->Get3DSample() || pSoundInstance->GetSample() || pSoundInstance->GetStream())
        {
            m_nNumSoundsHeard++;
            pSoundInstance->UpdateOutput(dwFrameTime);
        }

        // If the sound has finished playing, it can be removed if it doesn't have a handle...
        if (!pSoundInstance->UpdateTimer(dwFrameTime))
        {
            pSoundInstance->Stop();

            // If the sound doesn't have a handle, or isn't being controlled by the server, then we can remove it.
            // Otherwise, the user will remove it, or the server will.
            if ((pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_ENDLOOP) || 
                !(pSoundInstance->GetPlaySoundFlags() & (PLAYSOUND_GETHANDLE | PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED)))
            {
                RemoveInstance(*pSoundInstance);
                dwIndex--;
            }
            // If the server needs to know when this sound is done, then tell it, but keep the instance around
            else if (!(pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_CLIENT) &&
                (pSoundInstance->GetPlaySoundFlags() & (PLAYSOUND_TIME | PLAYSOUND_TIMESYNC | PLAYSOUND_ATTACHED)) && 
                !(pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_LOOP) && 
                pSoundInstance->GetHSoundDE() != (HLTSOUND)INVALID_OBJECTID)
            {
                cNewSoundUpdatePacket.Writeuint16((uint16)pSoundInstance->GetHSoundDE());
            }
        }
    }

	m_SoundUpdatePacket = CPacket_Read(cNewSoundUpdatePacket);

	// commit any deferred settings

	g_pSoundSys->CommitDeferred();

    // Update the timer
    m_dwCurTime = dwCurTime;
    m_bCommitChanges = false;

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::CompareSoundInstances
//
//  Compares priority of two sound instances.  Return values are reversed so qsort sorts in 
//  decreasing order.
// 
//----------------------------------------------------------------------------------------------
int CSoundMgr::CompareSoundInstances(const void *pElem1, const void *pElem2)
{
    CSoundInstance *pSoundInstance1, *pSoundInstance2;
    int nTimePlaying1, nTimePlaying2;

    pSoundInstance1 = *(CSoundInstance **)pElem1;
    pSoundInstance2 = *(CSoundInstance **)pElem2;

    // Null pointers are considered low order
    if (pSoundInstance1)
    {
        if (!pSoundInstance2)
            return -1;
    }
    else if (pSoundInstance2)
        return 1;
    else
        return 0;

    // Local sounds have highest priority of all, and go before all non-local sounds in list
    if (pSoundInstance1->GetType() == SOUNDTYPE_LOCAL)
    {
        if (pSoundInstance2->GetType() != SOUNDTYPE_LOCAL)
            return -1;
    }
    else if (pSoundInstance2->GetType() == SOUNDTYPE_LOCAL)
        return 1;

    // If sound has higher priority, then insert it before the test sound
    if (pSoundInstance1->GetModifiedPriority() > pSoundInstance2->GetModifiedPriority())
        return -1;

    // Check if sounds have equal priority
    if (pSoundInstance1->GetModifiedPriority() == pSoundInstance2->GetModifiedPriority())
    {
        // Playing sounds have a slightly higher priority than non-playing
        if (pSoundInstance1->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_PLAYING)
        { 
            if (!(pSoundInstance2->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_PLAYING))
                return -1;
        }
        else if (pSoundInstance2->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_PLAYING)
            return 1;

        if (!pSoundInstance1->GetSoundBuffer() || !pSoundInstance2->GetSoundBuffer())
            return 0;
    
        // Get the amount of time the sound has been alive.  If there is a pre-delay, then it has low priority
        nTimePlaying1 = pSoundInstance1->GetDuration() - pSoundInstance1->GetTimer();
        if (nTimePlaying1 < 0)
            return 1;

        // Get the amount of time the sound has been alive.  If there is a pre-delay, then it has low priority
        nTimePlaying2 = pSoundInstance2->GetDuration() - pSoundInstance2->GetTimer();
        if (nTimePlaying2 < 0)
            return -1;

        // Neither sound is playing.  Give priority to newest sound.
        if (nTimePlaying1 < nTimePlaying2)
            return -1;
        else if (nTimePlaying1 > nTimePlaying2)
            return 1;
        else
            return 0;
    }

    return 1;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetListenerDoppler
//
//  Sets volume
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetListenerDoppler(float fDoppler)
{
	if (!g_pSoundSys)
		return LT_ERROR;

    g_pSoundSys->SetListenerDoppler( m_h3DListener, fDoppler );
	return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetVolume
//
//  Sets volume
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetVolume(uint16 nVolume)
{
    if (!m_bValid || !m_hDigDriver)
        return LT_ERROR;

    nVolume = CLAMP(nVolume, 0, 100);

	m_nGlobalSoundVolume = nVolume;

/*
    nVolume = (uint16)((float)nVolume * 127.0f / 100.0f);

    g_pSoundSys->SetDigitalMasterVolume(m_hDigDriver, nVolume);
*/
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetVolume
//
//  Gets volume
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetVolume(uint16 &nVolume)
{ 
    if (!m_bValid || !m_hDigDriver)
    {
        nVolume = 100;
        return LT_OK;
    }

//    nVolume = (uint16)((float) g_pSoundSys->GetDigitalMasterVolume(m_hDigDriver) * 100.0f / 127.0f);

	nVolume = m_nGlobalSoundVolume;

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetVolumeMultiplier
//
//  Gets volume multiplier for calculating current volume
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetVolumeMultiplier(float &fVolume)
{
	fVolume = m_nGlobalSoundVolume / 100.0f;

	return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::PlaySound
//
//  Plays a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::PlaySound(PlaySoundInfo &playSoundInfo, FileIdentifier &fileIdent, uint32 dwOffsetTime)
{
    CSoundBuffer *pSoundBuffer;
    CSoundInstance *pSoundInstance;
    SoundType eType;
    LTRESULT nError;

    // If client side sound, then initialize the handle just in case it fails
    if (playSoundInfo.m_dwFlags & PLAYSOUND_CLIENT)
        playSoundInfo.m_hSound = LTNULL;

    // Make sure it was initialized
    if (!m_bValid)
    {
        return LT_ERROR;
    }

    // Check if turned off
    if (!m_bEnabled)
    {
        return LT_OK;
    }

    // Make sure they have gethandle when the do anything that makes the sound stick around.
    if( !( playSoundInfo.m_dwFlags & PLAYSOUND_GETHANDLE ) &&
		( playSoundInfo.m_dwFlags & ( PLAYSOUND_TIME | PLAYSOUND_TIMESYNC )))
	{
		RETURN_ERROR(1, (CSoundMgr::PlaySound:  PLAYSOUND_GETHANDLE needed with specified flags.), LT_ERROR);
    }

    // Get the sound buffer for this file
    pSoundBuffer = CreateBuffer(fileIdent);
    if (!pSoundBuffer)
    {
        return LT_ERROR;
    }

    pSoundInstance = LTNULL;

    // Play sound in the player's head, if global local sound, or client local sound and listener is in client...
    if (((playSoundInfo.m_dwFlags & PLAYSOUND_CLIENTLOCAL) && m_bListenerInClient) || 
        (!(playSoundInfo.m_dwFlags & PLAYSOUND_AMBIENT) && !(playSoundInfo.m_dwFlags & PLAYSOUND_3D)))
    {
        pSoundInstance = m_LocalSoundInstanceBank.Allocate();
    }
    else if (playSoundInfo.m_dwFlags & PLAYSOUND_AMBIENT)
    {
        pSoundInstance = m_AmbientSoundInstanceBank.Allocate();
    }
    else //if(pPlaySoundInfo->m_dwFlags & PLAYSOUND_3D)
    {
        pSoundInstance = m_3DSoundInstanceBank.Allocate();
    }

    if (!pSoundInstance)
    {
        return LT_ERROR;
    }

    // Initialize the sound
    nError = pSoundInstance->Init(*pSoundBuffer, playSoundInfo, dwOffsetTime);

    if (nError == LT_OK)
    {
        // Make sure there is room
        if (m_dwNumSoundInstances < SOUNDMGR_MAXSOUNDINSTANCES)
        {
            m_SoundInstanceList[m_dwNumSoundInstances] = pSoundInstance;
            pSoundInstance->SetListIndex(m_dwNumSoundInstances);
            m_dwNumSoundInstances++;
        }
        else
            nError = LT_ERROR;
    }


    if (nError != LT_OK)
    {
        eType = pSoundInstance->GetType();
        pSoundInstance->Term();
        if (eType == SOUNDTYPE_LOCAL)
            m_LocalSoundInstanceBank.Free((CLocalSoundInstance *)pSoundInstance);
        if (eType == SOUNDTYPE_AMBIENT)
            m_AmbientSoundInstanceBank.Free((CAmbientSoundInstance *)pSoundInstance);
        if (eType == SOUNDTYPE_3D)
            m_3DSoundInstanceBank.Free((C3DSoundInstance *)pSoundInstance);
    }
	else
	{
		// if everything is O.K., see if we need to set up filtering for the sound
        i_client_shell->OnPlaySound(&playSoundInfo);
	}


    return nError;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::LinkSampleSoundInstance
//
//  Sets the link between the sample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

LTRESULT CSoundMgr::LinkSampleSoundInstance(HSAMPLE hSample, CSoundInstance *pSoundInstance)
{
    ASSERT(hSample);
    if (!hSample || !g_pSoundSys)
        return LT_ERROR;

    g_pSoundSys->SetSampleUserData(hSample, SAMPLE_INSTANCE, (uint32)pSoundInstance);

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSampleSoundInstance
//
//  Gets the link between the sample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

CSoundInstance *CSoundMgr::GetLinkSampleSoundInstance(HSAMPLE hSample)
{
    CSoundInstance *pSoundInstance;
    ASSERT(hSample);
    if (!hSample || !g_pSoundSys)
        return LTNULL;

    pSoundInstance = (CSoundInstance*) g_pSoundSys->GetSampleUserData(hSample, SAMPLE_INSTANCE);

    return pSoundInstance;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Link3DSampleSoundInstance
//
//  Sets the link between the 3dsample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

LTRESULT CSoundMgr::Link3DSampleSoundInstance(H3DSAMPLE h3DSample, CSoundInstance *pSoundInstance)
{
    if (!h3DSample || !g_pSoundSys)
        return LT_ERROR;

    g_pSoundSys->Set3DUserData(h3DSample, SAMPLE_INSTANCE, (uint32)pSoundInstance);

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetLink3DSampleSoundInstance
//
//  Gets the link between the 3dsample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

CSoundInstance *CSoundMgr::GetLink3DSampleSoundInstance(H3DSAMPLE h3DSample)
{
    CSoundInstance *pSoundInstance;
    if (!h3DSample || !g_pSoundSys)
        return LTNULL;

    pSoundInstance = (CSoundInstance*) g_pSoundSys->Get3DUserData(h3DSample, SAMPLE_INSTANCE);

    return pSoundInstance;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::LinkStreamSoundInstance
//
//  Sets the link between the 3dsample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

LTRESULT CSoundMgr::LinkStreamSoundInstance(HSTREAM hStream, CSoundInstance *pSoundInstance)
{
    if (!hStream || !g_pSoundSys)
        return LT_ERROR;

    g_pSoundSys->SetStreamUserData(hStream, SAMPLE_INSTANCE, (uint32)pSoundInstance);

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetLinkStreamSoundInstance
//
//  Gets the link between the 3dsample and the soundinstance
// 
//----------------------------------------------------------------------------------------------

CSoundInstance *CSoundMgr::GetLinkStreamSoundInstance(HSTREAM hStream)
{
    CSoundInstance *pSoundInstance;
    if (!hStream || !g_pSoundSys)
        return LTNULL;

    pSoundInstance = (CSoundInstance*) g_pSoundSys->GetStreamUserData(hStream, SAMPLE_INSTANCE);

    return pSoundInstance;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetFreeSWSample()
//
//  Get a free sw sample
// 
//----------------------------------------------------------------------------------------------

HSAMPLE CSoundMgr::GetFreeSWSample()
{
    LTLink *pLink;
    CSample *pSample;

    if (m_SWFreeSampleList.m_nElements > 0)
    {
        pLink = m_SWFreeSampleList.m_Head.m_pNext;
        dl_RemoveAt(&m_SWFreeSampleList, pLink);
        dl_TieOff(pLink);
        pSample = (CSample *)pLink->m_pData;      

        return pSample->m_hSample;
    }

    return LTNULL;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetFree3DSample()
//
//  Get a free 3d sample.  Searches the free list for a sample that is already setup the
//  way the new sound needs.
// 
//----------------------------------------------------------------------------------------------

H3DSAMPLE CSoundMgr::GetFree3DSample(CSoundInstance *pSoundInstance)
{
    LTLink *pLink;
    CSample *pSample, *pTestSample;
    LTVector vTestPos, vPos;

    H3DSAMPLE h3DSample;

    CSoundBuffer *pSoundBuffer;

#ifdef MODPROVIDER

    if (IsKeyDown('Q'))
        return LTNULL;

#endif // MODPROVIDER

    pSample = LTNULL;
    h3DSample = LTNULL;

	if (!g_pSoundSys)
		return LTNULL;

    // If there is more than one element, search the list
    if (m_3DFreeSampleList.m_nElements > 0)
    {
        // Look through the free samples and find one that matches our new sound the best.
        pSoundInstance->Get3DSamplePosition(vPos);
        pLink = m_3DFreeSampleList.m_Head.m_pPrev;
        while (pLink != &m_3DFreeSampleList.m_Head)
        {
            pTestSample = (CSample *)pLink->m_pData;

            if (pTestSample)
            {

                pSoundBuffer = (CSoundBuffer*) g_pSoundSys->Get3DUserData(pTestSample->m_h3DSample, SAMPLE_BUFFER);

                // Check if this sample used the same soundbuffer that we need
                if (pSoundBuffer == pSoundInstance->GetSoundBuffer())
                {
                    // This is the best so far.
                    pSample = pTestSample;

                    // Check if the position is the same, if so, we can't get any better.
                    g_pSoundSys->Get3DPosition(pSample->m_h3DSample, &vTestPos.x, &vTestPos.y, &vTestPos.z);

                    if (vPos.DistSqr(vTestPos) < 0.001f)
                    {
                        break;
                    }
                }
            }

            pLink = pLink->m_pPrev;
        }

        // If we didn't find one we like, then just choose the head.
        if (!pSample && m_3DFreeSampleList.m_Head.m_pNext)
        {
            pSample = (CSample *)m_3DFreeSampleList.m_Head.m_pNext->m_pData;

			// Clear out the sample's buffer so we always re-init the sample.
			g_pSoundSys->Set3DUserData(pSample->m_h3DSample, SAMPLE_BUFFER, LTNULL);
        }
    }

    // Remove it from the free list.
    if (pSample)
    {
        dl_RemoveAt(&m_3DFreeSampleList, &pSample->m_Link);
        dl_TieOff(&pSample->m_Link);
        h3DSample = pSample->m_h3DSample;
    }

    // If there is only one free sample left, then make sure it has had end called on it.
    if (m_3DFreeSampleList.m_nElements == 1 && m_3DFreeSampleList.m_Head.m_pNext)
    {
        pSample = (CSample *)m_3DFreeSampleList.m_Head.m_pNext->m_pData;
        if (pSample)
        {
            g_pSoundSys->End3DSample(pSample->m_h3DSample);
        }
    }

    return h3DSample;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::ReleaseSWSample()
//
//  Releases a SW Sample back to free list
// 
//----------------------------------------------------------------------------------------------

void CSoundMgr::ReleaseSWSample(HSAMPLE hSample)
{
	if (!g_pSoundSys) return;

    CSample *pSample = (CSample*) g_pSoundSys->GetSampleUserData(hSample, SAMPLE_LISTITEM);

    if (pSample)
        dl_AddTail(&m_SWFreeSampleList, &pSample->m_Link, pSample);
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Release3DSample()
//
//  Releases a 3D Sample back to free list
// 
//----------------------------------------------------------------------------------------------

void CSoundMgr::Release3DSample(H3DSAMPLE h3DSample)
{
	if (!g_pSoundSys) return;

    CSample *pSample = (CSample*) g_pSoundSys->Get3DUserData(h3DSample, SAMPLE_LISTITEM);

    if (pSample)
        dl_AddTail(&m_3DFreeSampleList, &pSample->m_Link, pSample);
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::UntagAllSoundBuffers
//
//  Removes tag from all sound buffers so they can be checked if they were ever used
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::UntagAllSoundBuffers()
{
    LTLink *pCur, *pNext;
    
    if (!m_bValid)
        return LT_ERROR;

    // Untag the sounds...
    pCur = m_SoundBufferList.m_Head.m_pNext;
    while (pCur != &m_SoundBufferList.m_Head)
    {
        pNext = pCur->m_pNext;
        ((CSoundBuffer *)pCur->m_pData)->SetTouched(false);
        pCur = pNext;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveAllUntaggedSoundBuffers
//
//  Removes any sound buffers that were not tagged as used
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveAllUntaggedSoundBuffers()
{
    CSoundBuffer *pSoundBuffer;
    LTLink *pCurSound;

    if (!m_bValid)
        return LT_ERROR;

	LTRESULT res = RemoveAllUnusedSoundInstances( );
	if( res != LT_OK )
		return res;

    // Remove unused sound buffers
    pCurSound = m_SoundBufferList.m_Head.m_pNext;
    while (pCurSound != &m_SoundBufferList.m_Head)
    {
        pSoundBuffer = (CSoundBuffer *)pCurSound->m_pData;
        pCurSound = pCurSound->m_pNext;

        if (!pSoundBuffer)
            continue;

        // Check if not tagged
        if (!pSoundBuffer->IsTouched())
        {
            // Check if there are any sound instances using the buffer
            if (pSoundBuffer->GetInstanceList()->m_nElements == 0)
            {
                RemoveBuffer(*pSoundBuffer);
            }
        }
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::RemoveAllUnusedSoundInstances
//
//  Removes any sound instances that don't have handles on the client.
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::RemoveAllUnusedSoundInstances( )
{
    CSoundInstance *pSoundInstance;
    signed long nIndex;

    if (!m_bValid)
        return LT_ERROR;

    // Remove all sounds except client sounds with handles
    for (nIndex = 0; nIndex < (signed long)m_dwNumSoundInstances; nIndex++)
    {
        pSoundInstance = (CSoundInstance *)m_SoundInstanceList[nIndex];
        if (!pSoundInstance)
            continue;

        if (pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_GETHANDLE && 
            pSoundInstance->GetPlaySoundFlags() & PLAYSOUND_CLIENT)
            continue;

        RemoveInstance(*pSoundInstance);
        nIndex--;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetListener
//
//  Sets the listener position and orientation
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetListener(bool bListenerInClient, LTVector *pvListenerPos, LTVector *pvListenerForward, LTVector *pvListenerRight, bool bTeleport)
{
    m_bListenerInClient = bListenerInClient;

    if (!m_bValid || !m_bEnabled)
        return LT_ERROR;

    if (!m_bListenerInClient)
    {
        if (pvListenerPos)
        {
            // Update listener position and velocity
            m_vListenerPosition = *pvListenerPos;
            if (bTeleport)
            {
                m_vLastListenerPosition = *pvListenerPos;
            }
        }
        
        if (pvListenerForward)
		{
            m_vListenerForward = *pvListenerForward;
        }
		if (pvListenerRight)
        {
			m_vListenerRight = *pvListenerRight;
        }
		if (pvListenerForward && pvListenerRight)
		{
            m_vListenerUp = pvListenerRight->Cross(*pvListenerForward);
		}
	}
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::ReleaseDigitalHandle
//
//  Releases the control on sound hardware
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::ReleaseDigitalHandle()
{
    if (!m_hDigDriver || !g_pSoundSys)
        return LT_ERROR;

    if (!m_3DProvider.m_hProvider)
    {
        if (!m_bDigitalHandleReleased)
        {
            if (!g_pSoundSys->DigitalHandleRelease(m_hDigDriver))
            {
                return LT_ERROR;
            }

            m_bDigitalHandleReleased = true;
            m_bReacquireDigitalHandle = false;
        }
    }
    else
    {
        PauseSounds();
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::ReacquireDigitalHandle
//
//  Reacquires the control on sound hardware
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::ReacquireDigitalHandle()
{
	if (!g_pSoundSys) return LT_ERROR;

    if (!m_3DProvider.m_hProvider)
    {
        if (m_bDigitalHandleReleased)
        {
            if (!m_hDigDriver)
                return LT_ERROR;

            if (!g_pSoundSys->DigitalHandleReacquire(m_hDigDriver))
            {
                m_bReacquireDigitalHandle = true;
                return LT_ERROR;
            }

            m_bDigitalHandleReleased = false;
        }
    }
    else
    {
        ResumeSounds();
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::PauseSounds
//
//  Pauses all the current sounds
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::PauseSounds()
{
    uint32 dwIndex;
    CSoundInstance *pSoundInstance;

    if (!m_bValid)
        return LT_OK;

    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
    {
        pSoundInstance = m_SoundInstanceList[dwIndex];
        ASSERT(pSoundInstance);
        if (!pSoundInstance)
            continue;

        pSoundInstance->Pause();
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::Resume
//
//  Resumes sounds that were previously paused
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::ResumeSounds()
{
    uint32 dwIndex;
    CSoundInstance *pSoundInstance;

    if (!m_bValid)
        return LT_OK;

    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
    {
        pSoundInstance = m_SoundInstanceList[dwIndex];
        ASSERT(pSoundInstance);
        if (!pSoundInstance)
            continue;

        pSoundInstance->Resume();
    }

    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetSoundClassMultiplier
//
//  Sets the volume multiplier for a given sound class
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetSoundClassMultiplier( uint8 nSoundClass, float fMultiplier,
											 bool bUseGlobalVolume /*=true*/)
{
	if (!m_bValid || !m_hDigDriver)
        return LT_ERROR;

	// check for legitimate values
	if ( fMultiplier < 0.0f || fMultiplier > 1.0f )
		return LT_ERROR;
	if ( nSoundClass > MAX_SOUND_VOLUME_CLASSES )
		return LT_ERROR;

	m_afSoundClassMultipliers[ nSoundClass ] = fMultiplier;
	m_abUseGlobalVolume[ nSoundClass ] = bUseGlobalVolume;

	return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundClassMultiplier
//
//  Gets the volume multiplier for a given sound class
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundClassMultiplier( uint8 nSoundClass, float* pfMultiplier )
{
	if (!m_bValid || !m_hDigDriver)
        return LT_ERROR;

	// return default multiplier for safety
	*pfMultiplier = 1.0f;

	// check for legitimate values
	if ( nSoundClass > MAX_SOUND_VOLUME_CLASSES )
		return LT_ERROR;

	*pfMultiplier = this->m_afSoundClassMultipliers[ nSoundClass ];
	
	return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundClassMultiplier
//
//  Gets the volume multiplier for a given sound class
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundClassUseGlobalVolume( uint8 nSoundClass, bool* pbUseGlobalVolume )
{
	if (!m_bValid || !m_hDigDriver)
        return LT_ERROR;

	// return default multiplier for safety
	*pbUseGlobalVolume = true;

	// check for legitimate values
	if ( nSoundClass > MAX_SOUND_VOLUME_CLASSES )
		return LT_ERROR;

	*pbUseGlobalVolume = this->m_abUseGlobalVolume[ nSoundClass ];
	
	return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::UpdateVolumeSettings
//
//  Called from the game side when volume settings are changed to allow
//	all the playing sounds to be updated
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::UpdateVolumeSettings()
{
	uint32 dwIndex;
	CSoundInstance* pSoundInstance;

	// go through all the playing sounds and update their volume settings
    for (dwIndex = 0; dwIndex < m_dwNumSoundInstances; dwIndex++)
	{
        pSoundInstance = m_SoundInstanceList[dwIndex];
		uint16 nVolume;
		pSoundInstance->GetVolume(nVolume);
		pSoundInstance->SetVolume(nVolume);
	}

	return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetReverbProperties
//
//  Sets the global reverb properties
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetReverbProperties(ReverbProperties *pReverbProperties)
{
// This is the old system for setting reverb - MAG - 4/2/02, so I commented it out

/*
    bool bChange;

    if (!pReverbProperties)
        RETURN_ERROR(1, CSoundMgr::SetReverbProperties, LT_INVALIDPARAMS);

    if (!m_bValid || (!m_bSWReverb && !m_b3DReverb))
        return LT_ERROR;

    bChange = false;
    if (pReverbProperties->m_dwParams & REVERBPARAM_ACOUSTICS)
    {
        // Get new values and make sure they are in range
//      if (pReverbProperties->m_dwAcoustics >= EAX_ENVIRONMENT_COUNT)
//          pReverbProperties->m_dwAcoustics = EAX_ENVIRONMENT_GENERIC;
        pReverbProperties->m_dwAcoustics = 0;

        if (m_dwReverbAcoustics != pReverbProperties->m_dwAcoustics)
        {
            bChange = true;
            m_dwReverbAcoustics = pReverbProperties->m_dwAcoustics;
        }
    }

    if (pReverbProperties->m_dwParams & REVERBPARAM_VOLUME)
    {
        pReverbProperties->m_fVolume = CLAMP(pReverbProperties->m_fVolume, 0.0f, 1.0f);
        
        if (m_fReverbVolume != pReverbProperties->m_fVolume)
        {
            bChange = true;
            m_fReverbVolume = pReverbProperties->m_fVolume;
        }
    }

    if (pReverbProperties->m_dwParams & REVERBPARAM_REFLECTTIME)
        m_fReverbReflectTime = CLAMP(pReverbProperties->m_fReflectTime, 0.0f, 5.0f);
    if (pReverbProperties->m_dwParams & REVERBPARAM_DECAYTIME)
    {
        pReverbProperties->m_fDecayTime = CLAMP(pReverbProperties->m_fDecayTime, 0.001f, 20.0f);

        if (m_fReverbDecayTime != pReverbProperties->m_fDecayTime)
        {
            bChange = true;
            m_fReverbDecayTime = pReverbProperties->m_fDecayTime;
        }
    }
    if (pReverbProperties->m_dwParams & REVERBPARAM_DAMPING)
    {
        pReverbProperties->m_fDamping = CLAMP(pReverbProperties->m_fDamping, 0.0f, 2.0f);
        if (m_fReverbDamping != pReverbProperties->m_fDamping)
        {
            bChange = true;
            m_fReverbDamping = pReverbProperties->m_fDamping;
        }
    }

    if (m_b3DReverb && bChange)
    {
        g_pSoundSys->Set3DRoomType(m_3DProvider.m_hProvider, m_dwReverbAcoustics);
    }
*/
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetReverbProperties
//
//  Gets the global reverb properties
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetReverbProperties(ReverbProperties *pReverbProperties)
{
// This is the old system for setting reverb - MAG - 4/2/02, so I commented it out
/*
    if (!pReverbProperties)
        RETURN_ERROR(1, CSoundMgr::GetReverbProperties, LT_INVALIDPARAMS);

    if (!m_bValid || (!m_bSWReverb && !m_b3DReverb))
        return LT_ERROR;

    if (pReverbProperties->m_dwParams & REVERBPARAM_ACOUSTICS)
        pReverbProperties->m_dwAcoustics = m_dwReverbAcoustics;
    if (pReverbProperties->m_dwParams & REVERBPARAM_VOLUME)
        pReverbProperties->m_fVolume = m_fReverbVolume;
    if (pReverbProperties->m_dwParams & REVERBPARAM_REFLECTTIME)
        pReverbProperties->m_fReflectTime = m_fReverbReflectTime;
    if (pReverbProperties->m_dwParams & REVERBPARAM_DECAYTIME)
        pReverbProperties->m_fDecayTime = m_fReverbDecayTime;
    if (pReverbProperties->m_dwParams & REVERBPARAM_DECAYTIME)
        pReverbProperties->m_fDamping = m_fReverbDamping;

*/
    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::PlaySound  (ILTClientSound interface)
//
//  ILTClientSound-compatible interface to PlaySound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::PlaySound(PlaySoundInfo *pPlaySoundInfo, HLTSOUND &hResult)
{
    FileRef playSoundFileRef;
    FileIdentifier *pIdent;
    const char *pFileName;
    LTRESULT playResult;

    if (!pPlaySoundInfo)
        return LT_INVALIDPARAMS;

    if (!IsValid() || !IsEnabled())
        return LT_ERROR;

    if (!g_pClientMgr)
        return LT_ERROR;

    // These options don't work for client only sounds...
    pPlaySoundInfo->m_dwFlags &= ~PLAYSOUND_TIME & ~PLAYSOUND_ATTACHED & ~PLAYSOUND_TIMESYNC;

    // Make sure the client flag is set...
    pPlaySoundInfo->m_dwFlags |= PLAYSOUND_CLIENT;

    playSoundFileRef.m_pFilename = pPlaySoundInfo->m_szSoundName;
    playSoundFileRef.m_FileType = FILE_CLIENTFILE;
    
    pIdent = client_file_mgr->GetFileIdentifier(&playSoundFileRef, TYPECODE_SOUND);
    if (!pIdent)
    {       
        if (g_DebugLevel >= 2)
        {
            pFileName = client_file_mgr->GetFilename(&playSoundFileRef);

            dsi_PrintToConsole("Missing sound file %s", pFileName);
        }

        return LT_ERROR;
    }

    playResult = PlaySound(*pPlaySoundInfo, *pIdent, 0);
    if (playResult == LT_OK)
        hResult = (HLTSOUND)m_SoundInstanceList[m_dwNumSoundInstances - 1];

    return playResult;

}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundDuration 
//
//  Retrieves the duration of the sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundDuration(HLTSOUND hSound, LTFLOAT &fDuration)
{
    CSoundInstance *pSoundInstance;

    if (!IsValid())
    {
        fDuration = 0;
        return LT_OK;
    }

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::GetSoundDuration, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::GetSoundDuration, LT_INVALIDPARAMS);

    if (!pSoundInstance->GetSoundBuffer())
        RETURN_ERROR(1, CSoundMgr::GetSoundDuration, LT_INVALIDPARAMS);

    fDuration = pSoundInstance->GetSoundBuffer()->GetDuration() / 1000.0f;
    
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::IsSoundDone
//
//  Returns the finished state of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::IsSoundDone(HLTSOUND hSound, bool &bDone)
{
    CSoundInstance *pSoundInstance;

    bDone = true;

    pSoundInstance = FindSoundInstance(hSound, true);
    if ((pSoundInstance) && ((pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_DONE) == 0))
        bDone = false;

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::KillSound
//
//  Terminates a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::KillSound(HLTSOUND hSound)
{
    CSoundInstance *pSoundInstance;

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        return LT_INVALIDPARAMS;

    RemoveInstance(*pSoundInstance);
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::KillSoundLoop
//
//  Terminates a looping sound after the next loop
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::KillSoundLoop(HLTSOUND hSound)
{
    CSoundInstance *pSoundInstance;

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        return LT_INVALIDPARAMS;

    pSoundInstance->EndLoop();

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSound3DProviderLists
//
//  Retrieves and allocates a list of 3D sound providers
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSound3DProviderLists(Sound3DProvider *&pSound3DProviderList, bool bVerify, uint32 uiMax3DVoices )
{
    CProvider *pProvider, *p3DProviderList;
    Sound3DProvider *pSound3DProvider;

    pSound3DProviderList = LTNULL;
    if (Get3DProviderLists(p3DProviderList, bVerify, uiMax3DVoices) != LT_OK)
        return LT_ERROR;

    pProvider = p3DProviderList;
    while (pProvider)
    {
        LT_MEM_TRACK_ALLOC(pSound3DProvider = new Sound3DProvider,LT_MEM_TYPE_SOUND);
        pSound3DProvider->m_pNextProvider = pSound3DProviderList;
        pSound3DProviderList = pSound3DProvider;
        LTStrCpy(pSound3DProvider->m_szProvider, pProvider->m_szProviderName, sizeof(pSound3DProvider->m_szProvider));
        pSound3DProvider->m_dwCaps = pProvider->m_dwCaps;
        pSound3DProvider->m_dwProviderID = pProvider->m_dwProviderID;
        pProvider = pProvider->m_pNextProvider;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::ReleaseSound3DProviderList
//
//  Deallocates a list of 3D sound providers generated by GetSound3DProviderList
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::ReleaseSound3DProviderList(Sound3DProvider *pSound3DProviderList)
{
    Sound3DProvider *pSound3DProvider;

    if (!pSound3DProviderList)
        return LT_INVALIDPARAMS;

    while (pSound3DProviderList)
    {
        pSound3DProvider = pSound3DProviderList->m_pNextProvider;
        delete pSound3DProviderList;
        pSound3DProviderList = pSound3DProvider;
    }

    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::InitSound (ILTClientSound interface)
//
//  ILTClientSound-compatible interface to Init
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::InitSound(InitSoundInfo *pSoundInfo)
{
    if (!pSoundInfo)
        RETURN_ERROR(1, CSoundMgr::InitSound, LT_INVALIDPARAMS);

    return Init(*pSoundInfo);
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetSoundOcclusion
//
//  Sets the occlusion level of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetSoundOcclusion(HLTSOUND hSound, LTFLOAT fLevel)
{
    CSoundInstance *pSoundInstance;

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::SetSoundOcclusion, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::SetSoundOcclusion, LT_ERROR);
    
    pSoundInstance->SetOcclusion(fLevel);
    
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundOcclusion
//
//  Retrieves the current occlusion level of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundOcclusion(HLTSOUND hSound, LTFLOAT *pLevel)
{
    CSoundInstance *pSoundInstance;

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::GetSoundOcclusion, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::GetSoundOcclusion, LT_ERROR);
    
    if (pLevel)
        pSoundInstance->GetOcclusion(*pLevel);
    
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetSoundObstruction
//
//  Sets the obstruction level of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetSoundObstruction(HLTSOUND hSound, LTFLOAT fLevel)
{
    CSoundInstance *pSoundInstance;

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::SetSoundObstruction, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::SetSoundObstruction, LT_ERROR);
    
    pSoundInstance->SetObstruction(fLevel);
    
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetSoundObstruction
//
//  Retrieves the obstruction level of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundObstruction(HLTSOUND hSound, LTFLOAT *pLevel)
{
    CSoundInstance *pSoundInstance;

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::GetSoundObstruction, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::GetSoundObstruction, LT_ERROR);
    
    if (pLevel)
        pSoundInstance->GetObstruction(*pLevel);
    
    return LT_OK;
}

//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::GetSoundPosition
//
//  Retrieves the current position of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::GetSoundPosition(HLTSOUND hSound, LTVector *pPos)
{
    CSoundInstance *pSoundInstance;

	if (!m_bValid || !m_hDigDriver)
        RETURN_ERROR(1, CSoundMgr::GetSoundPosition, LT_ERROR);

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::GetSoundPosition, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::GetSoundPosition, LT_ERROR);
    
    if (pPos)
        pSoundInstance->GetPosition(*pPos);
    
    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetSoundPosition
//
//  Sets the current location of a sound
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetSoundPosition(HLTSOUND hSound, LTVector *pPos)
{
    CSoundInstance *pSoundInstance;

    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::SetSoundPosition, LT_INVALIDPARAMS);

    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::SetSoundPosition, LT_ERROR);
    
    if (pPos)
        pSoundInstance->SetPosition(*pPos);
    
    return LT_OK;
}


//----------------------------------------------------------------------------------------------
//
//  CSoundMgr::SetListener (ILTClientSound interface)
//
//  ILTClientSound-compatible interface to SetListener (Position & rotation rather than 
//      direction vectors)
// 
//----------------------------------------------------------------------------------------------
LTRESULT CSoundMgr::SetListener(bool bListenerInClient, LTVector *pPos, LTRotation *pRot, bool bTeleport)
{
    LTVector vForward, vRight, vUp;
    if (bListenerInClient || !pRot)
        SetListener(bListenerInClient, pPos, LTNULL, LTNULL, bTeleport);
    else
    {
        quat_GetVectors((float *)pRot, (float *)&vRight, (float *)&vUp, (float *)&vForward);
        SetListener(bListenerInClient, pPos, &vForward, &vRight, bTeleport);
    }

    return LT_OK;
}


#ifdef USE_DX8_SOFTWARE_FILTERS
LTRESULT CSoundMgr::SetSoundFilter(HLTSOUND hSound, const char *pFilter)
{
    // Make sure we've got a valid sound...
    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilter, LT_ERROR);

    CSoundInstance *pSoundInstance;
    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilter, LT_INVALIDPARAMS);

    // Set the filter
    if ( pSoundInstance->SetFilter(pFilter) != LT_OK )
        RETURN_ERROR(1, CSoundMgr::SetSoundFilter, LT_ERROR);

    return LT_OK;
}

LTRESULT CSoundMgr::SetSoundFilterParam(HLTSOUND hSound, const char *pParam, float fValue)
{
    // Parameter validation..
    if (!pParam)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_INVALIDPARAMS);

    // Make sure we've got a valid sound...
    if (!hSound)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_ERROR);

    CSoundInstance *pSoundInstance;
    pSoundInstance = FindSoundInstance(hSound, true);
    if (!pSoundInstance)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_INVALIDPARAMS);

    // Set the filter parameter
    if (!pSoundInstance->SetFilterParam(pParam, fValue))
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_NOTFOUND);

    return LT_OK;
}

#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

LTRESULT CSoundMgr::SetSoundFilter(const char *pFilter)
{
	// query the sound engine and get the ID of the 
	// filter that matches the requested one
	if ( pFilter == NULL || 
		strcmp( pFilter, "Reverb Filter" ) )
	{
		m_FilterData.bUseFilter = false;
		m_FilterData.uiFilterType = NULL_FILTER;
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_NOTFOUND);
	}
	else
	{
		m_FilterData.bUseFilter = true;
		m_FilterData.uiFilterType = FilterReverb;
		m_FilterData.pSoundFilter = &g_FilterReverb;
	}
   
	return LT_OK;
}
   
LTRESULT CSoundMgr::SetSoundFilterParam(const char *pParam, float fValue)
{
    // Parameter validation..
    if (!pParam)
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_INVALIDPARAMS);
   
    // Set the filter parameter
	if ( pParam == NULL || !m_FilterData.bUseFilter || m_FilterData.pSoundFilter == NULL )
	{
        RETURN_ERROR(1, CSoundMgr::SetSoundFilterParam, LT_NOTFOUND);
	}
	else
	{
		m_FilterData.pSoundFilter->SetParam( pParam, fValue );
	}
   
	return LT_OK;
}

LTRESULT CSoundMgr::EnableSoundFilter(bool bEnable)
{
	DWORD dwIndex;

	if (!g_pSoundSys) return LT_ERROR;

	g_pSoundSys->SetEAX20Filter( bEnable, &m_FilterData );

	// if turning a filter on, reset all the sound-specific filter parameters
	// for any 3D sample in use.
	for ( dwIndex = 0; dwIndex < m_nMax3DSamples; dwIndex++ )
    {
		H3DSAMPLE h3DSample = m_p3DSampleList[dwIndex].m_h3DSample;

		if ( h3DSample != NULL )
			g_pSoundSys->SetEAX20BufferSettings( h3DSample, &m_FilterData );
	}

	return LT_OK;
}
   
#endif
