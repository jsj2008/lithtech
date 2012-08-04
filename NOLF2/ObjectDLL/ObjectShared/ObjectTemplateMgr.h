//////////////////////////////////////////////////////////////////////////////
// Object Template handling code

#ifndef __OBJECTTEMPLATEMGR_H__
#define __OBJECTTEMPLATEMGR_H__

#include <hash_map>


#if _MSC_VER >= 1300

class ObjectTemplateMgrHashCompare
{
public:

	// parameters for hash table
	enum
	{
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8		// min_buckets = 2 ^^ N, 0 < N
	};

	ObjectTemplateMgrHashCompare()
	{
	}

	size_t 		operator()(const std::string &pKey) const
	{
		// hash _Keyval to size_t value
		return Convert(pKey);
	}

	bool 		operator()(const std::string &pKey1, const std::string &pKey2) const
	{
		// test if _Keyval1 ordered before _Keyval2
		return Compare(pKey1, pKey2);
	}

private:

	size_t		Convert(const std::string &pKey) const
	{
		uint32 nHash = 0;

		const char *pName = &(*pKey.begin());
		for (; *pName; ++pName)
		{
			nHash = 13 * nHash + (toupper(*pName) - '@');
		}

		return nHash;
	}

	bool 		Compare(const std::string &lhs, const std::string &rhs) const
	{
		return stricmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};

#endif // VC7



class CObjectTemplateMgr
{
public:
	CObjectTemplateMgr();
	~CObjectTemplateMgr();

	void AddTemplate(const ObjectCreateStruct *pOCS);
	const ObjectCreateStruct *FindTemplate(const char *pName) const;
	void Clear();

	void Save( ILTMessage_Write *pMsg );
	void Load( ILTMessage_Read *pMsg );

protected:

#if _MSC_VER == 1300

	typedef std::hash_map< std::string, ObjectCreateStruct, ObjectTemplateMgrHashCompare > TTemplateMap;

#elif _MSC_VER > 1300

	typedef stdext::hash_map< std::string, ObjectCreateStruct, ObjectTemplateMgrHashCompare > TTemplateMap;

#else

	// The template dictionary  (Note the startling similarity to GenericPropList..)

	// Template name comparison functor
	struct SCompare_TemplateName
	{
		bool operator ()(const std::string &cLHS, const std::string &cRHS) const {
			return stricmp(cLHS.c_str(), cRHS.c_str()) == 0;
		}
	};

	// Template name hash functor
	struct SHash_TemplateName
	{
		size_t operator()(const std::string &sName) const {
			uint32 nHash = 0;
			const char *pName = sName.begin();
			for (; *pName; ++pName)
				nHash = 13 * nHash + (toupper(*pName) - '@');
			return nHash;
		}
	};

	// The actual template dictionary type
	typedef std::hash_map<std::string, ObjectCreateStruct, SHash_TemplateName, SCompare_TemplateName> TTemplateMap;

#endif // VC7

	TTemplateMap m_cTemplates;
};

#endif //__OBJECTTEMPLATEMGR_H__
