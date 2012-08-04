#include "globalhotkeydb.h"
#include "eventnames.h"
#include "resource.h"
#include "bindkeydlg.h"
#include "extendedvk.h"
#include "keydefaultaggregate.h"

CHotKeyDB				CGlobalHotKeyDB::m_DB;
CKeyDefaultAggregate*	CGlobalHotKeyDB::m_pAggregate = NULL;

void CGlobalHotKeyDB::Init()
{
	m_DB.ClearKeys();
	SetDefaults(m_DB);
}

void CGlobalHotKeyDB::AddAggregate(CKeyDefaultAggregate* pAggregate)
{
	if(pAggregate)
	{
		//make sure the chain stays intact
		pAggregate->SetAggregate(m_pAggregate);

		//now set this as the lead aggregate
		m_pAggregate = pAggregate;
	}
}

void CGlobalHotKeyDB::ClearAggregateList()
{
	delete m_pAggregate;
	m_pAggregate = NULL;
}

//gets the text for a menu item from the hotkey database
CString	CGlobalHotKeyDB::GetMenuTextFromHotKey(const char* pszHotKeyName)
{
	CString sRV;

	//try and get the hotkey
	const CHotKey* pHotKey = CGlobalHotKeyDB::m_DB.GetHotKey( pszHotKeyName );
	
	if( pHotKey )
	{
		//get the text for the accelerator
		CString str;
		str = CBindKeyDlg::HotKeyToString( *pHotKey );

		//build up the final string
		sRV.Format( "%s\t%s", pHotKey->GetDescription(), str );
	}

	return sRV;
}

//determines if the name passed in is a valid hotkey for the default configuration
bool CGlobalHotKeyDB::IsValidKeyName(const char* pszKeyName)
{
	//just make a dummy hotkey and see if we can initialize it ok or not
	CHotKey HotKey;
	HotKey.SetEventName(pszKeyName);

	return SetKeyDefault(HotKey);
}


#define MAKE_HOTKEY(name, desc)		HotKey.SetEventName(name);		\
									HotKey.ResetEvents();			\
									sDesc.LoadString(desc);			\
									HotKey.SetDescription(sDesc);	\
									SetKeyDefault(HotKey);			\
									HotKey.SortEvents();			\
									DB.SetHotKey(HotKey);

