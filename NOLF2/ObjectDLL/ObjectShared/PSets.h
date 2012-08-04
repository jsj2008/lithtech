/****************************************************************************
;
;	MODULE:			PSets.h
;
;	PURPOSE:		Permission Set include file
;
;	HISTORY:		12/24/2001 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2001, Monolith Productions, Inc.
;
****************************************************************************/

#define ADD_PSETS_PROP(group) \
	PROP_DEFINEGROUP(PermissionSets, group) \
	ADD_BOOLPROP_FLAG(Key1,0,group)\
	ADD_BOOLPROP_FLAG(Key2,0,group)\
	ADD_BOOLPROP_FLAG(Key3,0,group)\
	ADD_BOOLPROP_FLAG(Key4,0,group)\
	ADD_BOOLPROP_FLAG(Key5,0,group)\
	ADD_BOOLPROP_FLAG(Key6,0,group)\
	ADD_BOOLPROP_FLAG(Key7,0,group)\
	ADD_BOOLPROP_FLAG(Key8,0,group)