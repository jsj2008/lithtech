// ----------------------------------------------------------------------- //
//
// MODULE  : TranslucentWorldModel.h
//
// PURPOSE : TranslucentWorldModel definition
//
// CREATED : 4/12/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRANSLUCENT_WORLD_MODEL_H__
#define __TRANSLUCENT_WORLD_MODEL_H__

#include "DestructibleModel.h"
#include "GameBase.h"
#include "SFXFuncs.h"

class TranslucentWorldModel : public GameBase
{
	public :

		TranslucentWorldModel();
		virtual ~TranslucentWorldModel();

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL   m_bLensFlare;      // Should we make a lens flare
        LTFLOAT  m_fInitialAlpha;   // Starting alpha
        LTFLOAT  m_fFinalAlpha;     // Ending alpha (if changing)
        LTFLOAT  m_fChangeTime;     // Time to change alpha
        LTFLOAT  m_fStartTime;      // Time we stated changing alpha

		LENSFLARE	m_LensInfo;		// Lens flare info
		LTBOOL	 m_bIsKeyframed;	// Is the world model keyframed.

	private :

		CDestructibleModel m_damage;

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);

        LTBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	Update();

		void	HandleTrigger(HOBJECT hSender, const char* szMsg);
};

class CTranslucentWorldModelPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :
	  CDestructibleModelPlugin m_DestructibleModelPlugin;
};

#endif // __TRANSLUCENT_WORLD_MODEL_H__