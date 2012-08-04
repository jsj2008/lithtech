// ----------------------------------------------------------------------- //
//
// MODULE  : Anim_Sound.h
//
// PURPOSE : Player and AI animations and sounds.  Anim_Sound.cpp
//
// CREATED : 
//
// ----------------------------------------------------------------------- //

#ifndef __ANIMSOUND_H__
#define __ANIMSOUND_H__

#include <mbstring.h>

//SCHLEGZ 3/6/98 3:31:56 PM: nice little array of node names
#define NUM_ALL_NODES	21
#define NUM_STD_NODES	18

#define NODE_NECK		1
#define NODE_TORSO		2
#define NODE_RARM		4
#define NODE_LARM		7
#define NODE_LLEG		10
#define NODE_RLEG		14

extern char *szNodes[NUM_ALL_NODES];

//
// Animation String names for the 3d models
//

#define ANIM_IDLE1                      "idle1"                 // Idle animations
#define ANIM_IDLE2                      "idle2"                 
#define ANIM_IDLE3                      "idle3"                 
#define ANIM_IDLE4                      "idle4"                 

#define ANIM_TALK1                      "talk1"                 // Idle animations
#define ANIM_TALK2                      "talk2"                 
#define ANIM_TALK3                      "talk3"                 
#define ANIM_TALK4                      "talk4"                 
#define ANIM_TALK5                      "talk5"                 
                                                                
#define ANIM_WALK_NOGUN                 "walk_nogun"            // Walking, no guns
#define ANIM_WALK_RIFLE                 "walk_rifle"            // Walking, one rifle sized gun
#define ANIM_WALK_PISTOL                "walk_pistol"           // Walking, one or two pistols
#define ANIM_WALK_KNIFE					"walk_knife1"
                                                                
#define ANIM_WALK_INJURED_RLEG_RIFLE    "walk_irleg_rifle"      // Walking injured right leg, one rifle sized gun
#define ANIM_WALK_INJURED_RLEG_PISTOL   "walk_irleg_nogun"     // Walking injured right leg, one or two pistols

#define ANIM_WALK_INJURED_LLEG_RIFLE    "walk_illeg_rifle"      // Walking injured left leg, one rifle sized gun
#define ANIM_WALK_INJURED_LLEG_PISTOL   "walk_illeg_nogun"     // Walking injured left leg, one or two pistols
                                                                
#define ANIM_RUN_NOGUN					"run_nogun"
#define ANIM_RUN_RIFLE                  "run_rifle"             // Running, one rifle sized gun
#define ANIM_RUN_PISTOL                 "run_pistol"            // Running, one or two pistols
#define ANIM_RUN_KNIFE					"run_knife1"
                                                                
#define ANIM_JUMP_RIFLE                 "jmp_rifle"             // Jump, rifle sized gun
#define ANIM_JUMP_PISTOL                "jmp_pistol"            // Jump, one or two pistols
#define ANIM_JUMP_KNIFE					"jmp_knife1"

#define ANIM_CROUCH_1PISTOL             "crouch_1pistol"        // no weapon or pistol weapon(s),   no movement
#define ANIM_CROUCH_RIFLE               "crouch_rifle"          // rifle-sized weapon,              no movement
#define ANIM_CROUCH_KNIFE				"crouch_knife1"
                                                                
#define ANIM_CRAWL_1PISTOL              "crawl_1pistol"          // no weapon or pistol weapon(s),   moving
#define ANIM_CRAWL_RIFLE                "crawl_rifle"           // rifle-sized weapon,              moving
#define ANIM_CRAWL_KNIFE				"crawl_knife1"
                                                                
#define ANIM_SWIM_NOGUN                 "swim_nogun"            // Swimming, no guns
#define ANIM_SWIM_RIFLE                 "swim_rifle"            // Swimming, one rifle sized gun
#define ANIM_SWIM_PISTOL                "swim_pistol"           // Swimming, one or two pistols
#define ANIM_SWIM_KNIFE					"swim_knife1"

// Strafe animations

#define ANIM_STRAFE_RIGHT_NOGUN	        "strafe_right_nogun"	// Strafing
#define ANIM_STRAFE_RIGHT_1PISTOL       "strafe_right_1pistol"
#define ANIM_STRAFE_RIGHT_2PISTOL       "strafe_right_2pistol"
#define ANIM_STRAFE_RIGHT_RIFLE         "strafe_right_rifle"

