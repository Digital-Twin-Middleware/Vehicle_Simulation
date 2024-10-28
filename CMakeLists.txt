cmake_minimum_required(VERSION 3.10)

project(VehicleSimulation)

set(SOURCES 
    src/main.cxx
    src/digital_twin_client.cxx
    src/json_helper.cxx
    src/car_simulation.cxx
    src/utility_functions.cxx
    src/process_manager.cxx
)

set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig REQUIRED)

pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)

find_package(nlohmann_json 3.2.0 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PkgConfig::Mosquitto)

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/config.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/config.json ${CMAKE_BINARY_DIR}/config.json
    DEPENDS ${CMAKE_SOURCE_DIR}/config.json
    COMMENT "Copying config.json to the build directory"
)

add_custom_target(copy_config ALL
    DEPENDS ${CMAKE_BINARY_DIR}/config.json
)

file(COPY ${CMAKE_SOURCE_DIR}/models/ DESTINATION ${CMAKE_BINARY_DIR}/models)
