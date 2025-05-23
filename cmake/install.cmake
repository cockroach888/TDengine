SET(PREPARE_ENV_CMD "prepare_env_cmd")
SET(PREPARE_ENV_TARGET "prepare_env_target")
ADD_CUSTOM_COMMAND(OUTPUT ${PREPARE_ENV_CMD}
    COMMAND echo "make test directory"
    DEPENDS taosd
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TD_TESTS_OUTPUT_DIR}/cfg/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TD_TESTS_OUTPUT_DIR}/log/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TD_TESTS_OUTPUT_DIR}/data/
    COMMAND ${CMAKE_COMMAND} -E echo firstEp localhost:6030 > ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo fqdn localhost >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo serverPort 6030 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo debugFlag 135 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo asyncLog 0 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo supportVnodes 1024 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo numOfLogLines 300000000 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo logKeepDays -1 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo checkpointInterval 60 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo snodeAddress 127.0.0.1:873 >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo dataDir ${TD_TESTS_OUTPUT_DIR}/data >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo logDir ${TD_TESTS_OUTPUT_DIR}/log  >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo charset UTF-8  >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMAND ${CMAKE_COMMAND} -E echo monitor 0  >> ${TD_TESTS_OUTPUT_DIR}/cfg/taos.cfg
    COMMENT "prepare taosd environment")
ADD_CUSTOM_TARGET(${PREPARE_ENV_TARGET} ALL WORKING_DIRECTORY ${TD_EXECUTABLE_OUTPUT_PATH} DEPENDS ${PREPARE_ENV_CMD})

IF (TD_LINUX)
  SET(TD_MAKE_INSTALL_SH "${TD_SOURCE_DIR}/packaging/tools/make_install.sh")
  INSTALL(CODE "MESSAGE(\"make install script: ${TD_MAKE_INSTALL_SH}\")")
  INSTALL(CODE "execute_process(COMMAND bash ${TD_MAKE_INSTALL_SH} ${TD_SOURCE_DIR} ${CMAKE_BINARY_DIR} Linux ${TD_VER_NUMBER})")
ELSEIF (TD_WINDOWS)
  SET(TD_MAKE_INSTALL_SH "${TD_SOURCE_DIR}/packaging/tools/make_install.bat")
  INSTALL(CODE "MESSAGE(\"make install script: ${TD_MAKE_INSTALL_SH}\")")
  INSTALL(CODE "execute_process(COMMAND ${TD_MAKE_INSTALL_SH} :needAdmin ${TD_SOURCE_DIR} ${CMAKE_BINARY_DIR} Windows ${TD_VER_NUMBER} ${TD_BUILD_TAOSA_INTERNAL})")
ELSEIF (TD_DARWIN)
  SET(TD_MAKE_INSTALL_SH "${TD_SOURCE_DIR}/packaging/tools/make_install.sh")
  INSTALL(CODE "MESSAGE(\"make install script: ${TD_MAKE_INSTALL_SH}\")")
  INSTALL(CODE "execute_process(COMMAND bash ${TD_MAKE_INSTALL_SH} ${TD_SOURCE_DIR} ${CMAKE_BINARY_DIR} Darwin ${TD_VER_NUMBER})")
ENDIF ()