#define ANIM_STRAFE_LEFT_NOGUN	        "strafe_right_nogun"	// Strafing
#define ANIM_STRAFE_LEFT_1PISTOL        "strafe_right_1pistol"
#define ANIM_STRAFE_LEFT_2PISTOL        "strafe_right_2pistol"
#define ANIM_STRAFE_LEFT_RIFLE          "strafe_right_rifle"


#define ANIM_PICKUP_WEAPON              "pickup_weapon"         // Pickup a weapon

#define ANIM_SWITCH_WEAPON_PISTOL       "switch_weapon_2pistol"  // Switch Weapons
#define ANIM_SWITCH_WEAPON_RIFLE        "switch_weapon_rifle"
#define ANIM_SWITCH_WEAPON_KNIFE        "switch_weapon_knife"
#define ANIM_SWITCH_WEAPON_NONE         "switch_weapon_none"

#define ANIM_FIRE_STAND_RIFLE           "fire_stand_rifle"      // Firing, rifle sized weapon,          no movement
#define ANIM_FIRE_STAND_AUTORIFLE       "fire_stand_autorifle"  // Firing, AUTO rifle sized weapon,          no movement
#define ANIM_FIRE_STAND_1PISTOL         "fire_stand_1pistol"    // Firing, single pistol sized weapon,  no movement
#define ANIM_FIRE_STAND_2PISTOL         "fire_stand_2pistol"    // Firing, dual pistol sized weapon,    no movement
#define ANIM_FIRE_STAND_KNIFE1          "fire_stand_knife1"     // Firing, knife strike 1,              no movement
#define ANIM_FIRE_STAND_KNIFE2          "fire_stand_knife2"     // Firing, knife strike 2,              no movement
#define ANIM_FIRE_STAND_KNIFE3          "fire_stand_knife3"     // Firing, knife strike 3,              no movement
#define ANIM_FIRE_STAND_GRENADE         "fire_stand_grenade"    // Firing, Grenade throw                no movement
#define ANIM_FIRE_STAND_MAGIC           "fire_stand_magic"      // Firing, Magic weapon                 no movement

#define ANIM_FIRE_WALK_RIFLE            "fire_walk_rifle"       // Firing, rifle sized weapon,          walking
#define ANIM_FIRE_WALK_AUTORIFLE        "fire_walk_autorifle"   // Firing, AUTO rifle sized weapon,          walking
#define ANIM_FIRE_WALK_1PISTOL          "fire_walk_1pistol"     // Firing, single pistol sized weapon,  walking
#define ANIM_FIRE_WALK_2PISTOL          "fire_walk_2pistol"     // Firing, dual pistol sized weapon,    walking
#define ANIM_FIRE_WALK_KNIFE1           "fire_walk_knife1"      // Firing, knife strike 1,              walking
#define ANIM_FIRE_WALK_KNIFE2           "fire_walk_knife2"      // Firing, knife strike 2,              walking
#define ANIM_FIRE_WALK_KNIFE3           "fire_walk_knife3"      // Firing, knife strike 3,              walking
#define ANIM_FIRE_WALK_GRENADE          "fire_walk_grenade"     // Firing, Grenade throw                walking
#define ANIM_FIRE_WALK_MAGIC            "fire_walk_magic"       // Firing, Magic weapon                 walking

#define ANIM_FIRE_RUN_RIFLE             "fire_run_rifle"        // Firing, rifle sized weapon,          running
#define ANIM_FIRE_RUN_AUTORIFLE         "fire_run_autorifle"    // Firing, AUTO rifle sized weapon,          running
#define ANIM_FIRE_RUN_1PISTOL           "fire_run_1pistol"      // Firing, single pistol sized weapon,  running
#define ANIM_FIRE_RUN_2PISTOL           "fire_run_2pistol"      // Firing, dual pistol sized weapon,    running
#define ANIM_FIRE_RUN_KNIFE1            "fire_run_knife1"       // Firing, knife strike 1,              running
#define ANIM_FIRE_RUN_KNIFE2            "fire_run_knife2"       // Firing, knife strike 2,              running
#define ANIM_FIRE_RUN_KNIFE3            "fire_run_knife3"       // Firing, knife strike 3,              running
#define ANIM_FIRE_RUN_GRENADE           "fire_run_grenade"      // Firing, Grenade throw                running
#define ANIM_FIRE_RUN_MAGIC             "fire_run_magic"        // Firing, Magic weapon                 running

