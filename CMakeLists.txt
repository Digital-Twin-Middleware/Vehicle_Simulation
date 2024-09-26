cmake_minimum_required(VERSION 3.10)

project(VehicleSimulation)

set(SOURCES 
    src/main.cxx
    src/mqtt_client.cxx
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)