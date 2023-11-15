cmake_minimum_required(VERSION 3.13)

set ( projectName vulkan_proto)

project(${projectName})
set(CMAKE_CXX_STANDARD 17)

find_path(VULKAN_SDK
  NAMES Bin/vkconfig.exe
  PATHS "C:"
  REQUIRED
  DOC "Directory for Vulkan SDK"
  )

find_path(GLM_PATH
  NAMES glm
  PATHS "C:"
  REQUIRED
  DOC "Directory of GLM"
  )

find_path(GLFW
  NAMES include/GLFW/glfw3.h
  PATHS "C:"
  REQUIRED
  DOC "Directory of GLFW"
  ) 

file(GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c)

add_executable(${projectName} ${SOURCE_FILES})

set ( VULKAN_INCLUDE ${VULKAN_SDK}/Include )
set ( GLFW_INCLUDE ${GLFW}/include )

target_include_directories( ${projectName} PUBLIC ${VULKAN_INCLUDE} ${GLFW_INCLUDE} )

set ( VULKAN_LIB ${VULKAN_SDK}/Lib )
find_path(GLFW_LIB
  NAMES glfw3.lib
  PATHS ${GLFW} ${GLFW}/out/src/Debug
  REQUIRED
  DOC "GLFW lib dir"
)


# add_library(glfw STATIC IMPORTED)

file( GLOB VULKAN_LIBRARIES ${VULKAN_LIB}/vulkan-1.lib)

target_link_libraries( ${projectName} ${VULKAN_LIBRARIES} ${GLFW_LIB}/glfw3.lib )

# SPIR-V compil
add_custom_command( 
	TARGET ${projectName}
	POST_BUILD
	COMMAND ${VULKAN_SDK}/Bin/glslc.exe ARGS ${CMAKE_SOURCE_DIR}/shader.vert -o "${CMAKE_BINARY_DIR}/$<CONFIG>/vert.spv"
	COMMAND ${VULKAN_SDK}/Bin/glslc.exe ARGS ${CMAKE_SOURCE_DIR}/shader.frag -o "${CMAKE_BINARY_DIR}/$<CONFIG>/frag.spv")

message( "Successfully built shaders to ${CMAKE_BINARY_DIR}/$<CONFIG>/vert.spv ${CMAKE_BINARY_DIR}/$<CONFIG>/frag.spv")

add_definitions( -DSHADER_FOLDER="${CMAKE_BINARY_DIR}/$<CONFIG>/")

set(VULKAN_EXCHANGE_FILE "D:/VulkanExchange" CACHE FILEPATH "file for interprocess communication with OGL")
add_definitions( -DVULKAN_EXCHANGE_FILE="${VULKAN_EXCHANGE_FILE}")

#set ( additionnalLibs
#  vulkan-1.lib
#  glfw3.lib)
#
#add_dependencies( ${projectName} ${additionnalLibs})