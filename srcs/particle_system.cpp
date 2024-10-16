/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 15:14:39 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/16 23:52:49 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"

namespace psys {
	particle_system::particle_system(const size_t &nbParticles) : nb_particles(nbParticles) {
		std::cout << "Starting particle system with: " << nb_particles << " particles" << std::endl;
		context = nullptr;
		queue = nullptr;
		update_program = nullptr;
		init_program = nullptr;
		calculate_position = nullptr;
		initialize_particles = nullptr;
		particleBufferCL = nullptr;
	}

	particle_system::~particle_system() {
		freeCLdata(false);
	}

	bool particle_system::initSharedBufferData() {
		std::cout << "Initialising OpenGL/OpenCL shared buffer" << std::endl;
		// Generate OpenGL buffer
		glGenBuffers(1, &particleBufferGL);
		glBindBuffer(GL_ARRAY_BUFFER, particleBufferGL);
		
		// Allocate space for particles in the OpenGL buffer
		glBufferData(GL_ARRAY_BUFFER, (sizeof(particle) * nb_particles), nullptr, GL_DYNAMIC_DRAW);
		
		// Check for OpenGL errors
		GLenum glErr = glGetError();
		if (glErr != GL_NO_ERROR) {
			std::cerr << "OpenGL error during buffer initialization: " << glErr << std::endl;
			return false;
		}

		// Unbind the buffer to avoid accidental modification later
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		// Ensure OpenGL commands are finished before proceeding
		glFinish();
		return true;
	}

	bool particle_system::freeCLdata(bool err, const std::string &err_msg) {
		std::cout << "OpenCL freeData call" << std::endl;
		if (err)
			std::cerr << "Error: " << err_msg << std::endl;
		// Avoid double frees by checking and setting to nullptr
		if (queue)
			clFlush(queue);
		if (particleBufferCL) {
			// TODO: dynamically release queue if acquired
			//clEnqueueReleaseGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
			clReleaseMemObject(particleBufferCL);
		}
		if (calculate_position)
			clReleaseKernel(calculate_position);
		if (initialize_particles)
			clReleaseKernel(initialize_particles);
		if (init_program)
			clReleaseProgram(init_program);
		if (queue)
			clReleaseCommandQueue(queue);
		if (context)
			clReleaseContext(context);
		context = nullptr;
		queue = nullptr;
		update_program = nullptr;
		init_program = nullptr;
		calculate_position = nullptr;
		initialize_particles = nullptr;
		particleBufferCL = nullptr;
		return !err;
	}

