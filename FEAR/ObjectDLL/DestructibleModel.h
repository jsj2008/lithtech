// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.h
//
// PURPOSE : Destructible model aggregate class
//
// CREATED : 4/23/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTIBLEMODEL_H__
#define __DESTRUCTIBLEMODEL_H__

#include "Destructible.h"
#include "Arsenal.h"
#include "AIEnumStimulusTypes.h"
#include "CategoryDB.h"
#include "ServerSpecialFX.h" // For SpecialFXPlugin
#include "SurfaceFlagsOverrideHelpers.h"

#define DEFAULT_DWMA_MASS			2000.0f


// Use ADD_DESTRUCTIBLE_MODEL_AGGREGATE() in your class definition
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_MODEL_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_MODEL_AGGREGATE(group, flags) \
	ADD_DESTRUCTIBLE_AGGREGATE((group), (flags)) \
	ADD_REALPROP_FLAG(Mass,					DEFAULT_DWMA_MASS,	(group) | (flags),			"This value sets the mass of the object within the game.") \
	ADD_STRINGPROP_FLAG(SURFACE_FLAGS_OVERRIDE,	SURFACE_FLAGS_UNKNOWN_STR,		(flags) | PF_STATICLIST,	"This allows the selection a specific surface flag for the object. NOTE:  If the Unknown surface flag is used, the surface type of the material applied to the brush will be used.") \
	ADD_STRINGPROP_FLAG(CollisionProperty,	StringList_None,	(flags) | PF_STATICLIST,	"This allows the selection of an overriding CollisionProperty.  If <none> is specified, the surface flag will be used.") \
	ADD_STRINGPROP_FLAG(DestroyedFX,		StringList_None,	(flags) | PF_STATICLIST,	"This allows the selection of a special effect to play when the object is destroyed.  If <none> is specified, no effect will be played.")

class CDestructibleModelPlugin : public CDestructiblePlugin
{

  public:
	CDestructibleModelPlugin();
	virtual ~CDestructibleModelPlugin();
    virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

  private:

	static bool		sm_bInitted;
	SpecialFXPlugin	m_FXPlugin;
};

class CDestructibleModel : public CDestructible
{
	public :

		CDestructibleModel();
		virtual ~CDestructibleModel();
		
		void		SetDestroyedStimulus(float fStimRadius, uint32 nDestroyAlarmLevel);
		float		GetDestroyedStimulusRadius( ) const { return m_fStimRadius; }
		uint32		GetDestroyedStimulusAlarmLevel( ) const { return m_nDestroyAlarmLevel; }

		void		Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void		Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		uint32		m_dwOriginalFlags;

		enum DestructibleModelFlags
		{
			kDestructibleModelFlag_Destroyed			= (1<<0),
			kDestructibleModelFlag_RemoveOnDeath		= (1<<1),
			kDestructibleModelFlag_SaveNeverDestroy		= (1<<2),
			kDestructibleModelFlag_SaveCanDamage		= (1<<3),
		};
		uint8	 m_DestructibleModelFlags;

		char const* m_pszDestroyedFXName;

	protected:

		EnumAIStimulusID	m_eStimID;
		float				m_fStimRadius;
		uint32				m_nDestroyAlarmLevel;

	protected:

		//called when the object has been damaged to the point of being destroyed. This can be overridden
		//by derived classes to apply any custom destruction behavior that is necessary, but it should
		//call up to this function when it is done with it's operations
		virtual void HandleObjectDestroyed(const DamageStruct& DamageInfo);

		uint32	EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);
		uint32	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		bool	ReadProp(const GenericPropList *pProps);
		void	InitialUpdate( const GenericPropList *pProps );
		
		void	SetSurfaceType( HSURFACE hSurfaceOverride );
		void	RegisterDestroyedStimulus();

		void	CreateDestroyedFX(const DamageStruct& DamageInfo);

		// Message Handlers...

		DECLARE_MSG_HANDLER( CDestructibleModel, HandleHiddenMsg );
};

#endif // __DESTRUCTABLE_MODEL_H__
