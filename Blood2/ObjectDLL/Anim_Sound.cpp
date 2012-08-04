// ----------------------------------------------------------------------- //
//
// MODULE  : Anim_Sound.cpp
//
// PURPOSE : Animation/Sound functions 
//
// CREATED : 
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "Anim_Sound.h"
#include "basecharacter.h"
#include "SoundTypes.h"

void BPrint(char*);

char *szNodes[NUM_ALL_NODES] = 
{
	"head",		//0
	"neck",		//1
	"torso",	//2
	"pelvis",	//3
	"ru_arm",	//4
	"rl_arm",	//5
	"r_hand",	//6
	"lu_arm",	//7
	"ll_arm",	//8
	"l_hand",	//9
	"lu_leg",	//10
	"ll_leg",	//11
	"l_ankle",	//12
	"l_foot",	//13
	"ru_leg",	//14
	"rl_leg",	//15
	"r_ankle",	//16
	"r_foot",	//17
	"obj",		//18
	"l_gun",	//19
	"r_gun",	//20
};

// ----------------------------------------------------------------------- //
// ROUTINE		: CheckAnims
// DESCRIPTION	: Check anims for validity
// RETURN TYPE	: void 
// PARAMS		: DDWORD* nAnim
// PARAMS		: int nSize
// ----------------------------------------------------------------------- //

void CheckAnims(DDWORD nAnim[], int nSize)
{
	for(int i = 1; i < nSize; i++)
	{
		if(nAnim[i] < 0 || nAnim[i] > 999)
			nAnim[i] = nAnim[i-1];
	}

	return;
}

CAnim_Sound::CAnim_Sound()
{
	memset(m_nAnim_IDLE,0,sizeof(m_nAnim_IDLE));
	memset(m_nAnim_TALK,0,sizeof(m_nAnim_TALK));
	memset(m_nAnim_WALK,0,sizeof(m_nAnim_WALK));

    m_nAnim_WALK_INJURED_RLEG_RIFLE  = 0;
    m_nAnim_WALK_INJURED_RLEG_PISTOL  = 0;

    m_nAnim_WALK_INJURED_LLEG_RIFLE  = 0;
    m_nAnim_WALK_INJURED_LLEG_PISTOL  = 0;

	memset(m_nAnim_RUN,0,sizeof(m_nAnim_RUN));
	memset(m_nAnim_JUMP,0,sizeof(m_nAnim_JUMP));
	memset(m_nAnim_CROUCH,0,sizeof(m_nAnim_JUMP));
	memset(m_nAnim_CRAWL,0,sizeof(m_nAnim_JUMP));
	memset(m_nAnim_SWIM,0,sizeof(m_nAnim_SWIM));
	memset(m_nAnim_STRAFERIGHT,0,sizeof(m_nAnim_STRAFERIGHT));
	memset(m_nAnim_STRAFELEFT,0,sizeof(m_nAnim_STRAFELEFT));

    m_nAnim_PICKUP_WEAPON  = 0;
   
	memset( m_nAnim_SWITCH_WEAPON,0,sizeof( m_nAnim_SWITCH_WEAPON));
	
   	memset(m_nAnim_FIRE_STAND,0,sizeof(m_nAnim_FIRE_STAND));
   	memset(m_nAnim_FIRE_WALK,0,sizeof(m_nAnim_FIRE_WALK));
   	memset(m_nAnim_FIRE_RUN,0,sizeof(m_nAnim_FIRE_RUN));
   	memset(m_nAnim_FIRE_JUMP,0,sizeof(m_nAnim_FIRE_JUMP));
   	memset(m_nAnim_FIRE_CROUCH,0,sizeof(m_nAnim_FIRE_CROUCH));
   	memset(m_nAnim_FIRE_CRAWL,0,sizeof(m_nAnim_FIRE_CRAWL));
 
    m_nAnim_FALLING  = 0;
    m_nAnim_FALLING_UNCONTROL  = 0;
    
    m_nAnim_ROLL_FORWARD  = 0;
    m_nAnim_ROLL_RIGHT  = 0;
    m_nAnim_ROLL_LEFT  = 0;
    m_nAnim_ROLL_BACK  = 0;
    
    m_nAnim_HANDSPRING_FORWARD  = 0;
    m_nAnim_HANDSPRING_RIGHT  = 0;
    m_nAnim_HANDSPRING_LEFT  = 0;
    m_nAnim_HANDSPRING_BACK  = 0;
    
    m_nAnim_FLIP_FORWARD  = 0;
    m_nAnim_FLIP_RIGHT  = 0;
    m_nAnim_FLIP_LEFT  = 0;
    m_nAnim_FLIP_BACK  = 0;
    
    m_nAnim_DODGE_RIGHT  = 0;
    m_nAnim_DODGE_LEFT  = 0;
    
	memset(m_nAnim_RECOIL,0,sizeof(m_nAnim_RECOIL));

	memset(m_nAnim_TAUNT,0,sizeof(m_nAnim_TAUNT));
        
    m_nAnim_SPOTPLAYER_RIGHT  = 0;
    m_nAnim_SPOTPLAYER_LEFT  = 0;
    m_nAnim_SPOTPLAYER_POINT  = 0;
 
	memset(m_nAnim_DEATH,0,sizeof(m_nAnim_DEATH));
	memset(m_nAnim_HUMILIATION,0,sizeof(m_nAnim_HUMILIATION));
	memset(m_nAnim_SPECIAL,0,sizeof(m_nAnim_SPECIAL));
	memset(m_nAnim_CORPSE,0,sizeof(m_nAnim_CORPSE));
	memset(m_nAnim_LIMB,0,sizeof(m_nAnim_LIMB));

    m_szSoundRoot[0] = '\0';

	m_hVoxSound = DNULL;

	m_bSilent = DFALSE;
}

