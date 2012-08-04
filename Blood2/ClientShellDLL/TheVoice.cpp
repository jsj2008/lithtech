/****************************************************************************
;
;	 MODULE:		THEVOICE (.CPP)
;
;	PURPOSE:		The multiplayer 'god' voice
;
;	HISTORY:		11/30/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/

// Includes...

#include "TheVoice.h"
#include "..\Shared\ClientRes.h"
#include <mbstring.h>


// Globals...

VoiceEntry g_aVoiceStartBB[NUM_VOICE_START_BB];
VoiceEntry g_aVoiceStartCTF[NUM_VOICE_START_CTF]; 
VoiceEntry g_aVoiceOverkill[NUM_VOICE_OVERKILL];
VoiceEntry g_aVoiceKill[NUM_VOICE_KILL];
VoiceEntry g_aVoiceSuicide[NUM_VOICE_SUICIDE];
VoiceEntry g_aVoiceHumiliation[NUM_VOICE_HUMILIATION]; 


// Statics...

static CClientDE*	s_pClientDE = NULL;


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Cve (Clear Voice Entries)
//
//	PURPOSE:	Clears the given voice entry array
//
// ----------------------------------------------------------------------- //

void Cve(VoiceEntry* pVe, int nNumEntries)
{
	// Sanity checks...

	if (!pVe) return;
	if (nNumEntries <= 0) return;


	// Clear the entire array...

	int nSize = sizeof(VoiceEntry) * nNumEntries;

	memset(pVe, 0, nSize);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ive (Init Voice Entry)
//
//	PURPOSE:	Initializes the given voice entry
//
// ----------------------------------------------------------------------- //

void Ive(VoiceEntry* pVe, int id, char* sText, char* sWave, DBYTE byFlags)
{
	// Sanity checks...

	if (!s_pClientDE) return;
	if (!pVe) return;
	if (id == 0) return;


	// Get the string resource and set the text...

	HSTRING hString = s_pClientDE->FormatString(id);

	if (hString)
	{
		_mbsncpy((unsigned char*)pVe->m_szText, (const unsigned char*)s_pClientDE->GetStringData(hString), MAX_LEN_VOICE_TEXT);
		s_pClientDE->FreeString(hString);
	}
	else
	{
		if (sText)
		{
			_mbsncpy((unsigned char*)pVe->m_szText, (const unsigned char*)sText, MAX_LEN_VOICE_TEXT);
		}
	}


	// Set the wave file string...

	if (sWave)
	{
		_mbsncpy((unsigned char*)pVe->m_szFile, (const unsigned char*)sWave, MAX_LEN_VOICE_FILE);
	}


	// Set the flags...

	pVe->m_byFlags = byFlags;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TheVoice_Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL TheVoice_Init(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Set our static client pointer...

	s_pClientDE = pClientDE;


	// Init g_aVoiceStartBB...

	Cve(g_aVoiceStartBB, NUM_VOICE_START_BB);

	Ive(&g_aVoiceStartBB[0], IDS_THEVOICE_1, "Let The Bloodbath Begin!", "Voice248.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[1], IDS_THEVOICE_2, "May The Bloodbath Continue!", "Voice249.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[2], IDS_THEVOICE_3, "Determine The Master Of The Bloodbath", "Voice250.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[3], IDS_THEVOICE_4, "The Carnage Continues!"	, "Voice251.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[4], IDS_THEVOICE_5, "Let The Bloodbath Start!"	, "Voice252.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[5], IDS_THEVOICE_6, "May The Killing Continue", "Voice253.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[6], IDS_THEVOICE_7, "Begin The Bloodbath!"	, "Voice254.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartBB[7], IDS_THEVOICE_8, "Bloodbath, My Children!", "Voice255.wav", VOICEFLAG_ALL);


	// Init g_aVoiceStartCTF...

	Cve(g_aVoiceStartCTF, NUM_VOICE_START_CTF);

	Ive(&g_aVoiceStartCTF[0], IDS_THEVOICE_10, "Let The BloodFeud Begin!", "Voice256.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[1], IDS_THEVOICE_11, "May The BloodFeud Continue!", "Voice257.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[2], IDS_THEVOICE_12, "Determine The Masters Of The BloodFeud", "Voice258.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[3], IDS_THEVOICE_13, "The Carnage Continues!", "Voice259.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[4], IDS_THEVOICE_14, "Let The BloodFeud Start!", "Voice260.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[5], IDS_THEVOICE_15, "May The Killing Continue", "Voice261.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[6], IDS_THEVOICE_16, "Begin The BloodFeud!", "Voice262.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceStartCTF[7], IDS_THEVOICE_17, "BloodFeud, My Children!", "Voice263.wav", VOICEFLAG_ALL);


	// Init g_aVoiceOverkill...

	Cve(g_aVoiceOverkill, NUM_VOICE_OVERKILL);

	Ive(&g_aVoiceOverkill[0], IDS_THEVOICE_20, "Maximum Bloodshed", "Voice300.wav", VOICEFLAG_ALL);


	// Init g_aVoiceKill...

	Cve(g_aVoiceKill, NUM_VOICE_KILL);

	Ive(&g_aVoiceKill[0], IDS_THEVOICE_100, "Destroyed", "Voice1.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[1], IDS_THEVOICE_101, "Hosed", "Voice2.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[2], IDS_THEVOICE_102, "Humiliated", "Voice3.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[3], IDS_THEVOICE_103, "Toasted", "Voice4.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[4], IDS_THEVOICE_104, "Sent to hell", "Voice5.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[5], IDS_THEVOICE_105, "Pass the chili", "Voice6.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[6], IDS_THEVOICE_106, "Punishment delivered", "Voice7.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[7], IDS_THEVOICE_107, "Bobbetttized!", "Voice8.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[8], IDS_THEVOICE_108, "Stiffed", "Voice9.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[9], IDS_THEVOICE_109, "He shoots! He scores!", "Voice10.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[10], IDS_THEVOICE_110, "Spillage", "Voice11.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[11], IDS_THEVOICE_111, "Sprayed", "Voice12.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[12], IDS_THEVOICE_112, "Dog meat", "Voice13.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[13], IDS_THEVOICE_113, "Bye, bye, now", "Voice14.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[14], IDS_THEVOICE_114, "Ripped 'em loose", "Voice15.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[15], IDS_THEVOICE_115, "Beaten like a kirk", "Voice16.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[16], IDS_THEVOICE_116, "Whipped and creamed", "Voice17.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[17], IDS_THEVOICE_117, "Snuffed", "Voice18.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[18], IDS_THEVOICE_118, "Spleened", "Voice19.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[19], IDS_THEVOICE_119, "Vaporized!", "Voice20.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[20], IDS_THEVOICE_120, "Excellent!", "Voice21.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[21], IDS_THEVOICE_121, "Excellent!", "Voice22.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[22], IDS_THEVOICE_122, "%s Sodomized %s", "Voice23.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[23], IDS_THEVOICE_123, "%s Gave %s Anal Justice", "Voice24.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[24], IDS_THEVOICE_124, "%s Shat Upon %s", "Voice25.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[25], IDS_THEVOICE_125, "%s Cavity Searched %s", "Voice26.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[26], IDS_THEVOICE_126, "%s Gave %s The Lickety Split", "Voice27.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[27], IDS_THEVOICE_127, "%s Whipped %s’s Cream", "Voice28.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[28], IDS_THEVOICE_128, "%s Fed %s Some Chunky Man Chowder!", "Voice29.wav", VOICEFLAG_MALE_ATTACKER | VOICEFLAG_ALL_VICTIM);
	Ive(&g_aVoiceKill[29], IDS_THEVOICE_129, "%s Fed %s Some Chunky Woman Chowder!", "Voice30.wav", VOICEFLAG_FEMALE_ATTACKER | VOICEFLAG_ALL_VICTIM);
	Ive(&g_aVoiceKill[30], IDS_THEVOICE_130, "%s slagged %s", "Voice31.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[31], IDS_THEVOICE_131, "%s annihilated %s", "Voice32.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[32], IDS_THEVOICE_132, "%s sterilized %s", "Voice33.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[33], IDS_THEVOICE_133, "%s brained %s", "Voice34.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[34], IDS_THEVOICE_134, "%s stained the floor with %s", "Voice35.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[35], IDS_THEVOICE_135, "%s squashed %s like a bug", "Voice36.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[36], IDS_THEVOICE_136, "%s did a dance in %s's sphincter", "Voice37.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[37], IDS_THEVOICE_137, "%s smacked %s like a bad child", "Voice38.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[38], IDS_THEVOICE_138, "%s greased %s like a dry phallus", "Voice39.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[39], IDS_THEVOICE_139, "%s Obliterated %s", "Voice40.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[40], IDS_THEVOICE_140, "%s gutted %s like a fish", "Voice41.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[41], IDS_THEVOICE_141, "%s terminated %s", "Voice42.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[42], IDS_THEVOICE_142, "%s eviscerated %s", "Voice43.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[43], IDS_THEVOICE_143, "%s snuffed %s like a candle", "Voice44.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[44], IDS_THEVOICE_144, "%s whupped %s like a rented mule", "Voice45.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[45], IDS_THEVOICE_145, "%s boned %s like a chicken breast", "Voice46.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[46], IDS_THEVOICE_146, "%s spanked %s like a monkey", "Voice47.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[47], IDS_THEVOICE_147, "%s slapped %s down", "Voice48.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[48], IDS_THEVOICE_148, "%s extinguished %s", "Voice49.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[49], IDS_THEVOICE_149, "%s roasted %s like a loin", "Voice50.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[50], IDS_THEVOICE_150, "%s disemboweled %s", "Voice51.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[51], IDS_THEVOICE_151, "%s crushed %s like a grape", "Voice52.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[52], IDS_THEVOICE_152, "%s molested %s’s corpse", "Voice53.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[53], IDS_THEVOICE_153, "%s impaled %s with a middle digit", "Voice54.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[54], IDS_THEVOICE_154, "%s mutilated %s", "Voice55.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[55], IDS_THEVOICE_155, "%s OJ’d %s", "Voice56.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[56], IDS_THEVOICE_156, "%s interned %s like a Slick Willy", "Voice57.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[57], IDS_THEVOICE_157, "%s penetrated %s violently", "Voice58.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[58], IDS_THEVOICE_158, "%s shanked %s like an inmate", "Voice59.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[59], IDS_THEVOICE_159, "%s smoked %s like a carp", "Voice60.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[60], IDS_THEVOICE_160, "%s skidmarked %s on the floor", "Voice61.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[61], IDS_THEVOICE_161, "%s lobotomized %s", "Voice62.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[62], IDS_THEVOICE_162, "%s dogged %s", "Voice63.wav", VOICEFLAG_ALL);
 	Ive(&g_aVoiceKill[63], IDS_THEVOICE_163, "%s toasted %s like a marshmallow", "Voice64.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[64], IDS_THEVOICE_164, "%s made %s roadkill", "Voice65.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[65], IDS_THEVOICE_165, "%s pimped %s", "Voice66.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[66], IDS_THEVOICE_166, "%s gargled %s’s remains", "Voice67.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[67], IDS_THEVOICE_167, "%s gagged %s with pleasure", "Voice68.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[68], IDS_THEVOICE_168, "%s shredded %s like a White House document", "Voice69.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[69], IDS_THEVOICE_169, "%s busted down %s and called him Debbie", "Voice70.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[70], IDS_THEVOICE_170, "%s busted down %s and called her Debbie", "Voice70.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[71], IDS_THEVOICE_171, "%s executed %s", "Voice71.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[72], IDS_THEVOICE_172, "%s fed it to %s", "Voice72.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[73], IDS_THEVOICE_173, "%s mashed %s like a moldy potato", "Voice73.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[74], IDS_THEVOICE_174, "%s nutted %s", "Voice74.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[75], IDS_THEVOICE_175, "%s gave %s the pink slip", "Voice75.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[76], IDS_THEVOICE_176, "%s barbecued %s like a suckling pig", "Voice76.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[77], IDS_THEVOICE_177, "%s deballed %s", "Voice77.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[78], IDS_THEVOICE_178, "%s neutered %s"	, "Voice78.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[79], IDS_THEVOICE_179, "%s collected %s like a bad debt", "Voice79.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[80], IDS_THEVOICE_180, "%s Said Bye-Bye Now to %s", "Voice80.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[81], IDS_THEVOICE_181, "%s gave %s anal leakage like a fat free chip", "Voice81.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[82], IDS_THEVOICE_182, "%s exploited %s’s lack of prowess", "Voice82.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[83], IDS_THEVOICE_183, "%s sliced %s", "Voice83.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[84], IDS_THEVOICE_184, "%s diced %s", "Voice84.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[85], IDS_THEVOICE_185, "%s pummeled %s like a side of beef", "Voice85.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[86], IDS_THEVOICE_186, "%s gave %s a dishonorable discharge", "Voice86.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[87], IDS_THEVOICE_187, "%s dosed %s like a venereal disease", "Voice87.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[88], IDS_THEVOICE_188, "%s bushwhacked %s", "Voice88.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[89], IDS_THEVOICE_189, "%s worked %s like a nickel prostitute", "Voice89.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[90], IDS_THEVOICE_190, "%s gave %s a colon-cleansing", "Voice90.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[91], IDS_THEVOICE_191, "%s membered %s like a freemason", "Voice91.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[92], IDS_THEVOICE_192, "%s made sure %s sucked it down", "Voice92.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[93], IDS_THEVOICE_193, "%s gave %s oral redemption", "Voice93.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[94], IDS_THEVOICE_194, "%s gave %s an oral apocalypse", "Voice94.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[95], IDS_THEVOICE_195, "%s danced in %s's mouth", "Voice95.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[96], IDS_THEVOICE_196, "%s showed %s his mortality", "Voice96.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[97], IDS_THEVOICE_197, "%s showed %s her mortality", "Voice96.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[98], IDS_THEVOICE_198, "%s gave %s a rectal examination", "Voice97.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[99], IDS_THEVOICE_199, "%s herniated %s without the cough", "Voice98.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[100], IDS_THEVOICE_200, "%s circumcised %s with an unclean tool", "Voice99.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[101], IDS_THEVOICE_201, "%s decaffeinated %s like a bad cup of coffee", "Voice100.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[102], IDS_THEVOICE_202, "%s jacked %s’s beanstalk", "Voice101.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[103], IDS_THEVOICE_203, "%s pissed in %s’s grave", "Voice102.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[104], IDS_THEVOICE_204, "%s FUBAR’d %s", "Voice103.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[105], IDS_THEVOICE_205, "%s gave %s proper payback", "Voice104.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[106], IDS_THEVOICE_206, "%s ventilated %s", "Voice105.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[107], IDS_THEVOICE_207, "%s mangled %s", "Voice106.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[108], IDS_THEVOICE_208, "%s plastered %s like a bad wall", "Voice107.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[109], IDS_THEVOICE_209, "%s decimated %s", "Voice108.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[110], IDS_THEVOICE_210, "%s Read %s's Obituary", "Voice109.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[111], IDS_THEVOICE_211, "%s Demonstrated The Ankle Grab for %s", "Voice110.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[112], IDS_THEVOICE_212, "%s Cancelled %s like a stamp", "Voice111.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[113], IDS_THEVOICE_213, "%s ripped %s like a scrotum", "Voice112.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[114], IDS_THEVOICE_214, "%s decorated with %s's Giblets", "Voice113.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[115], IDS_THEVOICE_215, "%s baked %s’s cookies.", "Voice114.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[116], IDS_THEVOICE_216, "%s showed %s the anger", "Voice115.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[117], IDS_THEVOICE_217, "%s showed %s the love", "Voice116.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[118], IDS_THEVOICE_218, "%s showed %s the tears", "Voice117.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[119], IDS_THEVOICE_219, "%s showed %s the humiliation", "Voice118.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[120], IDS_THEVOICE_220, "%s showed %s the member", "Voice119.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[121], IDS_THEVOICE_221, "%s showed %s the trauma", "Voice120.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[122], IDS_THEVOICE_222, "%s showed %s the blood", "Voice121.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[123], IDS_THEVOICE_223, "%s showed %s the brutality", "Voice122.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[124], IDS_THEVOICE_224, "%s showed %s the butchery", "Voice123.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[125], IDS_THEVOICE_225, "%s showed %s the maiming", "Voice124.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[126], IDS_THEVOICE_226, "%s showed %s the fisting", "Voice125.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[127], IDS_THEVOICE_227, "%s showed %s the slaughter", "Voice126.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[128], IDS_THEVOICE_228, "%s showed %s the torture", "Voice127.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[129], IDS_THEVOICE_229, "%s showed %s the needlessness", "Voice128.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[130], IDS_THEVOICE_230, "%s showed %s the pointlessness", "Voice129.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[131], IDS_THEVOICE_231, "%s offed %s like a switch", "Voice130.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[132], IDS_THEVOICE_232, "%s showed %s the horror", "Voice131.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[133], IDS_THEVOICE_233, "%s reamed %s with his weapon", "Voice132.wav", VOICEFLAG_MALE_ATTACKER | VOICEFLAG_ALL_VICTIM);
	Ive(&g_aVoiceKill[134], IDS_THEVOICE_234, "%s reamed %s with her weapon", "Voice132.wav", VOICEFLAG_FEMALE_ATTACKER | VOICEFLAG_ALL_VICTIM);
	Ive(&g_aVoiceKill[135], IDS_THEVOICE_235, "%s stapled %s’s innards to the floor", "Voice133.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[136], IDS_THEVOICE_236, "%s Humped %s like a dog in heat", "Voice134.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[137], IDS_THEVOICE_237, "%s Nailed %s With An Improper Tool", "Voice135.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[138], IDS_THEVOICE_238, "%s Gave %s Scrotal Separation!", "Voice136.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[139], IDS_THEVOICE_239, "%s Had %s Rolled And Stuffed.", "Voice137.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[140], IDS_THEVOICE_240, "%s Made %s Room Temperature!", "Voice138.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[141], IDS_THEVOICE_241, "%s Smacked %s Like A Rented Stepchild", "Voice139.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[142], IDS_THEVOICE_242, "%s Delivered %s To The Dumpster", "Voice140.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[143], IDS_THEVOICE_243, "%s Played In %s's Caboose", "Voice141.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[144], IDS_THEVOICE_244, "%s Pounded %s's Prostate", "Voice142.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[145], IDS_THEVOICE_245, "%s Nailed %s Like A Dog In Heat", "Voice143.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[146], IDS_THEVOICE_246, "%s Gave %s Rectal Redemption!", "Voice144.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[147], IDS_THEVOICE_247, "%s Gave %s A Cavity Search!", "Voice145.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[148], IDS_THEVOICE_248, "Used", "Voice146.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[149], IDS_THEVOICE_249, "The Hurtfulness", "Voice147.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[150], IDS_THEVOICE_250, "%s Made %s Drink Father's Milk!", "Voice148.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[151], IDS_THEVOICE_251, "%s Disemboweled %s", "Voice149.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[152], IDS_THEVOICE_252, "%s Nippled %s", "Voice150.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[153], IDS_THEVOICE_253, "%s Ripped %s A New Sphincter!", "Voice151.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[154], IDS_THEVOICE_254, "%s Gave %s Some Crème De Banane!", "Voice152.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[155], IDS_THEVOICE_255, "%s Danced On %s’s Broken Corpse", "Voice153.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[156], IDS_THEVOICE_256, "%s Served %s His Own Kidney", "Voice154.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[157], IDS_THEVOICE_257, "%s Served %s Her Own Kidney", "Voice154.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[158], IDS_THEVOICE_258, "%s Made Fresh Head Cheese Of %s", "Voice155.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[159], IDS_THEVOICE_259, "%s Offered %s", "Voice156.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[160], IDS_THEVOICE_260, "%s Offered %s A Hot Polish Sausage", "Voice157.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[161], IDS_THEVOICE_261, "%s Played Postman And Delivered The Male To %s", "Voice158.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[162], IDS_THEVOICE_262, "%s Popped %s Like A Grape", "Voice159.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[163], IDS_THEVOICE_263, "%s Rained Gobbets Of %s On The Floor", "Voice160.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[164], IDS_THEVOICE_264, "%s Had %s Stuffed And Mounted!", "Voice161.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[165], IDS_THEVOICE_265, "%s Fed %s A Fresh Loaf", "Voice162.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[166], IDS_THEVOICE_266, "%s Lobotomized %s", "Voice163.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[167], IDS_THEVOICE_267, "%s Brained %s And Fed It To Him", "Voice164.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[168], IDS_THEVOICE_268, "%s Brained %s And Fed It To Her", "Voice164.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceKill[169], IDS_THEVOICE_269, "%s Pulped %s Like An Orange", "Voice165.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[170], IDS_THEVOICE_270, "%s Crushed %s Like A Roach", "Voice166.wav", VOICEFLAG_ALL);	
	Ive(&g_aVoiceKill[171], IDS_THEVOICE_271, "%s Played Operation With %s", "Voice167.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[172], IDS_THEVOICE_272, "%s Made Certain That %s Was Properly Penetrated!", "Voice168.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[173], IDS_THEVOICE_273, "%s Showed %s The Agony Of Defeat", "Voice169.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[174], IDS_THEVOICE_274, "%s Roasted %s’s Agates!", "Voice170.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[175], IDS_THEVOICE_275, "%s Slipped %s Some Bile!", "Voice171.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[176], IDS_THEVOICE_276, "%s Defiled %s", "Voice172.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[177], IDS_THEVOICE_277, "%s Fisted %s", "Voice173.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[178], IDS_THEVOICE_278, "%s Made %s Salute The Flagpole!", "Voice174.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[179], IDS_THEVOICE_279, "%s disassembled %s like a chicken corpse", "Voice175.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[180], IDS_THEVOICE_280, "%s liquefied %s", "Voice176.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[181], IDS_THEVOICE_281, "%s drop-kicked %s into oblivion", "Voice177.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[182], IDS_THEVOICE_282, "%s flattened %s like a steam roller", "Voice178.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceKill[183], IDS_THEVOICE_283, "%s Eviscerated %s", "Voice179.wav", VOICEFLAG_ALL);


	// Init g_aVoiceSuicide...

	Cve(g_aVoiceSuicide, NUM_VOICE_SUICIDE);

	Ive(&g_aVoiceSuicide[0], IDS_THEVOICE_300, "Kevorkian Approves of %s!", "Voice200.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[1], IDS_THEVOICE_301, "%s Given Darwin Award!", "Voice201.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[2], IDS_THEVOICE_302, "%s Is An Idiot!", "Voice202.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[3], IDS_THEVOICE_303, "%s Enjoys Self-Flagellation!", "Voice203.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[4], IDS_THEVOICE_304, "%s Ought To Play Mario!", "Voice204.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[5], IDS_THEVOICE_305, "%s Go Bye-Bye!", "Voice205.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[6], IDS_THEVOICE_306, "%s Exited Stage-Left", "Voice206.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[7], IDS_THEVOICE_307, "%s Chose Early Retirement!", "Voice207.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[8], IDS_THEVOICE_308, "%s Is Among The Dumbly Departed", "Voice208.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[9], IDS_THEVOICE_309, "%s Needs Supervision", "Voice209.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[10], IDS_THEVOICE_310, "%s Is A Pantywaist!", "Voice210.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[11], IDS_THEVOICE_311, "%s Believes In The Right To Die!", "Voice211.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[12], IDS_THEVOICE_312, "%s Joined The Hemlock Society", "Voice212.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[13], IDS_THEVOICE_313, "%s Tested His Afterlife Theories", "Voice213.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceSuicide[14], IDS_THEVOICE_314, "%s Tested Her Afterlife Theories", "Voice213.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceSuicide[15], IDS_THEVOICE_315, "%s Was Planted In The Carcass Field", "Voice214.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[16], IDS_THEVOICE_316, "%s Had An Episode of Stupidity", "Voice215.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[17], IDS_THEVOICE_317, "%s Suffered From Felonious Foolishness!", "Voice216.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[18], IDS_THEVOICE_318, "%s Attempted Freeform Expressionism - Poorly.", "Voice217.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[19], IDS_THEVOICE_319, "%s tried self-love with a hammer", "Voice218.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[20], IDS_THEVOICE_320, "%s Auto-Immolated!", "Voice219.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[21], IDS_THEVOICE_321, "%s Kamikazed!", "Voice220.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[22], IDS_THEVOICE_322, "%s Drowned In Own Vomit", "Voice221.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceSuicide[23], IDS_THEVOICE_323, "%s Should Go Home To Mommy!", "Voice222.wav", VOICEFLAG_ALL);


	// Init g_aVoiceHumiliation...

	Cve(g_aVoiceHumiliation, NUM_VOICE_HUMILIATION);

	Ive(&g_aVoiceHumiliation[0], IDS_THEVOICE_330, "Humiliate Him!", "Voice241.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceHumiliation[1], IDS_THEVOICE_331, "Humiliate Her!", "Voice241.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceHumiliation[2], IDS_THEVOICE_332, "End It! End It!", "Voice242.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceHumiliation[3], IDS_THEVOICE_333, "Complete The Kill!", "Voice243.wav", VOICEFLAG_ALL);
	Ive(&g_aVoiceHumiliation[4], IDS_THEVOICE_334, "He Is Yours!", "Voice244.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceHumiliation[5], IDS_THEVOICE_335, "She Is Yours!", "Voice245.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceHumiliation[6], IDS_THEVOICE_336, "Desecrate Him!", "Voice247.wav", VOICEFLAG_MALE_VICTIM | VOICEFLAG_ALL_ATTACKER);
	Ive(&g_aVoiceHumiliation[7], IDS_THEVOICE_337, "Desecrate Her!", "Voice246.wav", VOICEFLAG_FEMALE_VICTIM | VOICEFLAG_ALL_ATTACKER);


	// All done...

	return(DTRUE);
}