#define ANIM_FIRE_JUMP_RIFLE            "fire_jump_rifle"       // Firing, rifle sized weapon,          Jumping
#define ANIM_FIRE_JUMP_AUTORIFLE        "fire_jump_autorifle"   // Firing, AUTO rifle sized weapon,          Jumping
#define ANIM_FIRE_JUMP_1PISTOL          "fire_jump_1pistol"     // Firing, single pistol sized weapon,  Jumping
#define ANIM_FIRE_JUMP_2PISTOL          "fire_jump_2pistol"     // Firing, dual pistol sized weapon,    Jumping
#define ANIM_FIRE_JUMP_KNIFE1           "fire_jump_knife1"      // Firing, knife strike 1,              Jumping
#define ANIM_FIRE_JUMP_KNIFE2           "fire_jump_knife2"      // Firing, knife strike 2,              Jumping
#define ANIM_FIRE_JUMP_KNIFE3           "fire_jump_knife3"      // Firing, knife strike 3,              Jumping
#define ANIM_FIRE_JUMP_GRENADE          "fire_jump_grenade"     // Firing, Grenade throw                Jumping
#define ANIM_FIRE_JUMP_MAGIC            "fire_jump_magic"       // Firing, Magic weapon                 Jumping

#define ANIM_FIRE_CROUCH_RIFLE          "fire_crouch_rifle"     // Firing, rifle sized weapon,          Crouching, no movement
#define ANIM_FIRE_CROUCH_AUTORIFLE      "fire_crouch_autorifle" // Firing, AUTO rifle sized weapon,          Crouching, no movement
#define ANIM_FIRE_CROUCH_1PISTOL        "fire_crouch_1pistol"   // Firing, single pistol sized weapon,  Crouching, no movement
#define ANIM_FIRE_CROUCH_2PISTOL        "fire_crouch_2pistol"   // Firing, dual pistol sized weapon,    Crouching, no movement
#define ANIM_FIRE_CROUCH_KNIFE1         "fire_crouch_knife1"    // Firing, knife strike 1,              Crouching, no movement
#define ANIM_FIRE_CROUCH_KNIFE2         "fire_crouch_knife2"    // Firing, knife strike 2,              Crouching, no movement
#define ANIM_FIRE_CROUCH_KNIFE3         "fire_crouch_knife3"    // Firing, knife strike 3,              Crouching, no movement
#define ANIM_FIRE_CROUCH_GRENADE		"fire_crouch_grenade"	
#define ANIM_FIRE_CROUCH_MAGIC          "fire_crouch_magic"     // Firing, Magic weapon                 Crouching, no movement

#define ANIM_FIRE_CRAWL_RIFLE           "fire_crawl_rifle"      // Firing, rifle sized weapon,          Crawling, moving
#define ANIM_FIRE_CRAWL_AUTORIFLE       "fire_crawl_autorifle"  // Firing, AUTO rifle sized weapon,          Crawling, moving
#define ANIM_FIRE_CRAWL_1PISTOL         "fire_crawl_1pistol"    // Firing, single pistol sized weapon,  Crawling, moving
#define ANIM_FIRE_CRAWL_2PISTOL         "fire_crawl_2pistol"    // Firing, dual pistol sized weapon,    Crawling, moving
#define ANIM_FIRE_CRAWL_KNIFE1          "fire_crawl_knife1"     // Firing, knife strike 1,              Crawling, moving
#define ANIM_FIRE_CRAWL_KNIFE2          "fire_crawl_knife2"     // Firing, knife strike 2,              Crawling, moving
#define ANIM_FIRE_CRAWL_KNIFE3          "fire_crawl_knife3"     // Firing, knife strike 3,              Crawling, moving
#define ANIM_FIRE_CRAWL_GRENADE         "fire_crawl_grenade"    // Firing, Grenade throw                Crawling, moving
#define ANIM_FIRE_CRAWL_MAGIC           "fire_crawl_magic"      // Firing, Magic weapon                 Crawling, moving


