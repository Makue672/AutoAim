cmake_minimum_required(VERSION 3.10)


project(RoboMasterAutoAim)


set(CMAKE_CXX_STANDARD 17)


find_package(OpenCV REQUIRED)


message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.cpp")


add_executable(AutoAimApp ${SOURCES} "include/number_recognizer.h" "src/armor_matcher.cpp" "src/number_recognizer.cpp")


target_link_libraries(AutoAimApp ${OpenCV_LIBS})
