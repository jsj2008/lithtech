BLOOD 2 SOURCE CODE
-------------------

You will want to get the Blood2 Source Tools because it contains
a HELP file with the LithTech APIs and overview.

Note: you will need to add the APPHEADERS, SHARED, and MISC folders
as an include search directory (either via the project settings or
the compiler settings).

Note: you will need to add the MISC folder as a library search
directory (either via the project settings or the compiler
settings).


DirectPlay
----------

For the DirectPlay Lobby code in the Clientshell, you must have 
DPLobby.h (part of the DirectX 6 SDK).

We cannot legally redistribute the SDK itself; it must be obtained 
from Microsoft.


Debugging your Code
-------------------

To help with debugging, the Lithtech 'SOFTDIB.REN' renderer has been
included in this package; it is in the directory you installed the source
to.  Merely copy this .REN file to the same directory as your BLOOD2 client.exe
and it will be made available to you by the engine.


Compiling the Code
------------------

Once you have compiled your CSHELL.DLL, CRES.DLL, and OBJECT.LTO
files you will want to put them into a new project directory, 
and then stack that project directory on top of the Blood2 project 
as described in the README in the Tools portion of this release.
The SRES for Blood 2 is basically a dummy file, you do not need to
recompile it; simply use the SRES.DLL that is in the appropriate
Blood 2 REZ file.


Directories
-----------

APPHEADERS
	This directory contains the headers for Lithtech.
	This needs to be added to your INCLUDE file path in 
	your compiler or the project settings.

CLIENTRES
	This directory contains the source for the
	cres.dll file.  This is used for localization;
	it's good to store text here so the main game
	code doesn't have to change when localized. :)

CLIENTSHELLDLL
	This directory contains the source for the
	cshell.dll file.  This is the client-side source;
	HUDs, special effects, client-side stuff is all 
	written here.

MISC
	This directory contains files required to compile
	and link the various projects. This directory needs
	to be added to your LIBRARY and INCLUDE file paths
	(either	via the project settings or the compiler
	settings).

OBJECTDLL
	This directory contains the source for the
	object.lto file.  This is the server-side source;
	it ALSO contains all the definitions of objects 
	that you can place in DEdit (the editor).  It 
	should be apparent why after perusing the code
	for a little bit and realizing how Lithtech works. :)

SHARED
	This directory contains game-specific headers which
	are shared between client and server.  This should be
	added to your include path.

BLOOD2SERV
	This is the source code to the standalone listen server.
	Yes, believe it or not, we ARE releasing this.  Maybe
	we're insane, or maybe we just care about the mod
	community. :)


Information/Help
----------------

If you have problems, there are several good resources:

* The official Blood2 website at http://www.the-chosen.com/
* PlanetBlood, a fan community site at http://www.planetblood.com/
* The b2-mod mailing list, a mailing list for mod authors.
  To subscribe, send e-mail to listar@lists.lith.com with the
  subject 'subscribe b2-mod' (minus the quotes, of course).

This is not officially supported by Monolith; Monolith employees
do read the b2-mod mailing list and will provide support there.
Please try to refrain from e-mailing them directly. :)


Thanks
------

We hope you enjoy Blood 2 and look forward to playing all
the modifications you come up with...

- The Blood 2 Team
- The LithTech Team

   Where's da luv?
-> M o n o l i t h <-