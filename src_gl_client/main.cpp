/*

	Very simple, very triangle.
	Goal is to get image from Vulkan and render it as quad to screen.

*/

#include <windows.h>

//#include <glad/gl.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <linmath.h>

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct VertexVulkan {
	vec2 position;
	vec2 uv;
	vec3 color;
};

struct Vertex3D {
	vec3 position;
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

int CreateProgram(GLuint& program, std::string vertShaderFile, std::string fragShaderFile)
{
	GLuint vertex_shader, fragment_shader;
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
	return 0;
}

int main()
{
	GLFWwindow* window;
	GLuint vx_vulkan_buffer, vx_cube_buffer, programVk, programCube;
	GLint mvp_location, mvp_cube_location;//, vpos_location, vcol_location, vuv_location;

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
	VertexVulkan vert0 = { {-1.f, -1.f},	{0.f,1.f},	{1.f,0.f,0.f} };
	VertexVulkan vert1 = { {1.f, -1.f},		{1.f,1.f},	{0.f,1.f,0.f} };
	VertexVulkan vert2 = { {-1.f, 1.f},		{0.f,0.f},	{0.f,0.f,1.f} };
	VertexVulkan vert3 = { {1.f, 1.f},		{1.f,0.f},	{1.f,1.f,1.f} };

	std::vector<VertexVulkan> mesh;
	mesh.push_back(vert0);
	mesh.push_back(vert1);
	mesh.push_back(vert2);
	mesh.push_back(vert2);
	mesh.push_back(vert1);
	mesh.push_back(vert3);

	if(CreateProgram(programVk, "shader.vert", "shader.frag") != 0)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glUseProgram(programVk);
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &vx_vulkan_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vx_vulkan_buffer);
	glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(VertexVulkan), &mesh[0], GL_STATIC_DRAW);

	mvp_location = glGetUniformLocation(programVk, "MVP");

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexVulkan), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexVulkan), (void*)sizeof(vec2));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexVulkan), (void*)(sizeof(vec2) + sizeof(vec2)));

	// Load all vulkan related data
	VulkanOGLInterop vulkanData;
	vulkanData.loadVulkanData();

	GLuint diffTex = glGetUniformLocation(programVk, "colorSampler");
	GLuint depthTex = glGetUniformLocation(programVk, "depthSampler");

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


	// Create awesome cube.
	std::vector<Vertex3D> cubeVertices;
	cubeVertices.push_back({ {-1.f, -1.f, -1.f},{0.f,0.f},	{1.f,0.f,0.f} });
	cubeVertices.push_back({ {1.f, -1.f, -1.f},	{1.f,0.f},	{0.f,1.f,0.f} });
	cubeVertices.push_back({ {-1.f, 1.f, -1.f},	{0.f,1.f},	{0.f,0.f,1.f} });
	cubeVertices.push_back({ {1.f, 1.f, -1.f},	{1.f,1.f},	{1.f,1.f,1.f} });
	cubeVertices.push_back({ {-1.f, -1.f, 1.f},	{0.f,0.f},	{1.f,0.f,0.f} });
	cubeVertices.push_back({ {1.f, -1.f, 1.f},	{1.f,0.f},	{0.f,1.f,0.f} });
	cubeVertices.push_back({ {-1.f, 1.f, 1.f},	{0.f,1.f},	{0.f,0.f,1.f} });
	cubeVertices.push_back({ {1.f, 1.f, 1.f},	{1.f,1.f},	{1.f,1.f,1.f} });
	float scale = 0.5f;
	for (auto& c : cubeVertices)
	{
		c.position[0] *= scale;
		c.position[1] *= scale;
		c.position[2] *= scale;

		c.color[0] = .7f;
		c.color[1] = .7f;
		c.color[2] = .7f;
	}

	std::vector<Vertex3D> cube;
	// FRONT FACE
	{
		cube.push_back(cubeVertices[0]);
		cube.push_back(cubeVertices[1]);
		cube.push_back(cubeVertices[2]);
		cube.push_back(cubeVertices[2]);
		cube.push_back(cubeVertices[1]);
		cube.push_back(cubeVertices[3]);
	}
	// BACK FACE
	{
		cube.push_back(cubeVertices[4]);
		cube.push_back(cubeVertices[5]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[5]);
		cube.push_back(cubeVertices[7]);
	}
	// TOP FACE
	{
		cube.push_back(cubeVertices[2]);
		cube.push_back(cubeVertices[3]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[3]);
		cube.push_back(cubeVertices[7]);
	}
	// BOTTOM FACE
	{
		cube.push_back(cubeVertices[0]);
		cube.push_back(cubeVertices[4]);
		cube.push_back(cubeVertices[1]);
		cube.push_back(cubeVertices[1]);
		cube.push_back(cubeVertices[4]);
		cube.push_back(cubeVertices[5]);
	}
	// LEFT FACE
	{
		cube.push_back(cubeVertices[4]);
		cube.push_back(cubeVertices[0]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[6]);
		cube.push_back(cubeVertices[0]);
		cube.push_back(cubeVertices[2]);
	}
	// RIGHT FACE
	{
		cube.push_back(cubeVertices[1]);
		cube.push_back(cubeVertices[5]);
		cube.push_back(cubeVertices[3]);
		cube.push_back(cubeVertices[3]);
		cube.push_back(cubeVertices[5]);
		cube.push_back(cubeVertices[7]);
	}

	// Add a cube to scene.
	if (CreateProgram(programCube, "cube.vert", "cube.frag") != 0)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glUseProgram(programCube);
	GLuint VCube;
	glGenVertexArrays(1, &VCube);
	glBindVertexArray(VCube);

	glGenBuffers(1, &vx_cube_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vx_cube_buffer);
	glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(Vertex3D), &cube[0], GL_STATIC_DRAW);

	mvp_cube_location = glGetUniformLocation(programCube, "MVP");

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)sizeof(vec3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)(sizeof(vec3) + sizeof(vec2)));

	GLuint timeUniform = glGetUniformLocation(programCube, "time");
	GLuint mUniform = glGetUniformLocation(programCube, "M");
	GLuint vUniform = glGetUniformLocation(programCube, "V");
	GLuint pUniform = glGetUniformLocation(programCube, "P");

	glClearColor(0.f, 0.f, 0.f, 1.f);

	auto startTime = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		//vec3 eye = { camera.x,camera.y,camera.z };
		//vec3 center = { camera.x,camera.y,camera.z + 1 };
		//vec3 up = { 0,1,0 };
		//mat4x4_look_at(v, eye, center, up);
		//mat4x4_perspective(p, 1.57f, ratio, 1, 10);
		//mat4x4_mul(mvp, p, m);

		glUseProgram(programVk);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glm::mat4 mvp, m, v, p;
		m = glm::identity<glm::mat4>();
		v = glm::identity<glm::mat4>();
		p = glm::ortho(-1.f, 1.f, -1.f, 1.f);
		mvp = p * v * m;
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&mvp[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthID);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glUseProgram(programCube);
		glEnable(GL_DEPTH_TEST);
		glm::mat4 mvpCube, mCube, vCube, pCube;
		mCube = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		vCube = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		pCube = glm::perspective(glm::radians(30.0f), ratio, 1.5f, 5.0f);
		mvpCube = pCube * vCube * mCube;

		glUniformMatrix4fv(mUniform, 1, GL_FALSE, (const GLfloat*)&mCube[0][0]);
		glUniformMatrix4fv(vUniform, 1, GL_FALSE, (const GLfloat*)&vCube[0][0]);
		glUniformMatrix4fv(pUniform, 1, GL_FALSE, (const GLfloat*)&pCube[0][0]);
		//glUniformMatrix4fv(mvp_cube_location, 1, GL_FALSE, (const GLfloat*)&mvpCube[0][0]);
		glUniform1f(timeUniform, glm::sin(time));

		glBindVertexArray(VCube);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}