CAnim_Sound::~CAnim_Sound()
{
	if( m_hVoxSound )
		g_pServerDE->KillSound( m_hVoxSound );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void CAnim_Sound::SetAnimationIndexes(HOBJECT m_hObject)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

    m_nAnim_IDLE[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE1                      );
    m_nAnim_IDLE[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE2                      );
    m_nAnim_IDLE[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE3                      );
    m_nAnim_IDLE[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_IDLE4                      );

    m_nAnim_TALK[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_TALK1                      );
    m_nAnim_TALK[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_TALK2                      );
    m_nAnim_TALK[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_TALK3                      );
    m_nAnim_TALK[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_TALK4                      );
    m_nAnim_TALK[4]						= pServerDE->GetAnimIndex(m_hObject, ANIM_TALK5                      );
	
    m_nAnim_WALK[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_RIFLE                 );
    m_nAnim_WALK[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_RIFLE                 );
    m_nAnim_WALK[2] 					= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_PISTOL                );
    m_nAnim_WALK[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_PISTOL                );
    m_nAnim_WALK[4]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_KNIFE                );
    m_nAnim_WALK[5] 					= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_KNIFE	             );
    m_nAnim_WALK[6]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_KNIFE                 );
    m_nAnim_WALK[7]						= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_PISTOL                 );
    m_nAnim_WALK[8] 					= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_PISTOL		         );
										
    m_nAnim_WALK_INJURED_RLEG_RIFLE		= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_INJURED_RLEG_RIFLE    );
    m_nAnim_WALK_INJURED_RLEG_PISTOL	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_INJURED_RLEG_PISTOL   );
										
    m_nAnim_WALK_INJURED_LLEG_RIFLE		= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_INJURED_LLEG_RIFLE    );
    m_nAnim_WALK_INJURED_LLEG_PISTOL	= pServerDE->GetAnimIndex(m_hObject, ANIM_WALK_INJURED_LLEG_PISTOL   );
										
    m_nAnim_RUN[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_RIFLE                  );
    m_nAnim_RUN[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_RIFLE                  );
    m_nAnim_RUN[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_PISTOL                 );
    m_nAnim_RUN[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_PISTOL                 );
    m_nAnim_RUN[4]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_KNIFE                 );
    m_nAnim_RUN[5]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_KNIFE				     );
    m_nAnim_RUN[6]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_KNIFE                  );
    m_nAnim_RUN[7]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_PISTOL                  );
    m_nAnim_RUN[8]						= pServerDE->GetAnimIndex(m_hObject, ANIM_RUN_PISTOL					 );
										
    m_nAnim_JUMP[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_RIFLE                 );
    m_nAnim_JUMP[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_RIFLE                 );
    m_nAnim_JUMP[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_PISTOL                );
    m_nAnim_JUMP[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_PISTOL                );
    m_nAnim_JUMP[4]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_KNIFE                 );
    m_nAnim_JUMP[5]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_KNIFE	             );
    m_nAnim_JUMP[6]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_KNIFE                 );
    m_nAnim_JUMP[7]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_PISTOL                 );
    m_nAnim_JUMP[8]						= pServerDE->GetAnimIndex(m_hObject, ANIM_JUMP_PISTOL	             );

    m_nAnim_CROUCH[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_RIFLE               );	
    m_nAnim_CROUCH[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_RIFLE               );
    m_nAnim_CROUCH[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_1PISTOL              );
    m_nAnim_CROUCH[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_1PISTOL              );	
    m_nAnim_CROUCH[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_KNIFE              );
    m_nAnim_CROUCH[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_KNIFE		         );
    m_nAnim_CROUCH[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_KNIFE              );	
    m_nAnim_CROUCH[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_1PISTOL              );
    m_nAnim_CROUCH[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CROUCH_1PISTOL			     );

    m_nAnim_CRAWL[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_RIFLE			     );	
    m_nAnim_CRAWL[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_RIFLE				 );
    m_nAnim_CRAWL[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_1PISTOL			     );
    m_nAnim_CRAWL[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_1PISTOL               );	
    m_nAnim_CRAWL[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_KNIFE               );
    m_nAnim_CRAWL[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_KNIFE	             );
    m_nAnim_CRAWL[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_KNIFE               );	
    m_nAnim_CRAWL[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_1PISTOL               );
    m_nAnim_CRAWL[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CRAWL_1PISTOL	             );
										
    m_nAnim_SWIM[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_RIFLE			     );
    m_nAnim_SWIM[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_RIFLE	             );
    m_nAnim_SWIM[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_PISTOL	             );
    m_nAnim_SWIM[3]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_PISTOL                );
    m_nAnim_SWIM[4]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_KNIFE                 );
    m_nAnim_SWIM[5]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_KNIFE	             );
    m_nAnim_SWIM[6]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_KNIFE                 );
    m_nAnim_SWIM[7]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_PISTOL                 );
    m_nAnim_SWIM[8]						= pServerDE->GetAnimIndex(m_hObject, ANIM_SWIM_PISTOL	             );

    m_nAnim_STRAFERIGHT[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_RIFLE         );
    m_nAnim_STRAFERIGHT[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_RIFLE         );
    m_nAnim_STRAFERIGHT[2] 				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_1PISTOL        );
    m_nAnim_STRAFERIGHT[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_2PISTOL         );
    m_nAnim_STRAFERIGHT[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_1PISTOL         );
										
    m_nAnim_STRAFELEFT[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_RIGHT_RIFLE         );
    m_nAnim_STRAFELEFT[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_LEFT_RIFLE         );
    m_nAnim_STRAFELEFT[2] 				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_LEFT_1PISTOL        );
    m_nAnim_STRAFELEFT[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_LEFT_2PISTOL         );
    m_nAnim_STRAFELEFT[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_STRAFE_LEFT_1PISTOL         );

    m_nAnim_PICKUP_WEAPON				= pServerDE->GetAnimIndex(m_hObject, ANIM_PICKUP_WEAPON              );

    m_nAnim_SWITCH_WEAPON[0]			= pServerDE->GetAnimIndex(m_hObject, ANIM_SWITCH_WEAPON_RIFLE        );	
    m_nAnim_SWITCH_WEAPON[1]			= pServerDE->GetAnimIndex(m_hObject, ANIM_SWITCH_WEAPON_RIFLE       );
    m_nAnim_SWITCH_WEAPON[2]			= pServerDE->GetAnimIndex(m_hObject, ANIM_SWITCH_WEAPON_PISTOL        );
										
    m_nAnim_FIRE_STAND[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_RIFLE           );
    m_nAnim_FIRE_STAND[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_AUTORIFLE       );
    m_nAnim_FIRE_STAND[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_1PISTOL         );
    m_nAnim_FIRE_STAND[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_2PISTOL         );
    m_nAnim_FIRE_STAND[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_KNIFE1          );
    m_nAnim_FIRE_STAND[5]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_KNIFE2          );
    m_nAnim_FIRE_STAND[6]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_KNIFE3          );
    m_nAnim_FIRE_STAND[7]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_GRENADE         );
    m_nAnim_FIRE_STAND[8]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_STAND_MAGIC           );
										
    m_nAnim_FIRE_WALK[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_RIFLE            );
    m_nAnim_FIRE_WALK[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_AUTORIFLE        );
    m_nAnim_FIRE_WALK[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_1PISTOL          );
    m_nAnim_FIRE_WALK[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_2PISTOL          );
    m_nAnim_FIRE_WALK[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_KNIFE1           );
    m_nAnim_FIRE_WALK[5]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_KNIFE2           );
    m_nAnim_FIRE_WALK[6]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_KNIFE3           );
    m_nAnim_FIRE_WALK[7]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_GRENADE          );
    m_nAnim_FIRE_WALK[8]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_WALK_MAGIC            );
										
    m_nAnim_FIRE_RUN[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_RIFLE             );
    m_nAnim_FIRE_RUN[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_AUTORIFLE         );
    m_nAnim_FIRE_RUN[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_1PISTOL           );
    m_nAnim_FIRE_RUN[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_2PISTOL           );
    m_nAnim_FIRE_RUN[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_KNIFE1            );
    m_nAnim_FIRE_RUN[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_KNIFE2            );
    m_nAnim_FIRE_RUN[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_KNIFE3            );
    m_nAnim_FIRE_RUN[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_GRENADE           );
    m_nAnim_FIRE_RUN[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_RUN_MAGIC             );
										
    m_nAnim_FIRE_JUMP[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_RIFLE            );
    m_nAnim_FIRE_JUMP[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_AUTORIFLE        );
    m_nAnim_FIRE_JUMP[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_1PISTOL          );
    m_nAnim_FIRE_JUMP[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_2PISTOL          );
    m_nAnim_FIRE_JUMP[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_KNIFE1           );
    m_nAnim_FIRE_JUMP[5]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_KNIFE2           );
    m_nAnim_FIRE_JUMP[6]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_KNIFE3           );
    m_nAnim_FIRE_JUMP[7]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_GRENADE          );
    m_nAnim_FIRE_JUMP[8]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_JUMP_MAGIC            );
										
    m_nAnim_FIRE_CROUCH[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_RIFLE          );
    m_nAnim_FIRE_CROUCH[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_AUTORIFLE      );
    m_nAnim_FIRE_CROUCH[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_1PISTOL        );
    m_nAnim_FIRE_CROUCH[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_2PISTOL        );
    m_nAnim_FIRE_CROUCH[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_KNIFE1         );
    m_nAnim_FIRE_CROUCH[5]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_KNIFE2         );
    m_nAnim_FIRE_CROUCH[6]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_KNIFE3         );
    m_nAnim_FIRE_CROUCH[7]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_GRENADE        );
    m_nAnim_FIRE_CROUCH[8]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CROUCH_MAGIC          );
										
    m_nAnim_FIRE_CRAWL[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_RIFLE           );
    m_nAnim_FIRE_CRAWL[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_AUTORIFLE       );
    m_nAnim_FIRE_CRAWL[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_1PISTOL         );
    m_nAnim_FIRE_CRAWL[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_2PISTOL         );
    m_nAnim_FIRE_CRAWL[4]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_KNIFE1          );
    m_nAnim_FIRE_CRAWL[5]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_KNIFE2          );
    m_nAnim_FIRE_CRAWL[6]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_KNIFE3          );
    m_nAnim_FIRE_CRAWL[7]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_GRENADE         );
    m_nAnim_FIRE_CRAWL[8]				= pServerDE->GetAnimIndex(m_hObject, ANIM_FIRE_CRAWL_MAGIC           );
										
    m_nAnim_FALLING						= pServerDE->GetAnimIndex(m_hObject, ANIM_FALLING                    );
    m_nAnim_FALLING_UNCONTROL			= pServerDE->GetAnimIndex(m_hObject, ANIM_FALLING_UNCONTROL          );
										
    m_nAnim_ROLL_FORWARD				= pServerDE->GetAnimIndex(m_hObject, ANIM_ROLL_FORWARD               );
    m_nAnim_ROLL_RIGHT					= pServerDE->GetAnimIndex(m_hObject, ANIM_ROLL_RIGHT                 );
    m_nAnim_ROLL_LEFT					= pServerDE->GetAnimIndex(m_hObject, ANIM_ROLL_LEFT                  );
    m_nAnim_ROLL_BACK					= pServerDE->GetAnimIndex(m_hObject, ANIM_ROLL_BACK                  );
										
    m_nAnim_HANDSPRING_FORWARD			= pServerDE->GetAnimIndex(m_hObject, ANIM_HANDSPRING_FORWARD         );
    m_nAnim_HANDSPRING_RIGHT			= pServerDE->GetAnimIndex(m_hObject, ANIM_HANDSPRING_RIGHT           );
    m_nAnim_HANDSPRING_LEFT				= pServerDE->GetAnimIndex(m_hObject, ANIM_HANDSPRING_LEFT            );
    m_nAnim_HANDSPRING_BACK				= pServerDE->GetAnimIndex(m_hObject, ANIM_HANDSPRING_BACK            );
										
    m_nAnim_FLIP_FORWARD				= pServerDE->GetAnimIndex(m_hObject, ANIM_FLIP_FORWARD               );
    m_nAnim_FLIP_RIGHT					= pServerDE->GetAnimIndex(m_hObject, ANIM_FLIP_RIGHT                 );
    m_nAnim_FLIP_LEFT					= pServerDE->GetAnimIndex(m_hObject, ANIM_FLIP_LEFT                  );
    m_nAnim_FLIP_BACK					= pServerDE->GetAnimIndex(m_hObject, ANIM_FLIP_BACK                  );
										
    m_nAnim_DODGE_RIGHT					= pServerDE->GetAnimIndex(m_hObject, ANIM_DODGE_RIGHT                );
    m_nAnim_DODGE_LEFT					= pServerDE->GetAnimIndex(m_hObject, ANIM_DODGE_LEFT                 );
										
    m_nAnim_RECOIL[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_HEAD1               );
    m_nAnim_RECOIL[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_CHEST               );
    m_nAnim_RECOIL[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_RCHEST              );
    m_nAnim_RECOIL[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_LCHEST              );
    m_nAnim_RECOIL[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_LLEG1               );
    m_nAnim_RECOIL[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_RLEG1               );
    m_nAnim_RECOIL[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_HEAD2               );
    m_nAnim_RECOIL[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_CHEST2              );
    m_nAnim_RECOIL[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_RCHEST2              );
    m_nAnim_RECOIL[9]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_LCHEST2              );
    m_nAnim_RECOIL[10]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_LLEG2               );
    m_nAnim_RECOIL[11]					= pServerDE->GetAnimIndex(m_hObject, ANIM_RECOIL_RLEG2               );
										
    m_nAnim_TAUNT[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_DANCE1               );
    m_nAnim_TAUNT[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_DANCE2               );
    m_nAnim_TAUNT[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_DANCE3               );
    m_nAnim_TAUNT[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_DANCE4               );
    m_nAnim_TAUNT[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_FLIP                 );
    m_nAnim_TAUNT[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_WAVE                 );
    m_nAnim_TAUNT[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_TAUNT_BEG                  );
										
    m_nAnim_SPOTPLAYER_RIGHT			= pServerDE->GetAnimIndex(m_hObject, ANIM_SPOTPLAYER_RIGHT           );
    m_nAnim_SPOTPLAYER_LEFT				= pServerDE->GetAnimIndex(m_hObject, ANIM_SPOTPLAYER_LEFT            );
    m_nAnim_SPOTPLAYER_POINT			= pServerDE->GetAnimIndex(m_hObject, ANIM_SPOTPLAYER_POINT           );


	// Set the humiliation animations...
    
    m_nAnim_HUMILIATION[0]				= pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_DAZED          );
    m_nAnim_HUMILIATION[1]				= pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_DIZZY          );
    m_nAnim_HUMILIATION[2]				= pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_MERCY          );
    m_nAnim_HUMILIATION[3]				= pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_DEAD           );

	if (m_nAnim_HUMILIATION[3] == -1)
	{
	    m_nAnim_HUMILIATION[3] = pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_DEAD2           );
	}


	// Gabby and Ishmael's humiliation animations are spelled wrong, so deal with that here...

	if (m_nAnim_HUMILIATION[0] == -1) m_nAnim_HUMILIATION[0] = pServerDE->GetAnimIndex(m_hObject, "humilation_01");
	if (m_nAnim_HUMILIATION[1] == -1) m_nAnim_HUMILIATION[1] = pServerDE->GetAnimIndex(m_hObject, "humilation_02");
	if (m_nAnim_HUMILIATION[2] == -1) m_nAnim_HUMILIATION[2] = pServerDE->GetAnimIndex(m_hObject, "humilation_03");
	if (m_nAnim_HUMILIATION[3] == -1) m_nAnim_HUMILIATION[3] = pServerDE->GetAnimIndex(m_hObject, "humilation_05");


	// Playable enemy characters only have 01, 03, and 05--so fill in index 1 if necessary...

	if (m_nAnim_HUMILIATION[1] == -1) m_nAnim_HUMILIATION[1] = pServerDE->GetAnimIndex(m_hObject, ANIM_HUMILIATION_DAZED);


	// Continue...

    m_nAnim_DEATH[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH1                     );
    m_nAnim_DEATH[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH2                     );
	m_nAnim_DEATH[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH3                     );
    m_nAnim_DEATH[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH4                     );
    m_nAnim_DEATH[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH5                     );
    m_nAnim_DEATH[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH6                     );
    m_nAnim_DEATH[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH7                     );
    m_nAnim_DEATH[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH8                     );
	m_nAnim_DEATH[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH9                     );
    m_nAnim_DEATH[9]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH10                     );
    m_nAnim_DEATH[10]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH11                     );
    m_nAnim_DEATH[11]					= pServerDE->GetAnimIndex(m_hObject, ANIM_DEATH12                     );
		
    m_nAnim_CORPSE[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE1                    );
    m_nAnim_CORPSE[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE2                    );
    m_nAnim_CORPSE[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE3                    );
    m_nAnim_CORPSE[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE4                    );
    m_nAnim_CORPSE[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE5                    );
    m_nAnim_CORPSE[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE6                    );
    m_nAnim_CORPSE[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE7                    );
    m_nAnim_CORPSE[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE8                    );
    m_nAnim_CORPSE[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE9                    );
    m_nAnim_CORPSE[9]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE10                    );
    m_nAnim_CORPSE[10]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE11                    );
    m_nAnim_CORPSE[11]					= pServerDE->GetAnimIndex(m_hObject, ANIM_CORPSE12                    );

    m_nAnim_LIMB[0]						= pServerDE->GetAnimIndex(m_hObject, ANIM_LIMB_HEAD                  );
    m_nAnim_LIMB[1]						= pServerDE->GetAnimIndex(m_hObject, ANIM_LIMB_ARM                   );
    m_nAnim_LIMB[2]						= pServerDE->GetAnimIndex(m_hObject, ANIM_LIMB_LEG                   );
	
    m_nAnim_SPECIAL[0]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL1                   );
    m_nAnim_SPECIAL[1]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL2                   );
    m_nAnim_SPECIAL[2]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL3                   );
    m_nAnim_SPECIAL[3]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL4                   );
    m_nAnim_SPECIAL[4]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL5                   );
    m_nAnim_SPECIAL[5]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL6                   );
    m_nAnim_SPECIAL[6]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL7                   );
    m_nAnim_SPECIAL[7]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL8                   );  
    m_nAnim_SPECIAL[8]					= pServerDE->GetAnimIndex(m_hObject, ANIM_SPECIAL9                  );  

	//SCHLEGZ 3/11/98 7:33:27 PM: make sure we have valid and appropriate anims
	CheckAnims(m_nAnim_IDLE,sizeof(m_nAnim_IDLE)/4);
	CheckAnims(m_nAnim_WALK,sizeof(m_nAnim_WALK)/4);
	CheckAnims(m_nAnim_RUN,sizeof(m_nAnim_RUN)/4);
	CheckAnims(m_nAnim_JUMP,sizeof(m_nAnim_JUMP)/4);
	CheckAnims(m_nAnim_CROUCH,sizeof(m_nAnim_CROUCH)/4);
	CheckAnims(m_nAnim_CRAWL,sizeof(m_nAnim_CRAWL)/4);
	CheckAnims(m_nAnim_SWIM,sizeof(m_nAnim_SWIM)/4);
	CheckAnims(m_nAnim_FIRE_STAND,sizeof(m_nAnim_FIRE_STAND)/4);
	CheckAnims(m_nAnim_FIRE_WALK,sizeof(m_nAnim_FIRE_WALK)/4);
	CheckAnims(m_nAnim_FIRE_RUN,sizeof(m_nAnim_FIRE_RUN)/4);
	CheckAnims(m_nAnim_FIRE_JUMP,sizeof(m_nAnim_FIRE_JUMP)/4);
	CheckAnims(m_nAnim_FIRE_CROUCH,sizeof(m_nAnim_FIRE_CROUCH)/4);
	CheckAnims(m_nAnim_FIRE_CRAWL,sizeof(m_nAnim_FIRE_CRAWL)/4);
	CheckAnims(m_nAnim_RECOIL,sizeof(m_nAnim_RECOIL)/4);
	CheckAnims(m_nAnim_DEATH,sizeof(m_nAnim_DEATH)/4);
	CheckAnims(m_nAnim_CORPSE,sizeof(m_nAnim_CORPSE)/4);

}

