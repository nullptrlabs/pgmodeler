# Determining the build date and build number
# The commands changes from OS to OS
if(LINUX OR APPLE)
    set(date_cmd date)
    set(date_cmd_param +%Y%m%d)
    set(build_num_cmd ${CMAKE_SOURCE_DIR}/getbuildnum.sh)
else()
    set(date_cmd ${CMAKE_SOURCE_DIR}/getbuildnum.bat)
    set(date_cmd_param "")
    set(build_num_cmd ${CMAKE_SOURCE_DIR}/getbuildnum.bat)
endif()

# Running the build date command
execute_process(
    COMMAND ${build_num_cmd}
    OUTPUT_VARIABLE build_num
    RESULT_VARIABLE build_num_res
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT build_num_res EQUAL 0)
    message(FATAL_ERROR "Command failed: ${build_num_cmd}")
endif()

# Running the build number command
execute_process(
    COMMAND ${date_cmd} ${date_cmd_param}
    OUTPUT_VARIABLE date
    RESULT_VARIABLE date_res
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT date_res EQUAL 0)
    message(FATAL_ERROR "Command failed: ${date_cmd} ${date_cmd_param}")
endif()

# The output of the commands above are used as the defines
# BUILDNUM and BUILDDATE in .cpp files
add_compile_definitions(
    BUILDNUM="${build_num}"
    BUILDDATE="${date}")
