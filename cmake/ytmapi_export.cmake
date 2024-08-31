add_library(ytmapi::ytmapi ALIAS ytmapi)

export(
  TARGETS ytmapi cpr
  NAMESPACE ytmapi::
  FILE "${PROJECT_BINARY_DIR}/ytmapi-targets.cmake"
)