#ifndef __OBJECTGROUPMGR_H__
#define __OBJECTGROUPMGR_H__

class CObjectGroupMgr
{
public:

	//initializes all of the object groups
	static void InitObjectGroups()
	{
		SetAllObjectGroupEnabled();
	}

	//determines if the specified object group is currently enabled
	static bool	IsObjectGroupEnabled(uint32 nGroup)
	{
		assert(nGroup < MAX_OBJECT_RENDER_GROUPS);
		return (s_nObjectGroups[nGroup] == 0);
	}

	//allows enabling or disabling of object groups
	static void SetObjectGroupEnabled(uint32 nGroup, bool bEnable)
	{
		assert(nGroup < (MAX_OBJECT_RENDER_GROUPS));

		if(bEnable)
		{
			if(s_nObjectGroups[nGroup])
				s_nObjectGroups[nGroup]--;
		}
		else
		{
			s_nObjectGroups[nGroup]++;
		}
	}

	//turns on all object groups
	static void SetAllObjectGroupEnabled()
	{
		for(uint32 nCurrGroup = 0; nCurrGroup < MAX_OBJECT_RENDER_GROUPS; nCurrGroup++)
			s_nObjectGroups[nCurrGroup] = 0;
	}

	//determines if this object group should be occluded
	static bool ShouldOccludeObjectGroup(uint32 nGroup)
	{
		static const uint32 knNonOccludedGroup = 255;

		return nGroup != knNonOccludedGroup;
	}

private:

	//the list of visible groups. Each object has an index into a group, and if that group is
	//disabled, it is not considered for rendering. 
	static uint16 s_nObjectGroups[MAX_OBJECT_RENDER_GROUPS];

	CObjectGroupMgr()	{}
};

#endif
