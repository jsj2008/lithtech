// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisTypes.cpp
//
// PURPOSE : Debris types
//
// CREATED : 7/2/98
//
// ----------------------------------------------------------------------- //

#include "DebrisTypes.h"


// Debris model related sheyot...

char* s_debrisGenericFilename[] =
{
	"Models\\Props\\Debris\\generic1.abc",
	"Models\\Props\\Debris\\generic2.abc",
	"Models\\Props\\Debris\\generic3.abc",
	"Models\\Props\\Debris\\generic4.abc",
	"Models\\Props\\Debris\\generic5.abc",
	"Models\\Props\\Debris\\generic6.abc",
	"Models\\Props\\Debris\\generic7.abc"
};

char* s_debrisGernicFlatFilename[] =
{
	"Models\\Props\\Debris\\generic_flat1.abc",
	"Models\\Props\\Debris\\generic_flat2.abc",
	"Models\\Props\\Debris\\generic_flat3.abc"
};

char* s_debrisBoardFilename[] =
{
	"Models\\Props\\Debris\\board1.abc",
	"Models\\Props\\Debris\\board2.abc",
	"Models\\Props\\Debris\\board3.abc"
};

char* s_debrisBranchFilename[] =
{
	"Models\\Props\\Debris\\branch1.abc",
	"Models\\Props\\Debris\\branch2.abc",
	"Models\\Props\\Debris\\branch3.abc"
};

char* s_debrisWoodChipFilename[] =
{
	"Models\\Props\\Debris\\woodchip1.abc",
	"Models\\Props\\Debris\\woodchip2.abc",
	"Models\\Props\\Debris\\woodchip3.abc"
};

char* s_debrisPlasticFilename[] =
{
	"Models\\Props\\Debris\\generic_flat1.abc",
	"Models\\Props\\Debris\\generic_flat1.abc",
	"Models\\Props\\Debris\\generic_flat1.abc"
};

char* s_debrisGlassFilename[] =
{
	"Models\\Props\\Debris\\Glass1.abc",
	"Models\\Props\\Debris\\Glass2.abc",
	"Models\\Props\\Debris\\Glass3.abc"
};

char* s_debrisFeatherFilename[] =
{
	"Models\\Props\\Debris\\Feather1.abc",
	"Models\\Props\\Debris\\Feather2.abc",
	"Models\\Props\\Debris\\Feather3.abc"
};

char* s_debrisStoneFilename[] =
{
	"Models\\Props\\Debris\\Stone1.abc",
	"Models\\Props\\Debris\\Stone2.abc",
	"Models\\Props\\Debris\\Stone3.abc"
};

char* s_debrisMetalFilename[] =
{
	"Models\\Props\\Debris\\Metal1.abc",
	"Models\\Props\\Debris\\Metal2.abc",
	"Models\\Props\\Debris\\Metal3.abc"
};

char* s_debrisCarPartFilename[] =
{
	"Models\\Props\\Debris\\CarPart1.abc",
	"Models\\Props\\Debris\\CarPart2.abc",
	"Models\\Props\\Debris\\CarPart3.abc"
};

char* s_debrisMechaPartFilename[] =
{
	"Models\\Props\\Debris\\MechaPart1.abc",
	"Models\\Props\\Debris\\MechaPart2.abc",
	"Models\\Props\\Debris\\MechaPart3.abc"
};

char* s_debrisVehiclePartFilename[] =
{
	"Models\\Props\\Debris\\VehiclePart1.abc",
	"Models\\Props\\Debris\\VehiclePart2.abc",
	"Models\\Props\\Debris\\VehiclePart3.abc"
};

char* s_debrisHumanFilename[] =
{
	"Models\\Props\\Debris\\HumanPart1.abc",
	"Models\\Props\\Debris\\HumanPart2.abc",
	"Models\\Props\\Debris\\HumanPart3.abc"
};

char** s_pDebrisModels[] =
{
	s_debrisGenericFilename,
	s_debrisGernicFlatFilename,
	s_debrisBoardFilename,
	s_debrisBranchFilename,
	s_debrisBranchFilename,
	s_debrisWoodChipFilename,
	s_debrisPlasticFilename,
	s_debrisGlassFilename,
	s_debrisGlassFilename,
	s_debrisFeatherFilename,
	s_debrisStoneFilename,
	s_debrisStoneFilename,
	s_debrisMetalFilename,
	s_debrisMetalFilename,
	s_debrisCarPartFilename,
	s_debrisMechaPartFilename,
	s_debrisVehiclePartFilename,
	s_debrisHumanFilename
};


