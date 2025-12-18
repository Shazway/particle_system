/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 01:04:07 by tmoragli          #+#    #+#             */
/*   Updated: 2025/12/17 15:56:24 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define CL_TARGET_OPENCL_VERSION 300
#define CL_GL_SHARING

// Useful includes
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>
#include <GL/glx.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include "error_msg.hpp"
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "define.hpp"

namespace psys {
	struct float3 {
		float x, y, z;
	};

	struct Color {
		//red, green, blue, opacity
		float r;
		float g;
		float b;
		float o;
		Color(): r(0), g(0), b(0), o(0) {}
		Color(float r, float g, float b, float o): r(r), g(g), b(b), o(o) {}
		Color(float r, float g, float b): r(r), g(g), b(b), o(1) {}
	};

	struct rgb {
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	// Constants
	//r, g, b
	//Panel of colors to distinguish faces
	const std::vector<Color> colors = {
		{1.0f, 0.0f, 0.0f},  // Red
		{0.0f, 1.0f, 0.0f},  // Green
		{0.0f, 0.0f, 1.0f},  // Blue
		{1.0f, 1.0f, 0.0f},  // Yellow
		{1.0f, 0.0f, 1.0f},  // Magenta
		{0.0f, 1.0f, 1.0f},  // Cyan
		{0.5f, 0.5f, 0.5f},  // Gray
		{1.0f, 0.5f, 0.0f},  // Orange
		{0.5f, 0.0f, 0.5f},  // Purple
		{0.0f, 0.5f, 0.5f},  // Teal
	};
	const std::vector<Color> grey_nuances = {
		{0.1f, 0.1f, 0.1f},
		{0.125f, 0.125f, 0.125f},
		{0.25f, 0.25f, 0.25f},
		{0.3f, 0.3f, 0.3f},
		{0.35f, 0.35f, 0.35f},
		{0.5f, 0.5f, 0.5f}
	};

	const float movespeed = 0.1f;
	const unsigned int cubeSize = 15;
	const float sphereRadius = 1.0f;

	struct particle {
		float3 pos;
		float3 velocity;
		float3 color;
		float3 pos_prev;
		float3 trail[TRAIL_SAMPLES];
		float trail_timer;
		float trail_head;
	};

	struct mass {
		float3 pos;
		float3 rotationTangent;
		float intensity;
		float radius;
	};

	enum particleShape {
		SPHERE,
		CUBE
	};

	class Camera;

	class particle_system
	{
		public:
			particle_system(const size_t &nbParticles);
			~particle_system();

			//Init functions
			bool initCLdata();
			void run();
		private:
			bool initContext();
			void initSimData();
			bool initQueue();
			bool initPrograms();
			bool initKernels();
			bool initSharedBufferData();
			void initShaders();
			const char *get_CL_program(const std::string &path);
			bool selectDevice();

			//Runtime functions
			cl_event enqueueUpdateParticles();
			bool enqueueInitCubeParticles();
			bool enqueueInitSphereParticles();
			void resetSimulation();
			void update_mass_tangent(float x, float y, float z);
			void update_mass_position(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
			void update_window_size(int height, int width);
			void setParticleCount(size_t newCount);

			//Exit functions
			bool freeCLdata(bool err, const std::string &err_msg = "");

			int initGLFW();

			// Event hook actions
			void keyAction(int key, int scancode, int action, int mods);
			void mouseAction(double x, double y);
			void reshapeAction(int width, int height);

			// Event hook callbacks
			static void reshape(GLFWwindow* window, int width, int height); 
			static void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void mouseCallback(GLFWwindow* window, double x, double y);
			void update();
			void updateParticles();
			void updateMovement();
			void findMoveRotationSpeed();
			void tickRandomMassRotation();
			void display();
			void renderParticles(glm::mat4 &viewMatrix);
			void calculateFps();

			void initData();
			bool initGlew();

		private:
			// OpenCL
			cl_int err;
			cl_context context;
			cl_command_queue queue;
			cl_program update_program;
			cl_program init_cube_program;
			cl_program init_sphere_program;
			cl_kernel calculate_position;
			cl_kernel init_particles_cube;
			cl_kernel init_particles_sphere;
			cl_platform_id selected_platform;
			cl_device_id selected_device;
			cl_uint num_platforms;
			cl_uint num_devices;
			cl_mem particleBufferCL;

			// OpenGL
			GLuint particleBufferGL;
			GLuint vao;
			GLuint shaderProgram;
			GLuint spaghettiShaderProgram;

			// Window		
			int windowHeight;
			int windowWidth;
			GLFWwindow* _window;

			// Useful simulation
			bool resetSim;
			bool massFollow;
			bool massDisplay;
			bool trailingMode;
			bool spaghettiMode;
			bool randomMassRotation;
			particleShape reset_shape;
			size_t nb_particles;
			size_t default_nb_particles;
			size_t particleBufferSize;
			mass m;
			float randomRotationTimer;
			float nextRandomRotationDelay;

			// Camera and view
			glm::mat4 projectionMatrix;
			Camera camera;

			// Keys states and runtime booleans
			bool keyStates[348];
			bool ignoreMouseEvent;
			bool mouseCaptureToggle;
			bool firstMouseInput;
			double lastMouseX;
			double lastMouseY;

			// Player speed
			float moveSpeed;
			float rotationSpeed;

			// FPS counter
			int frameCount;
			double lastFrameTime;
			double currentFrameTime;
			int fps;

			// Simulation time
			std::chrono::steady_clock::time_point start;
			std::chrono::steady_clock::time_point end;
			float delta;
			std::mt19937 rng;
	};
};

GLuint compileShader(const char* filePath, GLenum shaderType);
GLuint createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath);