#define ANIM_FALLING                    "falling"               // Falling
#define ANIM_FALLING_UNCONTROL          "falling_uncontrol"     // Falling uncontrolled

#define ANIM_ROLL_FORWARD               "roll_forward"          // Roll forward
#define ANIM_ROLL_RIGHT                 "roll_right"            // Roll right side
#define ANIM_ROLL_LEFT                  "roll_left"             // Roll left side
#define ANIM_ROLL_BACK                  "roll_back"             // Back roll

#define ANIM_HANDSPRING_FORWARD         "handspring_forward"    // Hand spring forward
#define ANIM_HANDSPRING_RIGHT           "handspring_right"      // Hand spring right
#define ANIM_HANDSPRING_LEFT            "handspring_left"       // Hand spring left
#define ANIM_HANDSPRING_BACK            "handspring_back"       // Hand spring back

#define ANIM_FLIP_FORWARD               "flip_forward"          // Flip forward
#define ANIM_FLIP_RIGHT                 "flip_right"            // Flip right
#define ANIM_FLIP_LEFT                  "flip_left"             // Flip left
#define ANIM_FLIP_BACK                  "flip_back"             // Flip back

#define ANIM_DODGE_RIGHT                "dodge_right"            // Dodge right
#define ANIM_DODGE_LEFT                 "dodge_left"             // Dodge left


#define ANIM_RECOIL_HEAD1               "recoil_head1"          // Recoil Head
#define ANIM_RECOIL_CHEST               "recoil_chest1"          // Recoil chest
#define ANIM_RECOIL_RCHEST              "recoil_rchest1"         // Recoil right chest
#define ANIM_RECOIL_LCHEST              "recoil_lchest1"         // Recoil left chest
#define ANIM_RECOIL_LLEG1               "recoil_lleg1"          // Recoil left leg 1
#define ANIM_RECOIL_RLEG1               "recoil_rleg1"          // Recoil right leg 1
#define ANIM_RECOIL_HEAD2               "recoil_head2"          // Recoil Head
#define ANIM_RECOIL_CHEST2              "recoil_chest2"          // Recoil chest
#define ANIM_RECOIL_RCHEST2             "recoil_rchest2"         // Recoil right chest
#define ANIM_RECOIL_LCHEST2             "recoil_lchest2"         // Recoil left chest
#define ANIM_RECOIL_LLEG2               "recoil_lleg2"          // Recoil left leg 1
#define ANIM_RECOIL_RLEG2               "recoil_rleg2"          // Recoil right leg 1


#define ANIM_TAUNT_DANCE1               "taunt_dance1"          // Taunt Dance 1
#define ANIM_TAUNT_DANCE2               "taunt_dance2"          // Taunt Dance 2
#define ANIM_TAUNT_DANCE3               "taunt_dance3"          // Taunt Dance 3
#define ANIM_TAUNT_DANCE4               "taunt_dance4"          // Taunt Dance 4
#define ANIM_TAUNT_FLIP                 "taunt_flip"            // Taunt flip off
#define ANIM_TAUNT_WAVE                 "taunt_wave"            // Taunt Wave
#define ANIM_TAUNT_BEG                  "taunt_beg"             // Taunt begging for mercy

#define ANIM_SPOTPLAYER_RIGHT           "spot_right"            // Spot player to the right
#define ANIM_SPOTPLAYER_LEFT            "spot_left"             // Spot player to the left
#define ANIM_SPOTPLAYER_POINT           "spot_point"            // Spot player and point

#define ANIM_DEATH1                     "death_head1"           // Death, head
#define ANIM_DEATH2                     "death_chest1"           // Death, torso
#define ANIM_DEATH3                     "death_rchest1"          // Death, r_arm
#define ANIM_DEATH4                     "death_lchest1"          // Death, l_arm
#define ANIM_DEATH5                     "death_lleg1"           // Death, l_leg
#define ANIM_DEATH6                     "death_rleg1"           // Death, r_leg
#define ANIM_DEATH7                     "death_head2"           // Death, head2
#define ANIM_DEATH8                     "death_chest2"          // Death, torso2
#define ANIM_DEATH9                     "death_rchest2"         // Death, r_arm2
#define ANIM_DEATH10                    "death_lchest2"         // Death, l_arm2
#define ANIM_DEATH11                    "death_lleg2"           // Death, l_leg2
#define ANIM_DEATH12                    "death_rleg2"           // Death, r_leg2

