// ----------------------------------------------------------------------- //
//
// MODULE  : ModelNodes.h
//
// PURPOSE : Model node related utility functions
//
// CREATED : 6/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __MODEL_NODES_H__
#define __MODEL_NODES_H__

#include "ModelIds.h"

enum NodeType { NT_HEAD, NT_LARM, NT_RARM, NT_LLEG, NT_RLEG, NT_PELVIS, NT_TORSO };
		

static char* s_AVCModelNodes[] =
{
	"torso_bars",
	"torso",
	"right_armu_torso",
	"right_armu_shoulder",
	"right_armu",
	"right_arml",
	"right_arml_hand",
	"pelvis_3",
	"pelvis_2",
	"pelvis",
	"right_legu",
	"right_legl",
	"right_legl_1",
	"right_legl_2",
	"right_legl_foot",
	"left_legu",
	"left_legl",
	"left_legl_2",
	"left_legl_1",
	"left_legl_foot",
	"left_armu_torso",
	"left_armu_shoulder",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"head"
};

static char* s_AkumaModelNodes[] =
{
	"pelvis",
	"torso_1",
	"torso_2",
	"right_armu_1",
	"right_armu_2",
	"right_arml_1",
	"right_arml_2",
	"right_armu_3",
	"torso_3",
	"torso_4",
	"head_1",
	"head_2",
	"head_3",
	"head_4",
	"head_5",
	"head_6",
	"head_7",
	"head_8",
	"head_9",
	"torso_5",
	"torso_7",
	"left_armu_1",
	"left_armu_2",
	"left_arml_1",
	"left_arml_2",
	"left_armu_3",
	"torso_6",
	"torso_8",
	"torso_9",
	"torso_10",
	"torso_11",
	"torso_12",
	"torso_13",
	"torso_14",
	"right_legu_1",
	"right_legu_2",
	"right_legl_1",
	"right_legl_2",
	"right_legl_3",
	"right_legl_4",
	"right_legl_5",
	"right_legl_6",
	"right_legl_7",
	"left_legu_1",
	"left_legu_2",
	"left_legl_1",
	"left_legl_2",
	"left_legl_3",
	"left_legl_4",
	"left_legl_5",
	"left_legl_6",
	"left_legl_7"
};

static char* s_Andra5ModelNodes[] =
{
	"pelvis_1",
	"torso",
	"torso_2",
	"left_armu_1",
	"left_armu_2",
	"left_arml",
	"left_arml_hand",
	"torso_1",
	"right_armu_1",
	"right_armu_2",
	"right_arml",
	"right_arml_hand",
	"pelvis",
	"left_legu_1",
	"left_legu_2",
	"left_legl_1",
	"left_legl_foot",
	"right_legu_1",
	"right_legu_2",
	"right_legl_1",
	"right_legl_foot",
	"head"
};

static char* s_Andra10ModelNodes[] =
{
	"torso",
	"pelvis",
	"left_legu",
	"left_legu_1",
	"left_legl",
	"left_legl_heel",
	"left_legl_foot",
	"right_legu",
	"right_legu_1",
	"right_legl",
	"right_legl_heel",
	"right_legl_foot",
	"torso_2",
	"right_armu_shoulder",
	"right_armu",
	"right_arml",
	"right_arml_hand",
	"head",
	"torso_1",
	"left_armu_shoulder",
	"left_armu",
	"left_arml",
	"left_arml_shield",
	"left_arml_hand"
};

static char* s_AssassinModelNodes[] =
{
	"head",
	"pelvis",
	"left_legu",
	"left_legl",
	"left_legl_foot",
	"right_legu",
	"right_legl",
	"right_legl_foot",
	"left_armu_missle",
	"left_armu_wing",
	"left_armu_shoulder",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"left_arml_armshield",
	"left_armu_shldrjnt",
	"left_armu_shoulderwing",
	"left_armu_antannae",
	"right_armu_missle1",
	"right_armu_wing",
	"right_armu_shoulder",
	"right_armu",
	"right_arml",
	"right_arml_hand",
	"right_arml_shield",
	"right_armu_shldjnt",
	"right_armu_shouldershld",
	"right_armu_antennae"
};

