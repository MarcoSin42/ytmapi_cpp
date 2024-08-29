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
project(ytmpapi
  LANGUAGES CXX
  VERSION 0.1.0
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
  STATIC
  include/ytmapi.hpp
  include/utils.hpp
  src/ytmapi.cpp
)

target_include_directories(ytmapi
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(ytmapi 
  PUBLIC cpr::cpr 
  PUBLIC nlohmann_json::nlohmann_json
)

install(TARGETS ytmapi cpr nlohmann_json
        EXPORT ytmapiTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ytmapi.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ytmapiTargets 
        FILE ytmapiTargets.cmake
        NAMESPACE ytmapi::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ytmapi
)


include(ytmapi_export.cmake)
add_subdirectory(tests)


