set(BIN_NAME "article_analysis")
set(LIB_NAME "aa")
set(TEST_NAME "test_article_analysis")


set(CMAKE_CXX_FLAGS "-Wall -O2 -std=c++11")

if (OPENCC_ROOT)
  set(OPENCC_INCLUDE_DIRS ${OPENCC_ROOT}/include)
  set(OPENCC_LIBRARIES ${OPENCC_ROOT}/lib/libopencc.so)
else ()
  find_package(OpenCC REQUIRED)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/deps/cppjieba/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/cppjieba/deps)
include_directories(${OPENCC_INCLUDE_DIRS})


file(GLOB BIN_ENTRY_SRC "main.cc")

file(GLOB_RECURSE TEST_SRCS "*-test.cc")
list(APPEND TEST_SRCS "test_main.cc")

file(GLOB_RECURSE FULL_LIB_SRCS "*.cc")
list(REMOVE_ITEM FULL_LIB_SRCS ${BIN_ENTRY_SRC} ${TEST_SRCS})


add_library(${LIB_NAME} ${FULL_LIB_SRCS})


add_executable(${BIN_NAME} ${BIN_ENTRY_SRC})
target_link_libraries(${BIN_NAME} ${LIB_NAME} ${OPENCC_LIBRARIES})


if (test)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})

  add_executable(${TEST_NAME} ${TEST_SRCS})
  target_link_libraries(${TEST_NAME} ${LIB_NAME} ${OPENCC_LIBRARIES}
                        ${GTEST_LIBRARIES} pthread)

  add_test(${TEST_NAME} ${TEST_NAME})
endif()


install(TARGETS ${BIN_NAME} DESTINATION bin)