	bool particle_system::selectDevice() {
		std::cout << "Selecting device (GPU)..." << std::endl;
		// Step 1: Get platform IDs
		cl_uint num_platforms;
		err = clGetPlatformIDs(0, nullptr, &num_platforms);
		if (err != CL_SUCCESS || num_platforms == 0)
			return freeCLdata(true, PLATFORM_ID_ERR);
		std::vector<cl_platform_id> platforms(num_platforms);
		err = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
		if (err != CL_SUCCESS)
			return freeCLdata(true, PLATFORM_GET_ERR);

		//Iterate and select right platform
		for (cl_uint i  = 0; i < num_platforms; ++i) {
			cl_platform_id platId = platforms[i];

			err = clGetDeviceIDs(platId, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
			if (err != CL_SUCCESS || num_devices == 0)
			{
				std::cerr << "No GPU on this platform" << i + 1 << std::endl;
				continue ;
			}

			std::vector<cl_device_id> devices(num_devices);
			err = clGetDeviceIDs(platId, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), nullptr);
			if (err != CL_SUCCESS)
			{
				std::cerr << "Error: Unable to get devices on this platform" << std::endl;
				continue ;
			}

			//Choosing the device of the platform
			for (cl_uint j = 0; j < num_devices; ++j)
			{
				cl_device_id device = devices[j];
				char extensions[4048];
				err = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, nullptr);
				if (err != CL_SUCCESS) {
					std::cerr << "Error: " << err << " Unable to get device extensions for device " << j + 1 << std::endl;
					continue;
				}
				//Mandatory check for the 'cl_khr_gl_sharing' extension
				std::string extensions_list(extensions);
				if (extensions_list.find("cl_khr_gl_sharing") != std::string::npos)
				{
					char device_name[128];
					err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
					if (err != CL_SUCCESS) {
						std::cerr << "Error: Unable to get device name for device " << j + 1 << std::endl;
						continue;
					}
					selected_device = device;
					selected_platform = platId;
					std::cout << device_name << " selected" << std::endl;
					return true;
				}
				else
				{
					// Display device name but note that cl_khr_gl_sharing is not supported
					char device_name[128];
					err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, nullptr);
					if (err != CL_SUCCESS)
					{
						std::cerr << "Error: Unable to get device name for device " << j + 1 << std::endl;
						continue;
					}
					std::cout << "Platform " << i + 1 << " Device " << j + 1 << " does NOT support cl_khr_gl_sharing: " << device_name << std::endl;
				}
			}
		}
		return true;
	}

	bool particle_system::initCLdata() {
		if (!selectDevice())
			return false;

		const cl_context_properties properties[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)selected_platform,  // the OpenCL platform you are using
			0
		};
		// Create OpenCL context
		context = clCreateContext(properties, num_devices, &selected_device, nullptr, nullptr, &err);
		if (err != CL_SUCCESS || !context) {
			std::cout << err << std::endl;
			return freeCLdata(true, CONTEXT_CREATE_ERR);
		}

		err = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(selected_platform), &selected_platform, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Error: Failed to get OpenCL device: " << err << std::endl;
			return freeCLdata(true, "");
		}

		cl_queue_properties queue_properties[] = {0};
		queue = clCreateCommandQueueWithProperties(context, selected_device, queue_properties, &err);
		if (err != CL_SUCCESS || !queue)
			return freeCLdata(true, QUEUE_CREATE_ERR);

		// OpenCL kernel source for updating particles
		const char *updateParticlesSrc = R"(
			typedef struct {
				float x, y, z;
			} vec3;

			typedef struct {
				float x, y;
			} vec2;

			typedef struct {
				float r, g, b, o;
			} Color;

			typedef struct {
				vec3 pos;
				Color color;
				float velocity;
			} particle;

			float length(vec3 v) {
				return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
			}

			vec3 normalize(vec3 v) {
				float len = length(v);
				if (len > 0.0f) {
					v.x /= len;
					v.y /= len;
					v.z /= len;
				}
				return v;
			}

			__kernel void updateParticles(__global particle *particles, vec3 mousePos, float deltaTime) {
				int id = get_global_id(0);
				// if (id == 0)
				// 	printf("Particle position: %.1f, %.1f\n", mousePos.x, mousePos.y);
				// Retrieve the particle
				particle p = particles[id];

				// Calculate the direction vector towards the mouse position
				vec3 direction;
				direction.x = mousePos.x - p.pos.x;
				direction.y = mousePos.y - p.pos.y;
				direction.z = mousePos.z - p.pos.z;

				// Calculate the length of the direction vector
				float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

				// Normalize the direction vector to get the unit direction
				if (length > 0.0f) {
					if (direction.x > 0.0f)
						direction.x /= length;
					if (direction.y > 0.0f)
						direction.y /= length;
					if (direction.z > 0.0f)
						direction.z /= length;
				}

				// Update the particle's position
				float movement = p.velocity * deltaTime;
				p.pos.x += direction.x * movement;
				p.pos.y += direction.y * movement;
				p.pos.z += direction.z * movement;

				// Write the updated particle back to the global memory
				particles[id] = p;
			}
		)";

		// Create program for updating particles
		update_program = clCreateProgramWithSource(context, 1, &updateParticlesSrc, nullptr, &err);
		if (err != CL_SUCCESS || !update_program)
			return freeCLdata(true, std::string(PROGRAM_CREATE_ERR) + " update_program");

		err = clBuildProgram(update_program, 1, &selected_device, nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS)
		{
			size_t len = 0;
			char buffer[2048];
			bzero(buffer, sizeof(buffer));
			clGetProgramBuildInfo(update_program, selected_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return freeCLdata(true, std::string(PROGRAM_BUILD_ERR) + " update_program");
		}

		// Create update particles kernel
		calculate_position = clCreateKernel(update_program, "updateParticles", &err);
		if (err != CL_SUCCESS || !calculate_position)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " update_program");

		if (nb_particles == 0)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);

		// Create a buffer that OpenCL can use
		particleBufferCL = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, particleBufferGL, &err);
		if (err != CL_SUCCESS || !particleBufferCL)
			return freeCLdata(true, BUFFER_CREATE_ERR);

		err = clEnqueueAcquireGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
			return freeCLdata(true, ENQUEUE_BUFFER_CL_GL_ERR);

		// OpenCL kernel source for initializing particles
		const char *initParticlesSrc = R"(
			typedef struct {
				float x, y, z;
			} vec3;

			typedef struct {
				float r, g, b, o;
			} Color;

			typedef struct {
				vec3 pos;
				Color color;
				float velocity;
			} particle;

			__kernel void init_particles(__global particle* particles) {
				int id = get_global_id(0);

				particles[id].pos.x = (id % 10 + 0.2) * 0.1;
				particles[id].pos.y = (id % 10 + 0.2) * 0.1;
				particles[id].pos.z = -10.0f;

				particles[id].color.r = 1.0f;
				particles[id].color.g = 0.5f;
				particles[id].color.b = 0.2f;
				particles[id].color.o = 1.0f;

				particles[id].velocity = (id % 10 + 1) * 0.1f;
			}
		)";

		// Create program for initializing particles
		init_program = clCreateProgramWithSource(context, 1, &initParticlesSrc, nullptr, &err);
		if (err != CL_SUCCESS || !init_program)
			return freeCLdata(true, std::string(PROGRAM_CREATE_ERR) + " init_program");

		// Build the init program
		err = clBuildProgram(init_program, 1, &selected_device, nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			size_t len;
			char buffer[2048];
			bzero(buffer, sizeof(buffer));
			clGetProgramBuildInfo(init_program, selected_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return freeCLdata(true, std::string(PROGRAM_BUILD_ERR) + " init_program");
		}

		// Create init particles kernel
		initialize_particles = clCreateKernel(init_program, "init_particles", &err);
		if (err != CL_SUCCESS || !initialize_particles)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " init_program");

		// Set kernel argument
		err = clSetKernelArg(initialize_particles, 0, sizeof(cl_mem), &particleBufferCL);
		if (err != CL_SUCCESS)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);

		// Execute the init kernel
		size_t globalWorkSize = nb_particles;
		err = clEnqueueNDRangeKernel(queue, initialize_particles, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
		if (err != CL_SUCCESS)
		{
			std::cout << "Error code: " << err << std::endl;
			return freeCLdata(true, ENQUEUE_NDRANGE_KERNEL_ERR);
		}
		clFinish(queue);
		err = clEnqueueReleaseGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
			return freeCLdata(true, RELEASE_BUFFER_CL_GL_ERR);
		clFinish(queue);

		std::cout << "OpenCL particles data initialized directly on GPU" << std::endl;
		return true;
	}
};
