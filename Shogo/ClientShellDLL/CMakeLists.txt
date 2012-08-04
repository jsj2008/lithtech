cmake_minimum_required (VERSION 2.8)
project (ClientShellDLL)

add_subdirectory(../../Engine/libs/MFCStub ${CMAKE_CURRENT_BINARY_DIR}/MFCStub)
add_subdirectory(../../Engine/libs/stdlith ${CMAKE_CURRENT_BINARY_DIR}/stdlith)
add_subdirectory(../../Engine/libs/ButeMgr ${CMAKE_CURRENT_BINARY_DIR}/ButeMgr)
add_subdirectory(../../Engine/libs/CryptMgr ${CMAKE_CURRENT_BINARY_DIR}/CryptMgr)
add_subdirectory(../../Engine/libs/lith ${CMAKE_CURRENT_BINARY_DIR}/lith)
add_subdirectory(../../Engine/libs/RegMgr ${CMAKE_CURRENT_BINARY_DIR}/RegMgr)
#add_subdirectory(../Libs/LTGUIMgr ${CMAKE_CURRENT_BINARY_DIR}/LTGUIMgr)
#add_subdirectory(../Libs/ServerDir ${CMAKE_CURRENT_BINARY_DIR}/ServerDir)

add_library (ClientShellDLL SHARED
	AnimeLineFX.cpp
	BaseLineSystemFX.cpp
	BaseMenu.cpp
	BaseParticleSystemFX.cpp
	BaseScaleFX.cpp
	BitmapFont.cpp
	BulletTrailFX.cpp
	CastLineFX.cpp
	client_physics.cpp
	ClientInfoMgr.cpp
	ClientUtilities.cpp
	ClientWeaponUtils.cpp
	CMoveMgr.cpp
	Credits.cpp
	CreditsWin.cpp
	CriticalHitFX.cpp
	DeathFX.cpp
	DebrisFX.cpp
	DetailSettingsMenu.cpp
	DisplayModeMenu.cpp
	DisplayOptionsMenu.cpp
	DynamicLightFX.cpp
	ExplosionFX.cpp
	GibFX.cpp
	InfoDisplay.cpp
	IpMgr.cpp
	JoystickMenu.cpp
	JuggernautFX.cpp
	KeyboardMenu.cpp
	KeyFixes.cpp
	LaserCannonFX.cpp
	LightFX.cpp
	LightScaleMgr.cpp
	LineBallFX.cpp
	LoadLevelMenu.cpp
	LoadSavedLevelMenu.cpp
	MainMenu.cpp
	MarkSFX.cpp
	MessageBox.cpp
	MessageMgr.cpp
	MissionObjectives.cpp
	MouseMenu.cpp
	Music.cpp
	NetInfo.cpp
	NetStart.cpp
	NewGameMenu.cpp
	OptionsMenu.cpp
	ParticleExplosionFX.cpp
	ParticleSystemFX.cpp
	ParticleTrailFX.cpp
	ParticleTrailSegmentFX.cpp
	PickupItemFX.cpp
	plasma1.cpp
	plasma2.cpp
	plasma3.cpp
	plasma4.cpp
	PlayerCamera.cpp
	PlayerFX.cpp
	PlayerInventory.cpp
	PlayerStats.cpp
	PolyGridFX.cpp
	PopupMenu.cpp
	ProjectileFX.cpp
	RiotClientShell.cpp
	RiotMenu.cpp
	RiotSettings.cpp
	RocketFlareFX.cpp
	SaveLevelMenu.cpp
	SFXMgr.cpp
	SFXReg.cpp
	ShellCasingFX.cpp
	SinglePlayerMenu.cpp
	Slider.cpp
	SmokeFX.cpp
	SmokeSphereFX.cpp
	SoundOptionsMenu.cpp
	Sparam.cpp
	SparksFX.cpp
	SpriteFX.cpp
	TextHelper.cpp
	TracerFX.cpp
	VKDefs.cpp
	WeaponFX.cpp
	WeaponFXLow.cpp
	WeaponFXMed.cpp
	WeaponModel.cpp
	WeaponSoundFX.cpp
	WinUtil.cpp
	ClientShellDLL.rc
	../Shared/DebrisTypes.cpp
	../Shared/ModelFuncs.cpp
	../Shared/RiotCommonUtilities.cpp
	../Shared/WeaponDefs.cpp)
set_target_properties(ClientShellDLL 
	PROPERTIES OUTPUT_NAME CShell)
add_definitions (-D_CRT_SECURE_NO_WARNINGS -DRIOT_BUILD)
include_directories (ClientShellShared
	../Shared
	../../Engine/sdk/inc
	../../Engine/libs/MFCStub
	../../Engine/libs/stdlith
	../../Engine/libs/ButeMgr
	../../Engine/libs/CryptMgr
	../../Engine/libs/lith
	../../Engine/libs/RegMgr
	C:\\Program Files (x86)\\Microsoft DirectX SDK (February 2006)\\Include)
target_link_libraries(ClientShellDLL
	MFCStub
	Lib_StdLith
	ButeMgr
	CryptMgr
	Lib_Lith
	RegMgr)