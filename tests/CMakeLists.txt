file(GLOB SOURCES_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# Build tests in both noexcept mode (default) and exception mode
foreach(local_file ${SOURCES_TEST} ${SOURCES_HASP_TEST})
  get_filename_component(local_filename ${local_file} NAME_WE)

  # Default mode: GLOBCPP_NOEXCEPT_ENABLED=1 (noexcept mode)
  add_executable(${local_filename} ${local_file})
  target_compile_definitions(${local_filename} PRIVATE GLOBCPP_NOEXCEPT_ENABLED=1)
  target_link_libraries(${local_filename} PRIVATE glob-cpp GTest::gtest_main)
  if (NOT WIN32)
    target_link_libraries(${local_filename} PRIVATE pthread)
  endif()
  add_test(${local_filename}_noexcept ${local_filename})

  # Exception mode: GLOBCPP_NOEXCEPT_ENABLED=0 (exception mode)
  add_executable(${local_filename}_throws ${local_file})
  target_compile_definitions(${local_filename}_throws PRIVATE GLOBCPP_NOEXCEPT_ENABLED=0)
  target_link_libraries(${local_filename}_throws PRIVATE glob-cpp GTest::gtest_main)
  if (NOT WIN32)
    target_link_libraries(${local_filename}_throws PRIVATE pthread)
  endif()
  add_test(${local_filename}_throws ${local_filename}_throws)
endforeach()
