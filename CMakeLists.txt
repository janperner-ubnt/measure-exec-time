##########
# project

cmake_minimum_required(VERSION 3.9)

project(measure-exec-time CXX)
set(PROJECT_VERSION 0.0.1)

######################################
# determine git commit of the sources

find_package(Git)
if(GIT_EXECUTABLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
	execute_process(COMMAND "${GIT_EXECUTABLE}" describe --always
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		OUTPUT_VARIABLE PROJECT_GIT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git_revision")
	file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/.git_revision" PROJECT_GIT_REVISION LIMIT_INPUT 10 NEWLINE_CONSUME)
endif()
if(NOT PROJECT_GIT_VERSION)
	if(PROJECT_GIT_REVISION)
		set(PROJECT_GIT_VERSION "${PROJECT_VERSION}-g${PROJECT_GIT_REVISION}")
	else()
		set(PROJECT_GIT_VERSION "${PROJECT_VERSION}-no-git")
	endif()
endif()

######################
# compilation options

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.9)
	set(CMAKE_CXX_FLAGS "-fdiagnostics-color=auto ${CMAKE_CXX_FLAGS}")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "-s -Os -DNDEBUG=1 -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -DNDEBUG=1")

set(SOURCES src/foo.cpp)

#############################
# libraries and dependencies

add_library(everything_but_main STATIC ${SOURCES})
target_link_libraries(everything_but_main pthread)
add_executable(measure-exec-time src/main.cpp)
target_link_libraries(measure-exec-time everything_but_main)

include_directories("src")

###############
# code quality

option(BE_PEDANTIC "Run compilation with -Wpedantic flag" ON)
if(BE_PEDANTIC)
	set(CMAKE_CXX_FLAGS "-Wpedantic ${CMAKE_CXX_FLAGS}")
endif()

option(CLANG_TIDY "Run clang-tidy on the source tree" ON)
if(CLANG_TIDY)
	find_program(
		CLANG_TIDY_EXE
		NAMES "clang-tidy"
		DOC "Path to clang-tidy executable")
	if(NOT CLANG_TIDY_EXE)
		message(FATAL_ERROR "CLANG_TIDY=ON but clang-tidy not found.")
	else()
		message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
		set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
		set(DO_CLANG_TIDY ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} -config= -format-style=file -quiet)
	endif()
	set_target_properties(
		everything_but_main PROPERTIES
		CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
	set_target_properties(
		measure-exec-time PROPERTIES
		CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

option(CLANG_FORMAT "Run clang-format on the source tree" ON)
if(CLANG_FORMAT)
	find_program(
		CLANG_FORMAT_EXE
		NAMES "clang-format"
		DOC "Path to clang-format executable")
	if(NOT CLANG_FORMAT_EXE)
		message(FATAL_ERROR "CLANG_FORMAT=ON but clang-format not found.")
	else()
		file(GLOB_RECURSE HEADERS "src/*.hpp")
		message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
		add_custom_target(
			run_clang_format
			COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${SOURCES} ${HEADERS} src/main.cpp
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		add_dependencies(everything_but_main run_clang_format)
	endif()
endif()

install(TARGETS measure-exec-time DESTINATION bin)
if(SYMBOLS_DIRECTORY)
	install(CODE "MESSAGE(\"-- Installing debugging symbols to ${SYMBOLS_DIRECTORY}\")")
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${SYMBOLS_DIRECTORY})")
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different measure-exec-time ${SYMBOLS_DIRECTORY}/)")
endif()

##########
# packing
#
# Can be used to generate .tar.gz archive of the installation files to be
# simply extracted to / on target device.

set(CPACK_SET_DESTDIR ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
include(CPack)
