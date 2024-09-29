cmake_minimum_required(VERSION 3.10)

project(VehicleSimulation)

set(SOURCES 
    src/main.cxx
    src/digital_twin_client.cxx
    src/json_helper.cxx
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)

find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PkgConfig::Mosquitto)

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

file(COPY ${CMAKE_SOURCE_DIR}/config.json DESTINATION ${CMAKE_BINARY_DIR})
