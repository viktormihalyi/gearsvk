
file (COPY ${CMAKE_SOURCE_DIR}/src/UserInterface/Sources/
      DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install (DIRECTORY ${CMAKE_SOURCE_DIR}/src/UserInterface/Sources/Project DESTINATION bin)
