configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)

add_custom_target(check
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/llvm-lit "${CMAKE_CURRENT_BINARY_DIR}" -sv
  DEPENDS sqlgen)
