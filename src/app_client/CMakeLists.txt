set(INSTALL_PREFIX "app_client")

set(FILES manifest.json)
set(DIRS inject popup)

set(SETUP setup.sh)


install(FILES ${FILES} DESTINATION ${INSTALL_PREFIX})
install(DIRECTORY ${DIRS} DESTINATION ${INSTALL_PREFIX})

install(FILES ${SETUP} DESTINATION ${INSTALL_PREFIX}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)



if (APP_SERVER_ADDR)
else()
  set(APP_SERVER_ADDR "localhost:8085")
endif ()


set(WD "${CMAKE_INSTALL_PREFIX}/${INSTALL_PREFIX}")
set(CMD "./${SETUP} ${APP_SERVER_ADDR}")
install(CODE "execute_process(COMMAND ${CMD} WORKING_DIRECTORY \"${WD}\")")
