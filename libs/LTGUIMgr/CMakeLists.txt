cmake_minimum_required (VERSION 2.8)
project (LTGUIMgr)

add_library (LTGUIMgr STATIC
	ltguibutton.cpp
	ltguicolumnctrl.cpp
	ltguicommandhandler.cpp
	ltguictrl.cpp
	ltguicyclectrl.cpp
	ltguieditctrl.cpp
	ltguiframe.cpp
	ltguilargetext.cpp
	ltguilistctrl.cpp
	ltguislider.cpp
	ltguitextitemctrl.cpp
	ltguitoggle.cpp
	ltguiwindow.cpp)
add_definitions (-D_LITHTECH2 -DLT15_COMPAT)
include_directories (../../sdk/inc)