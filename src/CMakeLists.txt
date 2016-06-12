cmake_minimum_required (VERSION 2.8)

project (PTTArticleRecommander)


if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed"
          CACHE PATH "default install path" FORCE)
endif ()


set(IP "${CMAKE_INSTALL_PREFIX}")
set(SD "${CMAKE_SOURCE_DIR}")


option (test "Build all tests." ON)


if (test)
  enable_testing()
endif ()


add_subdirectory(miner)
add_subdirectory(article_analysis)
add_subdirectory(app_server)
install(DIRECTORY app_client DESTINATION .)
install(DIRECTORY scripts DESTINATION .)
