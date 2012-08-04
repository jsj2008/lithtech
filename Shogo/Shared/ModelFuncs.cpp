// ----------------------------------------------------------------------- //
//
// MODULE  : ModelFuncs.cpp
//
// PURPOSE : Model related utility functions
//
// CREATED : 7/6/98
//
// ----------------------------------------------------------------------- //

#include "ModelFuncs.h"
#include "ModelIds.h"
#include "CharacterAlignment.h"
#include "GibTypes.h"

extern int GetRandom(int, int);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModel
//
//	PURPOSE:	Return the model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetModel(uint8 nId, ModelSize size)
{
	char* pRet = "Models\\Default.abc";

	switch (nId)
	{
		// PLAYER MODELS ///////////////////////////////////////////////////

		case MI_PLAYER_ONFOOT_ID:
			pRet = "Models\\Player\\sanjuro.abc";
		break;

		case MI_PLAYER_KID_ID:
			pRet = "Models\\Player\\kid.abc";
		break;

		case MI_PLAYER_AKUMA_ID:
			pRet = "Models\\Player\\Akuma.abc";
		break;

		case MI_PLAYER_PREDATOR_ID:
			pRet = "Models\\Player\\Predator.abc";
		break;

		case MI_PLAYER_ORDOG_ID:
			pRet = "Models\\Player\\Ordog.abc";
		break;

		case MI_PLAYER_ENFORCER_ID:
			pRet = "Models\\Player\\Enforcer.abc";
		break;


		// AI MODELS ///////////////////////////////////////////////////////

		case MI_AI_LITTLEBOY_ID:
			pRet = "Models\\Enemies\\Onfoot\\LittleBoy.abc";
		break;

		case MI_AI_LITTLEGIRL_ID:
			pRet = "Models\\Enemies\\Onfoot\\LittleGirl.abc";
		break;

		case MI_AI_CIVILIAN1_ID:
			pRet = "Models\\Enemies\\Onfoot\\Civilian1a.abc";
		break;

		case MI_AI_CIVILIAN1B_ID:
			pRet = "Models\\Enemies\\Onfoot\\Civilian1b.abc";
		break;

		case MI_AI_CIVILIAN2_ID:
			pRet = "Models\\Enemies\\Onfoot\\Civilian2.abc";
		break;

		case MI_AI_TROOPER_ID:
			pRet = "Models\\Enemies\\Onfoot\\Trooper.abc";
		break;

		case MI_AI_ETROOPER_ID:
			pRet = "Models\\Enemies\\Onfoot\\EliteTrooper.abc";
		break;

		case MI_AI_STROOPER_ID:
			pRet = "Models\\Enemies\\Onfoot\\ShockTrooper.abc";
		break;

		case MI_AI_ESTROOPER_ID:
			pRet = "Models\\Enemies\\Onfoot\\EliteShockTrooper.abc";
		break;

		case MI_AI_OFFICER_ID:
			pRet = "Models\\Enemies\\Onfoot\\Officer.abc";
		break;

		case MI_AI_AVC_ID:
			pRet = "Models\\Enemies\\Mca\\AVC_MEV.abc";
		break;

		case MI_AI_ORDOG_ID:
			pRet = "Models\\Enemies\\Mca\\Ordog.abc";
		break;

		case MI_AI_AKUMA_ID:
			pRet = "Models\\Enemies\\Mca\\Akuma.abc";
		break;

		case MI_AI_ENFORCER_ID:
			pRet = "Models\\Enemies\\Mca\\Enforcer.abc";
		break;

		case MI_AI_PREDATOR_ID:
			pRet = "Models\\Enemies\\Mca\\Predator.abc";
		break;

		case MI_AI_ASSASSIN_ID:
			pRet = "Models\\Enemies\\Mca\\Assassin.abc";
		break;

		case MI_AI_ANDRA5_ID:
			pRet = "Models\\Enemies\\Mca\\Andra5.abc";
		break;

		case MI_AI_ANDRA10_ID:
			pRet = "Models\\Enemies\\Mca\\Andra10.abc";
		break;

		case MI_AI_RAKSHA_ID:
			pRet = "Models\\Enemies\\Mca\\Raksha.abc";
		break;

		case MI_AI_TENMA_ID:
			pRet = "Models\\Enemies\\Mca\\Tenma.abc";
		break;


		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
			pRet = "Models\\Enemies\\OnFoot\\Rascal.abc";
		break;

		case MI_AI_VIGILANCE_ID:
			pRet = "Models\\Enemies\\OnFoot\\Vigilance.abc";
		break;

		case MI_AI_HAMMERHEAD_ID:
			pRet = "Models\\Enemies\\OnFoot\\HammerHead.abc";
		break;

		case MI_AI_VANDAL_ID:	
			pRet = "Models\\Enemies\\OnFoot\\Vandal.abc";
		break;

		case MI_AI_SPARROWHAWK_ID:
			pRet = "Models\\Enemies\\OnFoot\\Sparrowhawk.abc";
		break;

		case MI_AI_RUIN150_ID:
			pRet = "Models\\Enemies\\OnFoot\\Ruin150.abc";
		break;

		case MI_AI_UHLANA3_ID:
			pRet = "Models\\Enemies\\Mca\\UhlanA3.abc";
		break;


		// MAJOR CHARACTER MODELS ////////////////////////////////////////////

		case MI_AI_KURA_ID:
		{
			switch(size)
			{
				case MS_SMALL:
					pRet = "Models\\Characters\\Kid_Kura.abc";
				break;
				case MS_LARGE:
				case MS_NORMAL:
				default :
					pRet = "Models\\Characters\\Kura.abc";
				break;
			}
		}
		break;

		case MI_AI_HANK_ID:
			pRet = "Models\\Characters\\Hank.abc";
		break;

		case MI_AI_ADMIRAL_ID:
			pRet = "Models\\Characters\\Admiral.abc";
		break;

		case MI_AI_KATHRYN_ID:
			pRet = "Models\\Characters\\Kathryn.abc";
		break;

		case MI_AI_RYO_ID:
			pRet = "Models\\Characters\\Ryo.abc";
		break;

		case MI_AI_TOSHIRO_ID:
			pRet = "Models\\Characters\\Toshiro.abc";
		break;

		case MI_AI_COTHINEAL_ID:
			pRet = "Models\\Characters\\Cothineal.abc";
		break;

		case MI_AI_GABRIEL_ID:
			pRet = "Models\\Characters\\Gabriel.abc";
		break;

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
					pRet = "Models\\Characters\\Kid_Baku.abc";
				break;
				case MS_LARGE:
					pRet = "Models\\Characters\\Baku.abc";
				break;
				case MS_NORMAL:
				default :
					pRet = "Models\\Characters\\Baku_onfoot.abc";
				break;
			}
		}
		break;

		case MI_AI_SAMANTHA_ID:
			pRet = "Models\\Enemies\\Mca\\AVC_MEV.abc";
		break;

		default : break;
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSkin
//
//	PURPOSE:	Return the skin associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetSkin(uint8 nId, CharacterClass cc, ModelSize size, LTBOOL bMulti)
{
	char* pRet = NULL;

	switch (nId)
	{
		// PLAYER SKINS ///////////////////////////////////////////////////

		case MI_PLAYER_ONFOOT_ID:
			pRet = bMulti ? "Skins\\Player\\SanjuroM.dtx" : "Skins\\Player\\Sanjuro.dtx";
		break;

		case MI_PLAYER_KID_ID:
			pRet = bMulti ? "Skins\\Player\\kid.dtx" : "Skins\\Player\\kid.dtx";
		break;

		case MI_PLAYER_AKUMA_ID:
			pRet = bMulti ? "Skins\\Player\\AkumaM.dtx" : "Skins\\Enemies\\Akuma_UCA.dtx";
		break;

		case MI_PLAYER_PREDATOR_ID:
			pRet = bMulti ? "Skins\\Player\\PredatorM.dtx" : "Skins\\Enemies\\Predator_UCA.dtx";
		break;

		case MI_PLAYER_ORDOG_ID:
			pRet = bMulti ? "Skins\\Player\\OrdogM.dtx" : "Skins\\Enemies\\Ordog_UCA.dtx";
		break;

		case MI_PLAYER_ENFORCER_ID:
			pRet = bMulti ? "Skins\\Player\\EnforcerM.dtx" : "Skins\\Enemies\\Enforcer_UCA.dtx";
		break;


		// AI SKINS ////////////////////////////////////////////////////////

		case MI_AI_LITTLEBOY_ID:
			pRet = "Skins\\Enemies\\LittleBoy_a.dtx";
		break;

		case MI_AI_LITTLEGIRL_ID:
			pRet = "Skins\\Enemies\\LittleGirl_a.dtx";
		break;

		case MI_AI_CIVILIAN1_ID:
		case MI_AI_CIVILIAN1B_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\Civilian1_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\Civilian1_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet	= "Skins\\Enemies\\Civilian1_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\Civilian1_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\Civilian1_CRONIAN.dtx";
				break;
				case STRAGGLER:
					pRet	= "Skins\\Enemies\\Civilian1_STRAGGLER.dtx";
				break;
				case ROGUE:
					pRet	= "Skins\\Enemies\\Civilian1_ROGUE.dtx";
				break;
				case BYSTANDER:
				default :
					pRet	= "Skins\\Enemies\\Civilian1_BYSTANDER.dtx";
				break;
			}
		}
		break;

		case MI_AI_CIVILIAN2_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\Civilian2_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\Civilian2_CMC.dtx";
				break;
				case UCA:
					pRet	= "Skins\\Enemies\\Civilian2_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\Civilian2_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\Civilian2_CRONIAN.dtx";
				break;
				case STRAGGLER:
					pRet	= "Skins\\Enemies\\Civilian2_STRAGGLER.dtx";
				break;
				case ROGUE:
					pRet	= "Skins\\Enemies\\Civilian2_ROGUE.dtx";
				break;
				case BYSTANDER:
				default :
					pRet	= "Skins\\Enemies\\Civilian2_BYSTANDER.dtx";
				break;
			}
		}
		break;

		case MI_AI_ETROOPER_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\EliteTrooper_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\EliteTrooper_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet	= "Skins\\Enemies\\EliteTrooper_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\EliteTrooper_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\EliteTrooper_CRONIAN.dtx";
				break;
				default : 
					pRet	= "Skins\\Enemies\\EliteTrooper_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_TROOPER_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\Trooper_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\Trooper_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet	= "Skins\\Enemies\\Trooper_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\Trooper_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\Trooper_CRONIAN.dtx";
				break;
				case STRAGGLER:
					pRet	= "Skins\\Enemies\\Trooper_STRAGGLER.dtx";
				break;
				default : 
					pRet	= "Skins\\Enemies\\Trooper_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_ESTROOPER_ID:
		case MI_AI_STROOPER_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\ShockTrooper_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\ShockTrooper_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet	= "Skins\\Enemies\\ShockTrooper_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\ShockTrooper_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\ShockTrooper_CRONIAN.dtx";
				break;
				default : 
					pRet	= "Skins\\Enemies\\ShockTrooper_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_OFFICER_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet	= "Skins\\Enemies\\Officer_SHOGO.dtx";
				break;
				case CMC:
					pRet	= "Skins\\Enemies\\Officer_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet	= "Skins\\Enemies\\Officer_UCA.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\Officer_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\Officer_CRONIAN.dtx";
				break;
				default : 
					pRet	= "Skins\\Enemies\\Officer_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_AVC_ID:
		{
			switch (cc)
			{
				case CRONIAN:
					pRet = "Skins\\Enemies\\AVC_CRONIAN.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\AVC_FALLEN.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\AVC_CMC.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\AVC_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_ORDOG_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Ordog_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Ordog_CMC.dtx";
				break;
				case UCA:
				case UCA_BAD:
					pRet = "Skins\\Enemies\\Ordog_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Ordog_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Ordog_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Ordog_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_AKUMA_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Akuma_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Akuma_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Akuma_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Akuma_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Akuma_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Akuma_a.dtx";
				break;
			}
		}
		break;

		case MI_AI_ENFORCER_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Enforcer_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Enforcer_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Enforcer_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Enforcer_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Enforcer_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Enforcer_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_PREDATOR_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Predator_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Predator_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Predator_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Predator_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Predator_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Predator_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_ASSASSIN_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Assassin_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Assassin_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Assassin_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Assassin_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Assassin_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Assassin_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_ANDRA5_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Andra5_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Andra5_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Andra5_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Andra5_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Andra5_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Andra5_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_ANDRA10_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Andra10_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Andra10_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Andra10_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Andra10_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Andra10_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Andra10_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_RAKSHA_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Raksha_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Raksha_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Raksha_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Raksha_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Raksha_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Raksha_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_TENMA_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Tenma_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Tenma_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Tenma_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Tenma_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Tenma_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Tenma_FALLEN.dtx";
				break;
			}
		}
		break;


		// VEHICLE SKINS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
		{
			switch (cc)
			{
				case CMC:
					pRet	= "Skins\\Enemies\\Rascal_CMC.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\Rascal_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\Rascal_CRONIAN.dtx";
				break;
				case UCA:
					pRet	= "Skins\\Enemies\\Rascal_UCA.dtx";
				break;
				default : 
					pRet	= "Skins\\Enemies\\Rascal_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_VIGILANCE_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Vigilance_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Vigilance_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Vigilance_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Vigilance_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Vigilance_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Vigilance_a.dtx";
				break;
			}
		}
		break;

		case MI_AI_HAMMERHEAD_ID:
		{
			switch (cc)
			{
				case CMC:
					pRet	= "Skins\\Enemies\\HammerHead_CMC.dtx";
				break;
				case FALLEN:
					pRet	= "Skins\\Enemies\\HammerHead_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet	= "Skins\\Enemies\\HammerHead_CRONIAN.dtx";
				break;
				case UCA:
					pRet	= "Skins\\Enemies\\HammerHead_UCA.dtx";
				break;
				default :
					pRet	= "Skins\\Enemies\\HammerHead_CMC.dtx";
				break;
			}
		}
		break;

		case MI_AI_VANDAL_ID:	
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Vandal_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Vandal_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Vandal_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Vandal_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Vandal_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Vandal_a.dtx";
				break;
			}
		}
		break;

		case MI_AI_SPARROWHAWK_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Sparrowhawk_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Sparrowhawk_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Sparrowhawk_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Sparrowhawk_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Sparrowhawk_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Sparrowhawk_a.dtx";
				break;
			}
		}
		break;

		case MI_AI_RUIN150_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\Ruin150_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\Ruin150_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\Ruin150_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\Ruin150_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\Ruin150_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\Ruin150_a.dtx";
				break;
			}
		}
		break;

		case MI_AI_UHLANA3_ID:
		{
			switch (cc)
			{
				case SHOGO:
					pRet = "Skins\\Enemies\\UhlanA3_SHOGO.dtx";
				break;
				case CMC:
					pRet = "Skins\\Enemies\\UhlanA3_CMC.dtx";
				break;
				case UCA:
					pRet = "Skins\\Enemies\\UhlanA3_UCA.dtx";
				break;
				case FALLEN:
					pRet = "Skins\\Enemies\\UhlanA3_FALLEN.dtx";
				break;
				case CRONIAN:
					pRet = "Skins\\Enemies\\UhlanA3_CRONIAN.dtx";
				break;
				default : 
					pRet = "Skins\\Enemies\\UhlanA3_a.dtx";
				break;
			}
		}
		break;

		// MAJOR CHARACTER SKINS ////////////////////////////////////////////

		case MI_AI_KURA_ID:
		{
			switch(size)
			{
				case MS_SMALL:
					pRet = "Skins\\Characters\\Kid_Kura.dtx";
				break;
				case MS_LARGE:
				case MS_NORMAL:
				default :
					pRet = "Skins\\Characters\\Kura.dtx";
				break;
			}
		}
		break;

		case MI_AI_HANK_ID:
			pRet = "Skins\\Characters\\Hank.dtx";
		break;

		case MI_AI_ADMIRAL_ID:
			pRet = "Skins\\Characters\\Admiral.dtx";
		break;

		case MI_AI_KATHRYN_ID:
			pRet = "Skins\\Characters\\Kathryn.dtx";
		break;

		case MI_AI_RYO_ID:
			pRet = "Skins\\Characters\\Ryo.dtx";
		break;

		case MI_AI_TOSHIRO_ID:
			pRet = "Skins\\Characters\\Toshiro.dtx";
		break;

		case MI_AI_GABRIEL_ID:
			pRet = "Skins\\Characters\\Gabriel.dtx";
		break;

		case MI_AI_COTHINEAL_ID:
			pRet = "Skins\\Characters\\Cothineal.dtx";
		break;


		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
					pRet = "Skins\\Characters\\Kid_Baku.dtx";
				break;
				case MS_LARGE:
					pRet = "Skins\\Characters\\Baku.dtx";
				break;
				case MS_NORMAL:
				default :
					pRet = "Skins\\Characters\\Baku_onfoot.dtx";
				break;
			}
		}
		break;

		case MI_AI_SAMANTHA_ID:
			pRet = "Skins\\Characters\\Samantha.dtx";
		break;

		default : break;
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetGibModel
//
//	PURPOSE:	Return the gib model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetGibModel(uint8 nId, GibType eType, ModelSize size)
{
	char* pFile = NULL;

	switch (nId)
	{
		// MECHA MODELS ////////////////////////////////////////////////////

		case MI_PLAYER_AKUMA_ID:
		case MI_AI_AKUMA_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Akuma\\Akuma_head.abc",
				"Models\\Gibs\\Akuma\\Akuma_larm.abc",
				"Models\\Gibs\\Akuma\\Akuma_rarm.abc",
				"Models\\Gibs\\Akuma\\Akuma_lleg.abc",
				"Models\\Gibs\\Akuma\\Akuma_rleg.abc",
				"Models\\Gibs\\Akuma\\Akuma_ubody.abc",
				"Models\\Gibs\\Akuma\\Akuma_lbody.abc",
				"Models\\Player\\Akuma.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_SAMANTHA_ID:
		case MI_AI_AVC_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_head.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_larm.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_rarm.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_lleg.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_rleg.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_ubody.abc",
				"Models\\Gibs\\AVC_MEV\\AVC_MEV_lbody.abc",
				"Models\\Enemies\\MCA\\AVC_MEV.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_GABRIEL_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Gabriel\\Gabriel_head.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_larm.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_rarm.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_lleg.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_rleg.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_ubody.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_lbody.abc",
				"Models\\Characters\\Gabriel.abc"
			};

			char* GibFiles2[] = 
			{
				"Models\\Gibs\\Gabriel\\Gabriel_sword.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_lwing.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_rwing.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_lleg.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_rleg.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_torso.abc",
				"Models\\Gibs\\Gabriel\\Gabriel_lbody.abc",
				"Models\\Characters\\Gabriel.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GetRandom(0, 1) == 0 ? GibFiles[eType] : GibFiles2[eType];
			}
		}
		break;

		case MI_PLAYER_ORDOG_ID:
		case MI_AI_ORDOG_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Ordog\\Ordog_head.abc",
				"Models\\Gibs\\Ordog\\Ordog_larm.abc",
				"Models\\Gibs\\Ordog\\Ordog_rarm.abc",
				"Models\\Gibs\\Ordog\\Ordog_lleg.abc",
				"Models\\Gibs\\Ordog\\Ordog_rleg.abc",
				"Models\\Gibs\\Ordog\\Ordog_ubody.abc",
				"Models\\Gibs\\Ordog\\Ordog_lbody.abc",
				"Models\\Player\\Ordog.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_RAKSHA_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Raksha\\Raksha_head.abc",
				"Models\\Gibs\\Raksha\\Raksha_larm.abc",
				"Models\\Gibs\\Raksha\\Raksha_rarm.abc",
				"Models\\Gibs\\Raksha\\Raksha_lleg.abc",
				"Models\\Gibs\\Raksha\\Raksha_rleg.abc",
				"Models\\Gibs\\Raksha\\Raksha_ubody.abc",
				"Models\\Gibs\\Raksha\\Raksha_lbody.abc",
				"Models\\Enemies\\MCA\\Raksha.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_RYO_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Ryo\\Ryo_head.abc",
				"Models\\Gibs\\Ryo\\Ryo_larm.abc",
				"Models\\Gibs\\Ryo\\Ryo_rarm.abc",
				"Models\\Gibs\\Ryo\\Ryo_lleg.abc",
				"Models\\Gibs\\Ryo\\Ryo_rleg.abc",
				"Models\\Gibs\\Ryo\\Ryo_ubody.abc",
				"Models\\Gibs\\Ryo\\Ryo_lbody.abc",
				"Models\\Characters\\Ryo.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_TENMA_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Tenma\\Tenma_head.abc",
				"Models\\Gibs\\Tenma\\Tenma_larm.abc",
				"Models\\Gibs\\Tenma\\Tenma_rarm.abc",
				"Models\\Gibs\\Tenma\\Tenma_lleg.abc",
				"Models\\Gibs\\Tenma\\Tenma_rleg.abc",
				"Models\\Gibs\\Tenma\\Tenma_ubody.abc",
				"Models\\Gibs\\Tenma\\Tenma_lbody.abc",
				"Models\\Enemies\\MCA\\Tenma.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_ASSASSIN_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Assassin\\Assassin_head.abc",
				"Models\\Gibs\\Assassin\\Assassin_larm.abc",
				"Models\\Gibs\\Assassin\\Assassin_rarm.abc",
				"Models\\Gibs\\Assassin\\Assassin_lleg.abc",
				"Models\\Gibs\\Assassin\\Assassin_rleg.abc",
				"Models\\Gibs\\Assassin\\Assassin_ubody.abc",
				"Models\\Gibs\\Assassin\\Assassin_lbody.abc",
				"Models\\Enemies\\MCA\\Assassin.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_PLAYER_PREDATOR_ID:
		case MI_AI_PREDATOR_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Predator\\Predator_head.abc",
				"Models\\Gibs\\Predator\\Predator_larm.abc",
				"Models\\Gibs\\Predator\\Predator_rarm.abc",
				"Models\\Gibs\\Predator\\Predator_lleg.abc",
				"Models\\Gibs\\Predator\\Predator_rleg.abc",
				"Models\\Gibs\\Predator\\Predator_ubody.abc",
				"Models\\Gibs\\Predator\\Predator_lbody.abc",
				"Models\\Player\\Predator.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_PLAYER_ENFORCER_ID:
		case MI_AI_ENFORCER_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Enforcer\\enforcer_head.abc",
				"Models\\Gibs\\Enforcer\\enforcer_larm.abc",
				"Models\\Gibs\\Enforcer\\enforcer_rarm.abc",
				"Models\\Gibs\\Enforcer\\enforcer_lleg.abc",
				"Models\\Gibs\\Enforcer\\enforcer_rleg.abc",
				"Models\\Gibs\\Enforcer\\enforcer_ubody.abc",
				"Models\\Gibs\\Enforcer\\enforcer_lbody.abc",
				"Models\\Player\\Enforcer.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_ANDRA5_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Andra5\\Andra5_head.abc",
				"Models\\Gibs\\Andra5\\Andra5_larm.abc",
				"Models\\Gibs\\Andra5\\Andra5_rarm.abc",
				"Models\\Gibs\\Andra5\\Andra5_lleg.abc",
				"Models\\Gibs\\Andra5\\Andra5_rleg.abc",
				"Models\\Gibs\\Andra5\\Andra5_ubody.abc",
				"Models\\Gibs\\Andra5\\Andra5_lbody.abc",
				"Models\\Enemies\\Mca\\Andra5.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}		
		}
		break;

		case MI_AI_ANDRA10_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Andra10\\Andra10_head.abc",
				"Models\\Gibs\\Andra10\\Andra10_larm.abc",
				"Models\\Gibs\\Andra10\\Andra10_rarm.abc",
				"Models\\Gibs\\Andra10\\Andra10_lleg.abc",
				"Models\\Gibs\\Andra10\\Andra10_rleg.abc",
				"Models\\Gibs\\Andra10\\Andra10_ubody.abc",
				"Models\\Gibs\\Andra10\\Andra10_lbody.abc",
				"Models\\Enemies\\Mca\\Andra10.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}		
		}
		break;


		// HUMAN MODELS ////////////////////////////////////////////////////

		case MI_AI_HANK_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Hank\\Hank_head.abc",
				"Models\\Gibs\\Hank\\Hank_larm.abc",
				"Models\\Gibs\\Hank\\Hank_rarm.abc",
				"Models\\Gibs\\Hank\\Hank_lleg.abc",
				"Models\\Gibs\\Hank\\Hank_rleg.abc",
				"Models\\Gibs\\Hank\\Hank_ubody.abc",
				"Models\\Gibs\\Hank\\Hank_lbody.abc",
				"Models\\Characters\\Hank.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_PLAYER_ONFOOT_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Sanjuro\\Sanjuro_head.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_larm.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_rarm.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_lleg.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_rleg.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_ubody.abc",
				"Models\\Gibs\\Sanjuro\\Sanjuro_lbody.abc",
				"Models\\Player\\Sanjuro.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_ADMIRAL_ID:
		case MI_AI_TOSHIRO_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Officer\\Officer_head.abc",
				"Models\\Gibs\\Officer\\Officer_larm.abc",
				"Models\\Gibs\\Officer\\Officer_rarm.abc",
				"Models\\Gibs\\Officer\\Officer_lleg.abc",
				"Models\\Gibs\\Officer\\Officer_rleg.abc",
				"Models\\Gibs\\Officer\\Officer_ubody.abc",
				"Models\\Gibs\\Officer\\Officer_lbody.abc",
				"Models\\Enemies\\Onfoot\\Officer.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_PLAYER_KID_ID:
		case MI_AI_LITTLEBOY_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\LittleBoy\\LittleBoy_head.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_larm.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_rarm.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_lleg.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_rleg.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_ubody.abc",
				"Models\\Gibs\\LittleBoy\\LittleBoy_lbody.abc",
				"Models\\Enemies\\Onfoot\\LittleBoy.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_LITTLEGIRL_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\LittleGirl\\LittleGirl_head.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_larm.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_rarm.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_lleg.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_rleg.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_ubody.abc",
				"Models\\Gibs\\LittleGirl\\LittleGirl_lbody.abc",
				"Models\\Enemies\\Onfoot\\LittleGirl.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_KURA_ID:
		case MI_AI_KATHRYN_ID:
		case MI_AI_CIVILIAN2_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Civilian2\\Civilian2_head.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_larm.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_rarm.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_lleg.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_rleg.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_ubody.abc",
				"Models\\Gibs\\Civilian2\\Civilian2_lbody.abc",
				"Models\\Enemies\\Onfoot\\Civilian2.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_OFFICER_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Officer\\Officer_head.abc",
				"Models\\Gibs\\Officer\\Officer_larm.abc",
				"Models\\Gibs\\Officer\\Officer_rarm.abc",
				"Models\\Gibs\\Officer\\Officer_lleg.abc",
				"Models\\Gibs\\Officer\\Officer_rleg.abc",
				"Models\\Gibs\\Officer\\Officer_ubody.abc",
				"Models\\Gibs\\Officer\\Officer_lbody.abc",
				"Models\\Enemies\\Onfoot\\Officer.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_ETROOPER_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_head.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_larm.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_rarm.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_lleg.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_rleg.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_ubody.abc",
				"Models\\Gibs\\EliteTrooper\\EliteTrooper_lbody.abc",
				"Models\\Enemies\\Onfoot\\EliteTrooper.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_TROOPER_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Trooper\\trooper_head.abc",
				"Models\\Gibs\\Trooper\\trooper_larm.abc",
				"Models\\Gibs\\Trooper\\trooper_rarm.abc",
				"Models\\Gibs\\Trooper\\trooper_lleg.abc",
				"Models\\Gibs\\Trooper\\trooper_rleg.abc",
				"Models\\Gibs\\Trooper\\trooper_ubody.abc",
				"Models\\Gibs\\Trooper\\trooper_lbody.abc",
				"Models\\Enemies\\Onfoot\\Trooper.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_ESTROOPER_ID:
		case MI_AI_STROOPER_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Shocktrooper\\shocktrooper_head.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_larm.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_rarm.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_lleg.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_rleg.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_ubody.abc",
				"Models\\Gibs\\Shocktrooper\\shocktrooper_lbody.abc",
				"Models\\Enemies\\Onfoot\\ShockTrooper.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		}
		break;

		case MI_AI_CIVILIAN1B_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Civilian1b\\civilian1b_head.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_larm.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_rarm.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_lleg.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_rleg.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_ubody.abc",
				"Models\\Gibs\\Civilian1b\\civilian1b_lbody.abc",
				"Models\\Enemies\\Onfoot\\Civilian1b.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		} 
		break;

		case MI_AI_CIVILIAN1_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Civilian1a\\civilian1a_head.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_larm.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_rarm.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_lleg.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_rleg.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_ubody.abc",
				"Models\\Gibs\\Civilian1a\\civilian1a_lbody.abc",
				"Models\\Enemies\\Onfoot\\Civilian1a.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}
		} 
		break;


		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
		{
			if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Rascal.abc";
			}
		}
		break;

		case MI_AI_VIGILANCE_ID:
		{
			if (eType == GT_FIRST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Vigilance_turret.abc";
			}
			else if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Vigilance.abc";
			}
		}
		break;

		case MI_AI_HAMMERHEAD_ID:
		{
			if (eType == GT_FIRST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Hammerhead_turret.abc";
			}
			else if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Hammerhead.abc";
			}
		}
		break;

		case MI_AI_VANDAL_ID:
		{
			if (eType == GT_FIRST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Vandal_turret.abc";
			}
			else if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Vandal.abc";
			}
		}
		break;

		case MI_AI_SPARROWHAWK_ID:
		{
			if (eType == GT_FIRST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Sparrowhawk_turret.abc";
			}
			else if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Sparrowhawk.abc";
			}
		}
		break;

		case MI_AI_RUIN150_ID:
		{
			if (eType == GT_FIRST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Ruin150_turret.abc";
			}
			else if (eType == GT_LAST)
			{
				pFile = "Models\\Enemies\\Onfoot\\Ruin150.abc";
			}
		}
		break;

		case MI_AI_UHLANA3_ID:
		{
			char* GibFiles[] = 
			{
				"Models\\Gibs\\Uhlan\\Uhlan_lleg2.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_larm.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_rarm.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_lleg1.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_rleg1.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_body.abc",
				"Models\\Gibs\\Uhlan\\Uhlan_rleg2.abc",
				"Models\\Enemies\\Mca\\UhlanA3.abc"
			};

			if (GT_FIRST <= eType && eType < GT_LAST)
			{
				pFile = GibFiles[eType];
			}		
		}
		break;


		// MAJOR CHARACTER MODELS ////////////////////////////////////////////

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				{
					char* GibFiles[] = 
					{
						"Models\\Gibs\\LittleBoy\\LittleBoy_head.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_larm.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_rarm.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_lleg.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_rleg.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_ubody.abc",
						"Models\\Gibs\\LittleBoy\\LittleBoy_lbody.abc",
						"Models\\Enemies\\Onfoot\\LittleBoy.abc"
					};

					if (GT_FIRST <= eType && eType < GT_LAST)
					{
						pFile = GibFiles[eType];
					}
				}
				break;

				case MS_NORMAL:
				{
					char* GibFiles[] = 
					{
						"Models\\Gibs\\Shocktrooper\\shocktrooper_head.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_larm.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_rarm.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_lleg.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_rleg.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_ubody.abc",
						"Models\\Gibs\\Shocktrooper\\shocktrooper_lbody.abc",
						"Models\\Enemies\\Onfoot\\ShockTrooper.abc"
					};

					if (GT_FIRST <= eType && eType < GT_LAST)
					{
						pFile = GibFiles[eType];
					}
				}
				break;
	
				case MS_LARGE:
				{
					char* GibFiles[] = 
					{
						"Models\\Gibs\\Predator\\Predator_head.abc",
						"Models\\Gibs\\Predator\\Predator_larm.abc",
						"Models\\Gibs\\Predator\\Predator_rarm.abc",
						"Models\\Gibs\\Predator\\Predator_lleg.abc",
						"Models\\Gibs\\Predator\\Predator_rleg.abc",
						"Models\\Gibs\\Predator\\Predator_ubody.abc",
						"Models\\Gibs\\Predator\\Predator_lbody.abc",
						"Models\\Player\\Predator.abc"
					};

					if (GT_FIRST <= eType && eType < GT_LAST)
					{
						pFile = GibFiles[eType];
					}
				}
				break;

				default :
				break;
			}
		}
		break;

		default : break;
	}

	return pFile;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModelType
//
//	PURPOSE:	Return the type of the model
//
// ----------------------------------------------------------------------- //

ModelType GetModelType(uint8 nId, ModelSize size)
{
	ModelType eType = MT_UNSPECIFIED;

	switch (nId)
	{
		// MECHA MODELS ////////////////////////////////////////////////////

		case MI_AI_GABRIEL_ID:
		case MI_AI_SAMANTHA_ID:
		case MI_PLAYER_AKUMA_ID:
		case MI_PLAYER_PREDATOR_ID:
		case MI_PLAYER_ORDOG_ID:
		case MI_PLAYER_ENFORCER_ID:
		case MI_AI_AVC_ID:
		case MI_AI_ORDOG_ID:
		case MI_AI_AKUMA_ID:
		case MI_AI_ENFORCER_ID:
		case MI_AI_PREDATOR_ID:
		case MI_AI_ASSASSIN_ID:
		case MI_AI_ANDRA5_ID:
		case MI_AI_ANDRA10_ID:
		case MI_AI_RAKSHA_ID:
		case MI_AI_TENMA_ID:
			eType = MT_MECHA;
		break;

		// HUMAN MODELS ////////////////////////////////////////////////////

		case MI_AI_KURA_ID:
		case MI_AI_HANK_ID:
		case MI_AI_ADMIRAL_ID:
		case MI_AI_KATHRYN_ID:
		case MI_AI_RYO_ID:
		case MI_AI_TOSHIRO_ID:
		case MI_PLAYER_ONFOOT_ID:
		case MI_PLAYER_KID_ID:
		case MI_AI_LITTLEBOY_ID:
		case MI_AI_LITTLEGIRL_ID:
		case MI_AI_CIVILIAN1_ID:
		case MI_AI_CIVILIAN1B_ID:
		case MI_AI_CIVILIAN2_ID:
		case MI_AI_ESTROOPER_ID:
		case MI_AI_STROOPER_ID:
		case MI_AI_OFFICER_ID:
		case MI_AI_TROOPER_ID:
		case MI_AI_ETROOPER_ID:
		case MI_AI_COTHINEAL_ID:
			eType = MT_HUMAN;
		break;

		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
		case MI_AI_VIGILANCE_ID:
		case MI_AI_HAMMERHEAD_ID:
		case MI_AI_VANDAL_ID:	
		case MI_AI_SPARROWHAWK_ID:
		case MI_AI_RUIN150_ID:
		case MI_AI_UHLANA3_ID:
			eType = MT_VEHICLE;
		break;

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
				case MS_NORMAL:
					eType = MT_HUMAN;
				break;
				case MS_LARGE:
					eType = MT_MECHA;
				break;
				default :
				break;
			}
		}
		break;

		default : break;
	}

	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetTurretFilename
//
//	PURPOSE:	Return the turret model associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetTurretFilename(uint8 nId, ModelSize size)
{
	char* pRet = NULL;

	switch (nId)
	{
		case MI_AI_VANDAL_ID:	
		{
			pRet = "Models\\Enemies\\Onfoot\\Vandal_turret.abc";
		}
		break;

		case MI_AI_HAMMERHEAD_ID:
		{
			pRet = "Models\\Enemies\\Onfoot\\HammerHead_turret.abc";
		}
		break;

		case MI_AI_SPARROWHAWK_ID:
		{
			pRet = "Models\\Enemies\\Onfoot\\SparrowHawk_turret.abc";
		}
		break;

		case MI_AI_RUIN150_ID:
		{
			pRet = "Models\\Enemies\\Onfoot\\Ruin150_turret.abc";
		}
		break;

		case MI_AI_VIGILANCE_ID:
		{
			pRet = "Models\\Enemies\\OnFoot\\Vigilance_turret.abc";
		}
		break;

		default : break;
	}

	return pRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetModelName
//
//	PURPOSE:	Return the model name associated with a particular id
//
// ----------------------------------------------------------------------- //

char* GetModelName(uint8 nId, ModelSize size)
{
	char* pName = NULL;

	switch (nId)
	{
		// MECHA MODELS ////////////////////////////////////////////////////

		case MI_AI_GABRIEL_ID:
			pName = "Gabriel";
		break;

		case MI_AI_SAMANTHA_ID:
			pName = "Samantha";
		break;

		case MI_AI_AKUMA_ID:
		case MI_PLAYER_AKUMA_ID:
			pName = "Akuma";
		break;

		case MI_AI_PREDATOR_ID:
		case MI_PLAYER_PREDATOR_ID:
			pName = "Predator";
		break;

		case MI_AI_ORDOG_ID:
		case MI_PLAYER_ORDOG_ID:
			pName = "Ordog";
		break;

		case MI_AI_ENFORCER_ID:
		case MI_PLAYER_ENFORCER_ID:
			pName = "Ordog";
		break;

		case MI_AI_AVC_ID:
			pName = "AVC";
		break;

		case MI_AI_ASSASSIN_ID:
			pName = "Assassin";
		break;

		case MI_AI_ANDRA5_ID:
			pName = "Andra5";
		break;

		case MI_AI_ANDRA10_ID:
			pName = "Andra10";
		break;

		case MI_AI_RAKSHA_ID:
			pName = "Raksha";
		break;

		case MI_AI_TENMA_ID:
			pName = "Tenma";
		break;

		// HUMAN MODELS ////////////////////////////////////////////////////

		case MI_AI_KURA_ID:
			pName = "Kura";
		break;

		case MI_AI_HANK_ID:
			pName = "Hank";
		break;

		case MI_AI_ADMIRAL_ID:
			pName = "Admiral";
		break;

		case MI_AI_KATHRYN_ID:
			pName = "Kathryn";
		break;

		case MI_AI_RYO_ID:
			pName = "Ryo";
		break;

		case MI_AI_TOSHIRO_ID:
			pName = "Toshiro";
		break;

		case MI_PLAYER_ONFOOT_ID:
			pName = "Sanjuro";
		break;

		case MI_PLAYER_KID_ID:
			pName = "Kid";
		break;

		case MI_AI_LITTLEBOY_ID:
			pName = "LittleBoy";
		break;

		case MI_AI_LITTLEGIRL_ID:
			pName = "LittleGirl";
		break;

		case MI_AI_CIVILIAN1_ID:
		case MI_AI_CIVILIAN1B_ID:
			pName = "Civilian1";
		break;

		case MI_AI_CIVILIAN2_ID:
			pName = "Civilian2";
		break;

		case MI_AI_STROOPER_ID:
			pName = "ShockTrooper";
		break;

		case MI_AI_ESTROOPER_ID:
			pName = "ShockTrooper";
		break;

		case MI_AI_TROOPER_ID:
			pName = "Trooper";
		break;

		case MI_AI_ETROOPER_ID:
			pName = "Trooper";
		break;

		case MI_AI_OFFICER_ID:
			pName = "Officer";
		break;

		case MI_AI_COTHINEAL_ID:
			pName = "Cothineal";
		break;

		// VEHICLE MODELS ////////////////////////////////////////////////////

		case MI_AI_RASCAL_ID:	
			pName = "Rascal";
		break;

		case MI_AI_VIGILANCE_ID:
			pName = "Vigilance";
		break;

		case MI_AI_HAMMERHEAD_ID:
			pName = "Hammerhead";
		break;

		case MI_AI_VANDAL_ID:	
			pName = "Vandal";
		break;

		case MI_AI_SPARROWHAWK_ID:
			pName = "Sparrowhawk";
		break;

		case MI_AI_RUIN150_ID:
			pName = "Ruin150";
		break;

		case MI_AI_UHLANA3_ID:
			pName = "UhlanA3";
		break;

		case MI_AI_BAKU_ID:
		{
			switch(size)
			{
				case MS_SMALL:
					pName = "Kid_Baku";
				break;
				case MS_NORMAL:
					pName = "Baku";
				break;
				case MS_LARGE:
					pName = "Baku_mca";
				break;
				default :
				break;
			}
		}
		break;
	}

	return pName;
}