void CAnim_Sound::GenerateHitSpheres(HOBJECT hObject)
{
    CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !hObject) return;

	DVector vNodePos[NUM_STD_NODES];
	DRotation rRot;

	for(int i = 0; i < NUM_STD_NODES; i++)
	{
		if(!pServerDE->GetModelNodeTransform(hObject, szNodes[i], &vNodePos[i], &rRot))
		{
			if(i >= 1)
				VEC_COPY(vNodePos[i], vNodePos[i-1]);
		}
	}

	//HEAD/NECK
	m_fHitSpheres[0] = VEC_DIST(vNodePos[0],vNodePos[1]) * 0.66f;
	m_fHitSpheres[1] = m_fHitSpheres[0];

	//TORSO/PELVIS
	m_fHitSpheres[2] = VEC_DIST(vNodePos[2],vNodePos[3]) * 0.66f;
	m_fHitSpheres[3] = m_fHitSpheres[2];

	//R_ARM
	m_fHitSpheres[4] = VEC_DIST(vNodePos[4],vNodePos[5]) * 0.66f;
	m_fHitSpheres[5] = VEC_DIST(vNodePos[5],vNodePos[6]) * 0.66f;
	m_fHitSpheres[6] = m_fHitSpheres[5];

	//L_ARM
	m_fHitSpheres[7] = VEC_DIST(vNodePos[7],vNodePos[8]) * 0.66f;
	m_fHitSpheres[8] = VEC_DIST(vNodePos[8],vNodePos[9]) * 0.66f;
	m_fHitSpheres[9] = m_fHitSpheres[8];

	//L_LEG
	m_fHitSpheres[10] = VEC_DIST(vNodePos[10],vNodePos[11]) * 0.66f;
	m_fHitSpheres[11] = VEC_DIST(vNodePos[11],vNodePos[12]) * 0.66f;
	m_fHitSpheres[12] = VEC_DIST(vNodePos[12],vNodePos[13]) * 0.66f;
	m_fHitSpheres[13] = m_fHitSpheres[12];

	//R_LEG
	m_fHitSpheres[14] = VEC_DIST(vNodePos[14],vNodePos[15]) * 0.66f;
	m_fHitSpheres[15] = VEC_DIST(vNodePos[15],vNodePos[16]) * 0.66f;
	m_fHitSpheres[16] = VEC_DIST(vNodePos[16],vNodePos[17]) * 0.66f;
	m_fHitSpheres[17] = m_fHitSpheres[16];

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CAnim_Sound::ForceAnimation
// DESCRIPTION	: 
// RETURN TYPE	: void 
// PARAMS		: HOBJECT m_hObject
// PARAMS		: DDWORD nAni
// ----------------------------------------------------------------------- //

