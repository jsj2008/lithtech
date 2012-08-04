#ifndef __FINDWORLDMODEL_H__
#define __FINDWORLDMODEL_H__

	class CEditBrush;
	class CPropList;
	class CEditRegion;

	#define MAX_WORLD_NAME_LEN			64

	class CWorldModelDef
	{
	public:
								CWorldModelDef();

		// Total number of polies in all the brushes.
		uint32					CalcNumPolies();
	
	public:

		// Info flags..
		uint32					m_WorldInfoFlags; // WIF_ flags in de_world.h
		CMoArray<CEditBrush*>	m_Brushes;

		char					m_WorldName[MAX_WORLD_NAME_LEN];
	};


	//this will find a boolean property of the specified name and return the
	//property value associated with it. If it cannot find the property,
	//it will return bDefault
	bool GetBoolPropVal(CPropList *pList, char *pName, bool bDefault = false);
	
	// Finds all the world models.
	// The first world model is always the main world.
	void FindWorldModels(CEditRegion *pRegion, CMoArray<CWorldModelDef*> &worldModels);
	
	uint32 GetBrushType(CEditBrush *pBrush);
	uint8  GetLMGridSize(CEditBrush *pBrush);


#endif  // __FINDWORLDMODEL_H__

