# FIXME: Looks like the codes in Engine/clientfx is newer
cmake_minimum_required (VERSION 2.8)
project (ClientFXDLL)
add_library (ClientFXDLL SHARED
	BaseFX.cpp
	bouncychunkfx.cpp
	camjitterfx.cpp
	camwobblefx.cpp
	clientfx.cpp
	CreateFX.cpp
	cycletimer.cpp
	../Shared/DebugNew.cpp
	dynalightfx.cpp
	fallingstufffx.cpp
	FlareSpriteFX.cpp
	lightningfx.cpp
	LTBbouncychunkfx.cpp
	ltbmodelfx.cpp
	../../sdk/inc/ltmodule.cpp
	../../sdk/inc/ltquatbase.cpp
	nullfx.cpp
	ParticleSystemFX.cpp
	playrandomsoundfx.cpp
	playsoundfx.cpp
	polyfanfx.cpp
	polytubefx.cpp
	spritefx.cpp
	spritesystemfx.cpp
	stdafx.cpp)
set_target_properties(ClientFXDLL
	PROPERTIES OUTPUT_NAME ClientFx
	SUFFIX .fxd)
add_definitions (-D_CRT_SECURE_NO_WARNINGS -DNOPS2 -DMEMTRACK_CLIENTFX -D_NOMFC)
include_directories (../Shared 
	../../sdk/inc
	../../libs/MFCStub
	../../libs/stdlith)