static char* s_EnforcerModelNodes[] =
{
	"torso_1",
	"torso_2",
	"pelvis_1",
	"pelvis_2",
	"left_legu_1",
	"left_legu_2",
	"left_legl_1",
	"left_legl_2",
	"left_legl_3",
	"left_legl_shoe",
	"left_legl_4",
	"left_legl_5",
	"left_legl_6",
	"left_legl_7",
	"left_legl_8",
	"right_legu_1",
	"right_legu_2",
	"right_legl_1",
	"right_legl_2",
	"right_legl_3",
	"right_legl_shoe",
	"right_legl_4",
	"right_legl_5",
	"right_legl_6",
	"right_legl_7",
	"right_legl_8",
	"head_2",
	"head_3",
	"head_1",
	"head_4",
	"head_5",
	"head_6",
	"head_7",
	"head_8",
	"left_armu_4",
	"left_armu_5",
	"left_armu_1",
	"left_armu_2",
	"left_armu_3",
	"left_arml_1",
	"left_arml_2",
	"left_arml_3",
	"left_arml_hand",
	"right_armu_1",
	"right_armu_2",
	"right_armu_3",
	"right_armu_4",
	"right_armu_5",
	"right_arml_1",
	"right_arml_2",
	"right_arml_3",
	"right_arml_hand"
};

static char* s_OrdogModelNodes[] =
{
	"torso",
	"right_armu_spike",
	"left_armu_spike_lower",
	"right_armu_shoulder",
	"right_armu",
	"right_arml",
	"right_arml_hand",
	"head",
	"left_armu_shoulder",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"left_armu_spike",
	"left_armu_spike_lower",
	"torso_lower",
	"torso_lower_2",
	"pelvis",
	"right_legu",
	"right_legl",
	"right_legl_foot",
	"right_legu_crotchjoint",
	"left_legu_crotchjoint",
	"left_legu_2",
	"left_legl",
	"left_legl_foot"
};

static char* s_PredatorModelNodes[] =
{
	"torso_upper",
	"torso_lower",
	"pelvis_upper",
	"right_legu_thigh",
	"right_legu_hippack",
	"right_legu_knee",
	"right_legl_shin",
	"right_legl_knee",
	"right_legl_foot1",
	"right_legl_toe",
	"right_legl_heel",
	"pelvis_lower",
	"left_legu_thigh",
	"left_legu_hippack",
	"left_legu_knee",
	"left_legl_shin",
	"left_legl_knee",
	"left_legl_foot",
	"left_legl_toe",
	"left_legl_heel",
	"Torso_rightplate",
	"right_armu_chestlight1",
	"right_armu_chestlight2",
	"Torso_R_thruster_box",
	"torso_R_thruster_box_lid",
	"torso_R_thruster",
	"right_armu_1",
	"right_armu_elbospike",
	"right_arml",
	"right_arml_1",
	"right_arml_wrist",
	"Right_arml_hand",
	"right_armu_3",
	"right_armu_2",
	"head_back",
	"head_right_earbase",
	"head_rightear",
	"head_leftearbase",
	"head_leftear",
	"head_lowervisor",
	"head_neck",
	"head_visor",
	"head_top",
	"head_topvisor",
	"left_armu_chestlight1",
	"left_armu_chestlight2",
	"torso_leftplate",
	"torso_L_thruster_box",
	"torso_L_thruster_panel",
	"torso_L_thruster",
	"left_armu_1",
	"left_armu_elbowspike",
	"left_arml_elbow",
	"left_arml_1",
	"left_arml_wrist",
	"left_arml_hand",
	"left_armu_2",
	"left_armu_3"
};

static char* s_LittleBoyModelNodes[] =
{
	"pelvis_1",
	"torso_1",
	"head_1",
	"head_2",
	"head_3",
	"left_armu_1",
	"left_arml_1",
	"left_arml_2",
	"right_armu_1",
	"right_arml_1",
	"right_arml_2",
	"torso_2",
	"torso_3",
	"torso_4",
	"torso_5",
	"torso_6",
	"left_legu_1",
	"left_legl_1",
	"left_legl_2",
	"right_legu_1",
	"right_legl_1",
	"right_legl_2"
};

static char* s_LittleGirlModelNodes[] =
{
	"torso",
	"head_neck",
	"head",
	"head_hair",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"right_armu",
	"right_arml",
	"right_arml_hand",
	"torso_backpack",
	"torso_backpack1",
	"torso_backpack2",
	"left_armu_strap",
	"right_armu_strap",
	"right_legu_skirt",
	"right_legu",
	"right_legl",
	"right_legl_foot",
	"left_legu_skirt",
	"left_legu",
	"left_legl",
	"left_legl_foot"
};

