/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 01:04:07 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/25 01:05:02 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define CL_TARGET_OPENCL_VERSION 300
#define CL_GL_SHARING

// Useful includes
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <array>
#include <limits>
#include <memory>
#include <cmath>
#include <algorithm>
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


#define COMMANDS_LIST														\
	"Controls:\n"															\
	"'H': Display commands\n"												\
	"\n"																	\
	"Camera commands:\n"													\
	"'Z', 'W': Move forward\n"												\
	"'Q', 'A': Move left\n"													\
	"'S': Move back\n"														\
	"'D': Move right\n"														\
	"' ': Move up\n"														\
	"'V': Move down\n"														\
	"'C': Toggle mouse control for camera angle\n"							\
	"(Mouse control will hide the cursor until 'C' is pressed again)\n"		\
	"\n"																	\
	"Simulation controls:\n"												\
	"'0': Reset simulation to the cube\n"									\
	"'1': Reset simulation to the sphere\n"									\
	"\n"																	\
	"Mass commands:\n"														\
	"'M': Toggle mass activity\n"											\
	"'+': Increase gravitational pull\n"									\
	"'-': Decrease gravitational pull (becomes a push after 0 threshold)\n" \
	"'U': Increase X rotation angle for particles\n"						\
	"'I': Increase Y rotation angle for particles\n"						\
	"'O': Increase Z rotation angle for particles\n"						\
	"'J': Decrease X rotation angle for particles\n"						\
	"'K': Decrease Y rotation angle for particles\n"						\
	"'L': Decrease Z rotation angle for particles\n"						\
	"'T': Reset rotation angle to default (0, 1, 0)\n"						\
	"'F': Toggle mass follow on cursor (follows screen center if mouse control is active)"



#define W_WIDTH 1440
#define W_HEIGHT 1080

namespace psys {
	struct vec3 {
		float x, y, z;

		vec3(float x = 0.0, float y = 0.0, float z = 0.0) : x(x), y(y), z(z) {}
	};
	struct vec4 {
		float x, y, z, w;

		vec4(float x = 0.0, float y = 0.0, float z = 0.0, float w = 0.0) : x(x), y(y), z(z), w(w) {}
	};

	struct float3 {
		float x, y, z;
	};

	struct vec2 {
		float x, y;

		vec2(float x = 0.0, float y = 0.0) : x(x), y(y) {}
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

	class particle_system {
		public:
			particle_system(const size_t &nbParticles);
			~particle_system();

			//Init functions
			bool initCLdata();
			bool initContext();
			void initSimData();
			bool initQueue();
			bool initPrograms();
			bool initKernels();
			bool initSharedBufferData();
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
			void update_mouse_pos(int x, int y);

			//Exit functions
			bool freeCLdata(bool err, const std::string &err_msg = "");

			// OpenCL data
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

			// OpenGL data
			GLuint particleBufferGL;
			
			// Useful simulation data
			bool resetSim;
			bool massFollow;
			bool massDisplay;
			int windowHeight;
			int windowWidth;
			vec2 mousePos;
			particleShape reset_shape;
			size_t nb_particles;
			size_t particleBufferSize;
			mass m;
	};
};
