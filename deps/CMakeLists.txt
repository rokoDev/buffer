include(FetchContent)

# Setup directory to which dependencies will be downloaded
cmake_path(APPEND FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}" "deps_content" "${CMAKE_GENERATOR_NAME_WITHOUT_SPACES}")

# If dependencies are not available then download it and make available
FetchContent_Declare(
	boost_leaf
	GIT_REPOSITORY https://github.com/boostorg/leaf.git
	GIT_TAG        9d3ffa0f1c859a34da6175b53572fd4f2a61bb18
)

FetchContent_Declare(
	strong_type
	GIT_REPOSITORY https://github.com/rokoDev/strong_type.git
	GIT_TAG        d086c4313909c5f34a7a6fcf4992fb6dfc570496
)

# If necessary it's possible to use Boost.Leaf from different location instead of downloaded from GitHub
# string(TOUPPER boost_leaf UP_boost_leaf)
# set(FETCHCONTENT_SOURCE_DIR_${UP_boost_leaf} ${CMAKE_SOURCE_DIR}/../rokoDev/leaf)

FetchContent_MakeAvailable(boost_leaf strong_type)
target_link_libraries(buffer INTERFACE boost_leaf strong_type)
