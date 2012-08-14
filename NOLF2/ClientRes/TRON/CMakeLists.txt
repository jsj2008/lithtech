cmake_minimum_required (VERSION 2.8)
project (TRON_ClientRes)
add_library (TRON_ClientRes SHARED 
	main.cpp
	ClientRes.rc)
include_directories (../Shared
	../Shared/Lang/EN
	../../Shared/Lang/EN)
set_target_properties(TRON_ClientRes 
	PROPERTIES OUTPUT_NAME CRes)