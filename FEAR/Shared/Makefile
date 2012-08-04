###############################################################
#
# Makefile for Game_Shared_Server.
#
# (c) 2006 Monolith Productions, Inc. All Rights Reserved.
#
###############################################################

# Build tools
ifeq "$(CPP_TOOL)" "" 
CPP_TOOL=g++
endif
ifeq "$(CC_TOOL)" ""
CC_TOOL=gcc
endif
ifeq "$(LIB_TOOL)" ""
LIB_TOOL=ar
endif
ifeq "$(EXE_TOOL)" ""
EXE_TOOL=g++
endif

# Project name
ProjectName=Game_Shared_Server

# Set default configuration
ifeq "$(ConfigurationName)" ""
ConfigurationName = Debug
endif

# Platform-specific variables
LINUX_DEFINES=-D_LINUX -DDISABLE_LT_MEM_TRACKING -D_GLIBCPP_USE_WCHAR_T
HAVOK_INCLUDE_PATH=../../../libs/Havok231Linux/include
HAVOK_DEBUG_LIB_PATH=../../../libs/Havok231Linux/lib/unix-gcc/debug
HAVOK_RELEASE_LIB_PATH=../../../libs/Havok231Linux/lib/unix-gcc/release

# SourceFiles
SOURCEFILES=./AnimationContext.cpp \
		./AnimationPropStrings.cpp \
		./AnimationTreePacked.cpp \
		./AnimationTreePackedLoader.cpp \
		./AnimationTreePackedMgr.cpp \
		./Animator.cpp \
		./BroadcastDB.cpp \
		./CategoryDB.cpp \
		./CharacterAlignment.cpp \
		./CharacterDB.cpp \
		./CharacterPhysics.cpp \
		./ClientServerShared.cpp \
		./ColorUtilities.cpp \
		./CommonUtilities.cpp \
		./DamageTypes.cpp \
		./DatabaseUtils.cpp \
		./DebugLine.cpp \
		./DebugNew.cpp \
		./EngineTimer.cpp \
		./FXDB.cpp \
		./GameAlloc.cpp \
		./GameDatabaseMgr.cpp \
		./GameModeMgr.cpp \
		./GlobalMgr.cpp \
		../../Engine/sdk/inc/iltbaseclass.cpp \
		./LeanNodeController.cpp \
		../../Engine/sdk/inc/LTEulerAngles.cpp \
		../../Engine/sdk/inc/ltmemhook.cpp \
		../../Engine/sdk/inc/ltmodule.cpp \
		../../Engine/sdk/inc/ltobjref.cpp \
		./MissionDB.cpp \
		./ModelsDB.cpp \
		./NavMarkerTypeDB.cpp \
		./NetDefs.cpp \
		./NodeTracker.cpp \
		./NodeTrackerContext.cpp \
		./ParsedMsg.cpp \
		./PhysicsCollisionMgr.cpp \
		./PhysicsUtilities.cpp \
		./PlayerRigidBody.cpp \
		./ProfileUtils.cpp \
		./PropsDB.cpp \
		./SaveLoadMgr.cpp \
		./ScmdConsole.cpp \
		./SFXMsgIds.cpp \
		./SharedFXStructs.cpp \
		./SharedMission.cpp \
		./SharedMovement.cpp \
		./SharedScoring.cpp \
		./ShatterTypeDB.cpp \
		./SkillDefs.cpp \
		./SonicData.cpp \
		./SonicsDB.cpp \
		./SoundDB.cpp \
		./SoundMgr.cpp \
		./SoundMixerDB.cpp \
		./Stdafx.cpp \
		./StringEditMgr.cpp \
		./SurfaceFunctions.cpp \
		./TeamMgr.cpp \
		./TemplateList.cpp \
		./VersionMgr.cpp \
		./WaveFn.cpp \
		./WeaponDB.cpp \
		./WeaponPath.cpp \
		./WinUtil.cpp \
		
# Object files
OBJECTFILES=$(IntDir)/AnimationContext.o \
		$(IntDir)/AnimationPropStrings.o \
		$(IntDir)/AnimationTreePacked.o \
		$(IntDir)/AnimationTreePackedLoader.o \
		$(IntDir)/AnimationTreePackedMgr.o \
		$(IntDir)/Animator.o \
		$(IntDir)/BroadcastDB.o \
		$(IntDir)/CategoryDB.o \
		$(IntDir)/CharacterAlignment.o \
		$(IntDir)/CharacterDB.o \
		$(IntDir)/CharacterPhysics.o \
		$(IntDir)/ClientServerShared.o \
		$(IntDir)/ColorUtilities.o \
		$(IntDir)/CommonUtilities.o \
		$(IntDir)/DamageTypes.o \
		$(IntDir)/DatabaseUtils.o \
		$(IntDir)/DebugLine.o \
		$(IntDir)/DebugNew.o \
		$(IntDir)/EngineTimer.o \
		$(IntDir)/FXDB.o \
		$(IntDir)/GameAlloc.o \
		$(IntDir)/GameDatabaseMgr.o \
		$(IntDir)/GameModeMgr.o \
		$(IntDir)/GlobalMgr.o \
		$(IntDir)/iltbaseclass.o \
		$(IntDir)/LeanNodeController.o \
		$(IntDir)/LTEulerAngles.o \
		$(IntDir)/ltmemhook.o \
		$(IntDir)/ltmodule.o \
		$(IntDir)/ltobjref.o \
		$(IntDir)/MissionDB.o \
		$(IntDir)/ModelsDB.o \
		$(IntDir)/NavMarkerTypeDB.o \
		$(IntDir)/NetDefs.o \
		$(IntDir)/NodeTracker.o \
		$(IntDir)/NodeTrackerContext.o \
		$(IntDir)/ParsedMsg.o \
		$(IntDir)/PhysicsCollisionMgr.o \
		$(IntDir)/PhysicsUtilities.o \
		$(IntDir)/PlayerRigidBody.o \
		$(IntDir)/ProfileUtils.o \
		$(IntDir)/PropsDB.o \
		$(IntDir)/SaveLoadMgr.o \
		$(IntDir)/ScmdConsole.o \
		$(IntDir)/SFXMsgIds.o \
		$(IntDir)/SharedFXStructs.o \
		$(IntDir)/SharedMission.o \
		$(IntDir)/SharedMovement.o \
		$(IntDir)/SharedScoring.o \
		$(IntDir)/ShatterTypeDB.o \
		$(IntDir)/SkillDefs.o \
		$(IntDir)/SonicData.o \
		$(IntDir)/SonicsDB.o \
		$(IntDir)/SoundDB.o \
		$(IntDir)/SoundMgr.o \
		$(IntDir)/SoundMixerDB.o \
		$(IntDir)/Stdafx.o \
		$(IntDir)/StringEditMgr.o \
		$(IntDir)/SurfaceFunctions.o \
		$(IntDir)/TeamMgr.o \
		$(IntDir)/TemplateList.o \
		$(IntDir)/VersionMgr.o \
		$(IntDir)/WaveFn.o \
		$(IntDir)/WeaponDB.o \
		$(IntDir)/WeaponPath.o \
		$(IntDir)/WinUtil.o \
		
