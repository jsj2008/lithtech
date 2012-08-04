/****************************************************************************
;
;	MODULE:		LTDirectMusic_Impl (.H)
;
;	PURPOSE:	Implement DirectMusic capability for LithTech engine
;
;	If NOLITHTECH is defined then special lithtech things are not done.
;
***************************************************************************/

#ifndef __LTDIRECTMUSIC_IMPL_H__
#define __LTDIRECTMUSIC_IMPL_H__


#ifndef __DMUSICI_H__
#include <dmusici.h>
#define __DMUSICI_H__
#endif

#ifndef __DMUSICC_H__
#include <dmusicc.h>
#define __DMUSICC_H__
#endif

#ifndef __LITH_H__
#include "lith.h"
#endif

#ifndef __LITHTMPL_H__
#include "lithtmpl.h"
#endif

#ifndef __ILTDIRECTMUSIC_H__
#include "iltdirectmusic.h"
#endif

#ifndef __LTDIRECTMUSICLOADER_H__
#include "ltdirectmusicloader.h"
#endif

#include "winsync.h"


// Different command types that can be in a command queue
enum LTDMCommandTypes
{
	LTDMCommandNull = 0,
	LTDMCommandStopPlaying,
	LTDMCommandPauseQueue,
	LTDMCommandPlaySegment,
	LTDMCommandPlaySecondarySegment,
	LTDMCommandPlayMotif,
	LTDMCommandAdjustVolume,
	LTDMCommandClearOldCommands,
	LTDMCommandLoopToStart,
	LTDMCommandChangeIntensity,
	LTDMCommandStopSegment,
	LTDMCommandPlayTransition
};


enum LTDMFileTypes
{
	LTDMFileTypeNull = 0,
	LTDMFileTypeAny,
	LTDMFileTypeControlFile,
	LTDMFileTypeDLS,
	LTDMFileTypeStyle,
	LTDMFileTypeSegment,
	LTDMFileTypeChordMap,
};

// forward class defines
class CControlFileMgr;


