set(cmake_version 3.15.7)

cmake_minimum_required(VERSION ${cmake_version})

include(FetchContent)

# Download toolchains and make available
FetchContent_Declare(
    cmake-toolchains
    GIT_REPOSITORY https://github.com/rokoDev/cmake-toolchains.git
    GIT_TAG        baec2846aef97138ef9d693ee7162d23098aed73
)

FetchContent_MakeAvailable(cmake-toolchains)

set(ProjectName "buffer")

project(${ProjectName}
		VERSION 0.1.1
		LANGUAGES CXX
        DESCRIPTION "Buffers without exceptions.")

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# To organize targets into groups when using IDE like XCode or VisualStudio, etc.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Create library
add_library(buffer INTERFACE)

# Add dependencies
add_subdirectory(deps)

# Add sources, definitions, link dependencies, etc.
add_subdirectory(sources)

# Create groups in the IDE which mirrors directory structure on the hard disk
get_target_property(buffer_sources buffer SOURCES)
source_group(
  TREE   ${CMAKE_CURRENT_SOURCE_DIR}/sources/include
  FILES  ${buffer_sources}
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IS_TOP_LVL_PROJECT TRUE)
else()
    set(IS_TOP_LVL_PROJECT FALSE)
endif()

if(BUFFER_TEST OR IS_TOP_LVL_PROJECT)
    # Setup testing
    add_subdirectory(tests)
endif()
