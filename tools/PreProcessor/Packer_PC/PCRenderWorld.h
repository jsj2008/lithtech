//////////////////////////////////////////////////////////////////////////////
// PC-specific world geometry packer

#ifndef __PCRENDERWORLD_H__
#define __PCRENDERWORLD_H__

class CAbstractIO;
class CPreMainWorld;
class CPCRenderTree;

#include <string>
#include <vector>

class CPCRenderWorld
{
public:
	CPCRenderWorld();
	~CPCRenderWorld();

	bool Process(const CPreMainWorld *pWorld);

	bool Write(CAbstractIO &file);

private:
	class CWorldModel
	{
	public:
		CWorldModel();
		~CWorldModel();
		CWorldModel(const char *pName, CPCRenderTree *pOctree);
		CWorldModel(const CWorldModel &cOther);
		CWorldModel &operator=(const CWorldModel &cOther);
		bool Write(CAbstractIO &file) const;
	public:
		std::string m_sName;
		CPCRenderTree *m_pOctree;
	};
	typedef std::vector<CWorldModel> TWorldModelList;

	CPCRenderTree *m_pOctree;

	TWorldModelList m_aWorldModels;
};

#endif //__PCRENDERWORLD_H__