// Debris skins...

char* s_pDebrisSkins[] =
{
	"Skins\\Props\\Debris\\generic.dtx",
	"Skins\\Props\\Debris\\generic.dtx",
	"Skins\\Props\\Debris\\board.dtx",
	"Skins\\Props\\Debris\\branch.dtx",
	"Skins\\Props\\Debris\\branch.dtx",
	"Skins\\Props\\Debris\\woodchip.dtx",
	"Skins\\Props\\Debris\\plastic.dtx",
	"Skins\\Props\\Debris\\glass.dtx",
	"Skins\\Props\\Debris\\glass.dtx",
	"Skins\\Props\\Debris\\feather.dtx",
	"Skins\\Props\\Debris\\stone.dtx",
	"Skins\\Props\\Debris\\stone.dtx",
	"Skins\\Props\\Debris\\metal.dtx",
	"Skins\\Props\\Debris\\metal.dtx",
	"Skins\\Props\\Debris\\car.dtx",
	"Skins\\Props\\Debris\\mecha.dtx", 	
	"Skins\\Props\\Debris\\vehicle.dtx", 
	"Skins\\Props\\Debris\\human.dtx"
};


// Debris bounce sound related sheyot...

char* s_debrisBoardBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Board\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Board\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Board\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Board\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Board\\gib_5.wav"
};

char* s_debrisBranchBounceBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\gib_5.wav"
};

char* s_debrisBranchBounceSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\gib_5.wav"
};

char* s_debrisWoodChipBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\gib_5.wav"
};

char* s_debrisPlasticBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Plastic\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Plastic\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Plastic\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Plastic\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Plastic\\gib_5.wav"
};

char* s_debrisGlassBounceBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\gib_5.wav"
};

char* s_debrisGlassBounceSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\gib_5.wav"
};

char* s_debrisStoneBounceBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigStone\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigStone\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\BigStone\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\BigStone\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\BigStone\\gib_5.wav"
};

char* s_debrisStoneBounceSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\gib_5.wav"
};

char* s_debrisMetalBounceBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\gib_5.wav"
};

char* s_debrisMetalBounceSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\gib_5.wav"
};

char* s_debrisCarPartBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Car\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Car\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Car\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Car\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Car\\gib_5.wav"
};

char* s_debrisMechaPartBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Mca\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Mca\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Mca\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Mca\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Mca\\gib_5.wav"
};

char* s_debrisVehiclePartBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\gib_5.wav"
};

char* s_debrisHumanBounce[] =
{
	"Sounds\\SpecialFX\\Gibs\\Human\\gib_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\gib_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\gib_3.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\gib_4.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\gib_5.wav"
};

char** s_pDebrisBounceSounds[] =
{
	s_debrisStoneBounceBig,
	s_debrisStoneBounceBig,
	s_debrisBoardBounce,
	s_debrisBranchBounceBig,
	s_debrisBranchBounceSmall,
	s_debrisWoodChipBounce,
	s_debrisPlasticBounce,
	s_debrisGlassBounceBig,
	s_debrisGlassBounceSmall,
	LTNULL,
	s_debrisStoneBounceBig,
	s_debrisStoneBounceSmall,
	s_debrisMetalBounceBig,
	s_debrisMetalBounceSmall,
	s_debrisCarPartBounce,
	s_debrisMechaPartBounce,
	s_debrisVehiclePartBounce,
	s_debrisHumanBounce
};



// Debris explosion sound related sheyot...

char* s_debrisBoardExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Board\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Board\\Explode_2.wav",
};

char* s_debrisBranchExplodeBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigBranches\\Explode_2.wav"
};

char* s_debrisBranchExplodeSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallBranches\\Explode_2.wav"
};

char* s_debrisWoodChipExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\WoodChips\\Explode_2.wav",
};

char* s_debrisPlasticExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Plastic\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Plastic\\Explode_2.wav"
};

char* s_debrisGlassExplodeBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigGlass\\Explode_2.wav"
};

char* s_debrisGlassExplodeSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallGlass\\Explode_2.wav"
};

