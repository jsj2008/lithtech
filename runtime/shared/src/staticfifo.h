/*
	Static FIFO class

	Notes : 
		- Ctor/dtor handling is done at queue creation/destruction (it's a static array...)
		- Iterators don't support + and - operators
		- The size of the actual queue is actually SIZE + 1.  To access the extra entry,
			dereference the end iterator.  It will be overwritten on the next push.
*/

#pragma warning (disable:4284) // -> operator warning on PODT

// Forward declarations for iterators
template <class T, int SIZE>
struct CStaticFIFO_Iterator_Base;
template <class T, int SIZE>
struct CStaticFIFO_Iterator;
template <class T, int SIZE>
struct CStaticFIFO_Iterator_Const;

template <class T, int SIZE>
class CStaticFIFO
{
	friend struct CStaticFIFO_Iterator_Base<T,SIZE>;
	friend struct CStaticFIFO_Iterator<T,SIZE>;
	friend struct CStaticFIFO_Iterator_Const<T,SIZE>;
public:
	CStaticFIFO() : m_pBegin(m_aQueue), m_pEnd(m_aQueue) {}

	//////////////////////////////////////////////////////////////////////////////
	// Information

	// Is the queue full?
	bool full() const { return _next(m_pEnd) == m_pBegin; }
	// Is the queue empty?
	bool empty() const { return m_pBegin == m_pEnd; }
	// How many entries are active in the queue?
	bool size() const { return (m_pBegin < m_pEnd) ? (m_pEnd - m_pBegin) : ((k_nQueueSize - (m_pBegin - m_aQueue)) + (m_pEnd - m_aQueue)); }

	//////////////////////////////////////////////////////////////////////////////
	// Manipulation

	// Push returns true if the queue overflowed with this push
	bool push(const T &cValue) { 
		*m_pEnd = cValue; 
		m_pEnd = _next(m_pEnd); 
		if (m_pBegin == m_pEnd)
		{
			m_pBegin = _next(m_pBegin);
			return true;
		}
		else
			return false;
	}
	void pop() { 
		if (empty())
			return;
		m_pBegin = _next(m_pBegin);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Iterators

	typedef CStaticFIFO_Iterator<T,SIZE> iterator;
	typedef CStaticFIFO_Iterator_Const<T,SIZE> const_iterator;

	iterator begin() { return iterator(this, m_pBegin); }
	const_iterator begin() const { return const_iterator(this, m_pBegin); }
	iterator end() { return iterator(this, m_pEnd); }
	const_iterator end() const { return const_iterator(this, m_pEnd); }

	T &front() { return *m_pBegin; }
	const T &front() const { return *m_pBegin; }
	T &back() { return *_prev(m_pEnd); }
	const T &back() const { return *_prev(m_pEnd); }

private:
	T *_next(T *pIndex) const { return ((pIndex - m_aQueue) < (k_nQueueSize - 1)) ? (pIndex + 1) : const_cast<T*>(m_aQueue); }
	T *_prev(T *pIndex) const { return (pIndex == m_aQueue) ? const_cast<T*>(&m_aQueue[k_nQueueSize - 1]) : (pIndex - 1); }
	const T *_next(const T *pIndex) const { return ((pIndex - m_aQueue) < (k_nQueueSize - 1)) ? (pIndex + 1) : m_aQueue; }
	const T *_prev(const T *pIndex) const { return (pIndex == m_aQueue) ? &m_aQueue[k_nQueueSize - 1] : (pIndex - 1); }

	enum { k_nQueueSize = SIZE + 1 };
	T m_aQueue[k_nQueueSize];
	T *m_pBegin, *m_pEnd;
};

template <class T, int SIZE>
struct CStaticFIFO_Iterator_Base
{
	typedef CStaticFIFO<T,SIZE> t_FIFO;
	typedef CStaticFIFO_Iterator_Base<T,SIZE> t_This;

	friend class CStaticFIFO<T,SIZE>;
	
	bool operator==(const t_This &cOther) const { return (m_pFIFO == cOther.m_pFIFO) && (m_pIndex == cOther.m_pIndex); }
	bool operator!=(const t_This &cOther) const { return (m_pFIFO != cOther.m_pFIFO) || (m_pIndex != cOther.m_pIndex); }
protected:
	CStaticFIFO_Iterator_Base(const t_This &cOther) { new(this) t_This(cOther.m_pFIFO, cOther.m_pIndex); }
	CStaticFIFO_Iterator_Base(const t_FIFO *pFIFO, T *pIndex) : m_pFIFO(pFIFO), m_pIndex(pIndex) {}

	const t_FIFO *m_pFIFO;
	T *m_pIndex;
};

template <class T, int SIZE>
struct CStaticFIFO_Iterator : public CStaticFIFO_Iterator_Base<T,SIZE>
{
    typedef CStaticFIFO<T,SIZE> t_FIFO;
	typedef CStaticFIFO_Iterator<T,SIZE> t_This;
	typedef CStaticFIFO_Iterator_Base<T,SIZE> t_Parent;

	friend class CStaticFIFO<T,SIZE>;
	
	CStaticFIFO_Iterator() {}
	CStaticFIFO_Iterator(const t_This &cOther) : t_Parent(cOther) {}
	t_This &operator=(const t_This &cOther) { new(this) t_This(cOther); return *this; }

	T& operator*() const { return *m_pIndex; }
	T* operator->() const { return &(operator*()); }
	t_This &operator++() { m_pIndex = m_pFIFO->_next(m_pIndex); return *this; }
	t_This &operator--() { m_pIndex = m_pFIFO->_prev(m_pIndex); return *this; }
private:
	CStaticFIFO_Iterator(const t_FIFO *pFIFO, T *pIndex) : t_Parent(pFIFO, pIndex) {}
};

template <class T, int SIZE>
struct CStaticFIFO_Iterator_Const : public CStaticFIFO_Iterator_Base<T,SIZE>
{

	typedef CStaticFIFO<T,SIZE> t_FIFO;
	typedef CStaticFIFO_Iterator_Const<T,SIZE> t_This;
	typedef CStaticFIFO_Iterator_Base<T,SIZE> t_Parent;
	
	friend class CStaticFIFO<T,SIZE>;

	CStaticFIFO_Iterator_Const() {}
	CStaticFIFO_Iterator_Const(const t_Parent &cOther) : t_Parent(cOther) {}
	t_This &operator=(const t_Parent &cOther) { new(this) t_This(cOther); return *this; }

	const T& operator*() const { return *m_pIndex; }
	const T* operator->() const { return &(operator*()); }
	t_This &operator++() { m_pIndex = m_pFIFO->_next(m_pIndex); return *this; }
	t_This &operator--() { m_pIndex = m_pFIFO->_prev(m_pIndex); return *this; }
private:
	CStaticFIFO_Iterator_Const(const t_FIFO *pFIFO, T *pIndex) : t_Parent(pFIFO, pIndex) {}
};
