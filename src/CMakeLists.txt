add_library(VectorTree INTERFACE)
target_include_directories(VectorTree INTERFACE ${_INC_DIR})
set_target_properties(
  VectorTree
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
