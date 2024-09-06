project(zeus_foundation_gtest)

FILE(GLOB HEADER_FILES "*.h" "*.hpp")
FILE(GLOB SRC_FILES "*.cpp")

FILE(GLOB SYSTEM_HEADER_FILES "system/*.h" "system/*.hpp")
FILE(GLOB SYSTEM_SRC_FILES "system/*.cpp")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/system PREFIX "system" FILES ${SYSTEM_SRC_FILES} ${SYSTEM_HEADER_FILES})

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES}  ${SYSTEM_SRC_FILES} ${SYSTEM_HEADER_FILES} )

target_link_libraries(${PROJECT_NAME} zeus::foundation)

add_custom_command(TARGET zeus_foundation_gtest POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E copy_directory  "${CMAKE_CURRENT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:zeus_foundation_gtest>/")


find_package(GTest REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_link_libraries(${PROJECT_NAME} GTest::gtest)

IF(WIN32)
    set_property(TARGET zeus_foundation_gtest APPEND PROPERTY COMPILE_OPTIONS /bigobj)
ENDIF()
