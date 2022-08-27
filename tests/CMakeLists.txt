cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName ${ProjectName}_tests)
project(${ProjectName})

include(FetchContent)

cmake_path(APPEND FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}" "deps_content" "${CMAKE_GENERATOR_NAME_WITHOUT_SPACES}")

FetchContent_Declare(
    googletest
	  GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG        c0e032efe587efa99413dd9ac54c3953a7915588
)

FetchContent_Declare(
    type_name
    GIT_REPOSITORY https://github.com/rokoDev/type_name.git
    GIT_TAG        0c2371e539789836ed76901a31ce7554ccbf690a
)

# If necessary it's possible to use VersionHelper from different location instead of downloaded from GitHub
# string(TOUPPER type_name UP_type_name)
# set(FETCHCONTENT_SOURCE_DIR_${UP_type_name} ${CMAKE_SOURCE_DIR}/../type_name)

FetchContent_MakeAvailable(googletest type_name)

add_library(tests_main STATIC)
target_sources(tests_main
  PRIVATE
  src/main_tests.cpp
  )
target_link_libraries(tests_main PUBLIC gtest type_name)
get_target_property(tests_main_sources tests_main SOURCES)
source_group(
  TREE   ${CMAKE_CURRENT_SOURCE_DIR}/src
  FILES  ${tests_main_sources}
)

if (WIN32)
	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
	option(GTEST_DISABLE_PTHREADS "Disable uses of pthreads in gtest." ON)
  option(GTEST_FORCE_SHARED_CRT "Use shared (DLL) run-time lib even when Google Test is built as static lib." ON)
endif (WIN32)


macro(package_add_test TESTNAME)
  add_executable(${TESTNAME} "")
  target_compile_definitions(${TESTNAME} PRIVATE $<UPPER_CASE:$<CONFIG>>)
  target_sources(${TESTNAME} PRIVATE ${ARGN})
  target_link_libraries(${TESTNAME} PRIVATE buffer tests_main)
  add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
  set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)

  # Create groups in the IDE which mirrors directory structure on the hard disk
  get_target_property(buffer_tests_src ${TESTNAME} SOURCES)
  source_group(
    TREE   ${CMAKE_CURRENT_SOURCE_DIR}#/src
    FILES  ${buffer_tests_src}
  )
  # If colours in googletest are not disabled then in some versions of commandline tools which is used in Xcode 13.3 this
  # can lead to output with escape sequences. Because of that its better to disable colours in googletest.
  set_target_properties(${TESTNAME} PROPERTIES XCODE_GENERATE_SCHEME ON)
  set_target_properties(${TESTNAME} PROPERTIES XCODE_SCHEME_ARGUMENTS "--gtest_color=no")
endmacro()

package_add_test(${ProjectName}
	src/buffer_tests.cpp
	)

# If use IDE add gtest, gmock, gtest_main and gmock_main targets into deps/googletest group
set_target_properties(gtest gmock gtest_main gmock_main PROPERTIES FOLDER deps/googletest)
set_target_properties(type_name PROPERTIES FOLDER deps)
