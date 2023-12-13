cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName ${ProjectName}_tests)
project(${ProjectName})

include(FetchContent)

FetchContent_Declare(
    googletest
	  GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG        c0e032efe587efa99413dd9ac54c3953a7915588
)

# If necessary it's possible to use type_name from different location instead of downloaded from GitHub
# string(TOUPPER type_name UP_type_name)
# set(FETCHCONTENT_SOURCE_DIR_${UP_type_name} ${CMAKE_SOURCE_DIR}/../type_name)

FetchContent_MakeAvailable(googletest)

add_library(tests_main STATIC)
target_sources(tests_main
  PRIVATE
  src/main_tests.cpp
  )
target_link_libraries(tests_main PUBLIC gtest)
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


function(package_add_test)
  set(prefix ARG)
  set(noValues)
  set(singleValues TEST_TARGET_NAME)
  set(multiValues
    TEST_SOURCES
    EXTRA_TARGETS
    DEFS
  )

  cmake_parse_arguments(${prefix}
                        "${noValues}"
                        "${singleValues}"
                        "${multiValues}"
                        ${ARGN})

  foreach(arg IN LISTS noValues singleValues multiValues)
      set(${arg} ${${prefix}_${arg}})
  endforeach()

  add_executable(${TEST_TARGET_NAME})
  target_sources(${TEST_TARGET_NAME} PRIVATE ${TEST_SOURCES})
  target_include_directories(${TEST_TARGET_NAME} PUBLIC include)
  foreach(target_to_link IN LISTS EXTRA_TARGETS)
      target_link_libraries(${TEST_TARGET_NAME} PUBLIC ${target_to_link})
  endforeach()

  foreach(define IN LISTS DEFS)
      target_compile_definitions(${TEST_TARGET_NAME} PRIVATE ${define})
  endforeach()
  add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})

  # Create groups in the IDE which mirrors directory structure on the hard disk
  get_target_property(tests_src ${TEST_TARGET_NAME} SOURCES)
  source_group(
    TREE   ${CMAKE_CURRENT_SOURCE_DIR}#/src
    FILES  ${tests_src}
  )

  # Place all tests targets under "test" source group in IDE
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES FOLDER tests)

  # If colours in googletest are not disabled then in some versions of commandline tools which is used in Xcode 13.3 this
  # can lead to output with escape sequences. Because of that its better to disable colours in googletest.
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES XCODE_GENERATE_SCHEME ON)
  set_target_properties(${TEST_TARGET_NAME} PROPERTIES XCODE_SCHEME_ARGUMENTS "--gtest_color=no")
endfunction()

set(test_src
  src/buffer_tests.cpp
  )

package_add_test(
  TEST_TARGET_NAME buffer_tests
  TEST_SOURCES ${test_src}
  EXTRA_TARGETS tests_main buffer
  DEFS BOOST_LEAF_NO_EXCEPTIONS=0
       BOOST_LEAF_CFG_STD_SYSTEM_ERROR=0
       BOOST_LEAF_CFG_STD_STRING=0
       BOOST_LEAF_CFG_DIAGNOSTICS=0
       BOOST_LEAF_CFG_GNUC_STMTEXPR=0
  )

# If use IDE add gtest, gmock, gtest_main and gmock_main targets into deps/googletest group
set_target_properties(gtest gmock gtest_main gmock_main PROPERTIES FOLDER deps/googletest)
