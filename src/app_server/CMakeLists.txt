set(PYTHON "python3")

set(LIB_PATH "lib/app_server")
set(BIN_NAME "app_server")
set(TEST_NAME "test_app_server")


set(ENTRY main.py)
set(TEST_ENTRY "test_main.py")

set(MODULE_DIRS modules)


install(FILES ${ENTRY}  DESTINATION "${LIB_PATH}"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)
install(DIRECTORY ${MODULE_DIRS} DESTINATION "${LIB_PATH}")

set(FROM "${LIB_PATH}/${ENTRY}")
set(CMD "${SD}/install_symlink_bin.sh \"${BIN_NAME}\" \"${FROM}\"")
install(CODE "execute_process(COMMAND ${CMD} WORKING_DIRECTORY \"${IP}\")")


if (test)
  add_test(NAME ${TEST_NAME} COMMAND ${PYTHON} ${TEST_ENTRY}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
