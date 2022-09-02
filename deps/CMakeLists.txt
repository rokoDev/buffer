include(FetchContent)

# Setup directory to which dependencies will be downloaded
cmake_path(APPEND FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}" "deps_content" "${CMAKE_GENERATOR_NAME_WITHOUT_SPACES}")

# If dependencies are not available then download it and make available
FetchContent_Declare(
	boost_leaf
	GIT_REPOSITORY https://github.com/rokoDev/leaf.git
	GIT_TAG        d619d8225b7a781eed8fac340f81beaae7e7052e
)

# If necessary it's possible to use Boost.Leaf from different location instead of downloaded from GitHub
# string(TOUPPER boost_leaf UP_boost_leaf)
# set(FETCHCONTENT_SOURCE_DIR_${UP_boost_leaf} ${CMAKE_SOURCE_DIR}/../rokoDev/leaf)

FetchContent_MakeAvailable(boost_leaf)
target_link_libraries(buffer INTERFACE boost_leaf)
