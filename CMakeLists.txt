cmake_minimum_required(VERSION 3.13)
project(librabbitmq-cxx)
add_subdirectory(dependencies/rabbitmq-c)
add_subdirectory(dependencies/googletest)

option(BUILD_DOC "Build documentation" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_UNIT_TESTS "Build unit tests" OFF)
option(BUILD_PERF_TESTS "Build performance tests" OFF)

add_library(ilibrabbitmq-cxx INTERFACE)
add_dependencies(ilibrabbitmq-cxx rabbitmq)
target_include_directories(ilibrabbitmq-cxx INTERFACE include dependencies/rabbitmq-c/librabbitmq)
if(CMAKE_CXX_STANDARD LESS 17)
  target_include_directories(ilibrabbitmq-cxx INTERFACE dependencies/variant/include)
endif (CMAKE_CXX_STANDARD LESS 17)
target_compile_options(ilibrabbitmq-cxx INTERFACE -Wall)

add_library(librabbitmq-cxx INTERFACE)
target_link_libraries(librabbitmq-cxx INTERFACE ilibrabbitmq-cxx rabbitmq)

if (BUILD_UNIT_TESTS)
  add_executable(librabbitmq-cxx-tests tests/unit/main.cpp
    tests/unit/AMQPStructTests.cpp
    tests/unit/ChannelTests.cpp
    tests/unit/ConnectionTests.cpp
    tests/unit/EnvelopeTests.cpp
    tests/unit/ExchangeTests.cpp
    tests/unit/MessageTests.cpp
    tests/unit/QueueTests.cpp
    tests/unit/ReturnedMessageTests.cpp
    tests/unit/TableEntryTests.cpp

    tests/unit/comparison.cpp
    tests/unit/MockAMQP.cpp
  )
  target_link_libraries(librabbitmq-cxx-tests PRIVATE ilibrabbitmq-cxx gtest gmock)
endif (BUILD_UNIT_TESTS)

if (BUILD_PERF_TESTS)
# TODO: Perf tests
endif (BUILD_PERF_TESTS)

if (BUILD_EXAMPLES)
  add_executable(example-consumer examples/consumer.cpp)
  target_link_libraries(example-consumer PRIVATE librabbitmq-cxx)
  set_property(TARGET example-consumer PROPERTY CXX_STANDARD 11)

  add_executable(example-publisher examples/publisher.cpp)
  target_link_libraries(example-publisher PRIVATE librabbitmq-cxx)
  set_property(TARGET example-publisher PROPERTY CXX_STANDARD 11)
endif (BUILD_EXAMPLES)

if (BUILD_DOC)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
      # set input and output files
      set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

      # request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message("Doxygen build started")

      # note the option ALL which allows to build the docs together with the application
      add_custom_target( doc_doxygen ALL
          COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif (BUILD_DOC)
