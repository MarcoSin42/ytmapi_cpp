cmake_minimum_required (VERSION 3.24)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-narrowing")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "$-g")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- OPTIONS ------
option(YTMAPI_TESTS "Set to ON to build tests" OFF)


# ^^^^ OPTIONS ^^^^^^^
project(YTMApi_cpp
  LANGUAGES CXX
  VERSION 1.0.0
  DESCRIPTION "A CPP port of the similarly named Python library"
)


# Dependencies
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET ON)

FetchContent_Declare(cpr 
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
  GIT_PROGRESS   FALSE
  GIT_SHALLOW    TRUE
) 
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_PROGRESS FALSE
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)
# Dependencies ^


add_library(ytmapi
  include/ytmapi.hpp
  include/utils.hpp
  src/ytmapi.cpp
)

target_include_directories(ytmapi
  PRIVATE include
  PRIVATE src
)

target_link_libraries(ytmapi 
  PUBLIC cpr::cpr 
  PUBLIC nlohmann_json::nlohmann_json
)



include(ytmapi_export.cmake)
add_subdirectory(tests)


