//------------------------------------------------------------------
//
//   MODULE  : KEY.H
//
//   PURPOSE : Defines class CKey
//
//   CREATED : On 11/9/98 At 4:54:01 PM
//
//------------------------------------------------------------------

#ifndef __KEY__H_
	#define __KEY__H_

	// Includes....

	#include "basefx.h"
	#include "vector.h"

	// Structures....

	struct COLOURKEY
	{
						COLOURKEY()
						{
							m_tmKey = 0.0f;
							m_red = 0.0f;
							m_green = 0.0f;
							m_blue = 0.0f;
							m_alpha = 0.0f;
						}

		COLORREF		GetColorRef() { return RGB((int)(m_red * 255.0f),
												   (int)(m_green * 255.0f),
												   (int)(m_blue * 255.0f)); }	
		
		float			m_tmKey;
		float			m_red;
		float			m_green;
		float			m_blue;
		float			m_alpha;
	};

	struct SCALEKEY
	{
						SCALEKEY()
						{
						}

		float			m_tmKey;
		float			m_scale;
	};

	struct MOVEKEY
	{
						MOVEKEY()
						{
							m_tmKey		= 0.0f;
							m_bSelected = FALSE;
						}

		float			m_tmKey;
		CFXVector		m_pos;
		CFXVector		m_anchorPos;
		BOOL			m_bSelected;
	};

	class CKey
	{
		public :

			// Constuctor

											CKey();

			// Destructor

										   ~CKey();

			// Member Functions

			BOOL							Init();
			void							Term();

			void							Select(BOOL bSelect = TRUE) { m_bSelected = bSelect; }
			FX_PROP*						GetProp(char *sName);

			CKey*							Clone()
											{
												CKey *pNewKey = new CKey;

												pNewKey->m_pFxRef	   = m_pFxRef;
												pNewKey->m_tmStart     = m_tmStart;
												pNewKey->m_tmEnd       = m_tmEnd;
												pNewKey->m_dwKeyRepeat = m_dwKeyRepeat;
												pNewKey->m_bSelected   = FALSE;
												pNewKey->m_minScale    = m_minScale;
												pNewKey->m_maxScale    = m_maxScale;
												pNewKey->m_nTrack	   = m_nTrack;
												pNewKey->SetCustomName( GetCustomName() );

												// Copy the properties list

												CFastListNode<FX_PROP> *pNode = m_collProps.GetHead();

												while (pNode)
												{
													pNewKey->m_collProps.AddTail(pNode->m_Data); 

													pNode = pNode->m_pNext;
												}

												CLinkListNode<COLOURKEY> *pColourNode = m_collColourKeys.GetHead();

												while (pColourNode)
												{
													pNewKey->m_collColourKeys.AddTail(pColourNode->m_Data);
													
													pColourNode = pColourNode->m_pNext;
												}

												CLinkListNode<SCALEKEY> *pScaleNode = m_collScaleKeys.GetHead();

												while (pScaleNode)
												{
													pNewKey->m_collScaleKeys.AddTail(pScaleNode->m_Data);

													pScaleNode = pScaleNode->m_pNext;
												}

												CLinkListNode<MOVEKEY> *pMoveNode = m_collMoveKeys.GetHead();

												while (pMoveNode)
												{
													pNewKey->m_collMoveKeys.AddTail(pMoveNode->m_Data);

													pMoveNode = pMoveNode->m_pNext;
												}

												return pNewKey;
											}

			void							SetLink(BOOL bLink, DWORD dwLinkedKeyID, char *sLinkedNodeName)
											{
												if (bLink)
												{
													m_bLinkedToKey = TRUE;
													m_dwLinkedKeyID = dwLinkedKeyID;
													strcpy(m_sLinkedNodeName, sLinkedNodeName);
												}
												else
												{
													m_bLinkedToKey = FALSE;
													m_dwLinkedKeyID = 0;
													memset(m_sLinkedNodeName, 0, 32);
												}
											}

			// Accessors

			FX_REF*							GetFxRef() { return m_pFxRef; }
			DWORD							GetID() { return m_dwUniqueID; }
			DWORD							GetKeyRepeat() { return m_dwKeyRepeat; }
			CFastList<FX_PROP>*				GetCollProps() { return &m_collProps; }
			CLinkList<COLOURKEY>*			GetColourKeys() { return &m_collColourKeys; }
			CLinkList<SCALEKEY>*			GetScaleKeys() { return &m_collScaleKeys; }
			CLinkList<MOVEKEY>*				GetMoveKeys() { return &m_collMoveKeys; }
			int								GetTrack() { return m_nTrack; }
			int								GetStartTime() { return m_tmStart; }
			int								GetEndTime() { return m_tmEnd; }
			int								GetTotalTime() { return m_tmEnd - m_tmStart; }
			float							GetMinScale() { return m_minScale; }
			float							GetMaxScale() { return m_maxScale; }
			float*							GetMinScalePtr() { return &m_minScale; }
			float*							GetMaxScalePtr() { return &m_maxScale; }
			BOOL							IsSelected() { return m_bSelected; }
			int								GetAnchorOffset() { return m_nAnchorOffset; }
			int								GetAnchorLength() { return m_nAnchorLength; }
			BOOL							GetUsePresetAnim() { return m_bUsePreset; }
			int								GetPresetAnim() { return m_nPreset; }
			int								GetRepsAnim() { return m_nReps; }
			BOOL							IsLinked() { return m_bLinkedToKey; }
			DWORD							GetLinkedID() { return m_dwLinkedKeyID; }
			char*							GetLinkedNodeName() { return m_sLinkedNodeName; }

			void							SetFxRef(FX_REF *pFxRef);
			void							SetID(DWORD dwUniqueID) { m_dwUniqueID = dwUniqueID; }
			void							SetKeyRepeat(DWORD dwKeyRepeat) { m_dwKeyRepeat = dwKeyRepeat; }
			void							SetTrack(int nTrack) { m_nTrack = nTrack; }
			void							SetStartTime(int tmStart) { m_tmStart = tmStart; }
			void							SetEndTime(int tmEnd) { m_tmEnd = tmEnd; }
			void							SetMinScale(float minScale) { m_minScale = minScale; }
			void							SetMaxScale(float maxScale) { m_maxScale = maxScale; }
			void							SetAnchorOffset(int nAnchorOffset) { m_nAnchorOffset = nAnchorOffset; }
			void							SetAnchorLength(int nAnchorLength) { m_nAnchorLength = nAnchorLength; }
			void							SetUsePresetAnim(BOOL bUse) { m_bUsePreset = bUse; }
			void							SetPresetAnim(int nAnim) { m_nPreset = nAnim; }
			void							SetRepsAnim(int nReps) { m_nReps = nReps; }

			inline void						SetCustomName( const char *szName )
											{
												if( !szName ) return;
												strcpy( m_szCustomName, szName );
											}
			
			inline const char*				GetCustomName( ) const { return m_szCustomName; }

		public :

			// Member Variables

			FX_REF						   *m_pFxRef;
			DWORD							m_dwUniqueID;
			DWORD							m_dwKeyRepeat;
			CFastList<FX_PROP>				m_collProps;
			CLinkList<COLOURKEY>			m_collColourKeys;
			CLinkList<SCALEKEY>				m_collScaleKeys;
			CLinkList<MOVEKEY>				m_collMoveKeys;
			int								m_nTrack;
			int								m_tmStart;
			int								m_tmEnd;
			float							m_minScale;
			float							m_maxScale;
			BOOL							m_bSelected;
			int								m_nAnchorOffset;
			int								m_nAnchorLength;
			BOOL							m_bUsePreset;
			int								m_nPreset;
			int								m_nReps;

			BOOL							m_bLinkedToKey;
			DWORD							m_dwLinkedKeyID;
			char							m_sLinkedNodeName[32];
	
		private:

			char							m_szCustomName[32];
	};

#endif