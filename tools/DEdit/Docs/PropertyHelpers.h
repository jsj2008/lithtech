//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __PROPERTYHELPERS_H__
#define __PROPERTYHELPERS_H__


	CPropList* CreateMainPropertyList(CRegionDoc *pDoc);
	void ReadPropertiesIntoSelections(CRegionDoc *pDoc);
	void ReadPropertyIntoSelections( CRegionDoc *pDoc, CBaseProp *pProp );


#endif  // __PROPERTYHELPERS_H__
