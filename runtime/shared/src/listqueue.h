/*
	Simplistic templatized FIFO/FILO queue class
	Notes : 
		- Add items at the front or back, removal can only be done from the front
			(i.e. can be used as FIFO or FILO queue)
		- Supports allocation via pools
		- Pool allocation/deallocation does not call the ctor/dtor for the type
*/

#ifndef __LISTQUEUE_H__
#define __LISTQUEUE_H__

template <class T>
struct CListQueue_Iterator;
template <class T>
struct CListQueue_Iterator_Base;
template <class T>
struct CListQueue_Const_Iterator;

template <class T>
class CListQueue
{
public:
	typedef CListQueue<T> t_This;

	CListQueue() : m_pFront(0), m_pBack(0) {}
	~CListQueue() { clear(); }

	//////////////////////////////////////////////////////////////////////////////
	// Iterator declarations

	typedef CListQueue_Iterator<T> iterator;
	typedef CListQueue_Const_Iterator<T> const_iterator;

	iterator begin() { return iterator(m_pFront); }
	iterator end() { return iterator(0); }
	const_iterator begin() const { return const_iterator(m_pFront); }
	const_iterator end() const { return const_iterator(0); }

	// Iterator for the last element
	iterator back_i() { return iterator(m_pBack); }
	const iterator back_i() const { return const_iterator(m_pBack); }

	// Information

	bool empty() const { return m_pFront == 0; }
	size_t size() const { 
		size_t nCount = 0;
		const CItem *pFinger = m_pFront;
		while (pFinger)
		{
			++nCount;
			pFinger = pFinger->m_pNext;
		}
		return nCount;
	}

	// Front/back 

	T &front() { return m_pFront->m_cValue; }
	T &back() { return m_pBack->m_cValue; }

	const T &front() const { return m_pFront->m_cValue; }
	const T &back() const { return m_pBack->m_cValue; }

	// Addition

	void push_front(const T &cValue) { push_front(new CItem(cValue)); }
	void push_back(const T &cValue) { push_back(new CItem(cValue)); }

	void push_front(const T &cValue, t_This &cPool) {
		if (cPool.empty())
			push_front(cValue);
		else
		{
			CItem *pNext = cPool.m_pFront->m_pNext;
			cPool.m_pFront->m_cValue = cValue;
			push_front(cPool.m_pFront);
			cPool.m_pFront = pNext;
		}
	}
	void push_back(const T &cValue, t_This &cPool) {
		if (cPool.empty())
			push_back(cValue);
		else
		{
			CItem *pNext = cPool.m_pFront->m_pNext;
			cPool.m_pFront->m_cValue = cValue;
			push_back(cPool.m_pFront);
			cPool.m_pFront = pNext;
		}
	}

	// Removal

	void pop_front() { delete remove_front(); }
	void pop_front(t_This &cPool) { 
		if (empty()) 
			return;
		cPool.push_front(remove_front());
	}
	void remove(iterator &iPrevious, iterator &iEntry) {
		if (iEntry == end())
			return;
		if (iEntry == begin())
			pop_front();
		else
		{
			ASSERT(iPrevious.m_pFinger->m_pNext == iEntry.m_pFinger);
			if (iEntry == back_i())
				m_pBack = iPrevious->m_pFinger;
			iPrevious.m_pFinger->m_pNext = iEntry.m_pFinger->m_pNext;
			delete iEntry.m_pFinger;
		}
	}
	void remove(iterator &iPrevious, iterator &iEntry, t_This &cPool) {
		if (iEntry == end())
			return;
		if (iEntry == begin())
			pop_front(cPool);
		else
		{
			ASSERT(iPrevious.m_pFinger->m_pNext == iEntry.m_pFinger);
			if (iEntry == back_i())
				m_pBack = iPrevious.m_pFinger;
			iPrevious.m_pFinger->m_pNext = iEntry.m_pFinger->m_pNext;
			cPool.push_front(iEntry.m_pFinger);
		}
	}

	// Misc

	// Splice another list onto this list.  (Clears the "other" list..)
	void splice_front(t_This &cList) {
		if (cList.empty())
			return;
		if (empty())
		{
			swap(cList);
			return;
		}

		// Point at the other guy's list
		cList.m_pBack->m_pNext = m_pFront;
		m_pFront = cList.m_pFront;

		// Clear out the other guy
		cList.m_pFront = 0;
		cList.m_pBack = 0;
	}
	void splice_back(t_This &cList) {
		if (cList.empty())
			return;
		if (empty())
		{
			swap(cList);
			return;
		}

		// Point at the other guy's list
		m_pBack->m_pNext = cList.m_pFront;
		m_pBack = cList.m_pBack;

		// Clear out the other guy
		cList.m_pFront = 0;
		cList.m_pBack = 0;
	}