static char* s_Civilian1ModelNodes[] =
{
	"pelvis_1",
	"torso_1",
	"head_1",
	"head_2",
	"head_3",
	"left_armu_1",
	"left_arml_1",
	"left_arml_2",
	"right_armu_1",
	"right_arml_1",
	"right_arml_2",
	"right_legu_1",
	"right_legl_1",
	"right_legl_2",
	"left_legu",
	"left_legl_1",
	"left_legl_2"
};

static char* s_Civilian1bModelNodes[] =
{
	"pelvis_1",
	"torso_1",
	"left_armu_1",
	"left_arml_1",
	"left_arml_2",
	"torso_2",
	"head_2",
	"head_1",
	"right_armu_1",
	"right_arml_1",
	"right_arml_2",
	"right_legu_1",
	"right_legl_1",
	"right_legl_2",
	"left_legu_1",
	"left_legl_1",
	"left_legl_2"
};

static char* s_Civilian2ModelNodes[] =
{
	"pelvis_1",
	"torso_1",
	"head_1",
	"head_2",
	"head_3",
	"right_armu_1",
	"right_arml_1",
	"right_arml_2",
	"left_armu_1",
	"left_arml_1",
	"left_arml_2",
	"right_legu_1",
	"right_legl_1",
	"right_legl_2",
	"left_legu_1",
	"left_legl_1",
	"left_legl_2"
};

static char* s_RakshaModelNodes[] =
{
	"torso",
	"pelvis",
	"left_legu",
	"left_legl",
	"left_legl_foot",
	"right_legu",
	"right_legl",
	"right_legl_foot",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"head",
	"right_armu",
	"right_arml",
	"right_arml_hand"
};

static char* s_OfficerModelNodes[] =
{
	"pelvis",
	"left_legu",
	"left_legl",
	"left_legl_foot",
	"pelvis_belt",
	"right_legu1",
	"right_legl1",
	"right_legl_foot",
	"torso",
	"torso_neck",
	"head",
	"head_hair",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"torso_armor",
	"right_armu1",
	"right_arml1",
	"right_arml_hand"
};

static char* s_TrooperModelNodes[] =
{
	"torso",
	"left_armu_1",
	"left_arml_1",
	"left_arml_hand",
	"head_1",
	"head_2",
	"pelvis",
	"left_legu_3",
	"left_legl_1",
	"left_legl_shoe",
	"right_legu_3",
	"right_legl_1",
	"right_legl_shoe",
	"right_legu_1",
	"right_legu_2",
	"left_legu_1",
	"left_legu_2",
	"pelvis_3",
	"pelvis_2",
	"pelvis_4",
	"right_armu_1",
	"right_arml_1",
	"right_arml_hand"
};

static char* s_EliteTrooperModelNodes[] =
{
	"torso",
	"pelvis",
	"right_legu_1",
	"right_legl_1",
	"right_legl_foot",
	"left_legu",
	"left_legl_shin",
	"left_legl_foot",
	"head_neck1",
	"head",
	"head_visor",
	"left_armu_shoulder",
	"left_armu",
	"left_arml",
	"right_armu_shoulder",
	"right_armu",
	"right_arml"
};

static char* s_ShockTrooperModelNodes[] =
{
	"torso",
	"pelvis",
	"left_legu",
	"left_legl",
	"left_legl_foot",
	"right_legu",
	"right_legl",
	"right_legl_foot",
	"head",
	"torso_1",
	"left_armu",
	"left_arml",
	"left_arml_hand",
	"right_armu",
	"right_arml",
	"right_arml_hand"
};

static char* s_EliteShockTrooperModelNodes[] =
{
	"torso_1",
	"pelvis",
	"left_legu",
	"left_legl_shin",
	"left_legl_foot",
	"right_legu",
	"right_legl_shin",
	"right_legl_foot",
	"head",
	"torso_2",
	"left_armu_shoulder",
	"left_arml_forearm",
	"left_arml_hand",
	"left_armu_bigspike",
	"left_armu_smallspike",
	"right_armu_shoulder",
	"right_arml_forearm",
	"right_arml_hand",
	"right_armu_bigspike",
	"right_armu_smallspike"
};

