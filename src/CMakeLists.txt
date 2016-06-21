set(SD "${CMAKE_CURRENT_SOURCE_DIR}")


add_subdirectory(miner)
add_subdirectory(article_analysis)
add_subdirectory(app_server)
add_subdirectory(app_client)
install(DIRECTORY scripts DESTINATION .)
