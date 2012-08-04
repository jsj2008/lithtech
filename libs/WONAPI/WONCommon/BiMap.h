#ifndef __WON_BIMAP_H__
#define __WON_BIMAP_H__
#include "WONShared.h"

#include <map>
namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class Key, class Val, class KeyMapPred = std::less<Key>, class ValMapPred = std::less<Val> > class BiMap 
{
public:
	struct KeyMapValue;

	typedef std::multimap<Key, KeyMapValue*, KeyMapPred> KeyMap;
	typedef std::multimap<Val, typename KeyMap::iterator, ValMapPred> ValMap;
	typedef typename KeyMap::iterator KeyItr;
	typedef typename ValMap::iterator ValItr;
	typedef typename KeyMap::const_iterator ConstKeyItr;
	typedef typename ValMap::const_iterator ConstValItr;

	typedef KeyItr iterator;
	typedef ConstKeyItr const_iterator;

//	typedef std::pair<KeyItr,bool> KeyRet;
	typedef std::pair<ValItr,bool> ValRet;

	struct KeyMapValue
	{
		const Val val;
		ValItr itr;

		KeyMapValue(const Val &theVal, const ValItr &theItr) : val(theVal), itr(theItr) {}
	};

private:
	KeyMap mKeyMap;
	ValMap mValMap;
	bool mKeyMultiMap; // should the key map be treated as a multimap (true) or a normal map (false)?
	bool mValMultiMap; // should the val map be treated as a multimap (true) or a normal map (false)?

public:
	BiMap(bool keymultimap =false, bool valmultimap =false) : mKeyMultiMap(keymultimap), mValMultiMap(valmultimap) {}

	~BiMap()
	{
		KeyItr anItr = mKeyMap.begin();
		while(anItr!=mKeyMap.end())
		{
			delete anItr->second;
			++anItr;
		}
	}

	BiMap(const BiMap& theOtherR) { Copy(theOtherR); }
	const BiMap& operator=(const BiMap& theOtherR) { Copy(theOtherR); return *this; }
	
	bool insert(const Key &theKey, const Val &theVal)
	{
		KeyItr aKeyItr = mKeyMap.find(theKey);
		if(!mKeyMultiMap && aKeyItr!=mKeyMap.end())
			return false;

		ValItr aValItr = mValMap.find(theVal);
		if(!mValMultiMap && aValItr!=mValMap.end())
			return false;

		aValItr = mValMap.insert(ValMap::value_type(theVal,mKeyMap.end()));

		KeyMapValue *aKeyMapVal = new KeyMapValue(theVal, aValItr);
		aKeyItr = mKeyMap.insert(aKeyItr,KeyMap::value_type(theKey,aKeyMapVal));
		aValItr->second = aKeyItr;
		return true;
	}

	
	KeyItr erase(KeyItr theItr) 
	{ 
		KeyMapValue *aVal = theItr->second;
		mKeyMap.erase(theItr++);
		mValMap.erase(aVal->itr);
		delete aVal;
		return theItr;
	}

	KeyItr erase(KeyItr first, KeyItr last) 
	{ 
		while(first!=last)
			first = erase(first);

		return first;
	}


	bool erase(const Key &theKey)
	{
		if(!mKeyMultiMap)
		{
			KeyItr anItr = find(theKey);
			if(anItr==mKeyMap.end())
				return false;

			erase(anItr);
		}
		else
		{
			KeyItr aLowerItr = mKeyMap.lower_bound(theKey);
			KeyItr aUpperItr = mKeyMap.upper_bound(theKey);
			if (aLowerItr == aUpperItr)
				return false;

			erase(aLowerItr, aUpperItr);
		}

		return true;
	}

	ValItr erase_val(ValItr theItr) 
	{ 
		erase((theItr++)->second);
		return theItr;
	}

	ValItr erase_val(ValItr first, ValItr last) 
	{ 
		while(first!=last)
			first = erase_val(first);

		return first;
	}

	bool erase_val(const Val &theVal)
	{
		if(!mValMultiMap)
		{
			ValItr anItr = mValMap.find(theVal);
			if(anItr==mValMap.end())
				return false;

			erase(anItr->second);
		}
		else
		{
			ValItr aLowerItr = mValMap.lower_bound(theVal);
			ValItr aUpperItr = mValMap.upper_bound(theVal);
			if (aLowerItr == aUpperItr)
				return false;

			erase_val(aLowerItr, aUpperItr);
		}

		return true;
	}

	void clear() { erase(begin(),end()); }

	KeyItr find(const Key &theKey) { return mKeyMap.find(theKey); }
	const KeyItr find(const Key &theKey) const { return mKeyMap.find(theKey); }

	KeyItr find_val(const Val &theVal)
	{
		ValItr anItr = mValMap.find(theVal);
		if(anItr==mValMap.end())
			return mKeyMap.end();
		else
			return anItr->second;
	}
	ConstKeyItr find_val(const Val &theVal) const { return const_cast<BiMap<Key, Val, KeyMapPred, ValMapPred>*>(this)->find_val(theVal); }

	KeyItr begin() { return mKeyMap.begin(); }
	KeyItr end()   { return mKeyMap.end(); }
	ConstKeyItr begin() const { return mKeyMap.begin(); }
	ConstKeyItr end()   const { return mKeyMap.end(); }
	
	bool empty() const { return mKeyMap.empty(); }
	int size() const { return mKeyMap.size(); }
protected:
	void Copy(const BiMap& theOtherR)
	{ 
		mKeyMultiMap = theOtherR.mKeyMultiMap;
		mValMultiMap = theOtherR.mValMultiMap;
		// I think this could be made more efficient by creating a custom allocator which took care
		// of allocating the KeyMapValue* and the KeyMapValue for each node.  However, I tried 
		// creating an allocator, and got a bit stumped.  Seemed like DinkumWare's allocator interface
		// might be non-standard.
		// Also might be more efficient to copy the ValMap using DinkumWare's map copying code, and then
		// only deal with inserts into the KeyMap.
		for (const_iterator itr = theOtherR.begin(); itr != theOtherR.end(); ++itr)
			insert(itr->first, itr->second->val);
	}
};

template<class Key, class Val, class KeyMapPred = std::less<Key>, class ValMapPred = std::less<Val> > 
class BiMultiMap : public BiMap<Key,Val,KeyMapPred,ValMapPred>
{
public:
	BiMultiMap() : BiMap<Key,Val,KeyMapPred,ValMapPred>(true,false) {}
};

template<class Key, class Val, class KeyMapPred = std::less<Key>, class ValMapPred = std::less<Val> > 
class BiMultiMultiMap : public BiMap<Key,Val,KeyMapPred,ValMapPred>
{
public:
	BiMultiMultiMap() : BiMap<Key,Val,KeyMapPred,ValMapPred>(true,true) {}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

}; // namespace WONAPI

#endif
