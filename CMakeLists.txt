cmake_minimum_required(VERSION 3.0)
project(WienerTakesAll)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
	${SDL2_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
	${SDL2_IMAGE_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${ASSIMP_INCLUDE_DIRS}
)

file(GLOB wienertakesall_src
    "src/*.cpp"
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1z -g")
add_executable(${PROJECT_NAME} ${wienertakesall_src})

target_include_directories(
	${PROJECT_NAME} PUBLIC
    "physx/include/"
)

if(UNIX AND NOT APPLE)
	file(GLOB physx_libs "physx/linux64/*" )
endif()

if(APPLE)
	file(GLOB physx_libs "physx/osx/*")
endif()


target_link_libraries(${PROJECT_NAME}
	${SDL2_LIBRARY}
	${SDL2_MIXER_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${GLEW_LIBRARIES}
	${GLM_LIBRARIES}
	${GLM_LIBRARIES}
	${ASSIMP_LIBRARIES}
	${OPENGL_LIBRARIES}
	${physx_libs}
)
