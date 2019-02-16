cmake_minimum_required(VERSION 3.0)
project(node_libuiohook)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#############################
# CMake Settings
#############################

include(NodeJS)
include(CPack)

nodejs_init()

#############################
# Source, Libraries & Directories
#############################
SET(PROJECT_SOURCE 
	"${PROJECT_SOURCE_DIR}/source/hook.cpp" 
	"${PROJECT_SOURCE_DIR}/source/hook.h"
	"${PROJECT_SOURCE_DIR}/source/module.cpp"
)

SET(PROJECT_LIBRARIES
	${NODEJS_LIBRARIES}
)

SET(PROJECT_INCLUDE_PATHS
	${NODEJS_INCLUDE_DIRS}
	"${CMAKE_SOURCE_DIR}/source/"
)

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

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE BUILDING_NODE_EXTENSION
)

if(WIN32)
	target_compile_definitions(node_libuiohook
		PRIVATE
		-D_CRT_SECURE_NO_WARNINGS
		-DUNICODE
		-D_UNICODE)
endif()

TARGET_INCLUDE_DIRECTORIES(
	${PROJECT_NAME}
	PUBLIC ${PROJECT_INCLUDE_PATHS}
)

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
INSTALL(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ./ OPTIONAL)


# JavaScript
INSTALL(
	FILES "${CMAKE_SOURCE_DIR}/package.json"
	DESTINATION "./"
)
INSTALL(
	FILES "${CMAKE_SOURCE_DIR}/main.js"
	DESTINATION "./"
)
