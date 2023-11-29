/*

	Very simple, very triangle.
	Goal is to get image from Vulkan and render it as quad to screen.

*/

#include <windows.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

//#include <glad/gl.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>

struct Vertex {
	vec2 position;
	vec2 uv;
	vec3 color;
};

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cout << "[OpenGL Error](" << type << ") " << message << std::endl;
}

void testWinError()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	if (errorMessageID != 0)
	{
		std::cout << "Win error : " << message << std::endl;
	}
}

struct VulkanOGLInterop {
	HANDLE	vulkanProc;

	HANDLE	hglReady;
	HANDLE	hglComplete;
	HANDLE	hglImage;
	HANDLE	hglDepth;
	GLuint	semaphoreReady;
	GLuint	semaphoreComplete;

	GLuint	memoryImage;
	GLuint	memoryDepth;
	uint32_t memoryImageSize;
	uint32_t memoryDepthSize;

	void loadVulkanData()
	{
		std::string fileExchangePath = VULKAN_EXCHANGE_FILE;
		std::ifstream fileExchange;
		fileExchange.open(fileExchangePath, std::ios::in);

		if (!fileExchange.is_open())
		{
			return;
		}

		HANDLE fromVulkanHglReady;
		HANDLE fromVulkanHglComplete;
		HANDLE fromVulkanHglImage;
		HANDLE fromVulkanHglDepth;

		DWORD vulkanProcessID;

		std::stringstream raw;
		raw << fileExchange.rdbuf();
		raw >> std::dec >> vulkanProcessID;
		raw >> std::hex >> fromVulkanHglReady;
		raw >> std::hex >> fromVulkanHglComplete;
		raw >> std::dec >> memoryImageSize;
		raw >> std::hex >> fromVulkanHglImage;
		raw >> std::dec >> memoryDepthSize;
		raw >> std::hex >> fromVulkanHglDepth;
		fileExchange.close();


		vulkanProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, vulkanProcessID);
		testWinError();

		// Convert vulkan handles to "real" handles
		DuplicateHandle(
			vulkanProc,
			fromVulkanHglReady,
			GetCurrentProcess(),
			&hglReady,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		testWinError();

		DuplicateHandle(
			vulkanProc,
			fromVulkanHglComplete,
			GetCurrentProcess(),
			&hglComplete,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		testWinError();

		DuplicateHandle(
			vulkanProc,
			fromVulkanHglImage,
			GetCurrentProcess(),
			&hglImage,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		testWinError();

		DuplicateHandle(
			vulkanProc,
			fromVulkanHglDepth,
			GetCurrentProcess(),
			&hglDepth,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		testWinError();

		glGenSemaphoresEXT(1, &semaphoreReady);
		glGenSemaphoresEXT(1, &semaphoreComplete);
		glImportSemaphoreWin32HandleEXT(semaphoreReady, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, hglReady);
		glImportSemaphoreWin32HandleEXT(semaphoreComplete, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, hglComplete);

		glCreateMemoryObjectsEXT(1, &memoryImage);
		glImportMemoryWin32HandleEXT(memoryImage, memoryImageSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, hglImage);

		glCreateMemoryObjectsEXT(1, &memoryDepth);
		glImportMemoryWin32HandleEXT(memoryDepth, memoryDepthSize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, hglDepth);



	}
};

int main()
{
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location;//, vpos_location, vcol_location, vuv_location;

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "OPEN GL CLIENT", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(/*glfwGetProcAddress*/);
	glfwSwapInterval(1);

	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);


	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	// Create awesome quad.
	Vertex vert0 = { {-1.f, -1.f},	{0.f,0.f},	{1.f,0.f,0.f} };
	Vertex vert1 = { {1.f, -1.f},	{1.f,0.f},	{0.f,1.f,0.f} };
	Vertex vert2 = { {-1.f, 1.f},	{0.f,1.f},	{0.f,0.f,1.f} };
	Vertex vert3 = { {1.f, 1.f},	{1.f,1.f},	{1.f,1.f,1.f} };

	std::vector<Vertex> mesh;
	mesh.push_back(vert0);
	mesh.push_back(vert1);
	mesh.push_back(vert2);
	mesh.push_back(vert2);
	mesh.push_back(vert1);
	mesh.push_back(vert3);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), &mesh[0], GL_STATIC_DRAW);

	{
		// Read shader text.
		std::string vertShaderFile = "shader.vert";
		std::string fragShaderFile = "shader.frag";
		std::string vertPath = SHADER_FOLDER + vertShaderFile;
		std::string fragPath = SHADER_FOLDER + fragShaderFile;
		std::ifstream fileStream;

		std::string vertText;
		{
			fileStream.open(vertPath, std::ios::in);
			if (!fileStream.is_open())
			{
				return 1;
			}
			std::stringstream rawVert;
			rawVert << fileStream.rdbuf();

			std::string vertText = rawVert.str();
			fileStream.close();
			const char* vert_shader_text = vertText.c_str();

			vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &vert_shader_text, NULL);
			glCompileShader(vertex_shader);

			GLint isCompiled = 0;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

				// Provide the infolog in whatever manor you deem best.
				// Exit with failure.
				glDeleteShader(vertex_shader); // Don't leak the shader.
				std::cout << "VERTEX COMPILE ERROR :\n" << errorLog.data() << std::endl;
				return 1;
			}
		}

		std::string fragText;
		{
			std::stringstream rawFrag;
			fileStream.open(fragPath, std::ios::in);
			if (!fileStream.is_open())
			{
				return 1;
			}
			rawFrag << fileStream.rdbuf();
			std::string fragText = rawFrag.str();
			fileStream.close();
			const char* fragment_shader_text = fragText.c_str();

			fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
			glCompileShader(fragment_shader);

			GLint isCompiled = 0;
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);

				// Provide the infolog in whatever manor you deem best.
				// Exit with failure.
				glDeleteShader(fragment_shader); // Don't leak the shader.
				std::cout << "FRAG COMPILE ERROR :\n" << errorLog.data() << std::endl;
				return 1;
			}
		}

		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
	}

	mvp_location = glGetUniformLocation(program, "MVP");

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec2));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec2) + sizeof(vec2)));

	// Load all vulkan related data
	VulkanOGLInterop vulkanData;
	vulkanData.loadVulkanData();

	glUseProgram(program);
	GLuint diffTex = glGetUniformLocation(program, "colorSampler");
	GLuint depthTex = glGetUniformLocation(program, "depthSampler");

	glUniform1i(diffTex, 0);
	glUniform1i(depthTex, 1);

	GLuint textureID;
	GLuint depthID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glTextureStorageMem2DEXT(textureID, 1, GL_RGB8, WIDTH, HEIGHT, vulkanData.memoryImage, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glCreateTextures(GL_TEXTURE_2D, 1, &depthID);
	// Commented but left for debug, this is the format if we want to display the tex
	//glTextureStorageMem2DEXT(depthID, 1, GL_R32F, WIDTH, HEIGHT, vulkanData.memoryDepth, 0);
	glTextureStorageMem2DEXT(depthID, 1, GL_DEPTH_COMPONENT32F, WIDTH, HEIGHT, vulkanData.memoryDepth, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, depthID);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		mat4x4_identity(m);
		mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthID);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}