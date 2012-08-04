lithtech
========

The engine code is from the apparent GPL release of the Jupiter Ex engine before Touchdown Entertainment disappeared. The GPL release was of the PC Enterprise Edition Build 69. Included were the game code and assets from NOLF2 and the game code from Tron 2.0. The code from the other games are from their respective public releases.

Goal
----
I found the GPL release of the Jupiter Engine and couldn't find anyone else who had really seemed to do anything with it. I wanted to see how hard it would be to adapt it into a modern, multi-platform source port. I also wanted to see if it would be possible to port the other Lithtech release game code to the GPL release of the engine. Both have turned out to be much more work than I originally envisioned. Even though the effort is initially *extremely* rough, I thought I would put it up on GitHub so that I have a backup of my work and that no matter how far (or not) I get, my work will not disappear with me.

Status
------
The state of the codebase on my initial GitHub check-in is admittedly a mess. I have successfully built both the engine and game code for NOLF2 on Visual Studio 2010 and run the game. When I tried to combine everything into one build with CMake, things fell apart for the game code. This is my first time trying to use CMake, so I am sure I have things set up wrong. (My guess is that the include directories got screwed up when I made the NOLF2 projects subprojects of my main build.) I have been trying to google and look at examples. I may just have to break down and borrow/buy the CMake book.

There are a few reasons why I am implementing a new build system:
* All code releases were either using VC++ 6 or Visual Studio 2003 (VC++ 7.1). Neither type of project file converted that well in VS2010.
* Since the ultimate goal is a source port, I would like to be able to build the games on Linux and OS X as well. There was a homegrown converter for VC++ 7.1 project files to makefiles, but given then problems converting between versions of Visual Studio, I decided against it.
* I don't want to require someone to use Visual Studio on Windows. It should at least be buildable by MinGW as well (if not OpenWatcom too).

So, despite the broken CMake build of everything in one tree, if you build the engine code and then build the game DLLs as separate CMake projects, then you can get NOLF2 running. Interestingly enough, a release build seems to run okay, but a debug build will trigger exceptions in the standard library. All the problem seem to have to do with Microsoft's secure CRT. Either things had slightly different semantics between compilers or certain iterator usages were technically not correct in the original code. I fixed maybe two of these. There are still probably another handful left. There also seems to be some differences in the NOLF2 retail data files and those supplied with the GPL release. I am not 100% certain of this, but I need to investigate more.

I also started an effort to port the Shogo source (originally from a really old version of the engine) to the engine in the GPL release. I have made a *ton* of changes, but it still doesn't compile completely. I decided focus on getting a single tree build and debug version of NOLF2 running first.

TODO
----
* At least get a single tree build of the engine and NOLF2 working.
* Long term, have a way to build the engine with any or all of the game source code.
* Fix the secure standard library exceptions so that a Debug build under VS2010 will run.
* Port/adapt all the game code to run on the GPL engine. Each game was written to a different version of the engine. So, theoretically this is possible, but it could take a substantial amount of work.
* Remove the Windows dependencies. The engine is heavily oriented towards Windows (even though there is a bit of Linux code).
    * Replace the usage of MFC with wxWidgets in the tools. (MFC and wxWidgets are close enough that it shouldn't be too painful.)
    * Replace DirectInput and other Windows specific code with SDL.
    * Create a new audio driver based on SDL and/or OpenAL.
    * Create a new OpenGL renderer.
* Long term, it might be cool to bring back something similar to the renderer DLLs that Shogo used. In this version of the engine, the renderer is statically linked to the engine.

Games I want to support and the order to work on them
-----------------------------------------------------
1. No One Lives Forever 2: A Spy in H.A.R.M.'s Way
2. Tron 2.0 (if I can find a copy)
3. Shogo
4. The Operative: No One Lives Forever
5. F.E.A.R.
6. Blood II: The Chosen (if I can find a copy)

These are all the Lithtech I am aware of that had a public source release. If there are more out there, I want to eventually support them too.