cmake_minimum_required(VERSION 3.0)
project(node_libuiohook)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(NodeJS)

nodejs_init()

set(node_libuiohook_HEADERS ${PROJECT_SOURCE_DIR}/source/hook.h)

set(node_libuiohook_SOURCE
	${PROJECT_SOURCE_DIR}/source/hook.cpp
	${PROJECT_SOURCE_DIR}/source/module.cpp)

add_nodejs_module(node_libuiohook
	${node_libuiohook_SOURCE}
	${node_libuiohook_HEADERS}
	${NODEJS_SOURCES})

set(node_libuiohook_EXTRAS
	"${node_libuiohook_SOURCE_DIR}/main.js"
	"${node_libuiohook_SOURCE_DIR}/package.json")

if(WIN32)
	target_compile_definitions(node_libuiohook
		PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(node_libuiohook
	${LIBUIOHOOK_LIBRARIES}
	${NODEJS_LIBRARIES})

target_include_directories(node_libuiohook
	PUBLIC ${NODEJS_INCLUDE_DIRS} ${UIOHOOKPATH}/include/)

install(
	FILES $<TARGET_PDB_FILE:node_libuiohook>
	DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)

install(
	FILES $<TARGET_FILE:node_libuiohook>
	DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	FILES ${node_libuiohook_EXTRAS}
	DESTINATION ${CMAKE_INSTALL_PREFIX})
