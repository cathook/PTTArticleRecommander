cmake_minimum_required (VERSION 2.8)

project (PTTArticleRecommender)


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed"
          CACHE PATH "default install path" FORCE)
endif ()


set(IP "${CMAKE_INSTALL_PREFIX}")


option (test "Build all tests." ON)


if (test)
  enable_testing()
endif ()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")


add_subdirectory(src)