void CGlobalHotKeyDB::SetDefaults(CHotKeyDB& DB)
{
	CHotKey	HotKey;
	CString sDesc;

	MAKE_HOTKEY(UIE_TRACKER_FOCUS, IDS_UIE_TRACKER_FOCUS);

	MAKE_HOTKEY(UIE_SELECT_DEPTH, IDS_UIE_SELECT_DEPTH);

	MAKE_HOTKEY(UIE_LOCK_FOCUS, IDS_UIE_LOCK_FOCUS);

	MAKE_HOTKEY(UIE_ZOOM, IDS_UIE_ZOOM);
	MAKE_HOTKEY(UIE_ZOOM_FAST, IDS_UIE_ZOOM_FAST);

	//various primitive creation hotkeys
	MAKE_HOTKEY(UIE_CREATE_DOME, IDS_UIE_CREATE_DOME);
	MAKE_HOTKEY(UIE_CREATE_SPHERE, IDS_UIE_CREATE_SPHERE);
	MAKE_HOTKEY(UIE_CREATE_PLANE, IDS_UIE_CREATE_PLANE);
	MAKE_HOTKEY(UIE_CREATE_PYRAMID, IDS_UIE_CREATE_PYRAMID);
	MAKE_HOTKEY(UIE_CREATE_CYLINDER, IDS_UIE_CREATE_CYLINDER);
	MAKE_HOTKEY(UIE_CREATE_BOX, IDS_UIE_CREATE_BOX);

	//make the hotkeys for the different views
	MAKE_HOTKEY(UIE_TOP_VIEW, IDS_UIE_TOP_VIEW);
	MAKE_HOTKEY(UIE_BOTTOM_VIEW, IDS_UIE_BOTTOM_VIEW);
	MAKE_HOTKEY(UIE_LEFT_VIEW, IDS_UIE_LEFT_VIEW);
	MAKE_HOTKEY(UIE_RIGHT_VIEW, IDS_UIE_RIGHT_VIEW);
	MAKE_HOTKEY(UIE_FRONT_VIEW, IDS_UIE_FRONT_VIEW);
	MAKE_HOTKEY(UIE_BACK_VIEW, IDS_UIE_BACK_VIEW);
	MAKE_HOTKEY(UIE_PERSPECTIVE_VIEW, IDS_UIE_PERSPECTIVE_VIEW);

	MAKE_HOTKEY(UIE_NAV_ARC_ROTATE, IDS_UIE_NAV_ARC_ROTATE);
	MAKE_HOTKEY(UIE_NAV_ARC_ROTATE_ROLL, IDS_UIE_NAV_ARC_ROTATE_ROLL);

	MAKE_HOTKEY(UIE_NAV_DRAG, IDS_UIE_NAV_DRAG);
	MAKE_HOTKEY(UIE_NAV_DRAG_FAST, IDS_UIE_NAV_DRAG_FAST);

	MAKE_HOTKEY(UIE_APPLY_TEXTURE_CLICK, IDS_UIE_APPLY_TEXTURE_CLICK);
	MAKE_HOTKEY(UIE_APPLY_TEXTURE_TO_SEL, IDS_UIE_APPLY_TEXTURE_TO_SEL);
	MAKE_HOTKEY(UIE_REMOVE_TEXTURE, IDS_UIE_REMOVE_TEXTURE);

	// Set up the speed test callback
	/*
	MAKE_HOTKEY(UIE_TEST_RENDER_SPEED, IDS_UIE_TEST_RENDER_SPEED);
	*/

	// Set up the split callback
	MAKE_HOTKEY(UIE_SPLIT_BRUSH, IDS_UIE_SPLIT_BRUSH);

	// Set up the unselect callback
	MAKE_HOTKEY(UIE_SELECT_NONE, IDS_UIE_SELECT_NONE);

	// Set up the grid direction callbacks
	MAKE_HOTKEY(UIE_SET_GRID_FORWARD, IDS_UIE_SET_GRID_FORWARD);
	MAKE_HOTKEY(UIE_SET_GRID_RIGHT, IDS_UIE_SET_GRID_RIGHT);
	MAKE_HOTKEY(UIE_SET_GRID_UP, IDS_UIE_SET_GRID_UP);

	// Set up the init texture space callback
	MAKE_HOTKEY(UIE_INIT_TEXTURE_SPACE, IDS_UIE_INIT_TEXTURE_SPACE);

	// Set up the delete callback
	MAKE_HOTKEY(UIE_DELETE, IDS_UIE_DELETE);

	// Set up the nudge callbacks
	MAKE_HOTKEY(UIE_NUDGE_UP, IDS_UIE_NUDGE_UP);
	MAKE_HOTKEY(UIE_NUDGE_DOWN, IDS_UIE_NUDGE_DOWN);
	MAKE_HOTKEY(UIE_NUDGE_LEFT, IDS_UIE_NUDGE_LEFT);
	MAKE_HOTKEY(UIE_NUDGE_RIGHT, IDS_UIE_NUDGE_RIGHT);
	
	// Set up the delete edge callbacks
	MAKE_HOTKEY(UIE_REMOVE_EDGES, IDS_UIE_REMOVE_EDGES);
	MAKE_HOTKEY(UIE_DELETE_EDGES, IDS_UIE_DELETE_EDGES);
	
	// Set up the split edge callback
	MAKE_HOTKEY(UIE_SPLIT_EDGE, IDS_UIE_SPLIT_EDGE);

	// Set up the delete tagged callback
	MAKE_HOTKEY(UIE_DELETE_TAGGED_POLIES, IDS_UIE_DELETE_TAGGED_POLIES);

	// Set up the grid on poly callback
	MAKE_HOTKEY(UIE_GRID_TO_POLY, IDS_UIE_GRID_TO_POLY);

	// Set up the poly vert tag callback
	MAKE_HOTKEY(UIE_TAG_POLY_VERTS, IDS_UIE_TAG_POLY_VERTS);

	// Set up the flip normal callback
	MAKE_HOTKEY(UIE_FLIP_NORMAL, IDS_UIE_FLIP_NORMAL);

	// Set up the orbit vertex callback
	MAKE_HOTKEY(UIE_ORBIT_VERTEX, IDS_UIE_ORBIT_VERTEX);

	// Set up the brush rotation tracker
	MAKE_HOTKEY(UIE_NODE_ROTATE, IDS_UIE_NODE_ROTATE);
	MAKE_HOTKEY(UIE_NODE_ROTATE_STEP, IDS_UIE_NODE_ROTATE_STEP);

	// Set up the texture movement tracker
	MAKE_HOTKEY(UIE_TEXTURE_MOVE, IDS_UIE_TEXTURE_MOVE);
	MAKE_HOTKEY(UIE_TEXTURE_MOVE_ROTATE, IDS_UIE_TEXTURE_MOVE_ROTATE);
	MAKE_HOTKEY(UIE_TEXTURE_MOVE_SCALE, IDS_UIE_TEXTURE_MOVE_SCALE);
	MAKE_HOTKEY(UIE_TEXTURE_MOVE_STEP, IDS_UIE_TEXTURE_MOVE_STEP);

	// Set up the brush sizing tracker
	MAKE_HOTKEY(UIE_BRUSH_SIZE, IDS_UIE_BRUSH_SIZE);

	// Set up the object sizing tracker
	MAKE_HOTKEY(UIE_OBJECT_SIZE, IDS_UIE_OBJECT_SIZE);

	// Set up the curve editing tracker
	MAKE_HOTKEY(UIE_CURVE_EDIT, IDS_UIE_CURVE_EDIT);

	// Set up the far clipping plane tracker
	MAKE_HOTKEY(UIE_FAR_DIST, IDS_UIE_FAR_DIST);

	// Set up the grid sizing tracker
	MAKE_HOTKEY(UIE_GRID_SIZE, IDS_UIE_GRID_SIZE);

	MAKE_HOTKEY(UIE_NODE_MOVE_LOCK_AXIS, IDS_UIE_NODE_MOVE_LOCK_AXIS);
	MAKE_HOTKEY(UIE_NODE_MOVE_CLONE, IDS_UIE_NODE_MOVE_CLONE);

	// Set up the marker placement tracker
	MAKE_HOTKEY(UIE_MARKER_MOVE, IDS_UIE_MARKER_MOVE);

	MAKE_HOTKEY(UIE_MARKER_CENTER, IDS_UIE_MARKER_CENTER);
	MAKE_HOTKEY(UIE_CENTER_SEL_ON_MARKER, IDS_UIE_CENTER_SEL_ON_MARKER);

	//hotkey for fitting a texture to a polygon
	MAKE_HOTKEY(UIE_FIT_TEXTURE_TO_POLY, IDS_UIE_FIT_TEXTURE_TO_POLY);

	// Set up the vertex movement trackers
	MAKE_HOTKEY(UIE_MOVE_VERT, IDS_UIE_MOVE_VERT);
	MAKE_HOTKEY(UIE_MOVE_VERT_PERP, IDS_UIE_MOVE_VERT_PERP);
	MAKE_HOTKEY(UIE_MOVE_VERT_SNAP, IDS_UIE_MOVE_VERT_SNAP);

	MAKE_HOTKEY(UIE_MOVE_SEL_VERT, IDS_UIE_MOVE_SEL_VERT);
	MAKE_HOTKEY(UIE_MOVE_IMM_VERT, IDS_UIE_MOVE_IMM_VERT);

	// Set up the snapping vertex movement trackers
	MAKE_HOTKEY(UIE_VERT_SNAP_EDGE_SEL, IDS_UIE_VERT_SNAP_EDGE_SEL);
	MAKE_HOTKEY(UIE_VERT_SNAP_EDGE, IDS_UIE_VERT_SNAP_EDGE);
	MAKE_HOTKEY(UIE_VERT_SNAP_VERT_SEL, IDS_UIE_VERT_SNAP_VERT_SEL);
	MAKE_HOTKEY(UIE_VERT_SNAP_VERT, IDS_UIE_VERT_SNAP_VERT);
	
	// Set up the face scaling trackers
	MAKE_HOTKEY(UIE_SCALE_POLY, IDS_UIE_SCALE_POLY);
	MAKE_HOTKEY(UIE_SCALE_POLY_DIVIDE, IDS_UIE_SCALE_POLY_DIVIDE);
	MAKE_HOTKEY(UIE_SCALE_POLY_EXTRUDE, IDS_UIE_SCALE_POLY_EXTRUDE);

	// Set up the vertex scaling trackers
	MAKE_HOTKEY(UIE_SCALE_VERT_SEL, IDS_UIE_SCALE_VERT_SEL);
	MAKE_HOTKEY(UIE_SCALE_VERT_IMM, IDS_UIE_SCALE_VERT_IMM);

	// Set up the extrusion trackers
	MAKE_HOTKEY(UIE_EXTRUDE_POLY, IDS_UIE_EXTRUDE_POLY);
	MAKE_HOTKEY(UIE_EXTRUDE_POLY_SNAP, IDS_UIE_EXTRUDE_POLY_SNAP);
	MAKE_HOTKEY(UIE_EXTRUDE_POLY_BRUSH, IDS_UIE_EXTRUDE_POLY_BRUSH);
	
	// Set up the node movement trackers
	MAKE_HOTKEY(UIE_MOVE_NODE_HANDLE, IDS_UIE_MOVE_NODE_HANDLE);
	MAKE_HOTKEY(UIE_SNAP_NODE, IDS_UIE_SNAP_NODE);
	MAKE_HOTKEY(UIE_MOVE_NODE_PERP, IDS_UIE_MOVE_NODE_PERP);

	// Set up the navigation movement tracker
	MAKE_HOTKEY(UIE_NAV_MOVE, IDS_UIE_NAV_MOVE);
	MAKE_HOTKEY(UIE_NAV_MOVE_PERP, IDS_UIE_NAV_MOVE_PERP);
	MAKE_HOTKEY(UIE_NAV_MOVE_FAST, IDS_UIE_NAV_MOVE_FAST);

	// Set up the navigation rotation tracker
	MAKE_HOTKEY(UIE_NAV_ROTATE, IDS_UIE_NAV_ROTATE);
	MAKE_HOTKEY(UIE_NAV_ROTATE_FORWARD, IDS_UIE_NAV_ROTATE_FORWARD);
	MAKE_HOTKEY(UIE_NAV_ROTATE_BACKWARD, IDS_UIE_NAV_ROTATE_BACKWARD);

	//set up the orbit rotation tracker
	MAKE_HOTKEY(UIE_NAV_ORBIT, IDS_UIE_NAV_ORBIT);
	MAKE_HOTKEY(UIE_NAV_ORBIT_MOVE, IDS_UIE_NAV_ORBIT_MOVE);

	// Set up the Draw Poly callback
	MAKE_HOTKEY(UIE_DRAW_POLY, IDS_UIE_DRAW_POLY);
	MAKE_HOTKEY(UIE_DRAW_POLY_UNDO, IDS_UIE_DRAW_POLY_UNDO);
	MAKE_HOTKEY(UIE_DRAW_POLY_NEW_VERTEX, IDS_UIE_DRAW_POLY_NEW_VERTEX);
	MAKE_HOTKEY(UIE_DRAW_POLY_SPLIT, IDS_UIE_DRAW_POLY_SPLIT);
	MAKE_HOTKEY(UIE_DRAW_POLY_ROTATE, IDS_UIE_DRAW_POLY_ROTATE);
	MAKE_HOTKEY(UIE_DRAW_POLY_INSERT_EDGE, IDS_UIE_DRAW_POLY_INSERT_EDGE);
	MAKE_HOTKEY(UIE_DRAW_POLY_VERT_SNAP, IDS_UIE_DRAW_POLY_VERT_SNAP);
	MAKE_HOTKEY(UIE_DRAW_POLY_CLOSE, IDS_UIE_DRAW_POLY_CLOSE);


	// Set up the tagging trackers
	MAKE_HOTKEY(UIE_TAG_ALL, IDS_UIE_TAG_ALL);
	MAKE_HOTKEY(UIE_TAG_ADD, IDS_UIE_TAG_ADD);
	MAKE_HOTKEY(UIE_TAG_INVERT, IDS_UIE_TAG_INVERT);

	MAKE_HOTKEY(UIE_UNJOIN_BRUSHES, IDS_UIE_UNJOIN_BRUSHES);
	MAKE_HOTKEY(UIE_JOIN_BRUSHES, IDS_UIE_JOIN_BRUSHES);
	MAKE_HOTKEY(UIE_TRI_BRUSHES, IDS_UIE_TRI_BRUSHES);

	//set up the maximize options
	MAKE_HOTKEY(UIE_MAXIMIZE_VIEW1, IDS_UIE_MAXIMIZE_VIEW1);
	MAKE_HOTKEY(UIE_MAXIMIZE_VIEW2, IDS_UIE_MAXIMIZE_VIEW2);
	MAKE_HOTKEY(UIE_MAXIMIZE_VIEW3, IDS_UIE_MAXIMIZE_VIEW3);
	MAKE_HOTKEY(UIE_MAXIMIZE_VIEW4, IDS_UIE_MAXIMIZE_VIEW4);
	MAKE_HOTKEY(UIE_MAXIMIZE_ACTIVE_VIEW, IDS_UIE_MAXIMIZE_ACTIVE_VIEW);
	MAKE_HOTKEY(UIE_RESET_VIEWPORTS, IDS_UIE_RESET_VIEWPORTS);

	MAKE_HOTKEY(UIE_FREEZE_SELECTED, IDS_UIE_FREEZE_SELECTED);
	MAKE_HOTKEY(UIE_UNFREEZE_ALL, IDS_UIE_UNFREEZE_ALL);

	MAKE_HOTKEY(UIE_EDIT_UNDO, IDS_UIE_EDIT_UNDO);
	MAKE_HOTKEY(UIE_EDIT_REDO, IDS_UIE_EDIT_REDO);

	MAKE_HOTKEY(UIE_ADD_OBJECT, IDS_UIE_ADD_OBJECT);

	MAKE_HOTKEY(UIE_FIT_TEXTURE_TO_POLY, IDS_UIE_FIT_TEXTURE_TO_POLY);

	MAKE_HOTKEY(UIE_TOGGLE_CLASS_ICONS, IDS_UIE_TOGGLE_CLASS_ICONS);
	MAKE_HOTKEY(UIE_HIDE_FROZEN_NODES,	IDS_UIE_HIDE_FROZEN_NODES);

	MAKE_HOTKEY(UIE_CAMERA_TO_OBJECT, IDS_UIE_CAMERA_TO_OBJECT);
	MAKE_HOTKEY(UIE_DISCONNECT_SELECTED_PREFABS, IDS_UIE_DISCONNECT_SELECTED_PREFABS);

	MAKE_HOTKEY(UIE_SHRINK_GRID_SPACING, IDS_UIE_SHRINK_GRID_SPACING);
	MAKE_HOTKEY(UIE_EXPAND_GRID_SPACING, IDS_UIE_EXPAND_GRID_SPACING);

	MAKE_HOTKEY(UIE_APPLY_COLOR, IDS_UIE_APPLY_COLOR);
	MAKE_HOTKEY(UIE_TEXTURE_WRAP, IDS_UIE_TEXTURE_WRAP);
	MAKE_HOTKEY(UIE_TEXTURE_WRAP_SKIP, IDS_UIE_TEXTURE_WRAP_SKIP)
	MAKE_HOTKEY(UIE_MAP_TEXTURE_COORDS, IDS_UIE_MAP_TEXTURE_COORDS);
	MAKE_HOTKEY(UIE_MAP_TEXTURE_TO_VIEW, IDS_UIE_MAP_TEXTURE_TO_VIEW);
	MAKE_HOTKEY(UIE_RESET_TEXTURE_COORDS, IDS_UIE_RESET_TEXTURE_COORDS);
	MAKE_HOTKEY(UIE_REPLACE_TEXTURES, IDS_UIE_REPLACE_TEXTURES);
	MAKE_HOTKEY(UIE_REPLACE_TEXTURES_IN_SEL, IDS_UIE_REPLACE_TEXTURES_IN_SEL);
	
	MAKE_HOTKEY(UIE_SHADE_WIREFRAME, IDS_UIE_SHADE_WIREFRAME);
	MAKE_HOTKEY(UIE_SHADE_TEXTURES, IDS_UIE_SHADE_TEXTURES);
	MAKE_HOTKEY(UIE_SHADE_LIGHTMAPS, IDS_UIE_SHADE_LIGHTMAPS);
	MAKE_HOTKEY(UIE_SHADE_FLAT, IDS_UIE_SHADE_FLAT);

	MAKE_HOTKEY(UIE_SELECT_TEXTURE, IDS_UIE_SELECT_TEXTURE);
	MAKE_HOTKEY(UIE_SELECT_PREFAB, IDS_UIE_SELECT_PREFAB);
	MAKE_HOTKEY(UIE_SELECT_BRUSH_COLOR, IDS_UIE_SELECT_BRUSH_COLOR);
	
	MAKE_HOTKEY(UIE_HIDE_INVERSE, IDS_UIE_HIDE_INVERSE);
	MAKE_HOTKEY(UIE_UNHIDE_INVERSE, IDS_UIE_UNHIDE_INVERSE);
	MAKE_HOTKEY(UIE_HIDE_SELECTED, IDS_UIE_HIDE_SELECTED);
	
	MAKE_HOTKEY(UIE_UNHIDE_SELECTED, IDS_UIE_UNHIDE_SELECTED);
	MAKE_HOTKEY(UIE_ROTATE_SELECTION, IDS_UIE_ROTATE_SELECTION);
	MAKE_HOTKEY(UIE_ROTATE_SELECTION_LEFT, IDS_UIE_ROTATE_SELECTION_LEFT);
	MAKE_HOTKEY(UIE_ROTATE_SELECTION_RIGHT, IDS_UIE_ROTATE_SELECTION_RIGHT);
	MAKE_HOTKEY(UIE_BIND_TO_OBJECT, IDS_UIE_BIND_TO_OBJECT);

	//hotkeys for navigator stuff
	MAKE_HOTKEY(UIE_NAVIGATOR_STORE, IDS_UIE_NAVIGATOR_STORE);
	MAKE_HOTKEY(UIE_NAVIGATOR_ORGANIZE, IDS_UIE_NAVIGATOR_ORGANIZE);
	MAKE_HOTKEY(UIE_NAVIGATOR_NEXT, IDS_UIE_NAVIGATOR_NEXT);
	MAKE_HOTKEY(UIE_NAVIGATOR_PREVIOUS, IDS_UIE_NAVIGATOR_PREVIOUS);

	//hot keys for model showing/hiding
	MAKE_HOTKEY(UIE_SHOW_ALL_MODELS, IDS_UIE_SHOW_ALL_MODELS);
	MAKE_HOTKEY(UIE_HIDE_ALL_MODELS, IDS_UIE_HIDE_ALL_MODELS);
	MAKE_HOTKEY(UIE_SHOW_MODELS_OF_CLASS, IDS_UIE_SHOW_MODELS_OF_CLASS);
	MAKE_HOTKEY(UIE_HIDE_MODELS_OF_CLASS, IDS_UIE_HIDE_MODELS_OF_CLASS);
	MAKE_HOTKEY(UIE_SHOW_SELECTED_MODELS, IDS_UIE_SHOW_SELECTED_MODELS);
	MAKE_HOTKEY(UIE_HIDE_SELECTED_MODELS, IDS_UIE_HIDE_SELECTED_MODELS);
	MAKE_HOTKEY(UIE_SHOW_MODEL_POLYCOUNT, IDS_UIE_SHOW_MODEL_POLYCOUNT);

	//mode selection hotkeys
	MAKE_HOTKEY(UIE_OBJECT_EDIT_MODE, IDS_UIE_OBJECT_EDIT_MODE);
	MAKE_HOTKEY(UIE_GEOMETRY_EDIT_MODE, IDS_UIE_GEOMETRY_EDIT_MODE);
	MAKE_HOTKEY(UIE_BRUSH_EDIT_MODE, IDS_UIE_BRUSH_EDIT_MODE);

	//texture layer hotkeys
	MAKE_HOTKEY(UIE_NEXT_TEXTURE_LAYER, IDS_UIE_NEXT_TEXTURE_LAYER);

	//texture mirroring hotkeys
	MAKE_HOTKEY(UIE_MIRROR_TEXTURE_X, IDS_UIE_MIRROR_TEXTURE_X);
	MAKE_HOTKEY(UIE_MIRROR_TEXTURE_Y, IDS_UIE_MIRROR_TEXTURE_Y);

	//texture matching
	MAKE_HOTKEY(UIE_MATCH_TEXTURE_COORDS, IDS_UIE_MATCH_TEXTURE_COORDS);

	//mirroring hotkeys
	MAKE_HOTKEY(UIE_MIRROR_X, IDS_UIE_MIRROR_X);
	MAKE_HOTKEY(UIE_MIRROR_Y, IDS_UIE_MIRROR_Y);
	MAKE_HOTKEY(UIE_MIRROR_Z, IDS_UIE_MIRROR_Z);

	//selection hotkeys
	MAKE_HOTKEY(UIE_SELECT_ALL, IDS_UIE_SELECT_ALL);
	MAKE_HOTKEY(UIE_SELECT_INVERSE, IDS_UIE_SELECT_INVERSE);
	MAKE_HOTKEY(UIE_SELECT_CONTAINER, IDS_UIE_SELECT_CONTAINER);
	MAKE_HOTKEY(UIE_ADVANCED_SELECT, IDS_UIE_ADVANCED_SELECT);
	MAKE_HOTKEY(UIE_SCALE_SELECTION, IDS_UIE_SCALE_SELECTION);
	MAKE_HOTKEY(UIE_SAVE_AS_PREFAB, IDS_UIE_SAVE_AS_PREFAB);
	MAKE_HOTKEY(UIE_GENERATE_UNIQUE_NAMES, IDS_UIE_GENERATE_UNIQUE_NAMES);
	MAKE_HOTKEY(UIE_GROUP_SELECTION, IDS_UIE_GROUP_SELECTION);
	MAKE_HOTKEY(UIE_FLIP_BRUSH, IDS_UIE_FLIP_BRUSH);
}

