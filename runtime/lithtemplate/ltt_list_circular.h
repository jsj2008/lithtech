#ifndef __LTT_LIST_CIRCULAR_H__
#define __LTT_LIST_CIRCULAR_H__

//---------------------------------------------------------------------
//
//  Simple circular linked list head and element template classes.
//
//---------------------------------------------------------------------

//classes implemented in this file.
template<class T> class CListCircularElement;
template<class T> class CListCircularHead;


class CListCircularElementPointers {
public:
    inline CListCircularElementPointers();

    //pointers to next and previous elements.
    CListCircularElementPointers *next, *prev;

    //adds an element after this one.
    inline void _Add(CListCircularElementPointers *element);
};

template <class T>
class CListCircularElement : public CListCircularElementPointers {
    //list heads get to access our private data
    friend class CListCircularHead<T>;

  public:
    inline ~CListCircularElement();
    
    //gets next list element.
    inline CListCircularElement<T> *Next();

    //gets previous list element.
    inline CListCircularElement<T> *Prev();

    //gets the actual object stored in this element.
    inline T &Item();
    inline operator T &();

    //removes this element from the list.
    inline void RemoveFromList();

  private:
    //the actual item we store.
    T item;
};

template <class T>
class CListCircularHead : public CListCircularElementPointers {
  public:
    inline ~CListCircularHead();

    inline void Clear();

    //adds an item to the front of the list.
    inline void Add(CListCircularElement<T> *item);

    //gets the first list element from the list.
    inline CListCircularElement<T> *First();

    //gets the last list element from the list.
    inline CListCircularElement<T> *Last();

    //returns true if the given list element is the head of the list
    //(And therefore not really a list element at all.)
    inline bool IsHead(CListCircularElement<T> *element);

    //returns true if the list is empty.
    inline bool IsEmpty();
};


//
//CListCircularElementPointers function definitions.
//

CListCircularElementPointers::CListCircularElementPointers() {
    next = prev = this;
}

void CListCircularElementPointers::_Add(CListCircularElementPointers *element) {
    //set pointers in the new item.
    element->prev = this;
    element->next = next;

    //set our next
    next = element;

    //set prev of element in front of new one
    element->next->prev = element;
}




//
//CListCircularElement function definitions.
//

template <class T>
CListCircularElement<T>::~CListCircularElement() {
    //remove ourself from the list.
    RemoveFromList();
}

template <class T>
CListCircularElement<T> *CListCircularElement<T>::Next() {
    return (CListCircularElement<T> *)next;
}

template <class T>
CListCircularElement<T> *CListCircularElement<T>::Prev() {
    return (CListCircularElement<T> *)prev;
}

template <class T>
T &CListCircularElement<T>::Item() {
    return item;
}

template <class T>
CListCircularElement<T>::operator T &() {
    return item;
}

template <class T>
void CListCircularElement<T>::RemoveFromList() {
    //set next's prev pointer
    next->prev = prev;

    //set prev's next pointer.
    prev->next = next;
}





//
//CListCircularHead function definitions.
//

template <class T>
CListCircularHead<T>::~CListCircularHead() {
    //delete all of our elements.
    while (IsHead(First()) == false) {
        //delete the first element.
        delete First();
    }
}

template <class T>
void CListCircularHead<T>::Clear() {
	// Delete everything in the list
	CListCircularElement<T> *pCur = First();
	while (!IsHead(pCur))
	{
		CListCircularElement<T> *pDeleteMe = pCur;
		pCur = pCur->Next();
		delete pDeleteMe;
	}
    //reset our pointers.
    next = prev = this;
}

template <class T>
void CListCircularHead<T>::Add(CListCircularElement<T> *element) {
    //add the given element after ourself.
    _Add(element);
}

template <class T>
CListCircularElement<T> *CListCircularHead<T>::First() {
    return (CListCircularElement<T> *)next;
}

template <class T>
CListCircularElement<T> *CListCircularHead<T>::Last() {
    return (CListCircularElement<T> *)prev;
}

template <class T>
bool CListCircularHead<T>::IsHead(CListCircularElement<T> *element) {
    return (CListCircularElementPointers *)element == 
           (CListCircularElementPointers *)this;
}

template <class T>
bool CListCircularHead<T>::IsEmpty() {
    return IsHead(First());
}




#endif

