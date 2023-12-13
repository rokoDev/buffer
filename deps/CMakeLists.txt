include(FetchContent)

# If dependencies are not available then download it and make available
FetchContent_Declare(
	boost_leaf
	GIT_REPOSITORY https://github.com/boostorg/leaf.git
	GIT_TAG        d8aae791332a1307ee5a14561a9f7636649e62b9
)

FetchContent_Declare(
	strong_type
	GIT_REPOSITORY https://github.com/rokoDev/strong_type.git
	GIT_TAG        b34f11ad2cbb164030a56a80e488cfd716d43b5d
)

# If necessary it's possible to use Boost.Leaf from different location instead of downloaded from GitHub
# string(TOUPPER boost_leaf UP_boost_leaf)
# set(FETCHCONTENT_SOURCE_DIR_${UP_boost_leaf} ${CMAKE_SOURCE_DIR}/../rokoDev/leaf)

FetchContent_MakeAvailable(boost_leaf strong_type)
target_link_libraries(buffer INTERFACE boost_leaf strong_type)