#define ANIM_HUMILIATION_DAZED			"humiliation_01"		// Humiliation: on knees and dazed
#define ANIM_HUMILIATION_DIZZY			"humiliation_02"		// Humiliation: on knees and dizzy
#define ANIM_HUMILIATION_MERCY			"humiliation_03"		// Humiliation: on knees begging for mercy
#define ANIM_HUMILIATION_DEAD			"humiliation_04"		// Humiliation: on knees and dead
#define ANIM_HUMILIATION_DEAD2			"humiliation_05"		// Humiliation: on knees and dead

// Use these for anything out of the norm

#define ANIM_SPECIAL1                   "special1"              // Special Animation 1
#define ANIM_SPECIAL2                   "special2"              // Special Animation 2
#define ANIM_SPECIAL3                   "special3"              // Special Animation 3
#define ANIM_SPECIAL4                   "special4"              // Special Animation 4
#define ANIM_SPECIAL5                   "special5"              // Special Animation 5
#define ANIM_SPECIAL6                   "special6"              // Special Animation 6
#define ANIM_SPECIAL7                   "special7"              // Special Animation 7
#define ANIM_SPECIAL8                   "special8"              // Special Animation 8
#define ANIM_SPECIAL9                   "special9"              // Special Animation 9

#define ANIM_CORPSE1                    "corpse_head1"               // Death, head
#define ANIM_CORPSE2                    "corpse_chest"               // Death, torso
#define ANIM_CORPSE3                    "corpse_rchest"               // Death, r_arm
#define ANIM_CORPSE4                    "corpse_lchest"               // Death, l_arm
#define ANIM_CORPSE5                    "corpse_lleg1"               // Death, l_leg
#define ANIM_CORPSE6                    "corpse_rleg1"               // Death, r_leg
#define ANIM_CORPSE7                    "corpse_head2"               // Death, head2
#define ANIM_CORPSE8                    "corpse_chest2"               // Death, torso2
#define ANIM_CORPSE9                    "corpse_rchest2"               // Death, r_arm2
#define ANIM_CORPSE10                   "corpse_lchest2"               // Death, l_arm2
#define ANIM_CORPSE11                   "corpse_lleg2"               // Death, l_leg2
#define ANIM_CORPSE12                   "corpse_rleg2"               // Death, r_leg2

#define ANIM_LIMB_HEAD                  "limb_head"              // 
#define ANIM_LIMB_ARM                   "limb_arm"              // 
#define ANIM_LIMB_LEG                   "limb_leg"              // 

//SOUNDS

#define SOUND_SPOT						"spot"
#define SOUND_IDLE						"idle"
#define SOUND_BATTLE_CRY				"battle_cry"
#define SOUND_LAUGH						"laugh"
#define SOUND_SCREAM					"scream"
#define SOUND_FEAR						"fear"
#define SOUND_ANGER						"anger"
#define SOUND_PAIN						"pain"
#define SOUND_FIRE						"fire"
#define SOUND_DEATH						"death"
#define SOUND_ATTACK					"attack"
#define SOUND_INSANE					"insane"

#define SOUND_FOOTSTEP					"footstep.wav"
#define SOUND_SPIT						"spit.wav"
#define SOUND_HOWL						"howl.wav"
#define SOUND_RUN						"run.wav"
#define SOUND_EAT						"eat.wav"
#define SOUND_BODY_CRUMPLE				"body_crumple.wav"




class CAnim_Sound 
{
	public :

	    CAnim_Sound();
    	~CAnim_Sound();

// 02/10/98
// Replace this function with 
// SetAnimationIndexes(m_hObject);
// This will set the animation based on the string name.

		void			SetAnimationIndexes(HOBJECT m_hObject);
        void			ForceAnimation(HOBJECT m_hObject, DDWORD nAni);
        
		void			GenerateHitSpheres(HOBJECT hObject);

