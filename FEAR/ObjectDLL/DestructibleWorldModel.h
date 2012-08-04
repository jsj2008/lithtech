//----------------------------------------------------------------------------------
// DestructibleWorldModel.h
//
// This provides a destructible aggregate that is meant to be used with world models
// that provides all the functionality of a normal destructible aggregate, but also
// adds support for shattering using the shattering information setup in the game
// database.
//
//----------------------------------------------------------------------------------
#ifndef __DESTRUCTIBLEWORLDMODEL_H__
#define __DESTRUCTIBLEWORLDMODEL_H__

#ifndef __DESTRUCTIBLEMODEL_H__
#	include "DestructibleModel.h"
#endif

// Use ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE() in your class definition
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE(PF_GROUP(10), 0)
//	ADD_STRINGPROP(Filename, "")
//  ...
//
#define ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE(group, flags) \
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE((group), (flags)) \
	ADD_STRINGPROP_FLAG(ShatterType, "", (flags) | PF_STATICLIST, "This specifies the type of shattering effect that should be played when the world model is destroyed. This should be set to 'none' if the world model should not shatter") \
	ADD_REALPROP_FLAG(BlindObjectIndex, -1.0f, PF_HIDDEN, "Internal property used to receive the index to the blind object data from the preprocessor")

//----------------------------------------------------------------
//CDestructibleWorldModelPlugin
// handles the plugin interface for WorldEdit

class CDestructibleWorldModelPlugin : 
	public CDestructibleModelPlugin
{
public:
	CDestructibleWorldModelPlugin();
	virtual ~CDestructibleWorldModelPlugin();

	//called by WorldEdit whenever the a string list associated with this object needs to be filled in
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, 
											char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, 
											const uint32 cMaxStringLength);

private:

};

//----------------------------------------------------------------
//CDestructibleWorldModel
// the actual damage aggregate for world models

class CDestructibleWorldModel :
	public CDestructibleModel
{
public :

	CDestructibleWorldModel();
	virtual ~CDestructibleWorldModel();

	//handle serialization to and from a message buffer
	void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
	
protected:

	//we don't allow copying of this object
	PREVENT_OBJECT_COPYING(CDestructibleWorldModel);

	//handle messages that we receive from the engine
	uint32	EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);

	//called to read in properties associated with the object
	bool	ReadProp(const GenericPropList *pProps);

	//called when the object is destroyed and should be shattered
	bool	ShatterWorldModel(const LTVector& vPos, const LTVector& vDir);

	//override the base class's when the object is destroyed so we can create our shatter effect
	virtual void HandleObjectDestroyed(const DamageStruct& DamageInfo);

	//the index to the appropriate blind object data, negative is invalid
	int32		m_nBlindObjectData;

	//the handle to the record that contains all the properties necessary for the shattering effect
	HRECORD		m_hShatterInfo;
};

#endif