static char* s_TenmaModelNodes[] =
{
	"torso",
	"pelvis",
	"right_legu",
	"right_legl_shin",
	"right_legl_foot",
	"left_legu",
	"left_legl_shin",
	"left_legl_foot",
	"head",
	"right_armu_chest",
	"right_armu",
	"right_armu_elbow",
	"right_arml_forearm",
	"right_arml_hand",
	"left_armu_chest",
	"left_armu",
	"left_armu_elbow",
	"left_arml_forearm",
	"left_arml_hand"
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumModelNodes
//
//	PURPOSE:	Return the number of nodes in the model associated with
//				the particular id
//
// ----------------------------------------------------------------------- //

inline DDWORD GetNumModelNodes(DDWORD nId, ModelSize size=MS_NORMAL)
{
	DDWORD nRet = 0;

	switch (nId)
	{
		// HUMAN MODELS /////////////////////////////////////////////////////

		case MI_AI_LITTLEBOY_ID:
			nRet = (sizeof(s_LittleBoyModelNodes) / sizeof(s_LittleBoyModelNodes[0]));
		break;

		case MI_AI_LITTLEGIRL_ID:
			nRet = (sizeof(s_LittleGirlModelNodes) / sizeof(s_LittleGirlModelNodes[0]));
		break;

		case MI_AI_CIVILIAN1_ID:
		case MI_AI_CIVILIAN1B_ID:
			nRet = (sizeof(s_Civilian1ModelNodes) / sizeof(s_Civilian1ModelNodes[0]));
		break;

		case MI_AI_CIVILIAN2_ID:
			nRet = (sizeof(s_Civilian2ModelNodes) / sizeof(s_Civilian2ModelNodes[0]));
		break;

		case MI_AI_OFFICER_ID:
			nRet = (sizeof(s_OfficerModelNodes) / sizeof(s_OfficerModelNodes[0]));
		break;

		case MI_AI_TROOPER_ID:
			nRet = (sizeof(s_TrooperModelNodes) / sizeof(s_TrooperModelNodes[0]));
		break;

		case MI_AI_ETROOPER_ID:
			nRet = (sizeof(s_EliteTrooperModelNodes) / sizeof(s_EliteTrooperModelNodes[0]));
		break;

		case MI_AI_STROOPER_ID:
			nRet = (sizeof(s_ShockTrooperModelNodes) / sizeof(s_ShockTrooperModelNodes[0]));
		break;

		case MI_AI_ESTROOPER_ID:
			nRet = (sizeof(s_EliteShockTrooperModelNodes) / sizeof(s_EliteShockTrooperModelNodes[0]));
		break;

		case MI_AI_SAMANTHA_ID:
		case MI_AI_AVC_ID:
			nRet = (sizeof(s_AVCModelNodes) / sizeof(s_AVCModelNodes[0]));
		break;

		case MI_AI_ORDOG_ID:
			nRet = (sizeof(s_OrdogModelNodes) / sizeof(s_OrdogModelNodes[0]));
		break;

		case MI_AI_AKUMA_ID:
			nRet = (sizeof(s_AkumaModelNodes) / sizeof(s_AkumaModelNodes[0]));
		break;

		case MI_AI_ENFORCER_ID:
			nRet = (sizeof(s_EnforcerModelNodes) / sizeof(s_EnforcerModelNodes[0]));
		break;

		case MI_AI_PREDATOR_ID:
			nRet = (sizeof(s_PredatorModelNodes) / sizeof(s_PredatorModelNodes[0]));
		break;

		case MI_AI_ASSASSIN_ID:
			nRet = (sizeof(s_AssassinModelNodes) / sizeof(s_AssassinModelNodes[0]));
		break;

		case MI_AI_ANDRA5_ID:
			nRet = (sizeof(s_Andra5ModelNodes) / sizeof(s_Andra5ModelNodes[0]));
		break;

		case MI_AI_ANDRA10_ID:
			nRet = (sizeof(s_Andra10ModelNodes) / sizeof(s_Andra10ModelNodes[0]));
		break;

		case MI_AI_RAKSHA_ID:
			nRet = (sizeof(s_RakshaModelNodes) / sizeof(s_RakshaModelNodes[0]));
		break;

		case MI_AI_TENMA_ID:
			nRet = (sizeof(s_TenmaModelNodes) / sizeof(s_TenmaModelNodes[0]));
		break;

		case MI_AI_COTHINEAL_ID:
		break;


		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
		case MI_AI_VIGILANCE_ID:
		case MI_AI_HAMMERHEAD_ID:
		case MI_AI_VANDAL_ID:	
		case MI_AI_SPARROWHAWK_ID:
		case MI_AI_RUIN150_ID:
		case MI_AI_UHLANA3_ID:
		break;


		// MAJOR CHARACTER MODELS ////////////////////////////////////////////

		case MI_AI_KURA_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		case MI_AI_HANK_ID:
		case MI_AI_ADMIRAL_ID:
		case MI_AI_KATHRYN_ID:
		case MI_AI_RYO_ID:
		break;

		case MI_AI_TOSHIRO_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		case MI_AI_GABRIEL_ID:
		break;

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		default : break;
	}

	return nRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModelNodeName
//
//	PURPOSE:	Return the name of a particular model node
//
// ----------------------------------------------------------------------- //

inline char* GetModelNodeName(DDWORD dwIndex, DDWORD nId, ModelSize size=MS_NORMAL)
{
	char* pRet = DNULL;

	if (dwIndex < 0 || dwIndex > GetNumModelNodes(nId, size)) return pRet;

	switch (nId)
	{
		// HUMAN MODELS /////////////////////////////////////////////////////

		case MI_AI_LITTLEBOY_ID:
			pRet = s_LittleBoyModelNodes[dwIndex];
		break;
	
		case MI_AI_LITTLEGIRL_ID:
			pRet = s_LittleGirlModelNodes[dwIndex];
		break;

		case MI_AI_CIVILIAN1_ID:
			pRet = s_Civilian1ModelNodes[dwIndex];
		break;

		case MI_AI_CIVILIAN1B_ID:
			pRet = s_Civilian1bModelNodes[dwIndex];
		break;

		case MI_AI_CIVILIAN2_ID:
			pRet = s_Civilian2ModelNodes[dwIndex];
		break;

		case MI_AI_OFFICER_ID:
			pRet = s_OfficerModelNodes[dwIndex];
		break;

		case MI_AI_TROOPER_ID:
			pRet = s_TrooperModelNodes[dwIndex];
		break;

		case MI_AI_ETROOPER_ID:
			pRet = s_EliteTrooperModelNodes[dwIndex];
		break;

		case MI_AI_STROOPER_ID:
			pRet = s_ShockTrooperModelNodes[dwIndex];
		break;

		case MI_AI_ESTROOPER_ID:
			pRet = s_EliteShockTrooperModelNodes[dwIndex];
		break;

		case MI_AI_SAMANTHA_ID:
		case MI_AI_AVC_ID:
			pRet = s_AVCModelNodes[dwIndex];
		break;

		case MI_AI_ORDOG_ID:
			pRet = s_OrdogModelNodes[dwIndex];
		break;

		case MI_AI_AKUMA_ID:
			pRet = s_AkumaModelNodes[dwIndex];
		break;

		case MI_AI_ENFORCER_ID:
			pRet = s_EnforcerModelNodes[dwIndex];
		break;

		case MI_AI_PREDATOR_ID:
			pRet = s_PredatorModelNodes[dwIndex];
		break;

		case MI_AI_ASSASSIN_ID:
			pRet = s_AssassinModelNodes[dwIndex];
		break;

		case MI_AI_ANDRA5_ID:
			pRet = s_Andra5ModelNodes[dwIndex];
		break;

		case MI_AI_ANDRA10_ID:
			pRet = s_Andra10ModelNodes[dwIndex];
		break;

		case MI_AI_RAKSHA_ID:
			pRet = s_RakshaModelNodes[dwIndex];
		break;

		case MI_AI_TENMA_ID:
			pRet = s_TenmaModelNodes[dwIndex];
		break;

		case MI_AI_COTHINEAL_ID:
		break;


		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
		case MI_AI_VIGILANCE_ID:
		case MI_AI_HAMMERHEAD_ID:
		case MI_AI_VANDAL_ID:	
		case MI_AI_SPARROWHAWK_ID:
		case MI_AI_RUIN150_ID:
		case MI_AI_UHLANA3_ID:
		break;


		// MAJOR CHARACTER MODELS ////////////////////////////////////////////

		case MI_AI_KURA_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		case MI_AI_HANK_ID:
		case MI_AI_ADMIRAL_ID:
		case MI_AI_KATHRYN_ID:
		case MI_AI_RYO_ID:
		break;

		case MI_AI_TOSHIRO_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		case MI_AI_GABRIEL_ID:
		break;

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_LARGE:
				case MS_NORMAL:
				default :
				break;
			}
		}
		break;

		default : break;
	}

	return pRet;
}


#endif // __MODEL_NODES_H__
