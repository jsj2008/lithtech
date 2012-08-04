//------------------------------------------------------------------
//
//	FILE	  : SoundTrack.h
//
//	PURPOSE	  : Defines the CSoundTrack classes 
//				so the server can keep track of certain sounds
//
//	CREATED	  : 5/11/98
//
//	COPYRIGHT : MONOLITH Inc 1998 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SOUNDTRACK_H__
#define __SOUNDTRACK_H__

struct UsedFile;
class CSoundData;
struct InterLink;
class CServerMgr;


//------------------------------------------------------------------------
//
//	CLASS:		CSoundTrack
//
//	PURPOSE:	Instance of a sound that is tracked by the server.
//				If the server must also track the time, then this has
//				has a pointer to a CSoundData object.
//
//------------------------------------------------------------------------
class CSoundTrack
{
	public:

		LTBOOL			Init( PlaySoundInfo *pPlaySoundInfo, float fStartTime, UsedFile *pFile, CSoundData *pSoundData = LTNULL, bool bTrackTime = TRUE );
		void			Term( );

		void			Update( float fDeltaTime );

		float			GetDuration( );
		float			GetTimeLeft( );
		LTBOOL			IsTrackTime( );
		float			GetStartTime( ) { return m_fStartTime; }
		LTBOOL			IsDone( );

		LTObject *		GetObject( );
		void			SetRemove( LTBOOL bRemove ) { m_bRemove = bRemove; }
		LTBOOL			GetRemove( ) { return m_bRemove; }

		void			AddRef( );
		void			Release( uint8 *nClientSoundFlags );

		LTLink			m_Link;			// Link for list of CSoundTracks

		LTLink *		m_pIDLink;		// Link to ID list

		LTBOOL			m_bRemove;

		bool			m_bTrackTime;
		
		UsedFile *		m_pFile;
		CSoundData *	m_pSoundData;
		float			m_fStartTime;
		float			m_fTimeLeft;
		float			m_fDuration;

		uint32			m_nClientRefs;

		uint16			m_wChangeFlags;
		CSoundTrack *	m_pChangedNext;

		// PlaySoundInfo information...
		uint32			m_dwFlags;
		union
		{
			InterLink *	m_pInterLink;
			LTObject *	m_pClientLocalObject;
		};
		unsigned char	m_nPriority;
		float			m_fOuterRadius;
		float			m_fInnerRadius;
		uint8			m_nVolume;
		float			m_fPitchShift;
		LTVector		m_vPosition;

		uint32			m_UserData;

};


// Adds the object to the list of changed objects.
void AddSoundTrackToChangeList(CSoundTrack *pSoundTrack);

// ORs the object's flags with the flags you specify and adds
// the object to the 'changed object' list.
LTRESULT SetSoundTrackChangeFlags(CSoundTrack *pSoundTrack, uint32 flags);

#endif  // __SOUNDTRACK_H__
