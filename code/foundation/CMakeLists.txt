project(zeus_foundation)
add_library(${PROJECT_NAME} STATIC)
add_library(zeus::foundation ALIAS ${PROJECT_NAME})

FILE(GLOB_RECURSE HEADER_FILES "include/*.h" "include/*.hpp")
FILE(GLOB_RECURSE SOURCE_DIR_FILES "src/*.cpp" "src/*.h" "src/*.hpp")

#整合要编译的源文件
IF(HEADER_FILES)
LIST (APPEND SRC_FILES ${HEADER_FILES})
ENDIF(HEADER_FILES)

IF(SOURCE_DIR_FILES)
LIST (APPEND SRC_FILES ${SOURCE_DIR_FILES})
ENDIF(SOURCE_DIR_FILES)

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/zeus/foundation PREFIX "include" FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "src" FILES ${SOURCE_DIR_FILES} ${IMPL_HEADER_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

find_package(zeus_expected REQUIRED)
find_package(cryptopp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC zeus::expected)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2::tinyxml2)
target_link_libraries(${PROJECT_NAME} PRIVATE cryptopp::cryptopp)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs pthread rt uuid dl udev mount )
endif()

IF(BUILD_TESTING)
    add_subdirectory(gtest)
ENDIF()
