// ------------------------------------------------------------------------- //
//
// FILE      : LTIDToObjectTable.h
//
// PURPOSE   : Table for doing fast object lookups by id
//
// CREATED   : 09/12/2002
//
// COPYRIGHT : LithTech, Inc.
//
// ------------------------------------------------------------------------- //

#ifndef __LTIDTOOBJECTTABLE_H__
#define __LTIDTOOBJECTTABLE_H__


#include "ltbasedefs.h"
#include <map>



//-------------------------------------------------------------------------------------------------
// LTIDToObjectTable
//-------------------------------------------------------------------------------------------------


// LTIDToObjectTable -- table for doing lookups using an integer id.
//
//	classes of Type are required to have the following member functions:
//		const char* GetName()
//		void SetID(int ID)
//		void SetName(const char *pName);
//		void SetNext(Type *pType);
//
template <class Type, class MapType>
class LTIDToObjectTable
{
public:

	LTIDToObjectTable()
	{
	}

	~LTIDToObjectTable()
	{
		Term();
	}

	// Remove all entries from the table.
	void		Term()
	{
		for (MapType::iterator iter(m_Map.begin()); iter != m_Map.end(); ++iter)
		{
			delete iter->second;
			iter->second = LTNULL;
		}

		m_Map.clear();
	}

	// Fast lookup by ID
	Type*		Get(int ID)
	{
		MapType::iterator iter(m_Map.find(ID));
		if (iter != m_Map.end())
		{
			return iter->second;
		}

		return LTNULL;
	}

	// Very slow lookup by name.
	// This function searches the map by iterating and comparing strings.
	Type*		Get(const char *pName)
	{
		for (MapType::iterator iter(m_Map.begin()); iter != m_Map.end(); ++iter)
		{
			if (lstrcmpi(pName, iter->second->GetName()) == 0)
			{
				return iter->second;
			}
		}

		return LTNULL;
	}

	// Access to the first element.
	Type*		GetFront()
	{
		return (!m_Map.empty()) ? m_Map.begin()->second : LTNULL;
	}

	// Add an entry to the table.
	Type*		Add(const char *pName, int ID)
	{
		// Make sure this is not a duplicate.
		Type* pType = Get(ID);
		if (pType == LTNULL)
		{
			LT_MEM_TRACK_ALLOC(pType = new Type, LT_MEM_TYPE_HASHTABLE);

			// id
			pType->SetID(ID);

			// name
			pType->SetName(pName);

			// Add it to the list.
			m_Map[ID] = pType;

			// Update the action pointers.
			UpdateNextPointers();

			return pType;
		}

		return LTNULL;
	}


	// Remove an entry from the table.
	void		Remove(int ID)
	{
		MapType::iterator iter(m_Map.find(ID));
		if (iter != m_Map.end())
		{
			// Delete the action.
			delete iter->second;
			iter->second = LTNULL;

			// Remove it from the list.
			m_Map.erase(iter);

			// Update the action pointers.
			UpdateNextPointers();
		}
	}

protected:

	// Classes of type "Type" contain a "next" pointer so that the list can be iterated without having to use the STL.
	// In order to maintain these pointers, we need to walk the map and reset each "next" pointer.
	void		UpdateNextPointers()
	{
		for (MapType::iterator iter(m_Map.begin()); iter != m_Map.end(); ++iter)
		{
			MapType::iterator iterNext = iter;
			++iterNext;

			if (iterNext == m_Map.end())
			{
				iter->second->SetNext(LTNULL);
			}
			else
			{
				iter->second->SetNext(iterNext->second);
			}
		}
	}

protected:

	MapType		m_Map;
};



#endif // __LTIDTOOBJECTTABLE_H__
