cmake_minimum_required (VERSION 2.8)
project (WONAPI)

add_library (WONAPI STATIC
	WONAuth/Auth.cpp
	WONCommon/Common.cpp
	WONCrypt/Crypt.cpp
	WONDB/DB.cpp
	WONDir/Dir.cpp
	WONMisc/gqueryreporting.c
	WONMisc/Misc.cpp
	WONMisc/nonport.c
	WONRouting/Routing.cpp
	WONServer/Server.cpp
	WONSocket/Socket.cpp
	WONAPI.cpp
	WONStatus.cpp)
add_definitions (-D__WON_MASTER_CPP_BUILD__)
include_directories (.
	WONAuth
	WONCommon
	WONConfig
	WONCrypt
	WONDB
	WONDir
	WONMisc
	WONRouting
	WONServer
	WONSocket
	WONUpdate)
if(MSVC)
	set_target_properties(WONAPI PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)