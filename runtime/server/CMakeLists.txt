cmake_minimum_required (VERSION 2.8)
project (DLL_Server)

add_definitions(-DDE_SERVER_COMPILE -DDIRECTENGINE_COMPILE -DSTDLITH_ALLOC_OVERRIDE -DLT15_COMPAT -DNO_PRAGMA_LIBS)

set(libsources
	src/server_iltcommon.cpp
	../shared/src/shared_iltcommon.cpp
	../shared/src/modellt_impl.cpp
	src/server_iltmodel.cpp
	src/server_iltphysics.cpp
	../shared/src/shared_iltphysics.cpp
	src/server_iltsoundmgr.cpp
	../shared/src/transformlt_impl.cpp
	src/world_server_bsp.cpp
	../world/src/world_shared_bsp.cpp
	../shared/src/interface_linkage.cpp
	../model/src/animtracker.cpp
	../shared/src/bdefs.cpp
	../kernel/src/sys/win/bindmgr.cpp
	../shared/src/classbind.cpp
	src/classmgr.cpp
	../shared/src/collision.cpp
	../shared/src/compress.cpp
	../shared/src/concommand.cpp
	../shared/src/conparse.cpp
	../model/src/sys/d3d/d3d_model_load.cpp
	../world/src/de_mainworld.cpp
	../world/src/de_nodes.cpp
	../world/src/de_objects.cpp
	../kernel/src/debugging.cpp
	../shared/src/dhashtable.cpp
	../kernel/src/sys/win/dutil.cpp
	../shared/src/engine_vars.cpp
	../shared/src/findobj.cpp
	../shared/src/ftserv.cpp
	../world/src/fullintersectline.cpp
	src/game_serialize.cpp
	../shared/src/gamemath.cpp
	../shared/src/genericprop_setup.cpp
	../shared/src/genltstream.cpp
	../shared/src/geometry.cpp
	../shared/src/geomroutines.cpp
	../shared/src/impl_common.cpp
	src/interlink.cpp
	../world/src/intersect_line.cpp
	../world/src/intersectsweptsphere.cpp
	../shared/src/leech.cpp
	../world/src/light_table.cpp
	../shared/src/lightmap_planes.cpp
	../kernel/net/src/localdriver.cpp
	../shared/src/ltmessage.cpp
	src/ltmessage_server.cpp
	../../sdk/inc/ltmodule.cpp
	../../sdk/inc/ltobjref.cpp
	../../sdk/inc/ltquatbase.cpp
	../shared/src/lttimer.cpp
	../model/src/model.cpp
	../model/src/model_load.cpp
	../model/src/modelallocations.cpp
	../shared/src/motion.cpp
	../shared/src/moveobject.cpp
	../shared/src/moveplayer.cpp
	../kernel/net/src/netmgr.cpp
	../shared/src/nexus.cpp
	../shared/src/objectmgr.cpp
	../kernel/net/src/packet.cpp
	../shared/src/parse_world_info.cpp
	../shared/src/ratetracker.cpp
	src/s_client.cpp
	src/s_concommand.cpp
	src/s_intersect.cpp
	src/s_net.cpp
	src/s_object.cpp
	src/server_consolestate.cpp
	src/server_extradata.cpp
	src/server_filemgr.cpp
	../kernel/src/server_interface.cpp
	src/server_loaderthread.cpp
	src/serverde_impl.cpp
	src/serverevent.cpp
	src/servermgr.cpp
	src/sloaderthread.cpp
	src/smoveabstract.cpp
	../sound/src/sounddata.cpp
	src/soundtrack.cpp
	../shared/src/spritecontrolimpl.cpp
	../shared/src/stacktrace.cpp
	../shared/src/stdlterror.cpp
	../kernel/src/sys/win/stringmgr.cpp
	../shared/src/strtools.cpp
	../model/src/transformmaker.cpp
	../kernel/net/src/sys/win/udpdriver.cpp
	../sound/src/wave.cpp
	../world/src/world_blind_object_data.cpp
	../world/src/world_blocker_data.cpp
	../world/src/world_blocker_math.cpp
	../world/src/world_particle_blocker_data.cpp
	../world/src/world_tree.cpp)

if(WIN32)
	SET(libsources ${libsources}
		../kernel/src/sys/win/counter.cpp
		../kernel/io/src/sys/win/de_file.cpp
		../kernel/mem/src/sys/win/de_memory.cpp
		../kernel/src/sys/win/dsys_interface.cpp
		../kernel/src/sys/win/lthread.cpp
		../kernel/src/sys/win/ltlibraryloader.cpp
		../kernel/src/sys/win/ltthread.cpp
		../kernel/src/sys/win/streamsim.cpp
		../kernel/src/sys/win/systeminfo.cpp
		../kernel/src/sys/win/timemgr.cpp)
else(WIN32)
	SET(libsources ${libsources}
		../kernel/src/sys/linux/counter.cpp
		../kernel/io/src/sys/linux/linuxfile.cpp
		../kernel/mem/src/sys/linux/de_memory.cpp
		../kernel/mem/src/sys/linux/linuxundata.cpp
		../kernel/src/sys/linux/linuxdsys.cpp
		../kernel/src/sys/linux/lthread.cpp
		../kernel/src/sys/linux/ltlibraryloader.cpp
		../kernel/src/sys/linux/ltthread.cpp
		../kernel/src/sys/linux/streamsim.cpp
		../kernel/src/sys/linux/timemgr.cpp)
endif(WIN32)

include_directories(src
	../../sdk/inc
	../../sdk/inc/physics
	../../libs/stdlith
	../../libs/rezmgr
	../../libs/lith
	../shared/src
	../kernel/src
	../kernel/mem/src
	../world/src
	../kernel/io/src
	../kernel/net/src
	../model/src
	../client/src
	../sound/src
	../shared/src/sys/win
	../lithtemplate
	../physics/src
	../info/src
	../render_b/src
	../ui/src
	../controlfilemgr)
	
if(NOT MSVC)	# FIXME: this is a hack for geomroutines.h:309
	include_directories(../../tools/PreProcessor)
endif(NOT MSVC)

if(WIN32)
	include_directories(../kernel/src/sys/win
		../shared/src/sys/win)
endif(WIN32)

if(ENABLE_D3D)
	include_directories(../render_a/src/sys/d3d)
endif(ENABLE_D3D)

if(ENABLE_DIRECTX)	# FIXME: set to find DirectX path
	include_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (February 2006)\\Include")
endif(ENABLE_DIRECTX)
	
add_library (DLL_Server SHARED ${libsources})

set_target_properties(DLL_Server
	PROPERTIES OUTPUT_NAME Server)
target_link_libraries(DLL_Server
	Lib_RezMgr
	Lib_Info
	Lib_Lith
	Lib_StdLith
	ltmem)
	
if(WIN32)
	target_link_libraries(DLL_Server 
		winmm
		ws2_32)
endif(WIN32)

if(MSVC)
	set_target_properties(DLL_Server PROPERTIES COMPILE_FLAGS "/wd4996")
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCXX)
	set_target_properties(DLL_Server PROPERTIES COMPILE_FLAGS "-fpermissive")
endif(CMAKE_COMPILER_IS_GNUCXX)