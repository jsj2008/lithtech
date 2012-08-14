cmake_minimum_required (VERSION 2.8)
project (NOLF2_ClientRes)
add_library (NOLF2_ClientRes SHARED 
	main.cpp
	Lang/EN/ClientRes.rc)
include_directories (../Shared
	../Shared/Lang/EN
	../../Shared/Lang/EN)
set_target_properties(NOLF2_ClientRes 
	PROPERTIES OUTPUT_NAME CRes)