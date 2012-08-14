cmake_minimum_required (VERSION 2.8)
project (Lib_UI)

set(libsources
	src/cuibase.cpp
	src/cuibase_impl.cpp
	src/cuibitmapfont.cpp
	src/cuibutton.cpp
	src/cuibutton_impl.cpp
	src/cuicheck.cpp
	src/cuicheck_impl.cpp
	src/cuidropdownlist.cpp
	src/cuidropdownlist_impl.cpp
	src/cuifont_impl.cpp
	src/cuifontmanager.cpp
	src/cuiformattedpolystring.cpp
	src/cuiformattedpolystring_impl.cpp
	src/cuiinterval.cpp
	src/cuiinterval_impl.cpp
	src/cuilinklist.cpp
	src/cuilist.cpp
	src/cuilist_impl.cpp
	src/cuimessagequeue.cpp
	src/cuioption.cpp
	src/cuioption_impl.cpp
	src/cuipolystring.cpp
	src/cuipolystring_impl.cpp
	src/cuipolytex.cpp
	src/cuiprogress.cpp
	src/cuiprogress_impl.cpp
	src/cuiredblack.cpp
	src/cuirenderstate.cpp
	src/cuislider.cpp
	src/cuislider_impl.cpp
	src/cuistaticimage.cpp
	src/cuistaticimage_impl.cpp
	src/cuistatictext.cpp
	src/cuistatictext_impl.cpp
	src/cuivectorfont.cpp
	src/cuiwidget.cpp
	src/cuiwidget_impl.cpp
	src/cuiwidgetmanager.cpp
	src/cuiwindow.cpp
	src/cuiwindow_impl.cpp)
	
if(ENABLE_D3D)
	set(libsources ${libsources}
		src/sys/d3d/cuidebug.cpp)
	include_directories(sys/d3d)
endif(ENABLE_D3D)

include_directories (src
	../../sdk/inc
	../../libs/stdlith
	../shared/src
	../shared/src/sys/win
	../kernel/src
	../kernel/mem/src
	../kernel/src/sys/win)

add_library(Lib_UI STATIC ${libsources})