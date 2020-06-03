cmake_minimum_required(VERSION 3.0)
project(node_libuiohook)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#############################
# CMake Settings
#############################

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v6.0.3" CACHE STRING "Node.JS Version")

include(NodeJS)
include(CPack)

nodejs_init()

if(APPLE AND NOT UIOHOOKDIR)
	message(FATAL_ERROR "UIOHOOKDIR variable is missing.  Please set this variable to specify path to the build dir of libuiohook")
endif()

message (STATUS "Path to uiohook dir " ${UIOHOOKDIR}/include/)

#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
	"${PROJECT_SOURCE_DIR}/source/hook.h"
	"${PROJECT_SOURCE_DIR}/source/module.cpp"
)

IF (WIN32)
	list(APPEND PROJECT_SOURCE "${PROJECT_SOURCE_DIR}/source/hook-win.cpp")
ELSEIF (APPLE)
	list(APPEND PROJECT_SOURCE "${PROJECT_SOURCE_DIR}/source/hook-osx.cpp")
ENDIF ()

SET(PROJECT_LIBRARIES
	${NODEJS_LIBRARIES}
)

SET(PROJECT_INCLUDE_PATHS
	${NODEJS_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/source/"
)

if(APPLE)
	list(APPEND PROJECT_INCLUDE_PATHS "${UIOHOOKDIR}/include/")
	list(APPEND PROJECT_LIBRARIES "${UIOHOOKDIR}/lib/libuiohook.dylib")
endif()

#############################
# Building
#############################
add_nodejs_module(
	${PROJECT_NAME}
	${PROJECT_SOURCE}
)

TARGET_LINK_LIBRARIES(
	${PROJECT_NAME}
	${PROJECT_LIBRARIES}
)

TARGET_INCLUDE_DIRECTORIES(
	${PROJECT_NAME}
	PUBLIC ${PROJECT_INCLUDE_PATHS}
)
target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE BUILDING_NODE_EXTENSION
)

set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

if(WIN32)
	target_compile_definitions(node_libuiohook
		PRIVATE
		-D_CRT_SECURE_NO_WARNINGS
		-DUNICODE
		-D_UNICODE)
endif()

SET_TARGET_PROPERTIES(
	${PROJECT_NAME}
	PROPERTIES
	PREFIX ""
	SUFFIX ".node"
)

#############################
# Distribute
#############################
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
)

if(WIN32)
	INSTALL(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ./ OPTIONAL)
endif()

if(APPLE)
INSTALL (
	DIRECTORY "${UIOHOOKDIR}/lib/"
	DESTINATION "./"
	FILES_MATCHING PATTERN "*.dylib"
)
endif()

# JavaScript
INSTALL(
	FILES "${CMAKE_SOURCE_DIR}/package.json"
	DESTINATION "./"
)
INSTALL(
	FILES "${CMAKE_SOURCE_DIR}/main.js"
	DESTINATION "./"
)