		void			SetSoundRoot(char* szRoot)		{_mbscpy((unsigned char*)m_szSoundRoot,(const unsigned char*)szRoot);}
		char*			GetSoundRoot()					{return m_szSoundRoot;}
		void			SetSilent(DBOOL bSilent)		{ m_bSilent = bSilent; }
		void			GetSoundPath(char* szSound, int nIndex = 0);
		DBOOL			PlayVoice(HOBJECT hObject, char* szSound, DFLOAT fRadius, int nVol);
		HSOUNDDE		PlaySound(HOBJECT hObject, char* szSound, DFLOAT fRadius, int nVol, DBOOL bGetHandle = DFALSE );
        
// Used for each Animation Index
        DDWORD m_nAnim_IDLE[4];

        DDWORD m_nAnim_TALK[5];

        DDWORD m_nAnim_WALK[9];

        DDWORD m_nAnim_WALK_INJURED_RLEG_RIFLE;
        DDWORD m_nAnim_WALK_INJURED_RLEG_PISTOL;
        
        DDWORD m_nAnim_WALK_INJURED_LLEG_RIFLE;
        DDWORD m_nAnim_WALK_INJURED_LLEG_PISTOL;

        DDWORD m_nAnim_RUN[9];        
		DDWORD m_nAnim_JUMP[9];        
		DDWORD m_nAnim_CROUCH[9];        
		DDWORD m_nAnim_CRAWL[9];
        DDWORD m_nAnim_SWIM[9];

		DDWORD m_nAnim_STRAFERIGHT[9];
		DDWORD m_nAnim_STRAFELEFT[9];
        
        DDWORD m_nAnim_PICKUP_WEAPON;
        
        DDWORD m_nAnim_SWITCH_WEAPON[3];
        
        DDWORD m_nAnim_FIRE_STAND[9];        
        DDWORD m_nAnim_FIRE_WALK[9];
        DDWORD m_nAnim_FIRE_RUN[9];        
        DDWORD m_nAnim_FIRE_JUMP[9];
        DDWORD m_nAnim_FIRE_CROUCH[9];        
        DDWORD m_nAnim_FIRE_CRAWL[9];
        
        DDWORD m_nAnim_FALLING;
        DDWORD m_nAnim_FALLING_UNCONTROL;
        
        DDWORD m_nAnim_ROLL_FORWARD;
        DDWORD m_nAnim_ROLL_RIGHT;
        DDWORD m_nAnim_ROLL_LEFT;
        DDWORD m_nAnim_ROLL_BACK;
        
        DDWORD m_nAnim_HANDSPRING_FORWARD;
        DDWORD m_nAnim_HANDSPRING_RIGHT;
        DDWORD m_nAnim_HANDSPRING_LEFT;
        DDWORD m_nAnim_HANDSPRING_BACK;
        
        DDWORD m_nAnim_FLIP_FORWARD;
        DDWORD m_nAnim_FLIP_RIGHT;
        DDWORD m_nAnim_FLIP_LEFT;
        DDWORD m_nAnim_FLIP_BACK;
        
        DDWORD m_nAnim_DODGE_RIGHT;
        DDWORD m_nAnim_DODGE_LEFT;
        
        DDWORD m_nAnim_RECOIL[12];
        
        DDWORD m_nAnim_TAUNT[7];
        
        DDWORD m_nAnim_SPOTPLAYER_RIGHT;
        DDWORD m_nAnim_SPOTPLAYER_LEFT;
        DDWORD m_nAnim_SPOTPLAYER_POINT;
        
		DDWORD m_nAnim_HUMILIATION[10];

        DDWORD m_nAnim_DEATH[12];

        DDWORD m_nAnim_CORPSE[12];
		DDWORD m_nAnim_LIMB[3];
               
        DDWORD m_nAnim_SPECIAL[9];               


        // Sounds
		char	m_szSoundRoot[256];
		char	m_szLastSound[256];

		HSOUNDDE	m_hVoxSound;
        
		DBOOL	m_bSilent;				// This object should play no sounds for now

		DFLOAT	m_fHitSpheres[NUM_STD_NODES];

	protected : // Member Variables

        
};

#endif // __ANIMSOUND_H__