#undef MAKE_HOTKEY

//sets the default to a single key
bool CGlobalHotKeyDB::SetKeyDefault(CHotKey& HotKey)
{
	BOOL bFoundBlock1 = TRUE;
	BOOL bFoundBlock2 = TRUE;

	//clean up the key
	HotKey.SetUserChangable(true);
	HotKey.ResetEvents();

	//okay, now let us see if the aggregate wants to define this
	//key and change the default
	if(m_pAggregate)
	{
		if(m_pAggregate->DefineKey(HotKey))
		{
			//the aggregate defined the key
			return true;
		}
	}
	//the aggregate did not define the key, use the default default


	const char* pszName = HotKey.GetEventName();

	if(stricmp(pszName, UIE_TRACKER_FOCUS) == 0)
	{
		HotKey.SetUserChangable(false);
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEMOVE, 0));
	}
	else if(stricmp(pszName, UIE_NAV_DRAG) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAV_DRAG_FAST) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAV_ARC_ROTATE) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAV_ARC_ROTATE_ROLL) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MIRROR_X) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MIRROR_Y) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MIRROR_Z) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_ALL) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_INVERSE) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_CONTAINER) == 0)
	{
	}
	else if(stricmp(pszName, UIE_ADVANCED_SELECT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'A'));
	}
	else if(stricmp(pszName, UIE_SCALE_SELECTION) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SAVE_AS_PREFAB) == 0)
	{
	}
	else if(stricmp(pszName, UIE_GENERATE_UNIQUE_NAMES) == 0)
	{
	}
	else if(stricmp(pszName, UIE_GROUP_SELECTION) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'S'));
	}
		else if(stricmp(pszName, UIE_HIDE_INVERSE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_SEMICOLON));
	}
	else if(stricmp(pszName, UIE_UNHIDE_INVERSE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_SINGLEQUOTE));
	}
	else if(stricmp(pszName, UIE_HIDE_SELECTED) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_OPENBRACKET));
	}	
	else if(stricmp(pszName, UIE_UNHIDE_SELECTED) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_CLOSEBRACKET));
	}
	else if(stricmp(pszName, UIE_ROTATE_SELECTION) == 0)
	{
	}
	else if(stricmp(pszName, UIE_ROTATE_SELECTION_LEFT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_LEFT));
	}
	else if(stricmp(pszName, UIE_ROTATE_SELECTION_RIGHT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_RIGHT));
	}
	else if(stricmp(pszName, UIE_FLIP_BRUSH) == 0)
	{
	}
	else if(stricmp(pszName, UIE_LOCK_FOCUS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_FIT_TEXTURE_TO_POLY) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'C'));
	}
	else if(stricmp(pszName, UIE_TOGGLE_CLASS_ICONS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'D'));
	}
	else if(stricmp(pszName, UIE_ZOOM) == 0)
	{
		HotKey.SetUserChangable(LTFALSE);
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEWHEEL, 0));
	}
	else if(stricmp(pszName, UIE_ZOOM_FAST) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_SELECT_DEPTH) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'T'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_APPLY_TEXTURE_CLICK) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'A'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_APPLY_TEXTURE_TO_SEL) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'T'));
	}
	else if(stricmp(pszName, UIE_REMOVE_TEXTURE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'T'));
	}
	else if(stricmp(pszName, UIE_TEST_RENDER_SPEED) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
	}
	else if(stricmp(pszName, UIE_SPLIT_BRUSH) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'S'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_SELECT_NONE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'U'));
	}
	else if(stricmp(pszName, UIE_SET_GRID_FORWARD) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '8'));
	}
	else if(stricmp(pszName, UIE_SET_GRID_RIGHT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '9'));
	}
	else if(stricmp(pszName, UIE_SET_GRID_UP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '0'));
	}
	else if(stricmp(pszName, UIE_INIT_TEXTURE_SPACE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'K'));
	}
	else if(stricmp(pszName, UIE_DELETE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_DELETE));
	}
	else if(stricmp(pszName, UIE_NUDGE_UP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_UP));
	}
	else if(stricmp(pszName, UIE_NUDGE_DOWN) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_DOWN));
	}
	else if(stricmp(pszName, UIE_NUDGE_LEFT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_LEFT));
	}
	else if(stricmp(pszName, UIE_NUDGE_RIGHT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_RIGHT));
	}
	else if(stricmp(pszName, UIE_REMOVE_EDGES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_HOME));
	}
	else if(stricmp(pszName, UIE_DELETE_EDGES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_END));
	}
	else if(stricmp(pszName, UIE_SPLIT_EDGE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_INSERT));
	}
	else if(stricmp(pszName, UIE_DELETE_TAGGED_POLIES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_NEXT));
	}
	else if(stricmp(pszName, UIE_GRID_TO_POLY) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'G'));
	}
	else if(stricmp(pszName, UIE_TAG_POLY_VERTS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	}
	else if(stricmp(pszName, UIE_FLIP_NORMAL) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'F'));
	}
	else if(stricmp(pszName, UIE_ORBIT_VERTEX) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'P'));
	}
	else if(stricmp(pszName, UIE_NODE_ROTATE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'N'));
	}
	else if(stricmp(pszName, UIE_NODE_ROTATE_STEP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else
	{
		bFoundBlock1 = FALSE;
	}

	//block two	
	if(stricmp(pszName, UIE_TEXTURE_MOVE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'R'));
	}
	else if(stricmp(pszName, UIE_TEXTURE_MOVE_ROTATE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_TEXTURE_MOVE_SCALE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_TEXTURE_MOVE_STEP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT, TRUE));
	}
	else if(stricmp(pszName, UIE_CREATE_PLANE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '6'));
	}
	else if(stricmp(pszName, UIE_CREATE_DOME) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '5'));
	}
	else if(stricmp(pszName, UIE_CREATE_SPHERE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '4'));
	}
	else if(stricmp(pszName, UIE_CREATE_PYRAMID) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '3'));
	}
	else if(stricmp(pszName, UIE_CREATE_CYLINDER) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '2'));
	}
	else if(stricmp(pszName, UIE_CREATE_BOX) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '1'));
	}
	else if(stricmp(pszName, UIE_MIRROR_TEXTURE_X) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MIRROR_TEXTURE_Y) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MATCH_TEXTURE_COORDS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_TOP_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_BOTTOM_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_LEFT_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_RIGHT_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_FRONT_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_BACK_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_PERSPECTIVE_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_BRUSH_SIZE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_OBJECT_SIZE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_CURVE_EDIT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'P'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_FAR_DIST) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'C'));
	}
	else if(stricmp(pszName, UIE_NODE_MOVE_LOCK_AXIS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_NODE_MOVE_CLONE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'D'));
	}
	else if(stricmp(pszName, UIE_GRID_SIZE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Z'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_MARKER_MOVE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'X'));
	}
	else if(stricmp(pszName, UIE_MARKER_CENTER) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'X'));
	}
	else if(stricmp(pszName, UIE_CENTER_SEL_ON_MARKER) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Z'));
	}
	else if(stricmp(pszName, UIE_MOVE_VERT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_MOVE_VERT_PERP) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_MOVE_VERT_SNAP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_MOVE_SEL_VERT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'S'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_MOVE_IMM_VERT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'M'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_SCALE_POLY) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'W'));
	}
	else if(stricmp(pszName, UIE_SCALE_POLY_DIVIDE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'W'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_SCALE_POLY_EXTRUDE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_SCALE_VERT_SEL) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Q'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	else if(stricmp(pszName, UIE_SCALE_VERT_IMM) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'W'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_EXTRUDE_POLY) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Q'));
	}
	else if(stricmp(pszName, UIE_EXTRUDE_POLY_SNAP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_EXTRUDE_POLY_BRUSH) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Q'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	else if(stricmp(pszName, UIE_MOVE_NODE_HANDLE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_SNAP_NODE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'M'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_MOVE_NODE_PERP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'M'));
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_NAV_MOVE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'I'));
	}
	else if(stricmp(pszName, UIE_NAV_MOVE_PERP) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_NAV_MOVE_FAST) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_NAV_ROTATE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'O'));
	}
	else if(stricmp(pszName, UIE_NAV_ROTATE_FORWARD) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_NAV_ROTATE_BACKWARD) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
	}
	else if(stricmp(pszName, UIE_NAV_ORBIT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'L'));
	}
	else if(stricmp(pszName, UIE_NAV_ORBIT_MOVE) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SPACE));
		HotKey.ClearEndEventList();
		HotKey.m_EndEventList.Add(new CUIKeyEvent(UIEVENT_KEYDOWN, VK_ESCAPE));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_UNDO) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_BACK));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_SPLIT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'S'));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_ROTATE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'R'));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_VERT_SNAP) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_CLOSE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'C'));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_INSERT_EDGE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
	}
	else if(stricmp(pszName, UIE_DRAW_POLY_NEW_VERTEX) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SPACE));
	}
	else if(stricmp(pszName, UIE_TAG_ALL) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_TAG_ADD) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_TAG_INVERT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	else if(stricmp(pszName, UIE_TAG_BRUSH) == 0)
	{
		HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT));
	}
	else if(stricmp(pszName, UIE_VERT_SNAP_EDGE_SEL) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'D'));
	}
	else if(stricmp(pszName, UIE_VERT_SNAP_EDGE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
	}
	else if(stricmp(pszName, UIE_VERT_SNAP_VERT_SEL) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'B'));
	}
	else if(stricmp(pszName, UIE_VERT_SNAP_VERT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
	}
	else if(stricmp(pszName, UIE_JOIN_BRUSHES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'J'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	}
	else if(stricmp(pszName, UIE_UNJOIN_BRUSHES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'J'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	else if(stricmp(pszName, UIE_TRI_BRUSHES) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	}
	else if(stricmp(pszName, UIE_MAXIMIZE_VIEW1) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '1'));
	}
	else if(stricmp(pszName, UIE_MAXIMIZE_VIEW2) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '2'));
	}
	else if(stricmp(pszName, UIE_MAXIMIZE_VIEW3) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '3'));
	}
	else if(stricmp(pszName, UIE_MAXIMIZE_VIEW4) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '4'));
	}
	else if(stricmp(pszName, UIE_RESET_VIEWPORTS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '6'));
	}
	else if(stricmp(pszName, UIE_MAXIMIZE_ACTIVE_VIEW) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, '5'));
	}
	else if(stricmp(pszName, UIE_FREEZE_SELECTED) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'J'));
	}
	else if(stricmp(pszName, UIE_UNFREEZE_ALL) == 0)
	{
	}
	else if(stricmp(pszName, UIE_TOGGLE_CLASS_ICONS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_CAMERA_TO_OBJECT) == 0)
	{
	}
	else if(stricmp(pszName, UIE_DISCONNECT_SELECTED_PREFABS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_HIDE_FROZEN_NODES) == 0)
	{
	}
	else if(stricmp(pszName, UIE_FIT_TEXTURE_TO_POLY) == 0)
	{
	}
	else if(stricmp(pszName, UIE_ADD_OBJECT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'K'));
	}
	else if(stricmp(pszName, UIE_EDIT_UNDO) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Z'));
	}
	else if(stricmp(pszName, UIE_EDIT_REDO) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	}
	else if(stricmp(pszName, UIE_SHRINK_GRID_SPACING) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SUBTRACT));
	}
	else if(stricmp(pszName, UIE_EXPAND_GRID_SPACING) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_ADD));
	}
	else if(stricmp(pszName, UIE_APPLY_COLOR) == 0)
	{
	}
	else if(stricmp(pszName, UIE_TEXTURE_WRAP) == 0)
	{
	}
	else if(stricmp(pszName, UIE_TEXTURE_WRAP_SKIP) == 0)
	{
	}
	else if(stricmp(pszName, UIE_MAP_TEXTURE_COORDS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'A'));
	}	
	else if(stricmp(pszName, UIE_MAP_TEXTURE_TO_VIEW) == 0)
	{
	}
	else if(stricmp(pszName, UIE_RESET_TEXTURE_COORDS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_REPLACE_TEXTURES_IN_SEL) == 0)
	{
	}
	else if(stricmp(pszName, UIE_REPLACE_TEXTURES) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAVIGATOR_STORE) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAVIGATOR_STORE) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NAVIGATOR_NEXT) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_CLOSEBRACKET));
	}
	else if(stricmp(pszName, UIE_NAVIGATOR_PREVIOUS) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, EXTVK_OPENBRACKET));
	}
	else if(stricmp(pszName, UIE_SHADE_WIREFRAME) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SHADE_TEXTURES) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SHADE_LIGHTMAPS) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SHADE_FLAT) == 0)
	{
	}
	else if(stricmp(pszName, UIE_NEXT_TEXTURE_LAYER) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_TEXTURE) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_PREFAB) == 0)
	{
	}
	else if(stricmp(pszName, UIE_SELECT_BRUSH_COLOR) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'G'));
	}
	else if(stricmp(pszName, UIE_OBJECT_EDIT_MODE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'H'));
	}
	else if(stricmp(pszName, UIE_BRUSH_EDIT_MODE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'B'));
	}
	else if(stricmp(pszName, UIE_GEOMETRY_EDIT_MODE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'G'));
	}
	else if(stricmp(pszName, UIE_OBJECT_EDIT_MODE) == 0)
	{
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
		HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'H'));
	}
	else if(stricmp(pszName, UIE_BIND_TO_OBJECT) == 0)
	{
	}
	else
	{
		//no match found
		bFoundBlock2 = FALSE;
	}

	//match was found
	return (bFoundBlock1 || bFoundBlock2) ? TRUE : FALSE;
}
