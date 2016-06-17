set(SD "${CMAKE_CURRENT_SOURCE_DIR}")


add_subdirectory(miner)
add_subdirectory(article_analysis)
add_subdirectory(app_server)
install(DIRECTORY app_client DESTINATION .)
install(DIRECTORY scripts DESTINATION .)