	void swap(t_This &cOther) {
		CItem *pTemp = cOther.m_pFront;
		cOther.m_pFront = m_pFront;
		m_pFront = pTemp;
		pTemp = cOther.m_pBack;
		cOther.m_pBack = m_pBack;
		m_pBack = pTemp;
	}

	void clear() { 
		while (!empty())
		{
			CItem *pFront = m_pFront;
			m_pFront = pFront->m_pNext;
			delete pFront;
		}
		m_pBack = 0;
	}
	void clear(t_This &cPool) { 
		cPool.splice_front(*this);
	}

private:
	friend struct CListQueue_Iterator_Base<T>;
	friend struct CListQueue_Iterator<T>;
	friend struct CListQueue_Const_Iterator<T>;
	
	struct CItem
	{
		CItem(const T &cValue) : m_cValue(cValue) {}
		T m_cValue;
		CItem *m_pNext;
	};
	CItem *m_pFront, *m_pBack;

	void push_front(CItem *pItem) {
		pItem->m_pNext = m_pFront;
		m_pFront = pItem;
		if (!m_pBack)
			m_pBack = pItem;
	}
	void push_back(CItem *pItem) {
		pItem->m_pNext = 0;
		if (m_pBack)
			m_pBack->m_pNext = pItem;
		else
			m_pFront = pItem;
		m_pBack = pItem;
	}

	CItem *remove_front() { 
		CItem *pResult = m_pFront;
		if (m_pFront)
			m_pFront = m_pFront->m_pNext;
		if (!m_pFront)
			m_pBack = 0;
		return pResult;
	}
};

//////////////////////////////////////////////////////////////////////////////
// CListQueue Iterator declarations

// Note : Please ignore the iterator base class.  Thank you.
template <class T>
struct CListQueue_Iterator_Base
{
	typedef CListQueue_Iterator_Base<T> t_This;

	bool operator==(const t_This &cOther) const { return m_pFinger == cOther.m_pFinger; }
	bool operator!=(const t_This &cOther) const { return m_pFinger != cOther.m_pFinger; }
protected:
	CListQueue_Iterator_Base(const CListQueue_Iterator_Base<T> &cOther) : m_pFinger(cOther.m_pFinger) {}
	CListQueue_Iterator_Base(typename CListQueue<T>::CItem *pFinger) : m_pFinger(pFinger) {}
	typename CListQueue<T>::CItem *m_pFinger;
};

// Non-const iterator
template <class T>
struct CListQueue_Iterator : public CListQueue_Iterator_Base<T>
{
	friend class CListQueue<T>;

	CListQueue_Iterator() {}
	CListQueue_Iterator(const CListQueue_Iterator<T> &cOther) : CListQueue_Iterator_Base<T>(cOther.m_pFinger) {}
	CListQueue_Iterator &operator=(const CListQueue_Iterator<T> &cOther) {
		CListQueue_Iterator_Base<T>::m_pFinger = cOther.m_pFinger;
		return *this;
	}
	T &operator*() const { return CListQueue_Iterator_Base<T>::m_pFinger->m_cValue; }
	T *operator->() const { return &CListQueue_Iterator_Base<T>::m_pFinger->m_cValue; }
	CListQueue_Iterator &operator++() { CListQueue_Iterator_Base<T>::m_pFinger = CListQueue_Iterator_Base<T>::m_pFinger->m_pNext; return *this; }
private:
	CListQueue_Iterator(typename CListQueue<T>::CItem *pItem) : CListQueue_Iterator_Base<T>(pItem) {}
};

// Const iterator
template <class T>
struct CListQueue_Const_Iterator : public CListQueue_Iterator_Base<T>
{
	friend class CListQueue<T>;

	CListQueue_Const_Iterator() {}
	CListQueue_Const_Iterator(const CListQueue_Iterator_Base<T> &cOther) : CListQueue_Iterator_Base<T>(cOther) {}
	CListQueue_Const_Iterator &operator=(const CListQueue_Iterator_Base<T> &cOther) 
	{
		if ( this == &cOther )
		{
			return *this;
		}
		
		CListQueue_Iterator_Base<T>::m_pFinger = cOther.m_pFinger;
		return *this;
	}
	const T &operator*() const { return CListQueue_Iterator_Base<T>::m_pFinger->m_cValue; }
	const T *operator->() const { return &CListQueue_Iterator_Base<T>::m_pFinger->m_cValue; }
	CListQueue_Const_Iterator &operator++() { CListQueue_Iterator_Base<T>::m_pFinger = CListQueue_Iterator_Base<T>::m_pFinger->m_pNext; return *this; }
private:
	CListQueue_Const_Iterator(const typename CListQueue<T>::CItem *pItem) : CListQueue_Iterator_Base<T>(const_cast< typename CListQueue<T>::CItem*>(pItem)) {}
};

#endif  // __LISTQUEUE_H__
