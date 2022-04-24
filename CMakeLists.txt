cmake_minimum_required(VERSION 3.4)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

project(filesystem_stdio)

set(CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
)

foreach(CompilerFlag ${CompilerFlags})
	string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

remove_definitions(-D_UNICODE -DUNICODE)
add_definitions(-D_MBCS -D_CRT_SECURE_NO_WARNINGS -DFILESYSTEMSTDIO_EXPORTS)

include_directories(${CMAKE_SOURCE_DIR}/ssdk2005)
include_directories(${CMAKE_SOURCE_DIR}/source)

set(SOURCES_SSDK2005
	ssdk2005/filesystem.h
	ssdk2005/IAppSystem.h
	ssdk2005/interface.cpp
	ssdk2005/interface.h
	ssdk2005/zip_uncompressed.h
)

set(SOURCES_LIBRARY
	source/CBspArchive.cpp
	source/CBspArchive.h
	source/CBspInternalFile.cpp
	source/CBspInternalFile.h
	source/CFile.cpp
	source/CFile.h
	source/CFileSystem.cpp
	source/CFileSystem.h
	source/CFileSystemAppSystem.cpp
	source/CFileSystemBase.cpp
	source/CFileSystemPrivate.cpp
	source/CFinder.cpp
	source/CFinder.h
	source/CPath.cpp
	source/CPath.h
	source/CPathList.cpp
	source/CPathList.h
	source/ExLib.cpp
	source/ExLib.h
	source/IFile.h
)

source_group("Source SDK 2005" FILES ${SOURCES_SSDK2005})
source_group("Source Files" FILES ${SOURCES_LIBRARY})

set(SOURCES
	${SOURCES_SSDK2005}
	${SOURCES_LIBRARY}
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

