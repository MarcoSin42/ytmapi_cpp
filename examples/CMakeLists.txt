message("Building Demo")

if(NOT YTMAPI_TESTS)
  return()
endif()

set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


function(new_example name)
  add_executable(ytmapi_test_${name} ${name}.cpp)
  target_include_directories(ytmapi_test_${name} 
    PRIVATE ../include/ytmapi
    PRIVATE ../src/ytmapi
  )
  
  target_link_libraries(ytmapi_test_${name}
    PRIVATE ytmapi
  )
  file(RELATIVE_PATH dir ${EXAMPLES_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(GLOBAL APPEND PROPERTY YTMAPI::TEST ${dir}/${name})
  target_compile_features(ytmapi_test_${name} PRIVATE cxx_std_20)
endfunction(new_example)


new_example(playlists)
new_example(getPlaylistTracks)
new_example(createPlaylist)
new_example(delPlaylist)
new_example(likeSong)
new_example(unlikeSong)