char* s_debrisStoneExplodeBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigStone\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigStone\\Explode_2.wav"
};

char* s_debrisStoneExplodeSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallStone\\Explode_2.wav"
};

char* s_debrisMetalExplodeBig[] =
{
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\BigMetal\\Explode_2.wav"
};

char* s_debrisMetalExplodeSmall[] =
{
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\SmallMetal\\Explode_2.wav",
};

char* s_debrisCarPartExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Car\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Car\\Explode_2.wav"
};

char* s_debrisMechaPartExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Mca\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Mca\\Explode_2.wav"
};

char* s_debrisVehiclePartExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Vehicle\\Explode_2.wav"
};

char* s_debrisHumanExplode[] =
{
	"Sounds\\SpecialFX\\Gibs\\Human\\Explode_1.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\Explode_2.wav",
	"Sounds\\SpecialFX\\Gibs\\Human\\Explode_3.wav"
};

char** s_pDebrisExplodeSounds[] =
{
	LTNULL,
	LTNULL,
	s_debrisBoardExplode,
	s_debrisBranchExplodeBig,
	s_debrisBranchExplodeSmall,
	s_debrisWoodChipExplode,
	s_debrisPlasticExplode,
	s_debrisGlassExplodeBig,
	s_debrisGlassExplodeSmall,
	LTNULL,
	s_debrisStoneExplodeBig,
	s_debrisStoneExplodeSmall,
	s_debrisMetalExplodeBig,
	s_debrisMetalExplodeSmall,
	s_debrisCarPartExplode,
	s_debrisMechaPartExplode,
	s_debrisVehiclePartExplode,
	s_debrisHumanExplode
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSurfaceType
//
//	PURPOSE:	Return the surface type of the debris
//
// ----------------------------------------------------------------------- //

SurfaceType GetDebrisSurfaceType(DebrisType eType)
{
	SurfaceType eSurfType = ST_UNKNOWN;

	switch (eType)
	{
		case DBT_BOARDS:
		case DBT_BRANCHES_BIG:
			eSurfType = ST_WOOD;
		break;

		case DBT_BRANCHES_SMALL:
		case DBT_WOODCHIPS:
			eSurfType = ST_LIGHT_WOOD;
		break;

		case DBT_PLASTIC:
			eSurfType = ST_PLASTIC;
		break;

		case DBT_GLASS_BIG:
		case DBT_GLASS_SMALL:
			eSurfType = ST_GLASS;
		break;

		case DBT_FEATHERS:
			eSurfType = ST_CLOTH;
		break;

		case DBT_STONE_BIG:
			eSurfType = ST_STONE_HEAVY;
		break;

		case DBT_STONE_SMALL:
			eSurfType = ST_STONE_LIGHT;
		break;

		case DBT_METAL_BIG:
			eSurfType = ST_METAL;
		break;

		case DBT_METAL_SMALL:
			eSurfType = ST_METAL_LIGHT;
		break;

		case DBT_CAR_PARTS:
			eSurfType = ST_METAL_HOLLOW_HEAVY;
		break;

		case DBT_MECHA_PARTS:
			eSurfType = ST_METAL_HEAVY;
		break;

		case DBT_HUMAN_PARTS:
			eSurfType = ST_FLESH;
		break;

		case DBT_GENERIC:
		case DBT_GENERIC_FLAT:
		default :
			eSurfType = ST_STONE;
		break;
	}

	return eSurfType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisType
//
//	PURPOSE:	Return the debris type of the surface
//
// ----------------------------------------------------------------------- //

DebrisType GetDebrisType(SurfaceType eType)
{
	DebrisType eDebrisType = DBT_GENERIC;

	switch (eType)
	{
		case ST_STONE_HEAVY:
			eDebrisType = DBT_STONE_BIG;
		break;

		case ST_STONE:
		case ST_STONE_LIGHT:
			eDebrisType = DBT_STONE_SMALL;
		break;

		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_HOLLOW_HEAVY:
			eDebrisType = DBT_METAL_BIG;
		break;

		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_LIGHT:
			eDebrisType = DBT_METAL_SMALL;
		break;

		case ST_DENSE_WOOD:
			eDebrisType = DBT_BOARDS;
		break;

		case ST_WOOD:
		case ST_LIGHT_WOOD:
			eDebrisType = DBT_WOODCHIPS;
		break;

		case ST_PLASTIC:
			eDebrisType = DBT_PLASTIC;
		break;

		case ST_GLASS:
		case ST_ENERGY:
			eDebrisType = DBT_GLASS_SMALL;
		break;
		
		case ST_BUILDING:
			eDebrisType = DBT_GENERIC;
		break;

		case ST_CLOTH:
			eDebrisType = DBT_FEATHERS;
		break;

		case ST_CHAINFENCE:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
			eDebrisType = DBT_GENERIC_FLAT;
		break;
		
		case ST_FLESH:
			eDebrisType = DBT_HUMAN_PARTS;
		break;

		case ST_MECHA:
			eDebrisType = DBT_MECHA_PARTS;
		break;

		case ST_TERRAIN:
		case ST_LIQUID:
		case ST_UNKNOWN:
		case ST_SKY:
		default : break;
	}

	return eDebrisType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetVectorDebrisType
//
//	PURPOSE:	Return the debris type of the surface (for vector weapons)
//
// ----------------------------------------------------------------------- //

DebrisType GetVectorDebrisType(SurfaceType eType)
{
	DebrisType eDebrisType = DBT_GENERIC;

	switch (eType)
	{
		case ST_BUILDING:
		case ST_STONE_HEAVY:
		case ST_STONE:
		case ST_STONE_LIGHT:
			eDebrisType = DBT_STONE_SMALL;
		break;

		case ST_METAL:
		case ST_METAL_HEAVY:
		case ST_METAL_HOLLOW_HEAVY:
		case ST_METAL_LIGHT:
		case ST_METAL_HOLLOW:
		case ST_METAL_HOLLOW_LIGHT:
			eDebrisType = DBT_METAL_SMALL;
		break;

		case ST_DENSE_WOOD:
		case ST_WOOD:
		case ST_LIGHT_WOOD:
			eDebrisType = DBT_WOODCHIPS;
		break;

		case ST_GLASS:
		case ST_ENERGY:
			eDebrisType = DBT_GLASS_SMALL;
		break;
		
		case ST_CLOTH:
			eDebrisType = DBT_FEATHERS;
		break;

		case ST_PLASTIC:
		case ST_CHAINFENCE:
		case ST_PLASTIC_HEAVY:
		case ST_PLASTIC_LIGHT:
			eDebrisType = DBT_GENERIC_FLAT;
		break;
		
		case ST_FLESH:
			eDebrisType = DBT_HUMAN_PARTS;
		break;

		case ST_MECHA:
			eDebrisType = DBT_MECHA_PARTS;
		break;

		case ST_TERRAIN:
		case ST_LIQUID:
		case ST_UNKNOWN:
		case ST_SKY:
		default : break;
	}

	return eDebrisType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsSmallDebris
//
//	PURPOSE:	Return  true if the debris type is small
//
// ----------------------------------------------------------------------- //

LTBOOL IsSmallDebris(DebrisType eType)
{
	if (eType == DBT_GLASS_SMALL || 
		eType == DBT_STONE_SMALL ||
		eType == DBT_METAL_SMALL)
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisModels
//
//	PURPOSE:	Return the number of debris models associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisModels(DebrisType eType)
{

	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		switch (eType)
		{
			case DBT_GENERIC:
				return (sizeof (s_debrisGenericFilename) / sizeof (s_debrisGenericFilename[0]));
			break;

			case DBT_GENERIC_FLAT:
				return (sizeof (s_debrisGernicFlatFilename) / sizeof (s_debrisGernicFlatFilename[0]));
			break;

			case DBT_BOARDS:
				return (sizeof (s_debrisBoardFilename) / sizeof (s_debrisBoardFilename[0]));
			break;

			case DBT_BRANCHES_BIG:
			case DBT_BRANCHES_SMALL:
				return (sizeof (s_debrisBranchFilename) / sizeof (s_debrisBranchFilename[0]));
			break;

			case DBT_WOODCHIPS:
				return (sizeof (s_debrisWoodChipFilename) / sizeof (s_debrisWoodChipFilename[0]));
			break;

			case DBT_PLASTIC:
				return (sizeof (s_debrisPlasticFilename) / sizeof (s_debrisPlasticFilename[0]));
			break;

			case DBT_GLASS_BIG:
			case DBT_GLASS_SMALL:
				return (sizeof (s_debrisGlassFilename) / sizeof (s_debrisGlassFilename[0]));
			break;

			case DBT_FEATHERS:
				return (sizeof (s_debrisFeatherFilename) / sizeof (s_debrisFeatherFilename[0]));
			break;

			case DBT_STONE_BIG:
			case DBT_STONE_SMALL:
				return (sizeof (s_debrisStoneFilename) / sizeof (s_debrisStoneFilename[0]));
			break;

			case DBT_METAL_BIG:
			case DBT_METAL_SMALL:
				return (sizeof (s_debrisMetalFilename) / sizeof (s_debrisMetalFilename[0]));
			break;

			case DBT_CAR_PARTS:
				return (sizeof (s_debrisCarPartFilename) / sizeof (s_debrisCarPartFilename[0]));
			break;

			case DBT_MECHA_PARTS:
				return (sizeof (s_debrisMechaPartFilename) / sizeof (s_debrisMechaPartFilename[0]));
			break;

			case DBT_VEHICLE_PARTS:
				return (sizeof (s_debrisVehiclePartFilename) / sizeof (s_debrisVehiclePartFilename[0]));
			break;

			case DBT_HUMAN_PARTS:
				return (sizeof (s_debrisHumanFilename) / sizeof (s_debrisHumanFilename[0]));
			break;

			default :
			break;
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisModel
//
//	PURPOSE:	Return the model associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisModel(DebrisType eType, LTVector & vScale, int nIndex)
{
	LTFLOAT s_fDebrisScale[] =
	{
		1.5f,	// Generic
		2.0f,	// Generic flat
		0.13f,	// Board
		0.2f,	// big branch
		0.1f,	// small branch
		0.05f,	// woodchip
		0.5f,	// plastic
		0.2f,	// big glass
		0.1f,   // small glass 
		0.2f,	// feather 
		0.2f,	// big stone
		0.1f,	// small stone
		0.15f,	// big metal
		0.05f,	// small metal
		0.02f,	// car
		0.05f,	// mecha
		0.015f,	// vehicle
		0.25f	// human
	};

	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		VEC_MULSCALAR(vScale, vScale, s_fDebrisScale[eType]);

		int nNum = GetNumDebrisModels(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		return s_pDebrisModels[eType][i];
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisSkin
//
//	PURPOSE:	Return the skin associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisSkin(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		return s_pDebrisSkins[eType];
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisBounceSounds
//
//	PURPOSE:	Return the number of debris bounce sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisBounceSounds(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		if (s_pDebrisBounceSounds[eType])
		{
			return (sizeof (s_pDebrisBounceSounds[eType]) / sizeof (s_pDebrisBounceSounds[eType][0]));
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisBounceSound
//
//	PURPOSE:	Return the bounce sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisBounceSound(DebrisType eType, int nIndex)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		int nNum = GetNumDebrisBounceSounds(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		if (s_pDebrisBounceSounds[eType])
		{
			return s_pDebrisBounceSounds[eType][i];
		}
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNumDebrisExplodeSounds
//
//	PURPOSE:	Return the number of debris explode sounds associated with a 
//				particular id
//
// ----------------------------------------------------------------------- //

int GetNumDebrisExplodeSounds(DebrisType eType)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		if (s_pDebrisExplodeSounds[eType])
		{
			return (sizeof (s_pDebrisExplodeSounds[eType]) / sizeof (s_pDebrisExplodeSounds[eType][0]));
		}
	}

	return 0;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDebrisExplodeSound
//
//	PURPOSE:	Return the explode sound associated with a debris particular id
//
// ----------------------------------------------------------------------- //

char* GetDebrisExplodeSound(DebrisType eType, int nIndex)
{
	if (DBT_FIRST <= eType && eType < DBT_LAST)
	{
		int nNum = GetNumDebrisExplodeSounds(eType);
		int i = 0;

		if (0 <= nIndex && nIndex < nNum)
		{
			i = nIndex;
		}
		else
		{
			i = GetRandom(0, nNum-1);
		}

		if (s_pDebrisExplodeSounds[eType])
		{
			return s_pDebrisExplodeSounds[eType][i];
		}
	}

	return LTNULL;
}
