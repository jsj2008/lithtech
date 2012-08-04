#ifndef __HOLDERLIST_H__
#define __HOLDERLIST_H__

class ISortableHolder;

class CHolderList
{
public:

	CHolderList();
	~CHolderList();

	//adds a holder onto the list. If the holder is already in the list, no
	//duplicate is added
	bool				AddHolder(ISortableHolder* pHolder, bool bSearchForDuplicates);

	//determines if this holder is the active holder
	bool				IsActiveHolder(const ISortableHolder* pHolder) const;

	//gets the active holder
	ISortableHolder*	GetActiveHolder();

	//removes a holder from the list
	bool				RemoveHolder(ISortableHolder* pHolder);

	//gets the specified holder
	ISortableHolder*	GetHolder(uint32 nIndex);

	//gets the number of holders
	uint32				GetNumHolders() const;

	//frees everything assocaited with the list
	void				Free();

	//sorts the holders in the list by the given camera position and orientation
	bool				Sort(const LTVector& vCameraPos, const LTVector& vCameraDir);


private:

	bool				ResizeArray(uint32 nSize);

	//the actual list of holders
	ISortableHolder**			m_ppHolders;

	//the number of holders in the list
	uint32						m_nNumHolders;

	//the size of the list
	uint32						m_nBufferSize;
};


#endif

