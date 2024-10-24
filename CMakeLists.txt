cmake_minimum_required (VERSION 3.24)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-narrowing")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- OPTIONS ------
option(YTMAPI_TESTS "Set to ON to build tests" ON)


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

# Dependencies ^

project(ytmpapi
  LANGUAGES CXX
  DESCRIPTION "A CPP port of the similarly named Python library"
)
add_library(ytmapi
  STATIC
  src/ytmapi/ytmapi.cpp
  src/ytmapi/simdjson.cpp
)

target_include_directories(ytmapi
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  PRIVATE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_link_libraries(ytmapi 
  PUBLIC cpr::cpr 
)

install(
  TARGETS ytmapi cpr 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


include(cmake/ytmapi_export.cmake)
add_subdirectory(examples)



