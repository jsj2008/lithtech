// ----------------------------------------------------------------------- //
//
// MODULE  : ConversationStrings.cpp
//
// PURPOSE : ConversationStrings - Filename to String mapper
//
// CREATED : 10/11/98
//
// ----------------------------------------------------------------------- //

#ifndef _CONVERSATION_STRINGS_H_
#define _CONVERSATION_STRINGS_H_

#include "cpp_server_de.h"
#include <stdio.h>


typedef struct ConversationEntry_t
{
	char *szFile;
	char *szText;
} ConversationEntry;


// LOCALIZATION:  The following strings will need to be localized.  Only the
// szText string should be changed.

static ConversationEntry s_ConversationStringMap[] =
{
	//	szFile				// szText
	{ "4Prophet1.wav",	"I'm staying."  },
	{ "1ACaleb1.wav",	"Bring it on!" },
	{ "1ACaleb2.wav",	"End of the line." },
	{ "2Caleb1.wav",	"You just made your second mistake.  You stuck around." },
	{ "2Caleb2.wav",	"Letting me live." },
	{ "3Caleb1.wav",	"...Gabriel?" },
	{ "3Caleb2.wav",	"Save it for someone who cares." },
	{ "3Caleb3.wav",	"Things have changed.  But I don't need to tell you that." },
	{ "3Caleb4.wav",	"You're not my type." },
	{ "4Caleb1.wav",	"The choice is yours.  Walk now and live.  Stay, and die." },
	{ "4Caleb2.wav",	"Then you're dying." },
	{ "Bmpr1Caleb1.wav","I hate to fly." },
	{ "5Caleb1.wav",	"This could be bad." },
	{ "6Caleb1.wav",	"These aren't Cabal." },
	{ "6Caleb2.wav",	"Something else." },
	{ "6Caleb3.wav",	"I've got a train to catch.  See you around." },
	{ "8Caleb1.wav",	"You've gotta be kidding." },
	{ "7Caleb1.wav",	"You have no idea." },
	{ "7Caleb2.wav",	"You bet, I'm just getting warmed up." },
	{ "7Caleb3.wav",	"I am denial.  I'll face nothing." },
	{ "7Caleb4.wav",	"I don't need babysitting." },
	{ "8Caleb1.wav",	"Easy big fella!" },
	{ "9Caleb1.wav",	"Oh, I'll do more than that." },
	{ "9Caleb2.wav",	"Groovy." },
	{ "9Caleb3.wav",	"Let it burn.  I never liked this place anyway." },
	{ "9Caleb4.wav",	"Pizza." },
	{ "9Caleb5.wav",	"Stripagram." },
	{ "11Caleb1.wav",	"I've been dreaming of you all tied up like that." },
	{ "11Caleb2.wav",	"Gimme some sugar, baby." },
	{ "11Caleb3.wav",	"Oh, you again. How about coming back in about 2 minutes." },
	{ "11Caleb4.wav",	"Ophelia! Okay, that's it.  It's time you die!" },
	{ "13Caleb1.wav",	"The rest of your life's gonna be measured in seconds, chump." },
	{ "14Caleb1.wav",	"Then I'll just have to keep on kicking your ass." },
	{ "14Caleb2.wav",	"Never said I was smart." },
	{ "15Caleb1.wav",	"Go ahead. I'll watch." },
	{ "15Caleb2.wav",	"Let's boogie, boogie man." },
	{ "16Caleb1.wav",	"When you get to hell, tell them I sent you.  You'll get a discount." },
	{ "16Caleb2.wav",	"Relax, you're dead." },
	{ "16Caleb3.wav",	"Too late, you're already dead." },
	{ "16Caleb4.wav",	"Life's not fair, but either is death.  Get use to it." },
	{ "16Caleb5.wav",	"Thank you, and goodnight." },
	{ "17Caleb1.wav",	"Nah." },
	{ "17Caleb2.wav",	"You're gonna hurt my feelings." },
	{ "17Caleb3.wav",	"Well?" },
	{ "17Caleb4.wav",	"Enough talk.  Let's fight." },
	{ "19Caleb1.wav",	"Well, hello, Mr. Fancypants.  Forgive me if I don't shake hands." },
	{ "20Caleb1.wav",	"I don't know nothing about that kind of stuff." },
	{ "20Caleb2.wav",	"What are gonna do, spank me?" },
	{ "20Caleb3.wav",	"Okay, just shut up, I need to focus. " },
	{ "20Caleb4.wav",	"Kinda like a spiritual enema.  In a good way." },
	{ "3Gab1.wav",		"What the hell is this?" },
	{ "3Gab2.wav",		"Caleb?" },
	{ "3Gab3.wav",		"Gabriella.  It's a long story." },
	{ "3Gab4.wav",		"Is that any way to greet an old friend?" },
	{ "3Gab5.wav",		"I need to go with you.  Find out what's going on." },
	{ "3Gab6.wav",		"Looked in the mirror lately?  You're no catch yourself." },
	{ "6Gab1.wav",		"These things are everywhere.  The Cabal must be pretty desperate to be creating such creatures." },
	{ "6Gab2.wav",		"Then what are they?" },
	{ "6Gab4.wav",		"What are you doing?" },
	{ "6Gab5.wav",		"Don't worry, little man.  We'll meet again." },
	{ "14Gab1.wav",		"We have to go after him.  Who knows what's waiting for him." },
	{ "17Gab1.wav",		"Insect, we are the Ancient One.  Bow down before us." },
	{ "17Gab2.wav",		"You are all that stands between us and dominion?  Pathetic." },
	{ "17Gab3.wav",		"All that is will no longer be.  All that was has never been.  All that was to come... um..." },
	{ "17Gab4.wav",		"All that was to come will never... happen." },
	{ "20Gab1.wav",		"I'm gonna kill you both if you don't shut up." },
	{ "1AGid1.wav",		"This is Gideon, your guest conductor!  Hold on tight, the ride might get a little... bloody!" },
	{ "1AGid2.wav",		"I know you're here, Caleb.  I look forward to ripping out your spleen and wearing it as a hat." },
	{ "2Gid1.wav",		"Don't have much luck with trains, do you?" },
	{ "2Gid2.wav",		"Oh really?  What was my first mistake?" },
	{ "3Gid1.wav",		"Time to die, Chosen One." },
	{ "7Gid1.wav",		"This better work this time." },
	{ "7Gid2.wav",		"Ishmael?" },
	{ "7Gid3.wav",		"You were lucky this time Betrayer.  In a few hours, everything will fall back into order and I'll be rid of you forever.  For now, I must take my leave.  Don't worry, though-my troops will keep you company.  Play nice." },
	{ "10Gid1.wav",		"Sounds like home." },
	{ "10Gid2.wav",		"Get out of my frikkin' way!" },
	{ "11Gid1.wav",		"I thought you might come here." },
	{ "11Gid2.wav",		"Sorry to break up this lovely reunion Betrayer, but I need to keep you motivated." },
	{ "14Gid1.wav",		"It's not over!  You will never truly defeat me!" },
	{ "15Gid1.wav",		"Ah, Caleb.  I've been expecting you, you predictable oaf.  I won't be a minute.  I'm just... changING..." },
	{ "15Gid2.wav",		"...into something ...Unpleasant." },
	{ "15Gid3.wav",		"What do you think of my new look?  You have no idea how exquisite pain can feel.  Allow me to demonstrate." },
	{ "15Gid4.wav",		"I mean on you!" },
	{ "16Gid1.wav",		"No!  It's not fair!  I'll kill you!" },
	{ "7Ish1.wav",		"This is a surprising development." },
	{ "7Ish2.wav",		"Having a bad day?" },
	{ "7Ish3.wav",		"Do you understand how much damage your doing?" },
	{ "7Ish4.wav",		"That's not what I mean.  You are the One That Binds.  Sooner or later you'll have to face that." },
	{ "7Ish5.wav",		"Then know this, we are the Chosen.  We're here for you whether you like it or not." },
	{ "7Ish6.wav",		"Yes, Ophelia too.  She was the first to come through." },
	{ "7Ish7.wav",		"I wouldn't miss this for the world.  Then again, you just might end up destroying it." },
	{ "9Ish1.wav",		"You're thinking about Ophelia.  Touching." },
	{ "9Ish2.wav",		"There is a permanent rift in Gideon's temple.  It is a focus for all the damage that is being done.  An epicenter, so to speak.  You will find Ophelia there." },
	{ "9Ish3.wav",		"Tchernobog's destruction opened many doors between the worlds.  They can only be sealed by the One That Binds.  That power lies within you.  This reality will perish unless you restore it." },
	{ "9Ish4.wav",		"There is another reality forcing its way through the rifts, ancient and ravenous.  It is neither good nor evil, it merely is.  But it will devour us all." },
	{ "14Ish1.wav",		"You're not going after him, are you?" },
	{ "14Ish2.wav",		"I'm speculating that it's something very unpleasant." },
	{ "17Ish1.wav",		"Insect, we are the Ancient One.  Bow down before us." },
	{ "17Ish2.wav",		"You are all that stands between us and dominion?  Pathetic." },
	{ "17Ish3.wav",		"All that is will no longer be.  All that was has never been.  All that was to come... um..." },
	{ "17Ish4.wav",		"All that was to come will never... happen." },
	{ "20Ish1.wav",		"It is not over, Caleb.  You must seal the rifts.  You must rebind the fabric of this reality before it tears itself apart." },
	{ "20Ish2.wav",		"The power is within you.  It is time you used it." },
	{ "3MadSci1.wav",	"That's unexpected.  Hmmm, I guess I'll need to recalibrate the singularity settings." },
	{ "7MadSci1.wav",	"Trust me, my calculations are perfect this time." },
	{ "10MadSci1.wav",	"Sir, I have to advise you to reconsider.  My calculations are still inconclusive.  The rifts may be a source of unlimited power, as you suggest, but they might also be deadly pitfalls in the fabric of the space-time continuum.  They might lead you to a realm of pure chaos and evil where you will be completely and utterly annihilated." },
	{ "10MadSci2.wav",	"But sir!" },
	{ "10MadSci3.wav",	"No! ...I never got a chance... to tell you... how much I love you..." },
	{ "12MadSci1.wav",	"Hello, hello, hello.  Welcome to Research and Development, or, as I like to call it... R&D. You may call me... doctor.  I've had some rather interesting subjects lately with all the fluctuations in the cosmic substrate, but I must say I'm especially looking forward to your... dissection.  Please proceed to the main lab and we'll begin the experiment..." },
	{ "11Oph1.wav",		"Prince Charming, I presume." },
	{ "11Oph2.wav",		"Cut me lose and I'll give you a whole vanilla pie." },
	{ "14Oph1.wav",		"He wants you to follow him, you dumb bastard." },
	{ "14Oph2.wav",		"What are we waiting for?" },
	{ "17Oph1.wav",		"Insect, we are the Ancient One.  Bow down before us." },
	{ "17Oph2.wav",		"You are all that stands between us and dominion?  Pathetic." },
	{ "17Oph3.wav",		"All that is will no longer be.  All that was has never been.  All that was to come... um..." },
	{ "17Oph4.wav",		"All that was to come will never... happen." },
	{ "20Oph1.wav",		"You have no choice.  Listen to Ishmael you idiot." },
	{ "20Oph2.wav",		"I'll spank you if you spank me." }
};

