#include "particle_system.hpp"

GLuint compileShader(const char* filePath, GLenum shaderType)
{
	std::ifstream shaderFile(filePath);
	if (!shaderFile.is_open()) {
		std::cerr << "Error: Shader file could not be opened: " << filePath << std::endl;
		return 0;
	}

	std::stringstream shaderStream;
	shaderStream << shaderFile.rdbuf();
	std::string shaderCode = shaderStream.str();
	const char* shaderSource = shaderCode.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "Error: Shader compilation failed\n" << infoLog << std::endl;
	}
	return shader;
}

GLuint createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath)
{
	GLuint shaderProgram = glCreateProgram();

	std::cout << "Compiling vertex shader" << std::endl;
	GLuint vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
	std::cout << "Compiling fragment shader" << std::endl;
	GLuint fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	GLuint geometryShader = -1;
	if (std::string(geometryShaderPath) != "")
	{
		std::cout << "Compiling geometry shader" << std::endl;
		geometryShader = compileShader(geometryShaderPath, GL_GEOMETRY_SHADER);
		glAttachShader(shaderProgram, geometryShader);
	}
	glLinkProgram(shaderProgram);

	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "Error: Shader program linking failed\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (std::string(geometryShaderPath) != "")
		glDeleteShader(geometryShader);
	return shaderProgram;
}
