A simple prototype to try out interprocess work between Vulkan and OpenGL.

Vulkan renders a spinning color-shifting triangle.
It creates and allocates memory for a VkImage.
It gets a windows handle for this image and writes it to a file.
Each frame it copies current swapchain image to this VkImage.

OpenGL reads the file, duplicates given windows handle to use it.
It creates a memory object pointing to this handle.
It interprets it as a 2D Texture with correct format.
It renders a simple spinning quad sampling this texture.


# Requirements

- Vulkan SDK with device supporting those VK extensions : 
	- VK_KHR_SWAPCHAIN_EXTENSION_NAME
	- VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME
	- VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME
	- VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME
	- VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME
- GLAD with support for those extensions : 
	- glGenSemaphoresEXT
	- glImportSemaphoreWin32HandleEXT
	- glCreateMemoryObjectsEXT
	- glImportMemoryWin32HandleEXT
	- glTextureStorageMem2DEXT
- GLFW
- GLM


# Install

There are separate CMakeLists for each project (gl, vk). Run cmake-gui and points necessary values to root folders of vulkan sdk, glfw lib, glm lib, and glad unzipped. You may need to build glfw to link with lib.

