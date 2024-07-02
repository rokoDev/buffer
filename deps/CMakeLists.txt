include(FetchContent)

# If dependencies are not available then download it and make available
FetchContent_Declare(
	strong_type
	GIT_REPOSITORY https://github.com/rokoDev/strong_type.git
	GIT_TAG        b34f11ad2cbb164030a56a80e488cfd716d43b5d
)

# If necessary it's possible to use sources from different location instead of downloaded from GitHub
# string(TOUPPER strong_type UP_strong_type)
# set(FETCHCONTENT_SOURCE_DIR_${UP_strong_type} ${CMAKE_SOURCE_DIR}/../rokoDev/strong_type)

FetchContent_MakeAvailable(strong_type)
target_link_libraries(buffer INTERFACE strong_type)
