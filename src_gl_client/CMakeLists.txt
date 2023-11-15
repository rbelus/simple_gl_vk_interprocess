cmake_minimum_required(VERSION 3.13)

set ( projectName opengl_proto)

project(${projectName})
set(CMAKE_CXX_STANDARD 17)

find_path(VULKAN_SDK
  NAMES Bin/vkconfig.exe
  PATHS "C:"
  REQUIRED
  DOC "Directory for Vulkan SDK (for glslangValidator)"
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

find_path(GLAD
  NAMES include/glad/glad.h
  PATHS "C:"
  REQUIRED
  DOC "GLAD loader"
)


file(GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c *.frag *.vert)

# set ( VULKAN_INCLUDE ${VULKAN_SDK}/Include )
set ( GLFW_INCLUDE ${GLFW}/include )
set ( GLFW_DEPS_INCLUDE ${GLFW}/deps )
set ( GLM_INCLUDE ${GLM_PATH} )
set ( GLAD_SOURCE ${GLAD}/src)
set ( GLAD_INCLUDE ${GLAD}/include )

set (SOURCE_FILES ${SOURCE_FILES} ${GLAD_SOURCE}/glad.c)

add_executable(${projectName} ${SOURCE_FILES})

target_include_directories( ${projectName} PUBLIC ${GLFW_INCLUDE}  ${GLM_INCLUDE} ${GLFW_DEPS_INCLUDE} ${GLAD_INCLUDE})

# set ( VULKAN_LIB ${VULKAN_SDK}/Lib )
find_path(GLFW_LIB
  NAMES glfw3.lib
  PATHS ${GLFW} ${GLFW}/out/src/Debug
  REQUIRED
  DOC "GLFW lib dir"
)

# add_library(glfw STATIC IMPORTED)

target_link_libraries( ${projectName} ${GLFW_LIB}/glfw3.lib )

# langValidator 
 add_custom_command( 
	TARGET ${projectName}
	POST_BUILD
	COMMAND ${VULKAN_SDK}/Bin/glslangValidator.exe ARGS "${CMAKE_SOURCE_DIR}/shader.vert"
 	COMMAND ${VULKAN_SDK}/Bin/glslangValidator.exe ARGS "${CMAKE_SOURCE_DIR}/shader.frag")

# message( "Successfully built shaders to ${CMAKE_BINARY_DIR}/$<CONFIG>/vert.spv ${CMAKE_BINARY_DIR}/$<CONFIG>/frag.spv")

add_definitions( -DSHADER_FOLDER="${CMAKE_SOURCE_DIR}/")

set(VULKAN_EXCHANGE_FILE "D:/VulkanExchange" CACHE FILEPATH "file for interprocess communication with OGL")
add_definitions( -DVULKAN_EXCHANGE_FILE="${VULKAN_EXCHANGE_FILE}")

# set ( additionnalLibs
#   vulkan-1.lib
#   glfw3.lib)

# add_dependencies( ${projectName} ${additionnalLibs})