###############################################################
# Configuration: Release
###############################################################
ifeq "$(ConfigurationName)" "Release"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -DNDEBUG -D_SERVERBUILD
INCLUDES=-I.  -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../libs/ltguimgr -I../libs/GameSpy -I../../libs/platform
LIBRARYPATHS=
LIBRARIES=

# Compile options
DEBUG_OPTION=-g
	OPTIMIZATION_OPTION=-O2
	

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/lib$(ProjectName).a
			
LINK=$(LIB_TOOL) -rs "$(TargetPath)" $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

	
###############################################################
# Configuration: Debug
###############################################################
ifeq "$(ConfigurationName)" "Debug"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -D_DEBUG -D_SERVERBUILD
INCLUDES=-I.  -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../libs/ltguimgr -I../libs/GameSpy -I../../libs/platform
LIBRARYPATHS=
LIBRARIES=

# Compile options
DEBUG_OPTION=-g
	

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/lib$(ProjectName).a
			
LINK=$(LIB_TOOL) -rs "$(TargetPath)" $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

	
###############################################################
# Configuration: Final
###############################################################
ifeq "$(ConfigurationName)" "Final"

# Output and Intermediate directories for this configuration
OutDir=./../../built/$(ConfigurationName)
IntDir=$(OutDir)/$(ProjectName)

# Defines, includes, and libraries
DEFINES=$(LINUX_DEFINES)  -DNDEBUG -D_SERVERBUILD -D_FINAL
INCLUDES=-I.  -I./ -I../../Engine/sdk/inc -I../../libs/stdlith -I../libs/ltguimgr -I../libs/GameSpy -I../../libs/platform
LIBRARYPATHS=
LIBRARIES=

# Compile options
OPTIMIZATION_OPTION=-O2
	

# Compile and Link commands
CPP_COMPILE=$(CPP_TOOL) -c $(DEBUG_OPTION) -pthread -fno-exceptions $(OPTIMIZATION_OPTION) $(RTTI_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)
CC_COMPILE=$(CC_TOOL) -c $(DEBUG_OPTION) -pthread $(OPTIMIZATION_OPTION) -o "$(IntDir)/$(*F).o" $(DEFINES) $(INCLUDES)

TargetPath=$(OutDir)/lib$(ProjectName).a
			
LINK=$(LIB_TOOL) -rs "$(TargetPath)" $(OBJECTFILES) $(LIBRARYPATHS) $(LIBRARIES)
    
MAKEDEPEND=$(CC_TOOL) $(DEFINES) $(INCLUDES) -w -MM -MP

# Build rules
$(TargetPath): $(IntDir) $(OutDir) $(OBJECTFILES)
	@($(SHELL) -ec 'if [ "$(filter '%.a', '$(TargetPath)')" != "" ]; then echo "Archiving $(TargetPath)"; else echo "Linking $(TargetPath)"; fi')
	@($(LINK))
	
	@(echo "Done.")

$(IntDir):
	@(mkdir -p "$(IntDir)")
	
$(OutDir):
	@(mkdir -p "$(OutDir)")

$(IntDir)/%.o:
	@(echo "Compiling $(filter %/$*.cpp %/$*.c, $(SOURCEFILES))")
	@($(SHELL) -ec 'if [ "$(filter %/$*.cpp, $(SOURCEFILES))" != "" ]; \
		then $(CPP_COMPILE) "$(filter %/$*.cpp, $(SOURCEFILES))"; \
		elif [ "$(filter %/$*.c, $(SOURCEFILES))" != "" ]; \
		then $(CC_COMPILE) "$(filter %/$*.c, $(SOURCEFILES))"; \
		fi')

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJECTFILES:%.o=%.d)
endif

$(IntDir)/%.d: $(IntDir)
	@($(SHELL) -ec 'if ! $(MAKEDEPEND) -MT "$(IntDir)/$(*F).o $(IntDir)/$(*F).d" "$(filter %/$*.cpp %/$*.c, $(SOURCEFILES))" > "$(IntDir)/$(*F).d"; then rm "$(IntDir)/$(*F).d"; fi')

clean:
	-rm -f $(TargetPath)
	-rm -f $(IntDir)/*.d
	-rm -f $(IntDir)/*.o
	
rebuild: clean $(TargetPath)
	
endif                               

	
