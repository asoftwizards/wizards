cmake_minimum_required (VERSION 2.4.7)
project (GPN)
set (BUILD_SHARED_LIBS true)
SET (BOOST_ROOT "/usr/local/boost/1.55.0" )

set (CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/gpn)

link_directories(/usr/local/effi/lib /usr/local/reporter/lib /usr/local/aofficeconnector/lib /usr/lib64/libreoffice/sdk/lib)
include_directories(/usr/local/effi/include /usr/local/reporter/include /usr/include/libxml2 /usr/local/aofficeconnector/include /usr/lib64/libreoffice/sdk/include /usr/lib64/libreoffice/sdk/inc)
if(EXISTS /usr/lib/libreoffice/basis-link/sdk/lib)
include_directories(/usr/lib/libreoffice/sdk/include /usr/lib/libreoffice/sdk/inc /usr/lib/libreoffice/basis-link/sdk/include /usr/lib/libreoffice/basis-link/sdk/inc)
endif (EXISTS /usr/lib/libreoffice/basis-link/sdk/lib)


include (/usr/local/effi/util/generate.cmake)
set_www_root(.)

set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} /usr/local/GPN/lib)

effi_project(GPN.xml mysql postgresql)
add_definitions(-DCPPU_ENV=gcc4)
add_definitions(-DUNX -DSAL_UNX)


add_library (GPN gpn.cc scheme.h scheme.cc gpn_auto.h gpn_auto.cc )
target_link_libraries (GPN -lReporter -ldbservice -ldatalist -lEvent -lAuthorizer -lacommon -lqpdf -lgd -fPIC)

# libraries
install (TARGETS GPN LIBRARY DESTINATION lib)
# locales
install (DIRECTORY dbpatches DESTINATION .)

#install (DIRECTORY wnd/context. DESTINATION wnd)
# custom css and xslt
install (DIRECTORY doc/css-GPN DESTINATION doc)
install (DIRECTORY doc/js-GPN DESTINATION doc)
install (DIRECTORY doc/images-GPN DESTINATION doc)
# install (DIRECTORY doc/help DESTINATION doc)
# install (DIRECTORY doc/skins/mvd DESTINATION doc/skins)

install (FILES
	locale.ru_RU.GPN
	my.cnf
	odbc.ini
	selinux
	ElementsInit.GPN.mysql
	CONFIG.example
	gpn_logrotate
	effi-version.GPN-01_01_01
	reporter-version.GPN-01_01_01
	aofficeconnector-version.GPN-01_01_01
	version
	arial.ttf
	DESTINATION .
)

install (FILES
	doc/gpn.html
	DESTINATION doc
)

install (FILES
	GPNautoTYPE.xml
	DESTINATION doc/types
)

# scripts
install (PROGRAMS
	post-install.sh
	chkconfig_for_scripts.sh
	cp_needing_file.sh
	pre_uninstall.sh
	ooffice
	gpn
	gpn_lib_include.sh
	start.sh
	process-watcher.sh
	stop.sh
	db_dump
	gpn-patch.sh
	XLS2XML.sh
	DESTINATION .
)

install (PROGRAMS db_dump DESTINATION bin)

add_custom_target(gcomplexsymlinks ALL
	COMMAND ${BASH} init-env.sh
	DEPENDS GPN
	COMMENT "GPN database initialization scripts"
)

add_custom_target (package
	COMMAND QA_RPATHS=65535 rpmbuild -bb gpn.spec --define "_topdir $$RPMROOT"
	COMMENT "RPM Packages")
