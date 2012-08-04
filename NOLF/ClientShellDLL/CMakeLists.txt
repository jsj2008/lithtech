cmake_minimum_required (VERSION 2.8)
project (ClientShellDLL)

add_subdirectory(../../Engine/libs/MFCStub ${CMAKE_CURRENT_BINARY_DIR}/MFCStub)
add_subdirectory(../../Engine/libs/stdlith ${CMAKE_CURRENT_BINARY_DIR}/stdlith)
add_subdirectory(../../Engine/libs/ButeMgr ${CMAKE_CURRENT_BINARY_DIR}/ButeMgr)
add_subdirectory(../../Engine/libs/CryptMgr ${CMAKE_CURRENT_BINARY_DIR}/CryptMgr)
add_subdirectory(../../Engine/libs/lith ${CMAKE_CURRENT_BINARY_DIR}/lith)
add_subdirectory(../../Engine/libs/RegMgr ${CMAKE_CURRENT_BINARY_DIR}/RegMgr)
add_subdirectory(../../NOLF2/Libs/LTGUIMgr ${CMAKE_CURRENT_BINARY_DIR}/LTGUIMgr)
#add_subdirectory(../Libs/ServerDir ${CMAKE_CURRENT_BINARY_DIR}/ServerDir)

add_library (ClientShellDLL SHARED
	../Shared/Animator.cpp
	../Shared/AssertMgr.cpp
	../Shared/AttachButeMgr.cpp
	../Shared/BareList.cpp
	BaseLineSystemFX.cpp
	BaseMenu.cpp
	BaseParticleSystemFX.cpp
	BasePolyDrawFX.cpp
	BaseScaleFX.cpp
	BeamFX.cpp
	BodyFX.cpp
	BulletTrailFX.cpp
	CameraOffsetMgr.cpp
	CharacterFX.cpp
	client_physics.cpp
	ClientButeMgr.cpp
	ClientInfoMgr.cpp
	ClientSoundMgr.cpp
	ClientUtilities.cpp
	ClientWeaponUtils.cpp
	CMoveMgr.cpp
	../Shared/CommonUtilities.cpp
	../Shared/CRC32.cpp
	Credits.cpp
	DamageFXMgr.cpp
	../Shared/DamageTypes.cpp
	DeathFX.cpp
	DebrisFX.cpp
	../Shared/DebrisMgr.cpp
	../Shared/DebugNew.cpp
	DynamicLightFX.cpp
	ExplosionFX.cpp
	FireFX.cpp
	FlashLight.cpp
	../Shared/FXButeMgr.cpp
	../Shared/FXStructs.cpp
	../Shared/GameButeMgr.cpp
	GameClientShell.cpp
	GibFX.cpp
	GlobalClientMgr.cpp
	../Shared/GlobalMgr.cpp
	HeadBobMgr.cpp
	IntelItemMgr.cpp
	InterfaceMgr.cpp
	InterfaceMeter.cpp
	InterfaceTimer.cpp
	IpMgr.cpp
	KeyFixes.cpp
	LaserBeam.cpp
	LaserTriggerFX.cpp
	LeashFX.cpp
	LensFlareFX.cpp
	LightFX.cpp
	LightningFX.cpp
	LightScaleMgr.cpp
	LineSystemFX.cpp
	MarkSFX.cpp
	MessageBox.cpp
	MessageBoxCtrl.cpp
	MessageMgr.cpp
	MineFX.cpp
	../Shared/MissionMgr.cpp
	MissionText.cpp
	../Shared/ModelButeMgr.cpp
	MultiplayerMenu.cpp
	Music.cpp
	MuzzleFlashFX.cpp
	MuzzleFlashParticleFX.cpp
	../Shared/NetDefs.cpp
	NodeController.cpp
	NodeLinesFX.cpp
	ObjEditMgr.cpp
	ObjSpriteFX.cpp
	ParticleExplosionFX.cpp
	ParticleShowerFX.cpp
	ParticleSystemFX.cpp
	ParticleTrailFX.cpp
	ParticleTrailSegmentFX.cpp
	PickupItemFX.cpp
	PlayerCamera.cpp
	PlayerSoundFX.cpp
	PlayerStats.cpp
	../Shared/PlayerSummary.cpp
	PlayerVehicleFX.cpp
	PolyDebrisFX.cpp
	PolyGridFX.cpp
	PolyLineFX.cpp
	ProjectileFX.cpp
	RandomSparksFX.cpp
	ScreenTintMgr.cpp
	SearchLightFX.cpp
	../Shared/ServerOptionMgr.cpp
	SFXMgr.cpp
	../Shared/SharedFXStructs.cpp
	../Shared/SharedMission.cpp
	ShellCasingFX.cpp
	SmokeFX.cpp
	../Shared/SoundFilterMgr.cpp
	SoundFX.cpp
	../Shared/SoundMgr.cpp
	Sparam.cpp
	SpecialFXList.cpp
	sprinklesfx.cpp
	../Shared/StdAfx.cpp
	SteamFX.cpp
	../Shared/SurfaceFunctions.cpp
	../Shared/SurfaceMgr.cpp
	../Shared/TeamMgr.cpp
	../Shared/TemplateList.cpp
	TracerFX.cpp
	VehicleMgr.cpp
	../Shared/VersionMgr.cpp
	VideoFX.cpp
	VKDefs.cpp
	WeaponChooser.cpp
	WeaponFX.cpp
	../Shared/WeaponMgr.cpp
	WeaponModel.cpp
	WeatherFX.cpp
	WinUtil.cpp)
set_target_properties(ClientShellDLL 
	PROPERTIES OUTPUT_NAME CShell)
add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_CLIENTBUILD)
include_directories (../Shared 
	../../Engine/sdk/inc
	../../Engine/libs/MFCStub
	../../Engine/libs/stdlith
	../../Engine/libs/ButeMgr
	../../Engine/libs/CryptMgr
	../../Engine/libs/lith
	../../Engine/libs/RegMgr
	../../NOLF2/Libs/LTGUIMgr
	#../Libs/ServerDir
	../ClientRes)
target_link_libraries(ClientShellDLL
	MFCStub
	Lib_StdLith
	ButeMgr
	CryptMgr
	Lib_Lith
	RegMgr
	LTGUIMgr
	#ServerDir
	winmm.lib)