//-----------------------------------------------------------------------------------------
// ShatterEffectMgr.h
//
// This class acts as the manager for a collection of shatter effects, each one of which
// represents a shattering world model that is composed of debris pieces that are simulated
// physically to create the illusion of breaking glass/tile, etc.
//
//-----------------------------------------------------------------------------------------
#ifndef __SHATTEREFFECTMGR_H__
#define __SHATTEREFFECTMGR_H__

//forward declarations
class CShatterEffect;

class CShatterEffectMgr
{
public:

	CShatterEffectMgr();
	~CShatterEffectMgr();

	//called when a new shatter effect needs to be created, and is passed all the parameters needed
	//to create the new effect
	bool CreateShatterEffect(	uint32 nShatterDataID, const LTRigidTransform& tObjTransform, 
								const LTVector& vHitPos, const LTVector& vHitDir,
								HRECORD hShatterType);

	//called to update all of the shatter effects
	void UpdateShatterEffects(float fElapsedS);

	//called to free all of the shatter effect objects
	void FreeShatterEffects();

private:

	//we don't allow copying of this object
	PREVENT_OBJECT_COPYING(CShatterEffectMgr);

	//the list of currently allocated shatter effects
	typedef std::vector<CShatterEffect*> TShatterList;
	TShatterList	m_ShatterList;

};

#endif