void CAnim_Sound::ForceAnimation(HOBJECT m_hObject, DDWORD nAni)
{
    CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !m_hObject) return;

    // Force to different animation
    pServerDE->SetModelAnimation(m_hObject, 0);

    pServerDE->SetModelAnimation(m_hObject, nAni);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CAnim_Sound::GetSoundPath
// DESCRIPTION	: 
// RETURN TYPE	: char* 
// PARAMS		: void
// ----------------------------------------------------------------------- //

void CAnim_Sound::GetSoundPath(char* szSound, int nIndex)
{
	char szTemp[256];

	if(nIndex)
	{
		sprintf(szTemp,"%s\\%s%d.wav",m_szSoundRoot,szSound,nIndex);
	}
	else
	{
		sprintf(szTemp,"%s\\%s",m_szSoundRoot,szSound);
	}

	_mbscpy((unsigned char*)szSound, (const unsigned char*)szTemp);

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CAnim_Sound::PlayAISound
// DESCRIPTION	: 
// RETURN TYPE	: DBOOL 
// PARAMS		: char* szSound
// PARAMS		: DFLOAT fRadius
// ----------------------------------------------------------------------- //

DBOOL CAnim_Sound::PlayVoice(HOBJECT hObject, char* szSound, DFLOAT fRadius, int nVol)
{
    CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !hObject) return DFALSE;

	if(m_hVoxSound)
	{
		DBOOL bDone = DFALSE;
		if(pServerDE->IsSoundDone(m_hVoxSound, &bDone) == LT_OK)
		{
			if(!bDone)
				return DFALSE;
		}

		pServerDE->KillSound( m_hVoxSound );
		m_hVoxSound = DNULL;
	}

	m_hVoxSound = PlaySound(hObject, szSound, fRadius, nVol, DTRUE );

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CAnim_Sound::PlayAISound
// DESCRIPTION	: 
// RETURN TYPE	: HSOUNDDE, if bGetHandle == DTRUE
// PARAMS		: char* szSound
// PARAMS		: DFLOAT fRadius
// PARAMS		: DBOOL bGetHandle - return handle.
// ----------------------------------------------------------------------- //

HSOUNDDE CAnim_Sound::PlaySound(HOBJECT hObject, char* szSound, DFLOAT fRadius, int nVol, DBOOL bGetHandle )
{
    CServerDE* pServerDE = BaseClass::GetServerDE();
    if (!pServerDE || !hObject) return DNULL;

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_ATTACHED | PLAYSOUND_CTRL_VOL | PLAYSOUND_REVERB;
	if( bGetHandle )
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;

//	playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
//	playSoundInfo.m_dwFlags |=  PLAYSOUND_GETHANDLE;
//	playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
//	playSoundInfo.m_dwFlags |=  PLAYSOUND_TIME;

	sprintf(playSoundInfo.m_szSoundName,"%s\\%s",m_szSoundRoot,szSound);
	playSoundInfo.m_hObject = hObject;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_AI_MEDIUM;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.1f;
	playSoundInfo.m_nVolume = nVol;
	
	pServerDE->PlaySound( &playSoundInfo );

	return playSoundInfo.m_hSound;
}