#define NUM_CONVERSATIONS  (sizeof(s_ConversationStringMap) / sizeof(s_ConversationStringMap[0]))

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ConvertFullPathToFilename()
//
//	PURPOSE:	Convert the full path and filename to just the filename.
//
// ----------------------------------------------------------------------- //

static char* ConvertFullPathToFilename(char* pPath)
{
	if (!pPath) return DNULL;

	char* pFilename = pPath;
	int nLen = _mbstrlen(pPath);

	for (int i=nLen-1; i >= 0; i--)
	{
		if (pPath[i] == '\\')
		{
			pFilename = &(pPath[i+1]);
			break;
		}
	}

	return pFilename;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FilenameToText()
//
//	PURPOSE:	Map the passed in filename to the appropriate string
//
// ----------------------------------------------------------------------- //

char* FilenameToText(HSTRING hFileName)
{
	if (!g_pServerDE || !hFileName) return DNULL;

	char* pFullFilename = g_pServerDE->GetStringData(hFileName);
	if (!pFullFilename) return DNULL;

	char* pFilename = ConvertFullPathToFilename(pFullFilename);
	if (!pFilename) return DNULL;

	ConversationEntry *pEntry = &s_ConversationStringMap[0];

	for (int i=0; i < NUM_CONVERSATIONS; i++)
	{
		pEntry = &s_ConversationStringMap[i];
		if (pEntry && pEntry->szFile)
		{
			if (_mbsicmp((const unsigned char*)pEntry->szFile, (const unsigned char*)pFilename) == 0)
			{
				return pEntry->szText;
			}
		}
	}

	return DNULL;
}

#endif // _CONVERSATION_STRINGS_H_