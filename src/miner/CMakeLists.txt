set(LIB_PATH "lib/miner")
set(BIN_NAME "miner")


set(ENTRY main.py)

set(MODULE_FILES
    example_module2.py)

set(MODULE_DIRS
    example_module1)


install(FILES ${ENTRY}  DESTINATION "${LIB_PATH}"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE)
install(FILES ${MODULE_FILES}  DESTINATION "${LIB_PATH}")
install(DIRECTORY ${MODULE_DIRS}  DESTINATION "${LIB_PATH}")

set(FROM "${LIB_PATH}/${ENTRY}")
set(CMD "${SD}/install_symlink_bin.sh \"${BIN_NAME}\" \"${FROM}\"")
install(CODE "execute_process(COMMAND ${CMD} WORKING_DIRECTORY \"${IP}\")")
