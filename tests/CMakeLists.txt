# /tests/CMakeLists.txt
#
# Build the libwobbly unit tests
#
# See LICENCE.md for Copyright information.

set (LIBWOBBLY_UNIT_TESTS_LIBRARIES
     ${LIBWOBBLY_EXTERNAL_LIBRARIES}
     ${LIBWOBBLY_LIBRARY})

set (LIBWOBBLY_UNIT_TESTS_SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/spring_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/model_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/point_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/anchor_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/euler_integration_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/constrainment_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/mesh_interpolation_test.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ostream_point_operator.h)

psq_add_test (${LIBWOBBLY_UNIT_TESTS_BINARY}
              INTERNAL_INCLUDE_DIRS
              ${LIBWOBBLY_EXPORTED_INCLUDE_DIRS}
              ${LIBWOBBLY_INTERNAL_INCLUDE_DIRS}
              ${CMAKE_CURRENT_SOURCE_DIR}
              EXTERNAL_INCLUDE_DIRS ${LIBWOBBLY_EXTERNAL_INCLUDE_DIRS}
              SOURCES ${LIBWOBBLY_UNIT_TESTS_SOURCES}
              LIBRARIES ${LIBWOBBLY_UNIT_TESTS_LIBRARIES}
              MATCHERS ${LIBWOBBLY_MATCHERS_LIBRARY}
              NO_CLANG_TIDY)