// Main class for LTDirectMusicMgr
class CLTDirectMusicMgr
#ifndef NOLITHTECH
: public ILTDirectMusicMgr
#endif
{
public:
#ifndef NOLITHTECH
    declare_interface(CLTDirectMusicMgr);
#endif

	////////////////////////////////
	// external functions

	// default constructor
	CLTDirectMusicMgr();

	// default destructor (calls Term if it has not been called)
	~CLTDirectMusicMgr();

	// Initialize the Mgr
	virtual LTRESULT Init();

	// Terminate the Mgr
	virtual LTRESULT Term();

	// Initialize a game level using the parameters in the given control file (up to 3 optional
	// defines can be given to use when processing the control file)
	virtual LTRESULT InitLevel(const char* sWorkingDirectory, const char* sControlFileName, const char* sDefine1 = LTNULL,
					  const char* sDefine2 = LTNULL, const char* sDefine3 = LTNULL);

	// Teminate the current game level
	virtual LTRESULT TermLevel();

	// Begin playing music
	virtual LTRESULT Play();

	// Stop playing music
	virtual LTRESULT Stop(const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Pause music playing
	virtual LTRESULT Pause(const LTDMEnactTypes nStart = LTDMEnactDefault);

	// UnPause music playing
	virtual LTRESULT UnPause();

	// Set current volume
	virtual LTRESULT SetVolume(const long nVolume);

	// Change the intensity level
	virtual LTRESULT ChangeIntensity(const int nNewIntensity, const LTDMEnactTypes nStart = LTDMEnactInvalid);

	// Play a secondary segment
	virtual LTRESULT PlaySecondary(const char* sSecondarySegment, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Stop all secondary segments with the specified name (if LTNULL it stops them all)
	virtual LTRESULT StopSecondary(const char* sSecondarySegment = LTNULL, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Play a motif
	virtual LTRESULT PlayMotif(const char* sMotifName, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Play a motif
	virtual LTRESULT PlayMotif(const char* sStyleName, const char* sMotifName, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Stop all motifs with the specified name (if LTNULL it stops them all)
	virtual LTRESULT StopMotif(const char* sMotifName = LTNULL, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// Stop all motifs with the specified name (if LTNULL it stops them all)
	virtual LTRESULT StopMotif(const char* sStyleName, const char* sMotifName = LTNULL, const LTDMEnactTypes nStart = LTDMEnactDefault);

	// return the current intenisty
	virtual int GetCurIntensity();

	// convert a string to an enact type
	virtual LTDMEnactTypes StringToEnactType(const char* sName);

	// convert an enact type to a string
	void EnactTypeToString(LTDMEnactTypes nType, char* sName);

	// return the number of intensities currently in this level (undefined if not in a level)
	int GetNumIntensities();

	// return the initial intensity value for this level
	int GetInitialIntensity();

	// return the initial volume.
	int GetInitialVolume();

	// return the volume offset.  This offset is applied to whatever volume is set.
	int GetVolumeOffset();

	// return the pointer to the DM performance object
	IDirectMusicPerformance8* GetPerformance() { return m_pPerformance; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	MOST USERS SHOULD NOT NEED TO LOOK PAST HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  ILTDirectMusic ends here.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////
	// Misc extra external functions the user doesn't normally need

	// get the list of all segments
	class CSegmentList;
	CSegmentList& GetSegments() { return m_lstSegments; };

	// get the directmusic performance
	IDirectMusicPerformance8* GetDMPerformance() { return m_pPerformance; };

	// return the directmusic flags value that corresponds to the specified enact value
	uint32 EnactTypeToFlags(LTDMEnactTypes nEnactVal);

	// Set the directmusic working directory
	void SetWorkingDirectory (const char* sWorkingDirectory, LTDMFileTypes nFileType = LTDMFileTypeAny);

#ifdef NOLITHTECH
	// rez file name to load from (this should be called before InitLevel and can be called multiple times for multiple rez files)
	void SetRezFile(const char* sRezFile);
#endif

public:

	// foward class defines
	class CSegment;
	class CTransition;

	////////////////////////////////////////////
	// internal classes for command queue

	// class for command queue item
	class CCommandItem : public CBaseListItem
	{
	public:
		friend class LTDirectMusicMgr;

		// default constructor
		CCommandItem() { m_nCommandType = LTDMCommandNull; };

		// get the command type
		LTDMCommandTypes GetCommandType() { return m_nCommandType; };

		// get the next segment in this list of segments
		CCommandItem* Next() { return (CCommandItem*)CBaseListItem::Next(); };

		// get the previous segment in this list of segments
		CCommandItem* Prev() { return (CCommandItem*)CBaseListItem::Prev(); };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CCommandItem)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CCommandItem*)p); };

		// Chunk allocator for this object (actually for the user derived ItemType class)
		static CLithChunkAllocator<CCommandItem> m_ChunkAllocator;

	protected:
		// set the command type
		void SetCommandType(LTDMCommandTypes nCommandType) { m_nCommandType = nCommandType; };

	private:
		// the type of command that this is
		LTDMCommandTypes m_nCommandType;

	protected:
		// pointer to direct music segment for use by various commands
		CSegment* m_pSegment;

		// misc use int value
		int m_nVal;
	};


	// CCommandItemStopPlaying derived class
	class CCommandItemStopPlaying : public CCommandItem
	{
	public:
		// constructor
		CCommandItemStopPlaying() { SetCommandType(LTDMCommandStopPlaying); };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};


	// CCommandItemPauseQueue derived class
	class CCommandItemPauseQueue : public CCommandItem
	{
	public:
		// constructor
		CCommandItemPauseQueue() { SetCommandType(LTDMCommandPauseQueue); };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};


	// CCommandItemPlaySegment derived class
	class CCommandItemPlaySegment : public CCommandItem
	{
	public:
		// constructor
		CCommandItemPlaySegment () { SetCommandType(LTDMCommandPlaySegment); };

		// get the segment pointer associated with this item
		CSegment* GetSegment() { return m_pSegment; };

		// set the segment pointer associated with this item
		void SetSegment(CSegment* pSeg) { m_pSegment = pSeg; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemPlayTransition derived class
	class CCommandItemPlayTransition : public CCommandItem
	{
	public:
		// constructor
		CCommandItemPlayTransition () { SetCommandType(LTDMCommandPlayTransition); };

		// get the segment pointer associated with this item
		CTransition* GetTransition() { return (CTransition*)m_pSegment; };

		// set the segment pointer associated with this item
		void SetTransition(CTransition* pSeg) { m_pSegment = (CSegment*)pSeg; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemPlaySecondarySegment derived class
	class CCommandItemPlaySecondarySegment : public CCommandItem
	{
	public:
		// constructor
		CCommandItemPlaySecondarySegment () { SetCommandType(LTDMCommandPlaySecondarySegment); };

		// get the segment pointer associated with this item
		CSegment* GetSegment() { return m_pSegment; };

		// set the segment pointer associated with this item
		void SetSegment(CSegment* pSeg) { m_pSegment = pSeg; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemPlayMotif derived class
	class CCommandItemPlayMotif : public CCommandItem
	{
	public:
		// constructor
		CCommandItemPlayMotif () { SetCommandType(LTDMCommandPlayMotif); };

		// get the segment pointer associated with this item
		CSegment* GetSegment() { return m_pSegment; };

		// set the segment pointer associated with this item
		void SetSegment(CSegment* pSeg) { m_pSegment = pSeg; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemAdjustVolume derived class
	class CCommandItemAdjustVolume : public CCommandItem
	{
	public:
		// constructor
		CCommandItemAdjustVolume () { SetCommandType(LTDMCommandAdjustVolume); };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemClearOldCommands derived class
	class CCommandItemClearOldCommands : public CCommandItem
	{
	public:
		// constructor
		CCommandItemClearOldCommands () { SetCommandType(LTDMCommandClearOldCommands); };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	// CCommandItemLoopToStart derived class
	class CCommandItemLoopToStart : public CCommandItem
	{
	public:
		// constructor
		CCommandItemLoopToStart () { SetCommandType(LTDMCommandLoopToStart); };

		// get the number of times to loop (-1 for infinite)
		int GetNumLoops() { return m_nVal; };

		// set the number of times to loop (-1 for infinite)
		void SetNumLoops(int nNumLoops) { m_nVal = nNumLoops; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};


	// CCommandChangeIntensity derived class
	class CCommandChangeIntensity : public CCommandItem
	{
	public:
		// constructor
		CCommandChangeIntensity () { SetCommandType(LTDMCommandChangeIntensity); };

		// get the new intensity to set
		int GetNewIntensity() { return m_nVal; };

		// set the new intensity to set value
		void SetNewIntensity(int nNewIntensity) { m_nVal = nNewIntensity; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};


	// class for list of segment items
	class CCommandItemList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CCommandItem* GetFirst() { return (CCommandItem*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CCommandItem* GetLast() { return (CCommandItem*)CLTBaseList::GetLast(); };
	};


	// CCommandItemStopSegment derived class
	class CCommandItemStopSegment : public CCommandItem
	{
	public:
		// constructor
		CCommandItemStopSegment () { SetCommandType(LTDMCommandStopSegment); };

		// get the segment pointer associated with this item
		CSegment* GetSegment() { return m_pSegment; };

		// set the segment pointer associated with this item
		void SetSegment(CSegment* pSeg) { m_pSegment = pSeg; };

	private:
		// ALL MEMBER VARIABLES MUST BE IN THE BASE CLASS BECAUSE OF CHUNK ALLOCTOR!!!
	};

	/////////////////////////////////////////////////
	// internal classes for segment items and lists
	// class for segment item
	class CSegment : public CBaseListItem
	{
	public:
		CSegment() { m_pDMSegment = LTNULL; m_sSegmentName[0] = '\0'; m_sSegmentNameLong = LTNULL; m_nDefaultEnact = LTDMEnactNextMeasure; };
		~CSegment() { if (m_sSegmentNameLong != LTNULL) delete [] m_sSegmentNameLong; };

		// get the segment pointer associated with this item
		IDirectMusicSegment8* GetDMSegment() { return m_pDMSegment; };

		// set the segment pointer associated with this item
		void SetDMSegment(IDirectMusicSegment8* pDMSeg) { m_pDMSegment = pDMSeg; };

		// get the next segment in this list of segments
		CSegment* Next() { return (CSegment*)CBaseListItem::Next(); };

		// get the previous segment in this list of segments
		CSegment* Prev() { return (CSegment*)CBaseListItem::Prev(); };

		// get the segment name
		const char* GetSegmentName();

		// set the segment name (returns true if successful)
		bool SetSegmentName(const char* sSegmentName);

		// set the default enact time
		void SetDefaultEnact(LTDMEnactTypes nDefaultEnact) {m_nDefaultEnact = nDefaultEnact;}

		// get the default enact time
		LTDMEnactTypes GetDefaultEnact() { return m_nDefaultEnact; };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CSegment)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CSegment*)p); };

		// Chunk allocator for this object
		static CLithChunkAllocator<CSegment> m_ChunkAllocator;

	private:

		// internal member variable conatining pointer to the direct music segment
		IDirectMusicSegment8* m_pDMSegment;

		// the name of the segment if it is shorter than 64 characters
		char m_sSegmentName[64];

		// the name of the segment if it is longer than 63
		char* m_sSegmentNameLong;

		// internal member variable containing the default enact time for this segment
		LTDMEnactTypes m_nDefaultEnact;
	};


	// class for list of segment items
	class CSegmentList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CSegment* GetFirst() { return (CSegment*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CSegment* GetLast() { return (CSegment*)CLTBaseList::GetLast(); };

		// find the segment object that has the given name
		CSegment* Find(const char* sName);

		// find the segment object that has the given direct music segment pointer
		CSegment* Find(const IDirectMusicSegment8* pDMSegment);

		// cleanup a segment (stops, deletes, and removes from list)
		void CleanupSegment(CLTDirectMusicMgr* pLTDMMgr, CSegment* pSegment, LTDMEnactTypes nStart = LTDMEnactImmediatly, bool bOnlyIfNotPlaying = false);

		// cleanup non-playing segments
		void CleanupSegments(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying = false);
	};


	/////////////////////////////////////////////////
	// internal classes for segment state items and lists

	// class for segment item
	class CSegmentState : public CBaseListItem
	{
	public:
		CSegmentState() { m_pDMSegmentState = LTNULL; m_pSegment = LTNULL; };
		~CSegmentState() { };

		// get the segment pointer associated with this item
		IDirectMusicSegmentState8* GetDMSegmentState() { return m_pDMSegmentState; };

		// set the segment pointer associated with this item
		void SetDMSegmentState(IDirectMusicSegmentState8* pDMSegState) { m_pDMSegmentState = pDMSegState; };

		// get the segment that was used to create this segment state
		CSegment* GetSegment() { return m_pSegment; };

		// set the segment that was used to create this segment state
		void SetSegment(CSegment* pSeg) { m_pSegment = pSeg; };

		// get the next segment in this list of segments
		CSegmentState* Next() { return (CSegmentState*)CBaseListItem::Next(); };

		// get the previous segment in this list of segments
		CSegmentState* Prev() { return (CSegmentState*)CBaseListItem::Prev(); };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CSegmentState)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CSegmentState*)p); };

		// Chunk allocator for this object
		static CLithChunkAllocator<CSegmentState> m_ChunkAllocator;

	private:

		// internal member variable conatining pointer to the direct music segment state
		IDirectMusicSegmentState8* m_pDMSegmentState;

		// pointer to the segment that this state was made from
		CSegment* m_pSegment;
	};


	// class for list of segment items
	class CSegmentStateList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CSegmentState* GetFirst() { return (CSegmentState*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CSegmentState* GetLast() { return (CSegmentState*)CLTBaseList::GetLast(); };

		// find the segment object that has the given name
		CSegmentState* Find(const char* sName);

		// find the segment object that has the given direct music segment pointer
		CSegmentState* Find(const IDirectMusicSegmentState8* pDMSegmentState);

		// add a new segment state (creates, and add to the list)
		void CreateSegmentState(IDirectMusicSegmentState8* pDMSegmentState, CSegment* pSegment);

		// cleanup a segment state (stops, deletes, and removes from list)
		void CleanupSegmentState(CLTDirectMusicMgr* pLTDMMgr, CSegmentState* pSegmentState, LTDMEnactTypes nStart = LTDMEnactImmediatly, bool bOnlyIfNotPlaying = false);

		// cleanup segment states
		void CleanupSegmentStates(CLTDirectMusicMgr* pLTDMMgr, bool bOnlyIfNotPlaying = false);
	};


	/////////////////////////////////////////////////
	// internal classes for band items and lists

	// class for band item
	class CBand : public CBaseListItem
	{
	public:
		// get the band pointer associated with this item
		IDirectMusicBand8* GetBand() { return m_pBand; };

		// set the band pointer associated with this item
		void SetBand(IDirectMusicBand8* pSeg) { m_pBand = pSeg; };

		// get the next band in this list of bands
		CBand* Next() { return (CBand*)CBaseListItem::Next(); };

		// get the previous band in this list of bands
		CBand* Prev() { return (CBand*)CBaseListItem::Prev(); };

		// get the segment pointer associated with this item
		IDirectMusicSegment8* GetDMSegment() { return m_pDMSegment; };

		// set the segment pointer associated with this item
		void SetDMSegment(IDirectMusicSegment8* pDMSeg) { m_pDMSegment = pDMSeg; };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CBand)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CBand*)p); };

		// Chunk allocator for this object
		static CLithChunkAllocator<CBand> m_ChunkAllocator;

	private:

		// internal member variable conatining pointer to the direct music band
		IDirectMusicBand8* m_pBand;

		// the band segment
		IDirectMusicSegment8* m_pDMSegment;
	};


	// class for list of band items
	class CBandList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CBand* GetFirst() { return (CBand*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CBand* GetLast() { return (CBand*)CLTBaseList::GetLast(); };
	};


	/////////////////////////////////////////////////
	// internal classes for style items and lists

	// class for style item
	class CStyle : public CBaseListItem
	{
	public:
		CStyle() { m_pDMStyle = LTNULL; m_sStyleName[0] = '\0'; m_sStyleNameLong = LTNULL; };
		~CStyle() { if (m_sStyleNameLong != LTNULL) delete [] m_sStyleNameLong; };

		// get the style pointer associated with this item
		IDirectMusicStyle8* GetDMStyle() { return m_pDMStyle; };

		// set the style pointer associated with this item
		void SetDMStyle(IDirectMusicStyle8* pStyle) { m_pDMStyle = pStyle; };

		// get the segment name
		const char* GetStyleName();

		// set the segment name (returns true if successful)
		bool SetStyleName(const char* sStyleName);

		// get the next style in this list of styles
		CStyle* Next() { return (CStyle*)CBaseListItem::Next(); };

		// get the previous style in this list of styles
		CStyle* Prev() { return (CStyle*)CBaseListItem::Prev(); };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CStyle)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CStyle*)p); };

		// Chunk allocator for this object
		static CLithChunkAllocator<CStyle> m_ChunkAllocator;

	private:

		// internal member variable conatining pointer to the direct music style
		IDirectMusicStyle8* m_pDMStyle;

		// the name of the style if it is shorter than 64 characters
		char m_sStyleName[64];

		// the name of the style if it is longer than 63
		char* m_sStyleNameLong;
	};


	// class for list of style items
	class CStyleList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CStyle* GetFirst() { return (CStyle*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CStyle* GetLast() { return (CStyle*)CLTBaseList::GetLast(); };

		// find the style object that has the given name
		CStyle* Find(const char* sName);
	};


	/////////////////////////////////////////////////
	// internal classes for dls bank items and lists

	// class for DLSBank item
	class CDLSBank : public CBaseListItem
	{
	public:
		// get the DLSBank pointer associated with this item
		IDirectMusicCollection8* GetDLSBank() { return m_pDLSBank; };

		// set the DLSBank pointer associated with this item
		void SetDLSBank(IDirectMusicCollection8* pSeg) { m_pDLSBank = pSeg; };

		// get the next DLSBank in this list of DLSBanks
		CDLSBank* Next() { return (CDLSBank*)CBaseListItem::Next(); };

		// get the previous DLSBank in this list of DLSBanks
		CDLSBank* Prev() { return (CDLSBank*)CBaseListItem::Prev(); };

		// Chunk allocation new and delete operators
		void *operator new(size_t sz) { ASSERT(sz == sizeof(CDLSBank)); return (void*)m_ChunkAllocator.Alloc(); };
		void operator delete(void* p) {	m_ChunkAllocator.Free((CDLSBank*)p); };

		// Chunk allocator for this object
		static CLithChunkAllocator<CDLSBank> m_ChunkAllocator;

	private:

		// internal member variable conatining pointer to the direct music DLSBank
		IDirectMusicCollection8* m_pDLSBank;
	};


	// class for list of DLSBank items
	class CDLSBankList : public CLTBaseList
	{
	public:
		// get the first item in the list
		CDLSBank* GetFirst() { return (CDLSBank*)CLTBaseList::GetFirst(); };

		// get the last item in the list
		CDLSBank* GetLast() { return (CDLSBank*)CLTBaseList::GetLast(); };
	};


	//////////////////////////////////////////////////
	// internal class that defines a transition
	class CTransition
	{
	public:
		// Get the EnactTime for this transition
		LTDMEnactTypes GetEnactTime() { return m_nEnactTime; };

		// Set the EnactTime for this transition
		void SetEnactTime(LTDMEnactTypes nEnactTime) { m_nEnactTime = nEnactTime; };

		// Check if this is a manual transition
		bool GetManual() { return m_bManual; };

		// Set the manual transition flag
		void SetManual(bool bManual) { m_bManual = bManual; };

		// get the segment pointer associated with this item
		IDirectMusicSegment8* GetDMSegment() { return m_pDMSegment; };

		// set the segment pointer associated with this item
		void SetDMSegment(IDirectMusicSegment8* pDMSeg) { m_pDMSegment = pDMSeg; };

	private:
		// Time that we will begin to enact this transition
		LTDMEnactTypes m_nEnactTime;

		// This is true if we are manually playing our own transition or using no transition
		// If false that means we are using DirectMusic transitions
		bool m_bManual;

		// internal member variable conatining pointer to the direct music segment
		IDirectMusicSegment8* m_pDMSegment;
	};


	//////////////////////////////////////////////////
	// internal class that defines an intensity level
	class CIntensity
	{
	public:
		~CIntensity();

		int GetNumLoops() { return m_nNumLoops; };
		void SetNumLoops(int nNumLoops) { m_nNumLoops = nNumLoops; };

		int GetIntensityToSetAtFinish() { return m_nIntensityToSetAtFinish; };
		void SetIntensityToSetAtFinish(int nIntensityToSetAtFinish) { m_nIntensityToSetAtFinish = nIntensityToSetAtFinish; };

		CSegmentList& GetSegmentList() { return m_lstSegments; };

	private:
		// number of times to play through list of segments before finished (-1 = infinite)
		int m_nNumLoops;

		// intensity level to switch to after this intensity is finished (not used if loop is infinite)
		int m_nIntensityToSetAtFinish;

		// list of segments to play for this intensity
		CSegmentList m_lstSegments;
	};

	////////////////////////////////////////////
	// Defines the state of playback when paused
	class CPauseState
	{
	public:
		CPauseState() :
			m_pDMSegment(0),
			m_nPlayTime(0)
			{
			}
		~CPauseState()
			{
				if (m_pDMSegment)
					m_pDMSegment->Release();
			}
		IDirectMusicSegment* GetSegment() const { return m_pDMSegment; }
		void SetSegment(IDirectMusicSegment* pState)
			{
				if (m_pDMSegment)
					m_pDMSegment->Release();
				m_pDMSegment = pState;
				m_pDMSegment->AddRef();
			}

		MUSIC_TIME GetPlayTime() const { return m_nPlayTime; }
		void SetPlayTime(MUSIC_TIME nTime) { m_nPlayTime = nTime; }

		void Clear()
			{
				if (m_pDMSegment)
					m_pDMSegment->Release();
				m_pDMSegment = 0;
				m_nPlayTime = 0;
			}

	private:
		IDirectMusicSegment* m_pDMSegment;
		MUSIC_TIME m_nPlayTime;
	};

private:

	////////////////////////////////////////////
	// internal functions

	// Initialize all the basic directmusic stuff (Com, Performance, Loader)
	bool InitDirectMusic();

	// Terminate everything that InitDirectMusic set up
	void TermDirectMusic();

	// read in and set up DLS Banks from control file
	bool ReadDLSBanks(CControlFileMgr& controlFile);

	// read in and load styles and bands from control file
	bool ReadStylesAndBands(CControlFileMgr& controlFile);

	// read in intensity descriptions from control file
	bool ReadIntensities(CControlFileMgr& controlFile);

	// read in secondary segments from control file
	bool ReadSecondarySegments(CControlFileMgr& controlFile);

	// read in motifs from control file
	bool ReadMotifs(CControlFileMgr& controlFile);

	// read in transition matrix from control file
	bool ReadTransitions(CControlFileMgr& controlFile);

	// Load Segment
	bool LoadSegment(const char* sSegmentName );

	// Load DLS Bank
	bool LoadDLSBank(const char* sFileName);

	// Load Style and associated bands
	bool LoadStyleAndBands(char* sStyleFileName, CControlFileMgr& controlFile);

	// Load Band
	bool LoadBand(IDirectMusicStyle8* pStyle, const char* sBandName);

	// Clear the command queue
	void ClearCommands();

	// Get a pointer to the transition from one intenisty to another
	CTransition* GetTransition(int nFrom, int nTo);

	// Set reverb parameters
	bool SetReverbParameters(LPDSFXWavesReverb pParams);

	// Enable reverb
	bool EnableReverb();

	// Disable reverb
	bool DisableReverb();

	// Initialize reverb parameters from control file and set up reverb in DirectMusic
	bool InitReverb(CControlFileMgr& controlFile);

	// Terminate reverb if it was enabled.
	bool TermReverb();

	// Initialize the synthesizer state
	bool InitPerformance();

	////////////////////////////////////////////
	// internal static functions

	void HandleSegmentNotification( DMUS_NOTIFICATION_PMSG* pPmsg);
		void HandleSegmentNotification_SegAlmostEnd( DMUS_NOTIFICATION_PMSG* pPmsg, IDirectMusicSegmentState8* pDMSegState, IDirectMusicSegment* pDMSegment);
		void HandleSegmentNotification_SegEnd( DMUS_NOTIFICATION_PMSG* pPmsg, IDirectMusicSegmentState8* pDMSegState, IDirectMusicSegment* pDMSegment);

	static DWORD __stdcall CommandThread_Bootstrap(void *pData);
	uint32 CommandThread();


	////////////////////////////////////////////
	// internal member variables

	// if true then Mgr is initialized
	bool m_bInitialized;

	// if true then a we currently are in an initialized level
	bool m_bLevelInitialized;

	// the direct music interface that we are using
	IDirectMusic* m_pDirectMusic;

	// the direct music loader that we are using
	CLTDMLoader* m_pLoader;

	// the direct music performance that we are using
	IDirectMusicPerformance8* m_pPerformance;

	// the DirectMusic default audiopath
	IDirectMusicAudioPath* m_pAudiopath;

	// the DirectMusic buffer in the default audiopath
	IDirectSoundBuffer8* m_pAudiopathBuffer;

	// the DirectMusic reverb FX for the audiopath
    LPDIRECTSOUNDFXWAVESREVERB8 m_pReverb;

	// synthesizer audio parameters
	DMUS_AUDIOPARAMS m_audioParams;

	// parameters for the current port
	DMUS_PORTPARAMS m_portParams;

	// all primairy and secondary segments in current level
	CSegmentList m_lstSegments;

	// all motif segments in current level
	CSegmentList m_lstMotifs;

	// bands in current level
	CBandList m_lstBands;

	// styles in current level
	CStyleList m_lstStyles;

	// DLS banks in current level
	CDLSBankList m_lstDLSBanks;

	// command queue for current level
	CCommandItemList m_lstCommands;

	// second command queue just for secondary segments and motifs
	CCommandItemList m_lstCommands2;

	// last command that was processed
	CCommandItem* m_pLastCommand;

	// list of all secondary segments that are currently playing
	CSegmentStateList m_lstPrimairySegmentsPlaying;

	// list of all secondary segments that are currently playing
	CSegmentStateList m_lstSecondarySegmentsPlaying;

	// list of all motifs that are currently playing
	CSegmentStateList m_lstMotifsPlaying;

	// critical section for the command queue
	CWinSync_CS m_CommandQueueCriticalSection;

	// the current intensity
	int m_nCurIntensity;

	// the previous intensity
	int m_nPrevIntensity;

	// number of intensities defined
	int m_nNumIntensities;

	// initial intensity to start playing at when Play function is called
	int m_nInitialIntensity;

	// initial volume to play at
	int m_nInitialVolume;

	// Volume offset to apply.
	int m_nVolumeOffset;

	// nunber of pchannels to use
	int m_nNumPChannels;

	// number of voices to use
	int m_nNumVoices;

	// sample rate to set synthesizer to
	int m_nSynthSampleRate;

	// ary of intensities for this level (0 position in array is always null for intensity 0)
	CIntensity* m_aryIntensities;

	// number of transitions in transition array
	int m_nNumTransitions;

	// transition matrix to use for intensity transitions stored [from,to]
	// contains m_nNumIntenisities*m_nNumIntenisities values.
	CTransition* m_aryTransitions;

	// current directmusic working directory for any file type
	char* m_sWorkingDirectoryAny;

	// current directmusic working directory for control files
	char* m_sWorkingDirectoryControlFile;

#ifdef NOLITHTECH
	// name of the rez file
	char* m_sRezFileName;
#endif

	// GUID for directmusic notifications
	GUID m_guid;

	// notification handle
	CWinSync_PulseEvent m_cNotify;

	// used to signal the thread it is time to exit
	CWinSync_Event m_cExitNotificationThread;

	// handle for our notification thread
	HANDLE m_hThread;

	// set to true if reverb is to be used
	bool m_bUseReverb;

	// contains the reverb parameters the user has chosen (or the defaults)
	DSFXWavesReverb m_ReverbParameters;

	// contains the FX descriptor for the reverb
    DSEFFECTDESC	m_ReverbDesc;

	// Paused status
	uint32	m_nPaused;
	CPauseState m_cCurPauseState;
};

#endif // __LTDIRECTMUSIC_IMPL_H__

