# ./CMakeLists.txt
cmake_minimum_required(VERSION 3.11)
project(libmge VERSION 0.1.0 LANGUAGES C)
include(GenerateExportHeader)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#####################################################
# MGE source and include files

set(MGE_SOURCE
	"src/mge/entry.c"
	"src/mge/config.c"
	"src/mge/log.c"
	"src/mge/resource/manager.c"
	"src/mge/resource/text.c"
	"src/mge/resource/shader.c"
	"src/mge/scene/manager.c"
	"src/mge/scene/node.c"
	"src/mge/graphics/manager.c"
	"src/mge/graphics/mesh_renderable.c"
)

set(MGE_INCLUDE
	"include/mge/game.h"
	"include/mge/config.h"
	"include/mge/log.h"
	"include/mge/resource/manager.h"
	"include/mge/resource/text.h"
	"include/mge/resource/shader.h"
	"include/mge/scene/manager.h"
	"include/mge/scene/node.h"
	"include/mge/scene/component.h"
	"include/mge/graphics/manager.h"
	"include/mge/graphics/material.h"
	"include/mge/graphics/mesh_renderable.h"
)

#####################################################
# Set options
set(MGE_VERBOSE_LEVEL "3" CACHE STRING "Verbose level (0 = no verbose, 1 = verbose, 2 = very verbose, 3 = debug")

#####################################################
# Create MGE target and set its properties

add_library(mge STATIC ${MGE_SOURCE} ${MGE_INCLUDE})
set_property(TARGET mge PROPERTY C_STANDARD 11)
add_library(MGE::mge ALIAS mge)

# Link MGL
find_package(MGL REQUIRED)
target_link_libraries(mge PUBLIC MGL::MGL)

# Link MRL
find_package(MRL REQUIRED)
target_link_libraries(mge PUBLIC MRL::MRL)

generate_export_header(mge)
target_include_directories(mge
	PUBLIC
		$<INSTALL_INTERFACE:include/>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(UNIX)
    target_link_libraries(mge INTERFACE m dl pthread) # Link math lib
    find_package(X11 REQUIRED)
    target_include_directories(mge PUBLIC ${X11_INCLUDE_DIR})
    target_link_libraries(mge PUBLIC ${X11_LIBRARIES})
endif()
if(!MSVC)
    target_compile_options(mge INTERFACE -msse4.1)
endif()

# Add compilation definitions
target_compile_definitions(mge
	PUBLIC
		MGE_VERSION="${PROJECT_VERSION}"
		MGE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
		MGE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
		MGE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
		MGE_VERBOSE_LEVEL=${MGE_VERBOSE_LEVEL}
		MGE_EXAMPLES_DATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/example-data"
	PRIVATE
		MGE_API_EXPORT
)

# Add file filters
foreach(_source IN ITEMS ${MGE_SOURCE} ${MGE_INCLUDE})
	if (IS_ABSOLUTE "${_source}")
		file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
	else()
		set(_source_rel "${_source}")
	endif()
	get_filename_component(_source_path "${_source_rel}" PATH)
	string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
	source_group("${_source_path_msvc}" FILES "${_source}")
endforeach()

#####################################################
# Installation instructions

include (GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/MGE)

install(TARGETS mge
    EXPORT mge-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(mge PROPERTIES EXPORT_NAME MGE)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets
install(EXPORT mge-targets
	FILE
		MGETargets.cmake
	NAMESPACE
		MGE::
	DESTINATION
		${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MGEConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/MGEConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MGEConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the Config, ConfigVersion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MGEConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MGEConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

##############################################
# Exporting from the build tree

export(EXPORT mge-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/MGETargets.cmake NAMESPACE MGE::)

# Register package in user's package registry
export(PACKAGE MGE)

##############################################
# Build examples
option(MGE_BUILD_EXAMPLES ON)
if(MGE_BUILD_EXAMPLES)
    file(GLOB_RECURSE files "src/examples/*.c")
    foreach(file ${files})
	string(REGEX REPLACE "(^.*\\/|\\.[^.]*$)" "" file_without_ext ${file})
	set(file_without_ext example_${file_without_ext})
	add_executable(${file_without_ext} ${file})
	target_link_libraries(${file_without_ext} mge)
	set_target_properties(${file_without_ext} PROPERTIES FOLDER Examples)
	install(TARGETS ${file_without_ext}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
	)
endforeach()
endif()
