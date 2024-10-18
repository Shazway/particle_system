/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 15:14:39 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/18 20:35:32 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"

namespace psys {
	particle_system::particle_system(const size_t &nbParticles) : nb_particles(nbParticles) {
		std::cout << "Starting particle system with: " << nb_particles << " particles" << std::endl;
		context = nullptr;
		queue = nullptr;
		update_program = nullptr;
		init_cube_program = nullptr;
		calculate_position = nullptr;
		init_particles_cube = nullptr;
		particleBufferCL = nullptr;
		
		// No mass or intensity at first
		m.intensity = 0.0f;
		m.radius = 5.0f;
		m.x = 0.0f;
		m.y = 0.0f;
		m.z = -10.0f;

		resetSim = false;
	}

	particle_system::~particle_system() {
		freeCLdata(false);
	}

	/*
		Resets the simulation,
		Releases all previous data and sets it back to the cube
	*/
	void particle_system::resetSimulation() {
		std::cout << "Resetting the simulation back to the cube" << std::endl;
		freeCLdata(false);
		initCLdata();
		resetSim = false;
	}

	/*
		Computes runtime particle positions depending on mass point and intensity
	*/
	cl_event particle_system::enqueueUpdateParticles() {
		cl_int err;
		cl_event kernel_event;
		err = clSetKernelArg(calculate_position, 0, sizeof(cl_mem), &particleBufferCL);
		if (err != CL_SUCCESS) {
			std::cerr << "Failed to set args 0 for OpenCL: "<<err<< std::endl;
			return 0;
		}
		err = clSetKernelArg(calculate_position, 1, sizeof(mass), &m);
		if (err != CL_SUCCESS) {
			std::cerr << "Failed to set args 1 for OpenCL: "<<err<< std::endl;
			return 0;
		}

		err = clEnqueueAcquireGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Failed to acquire GL objects for OpenCL: "<<err<< std::endl;
			return 0;
		}
		clFinish(queue);
		err = clEnqueueNDRangeKernel(queue, calculate_position, 1, NULL, &nb_particles, NULL, 0, NULL, &kernel_event);
		if (err != CL_SUCCESS) {
			std::cerr << "Failed to enqueue kernel for OpenCL: "<<err<< std::endl;
			return 0;
		}
		err = clEnqueueReleaseGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Failed to dequeue kernel for OpenCL: "<<err<< std::endl;
			return 0;
		}
		clFinish(queue);
		return kernel_event;
	}

	/*
		Computes the first particle positions inside a cube
		depending on cube size and number of particles
	*/
	bool particle_system::enqueueInitCubeParticles() {
		//Acquiring buffer
		err = clEnqueueAcquireGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
			return freeCLdata(true, ENQUEUE_BUFFER_CL_GL_ERR);

		// Set kernel arguments
		err = clSetKernelArg(init_particles_cube, 0, sizeof(cl_mem), &particleBufferCL);
		if (err != CL_SUCCESS)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);
		err = clSetKernelArg(init_particles_cube, 1, sizeof(unsigned int), &cubeSize);
		if (err != CL_SUCCESS)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);

		// Execute the init kernel
		size_t globalWorkSize = nb_particles;
		err = clEnqueueNDRangeKernel(queue, init_particles_cube, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
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
		return true;
	}
	/*
		Computes the first particle positions inside a sphere
		depending on sphere size and number of particles
	*/
	bool particle_system::enqueueInitSphereParticles() {
		// .. TODO
		return true;
	}

	/*
		Initialises and allocates the CL/GL shared buffer
		on the VRAM and checks for errors
	*/
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

	/*
		Releases all CL/GL data from the particle_system
	*/
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
		if (init_particles_cube)
			clReleaseKernel(init_particles_cube);
		if (init_cube_program)
			clReleaseProgram(init_cube_program);
		if (update_program)
			clReleaseProgram(update_program);
		if (queue)
			clReleaseCommandQueue(queue);
		if (context)
			clReleaseContext(context);
		context = nullptr;
		queue = nullptr;
		update_program = nullptr;
		init_cube_program = nullptr;
		calculate_position = nullptr;
		init_particles_cube = nullptr;
		particleBufferCL = nullptr;
		return !err;
	}

	/*
		Selects a device (GPU preferably) that supports
		cl_khr_gl_sharing, essential for such computing
	*/
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
						continue ;
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

	/*
		Parses .cl files in kernel_srcs/ for program loading
	*/
	const char *particle_system::get_CL_program(const std::string& path) {
		// Static string to hold the program content (so we can return a reference)
		static std::string content;
		content.clear();
		std::ifstream file(path, std::ios::in);
		if (!file.is_open()) {
			return nullptr;
		}

		// Ensure the file is read in a valid encoding (UTF-8)
		file.imbue(std::locale::classic());

		// Read the file content into a stringstream
		std::stringstream buffer;
		buffer << file.rdbuf();
		content = buffer.str();
		return content.c_str();
	}

	/*
		Initialises cl_context
	*/
	bool particle_system::initContext() {
		// Context properties for CL/GL buffer sharing
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

		// Checking context info for errors
		err = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(selected_platform), &selected_platform, nullptr);
		if (err != CL_SUCCESS) {
			std::cerr << "Error: Failed to get OpenCL device: " << err << std::endl;
			return freeCLdata(true, "");
		}
		return true;
	}

	/*
		Initialises command queue
	*/
	bool particle_system::initQueue() {
		// Creating command queue
		cl_queue_properties queue_properties[] = {0};
		queue = clCreateCommandQueueWithProperties(context, selected_device, queue_properties, &err);
		if (err != CL_SUCCESS || !queue)
			return freeCLdata(true, QUEUE_CREATE_ERR);
		return true;
	}

	/*
		Initialises and builds openCL programs (init and runtime) with the cl code in kernel_srcs/
	*/
	bool particle_system::initPrograms() {
		// Init update program
		// OpenCL kernel source for updating particles
		const char *updateParticlesSrc = get_CL_program("kernel_srcs/update_particles.cl");
		if (!updateParticlesSrc)
			return freeCLdata(true, FETCH_CL_FILE_ERR);

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
		// OpenCL kernel source for initializing particles
		const char *initParticlesCubeSrc = get_CL_program("kernel_srcs/init_particles_cube.cl");
		if (!initParticlesCubeSrc)
			return freeCLdata(true, FETCH_CL_FILE_ERR);

		//Init cube_init program
		// Create program for initializing particles
		init_cube_program = clCreateProgramWithSource(context, 1, &initParticlesCubeSrc, nullptr, &err);
		if (err != CL_SUCCESS || !init_cube_program)
			return freeCLdata(true, std::string(PROGRAM_CREATE_ERR) + " init_particles_cube");

		// Build the init program
		err = clBuildProgram(init_cube_program, 1, &selected_device, nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			size_t len;
			char buffer[2048];
			bzero(buffer, sizeof(buffer));
			clGetProgramBuildInfo(init_cube_program, selected_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return freeCLdata(true, std::string(PROGRAM_BUILD_ERR) + " init_cube_program");
		}
		return true;
	}

	/*
		Initialises openCL kernels with the cl programs
	*/
	bool particle_system::initKernels() {
		// Create init particles kernel
		init_particles_cube = clCreateKernel(init_cube_program, "init_particles_cube", &err);
		if (err != CL_SUCCESS || !init_particles_cube)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " init_cube_program");

		// Create update particles kernel
		calculate_position = clCreateKernel(update_program, "updateParticles", &err);
		if (err != CL_SUCCESS || !calculate_position)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " update_program");
		return true;
	}

	/*
		Calls all initialisation methods and enqueues the initCube
		kernel
	*/
	bool particle_system::initCLdata() {
		// Select device (GPU)
		if (!selectDevice())
			return freeCLdata(true, DEVICE_GET_ERR);

		if (!initContext()
			|| !initQueue()
			|| !initPrograms()
			|| !initKernels())
			return false;

		if (nb_particles == 0)
			return freeCLdata(true, NO_PARTICLES_ERR);
		// Create a buffer that OpenCL can use
		particleBufferCL = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, particleBufferGL, &err);
		if (err != CL_SUCCESS || !particleBufferCL)
			return freeCLdata(true, BUFFER_CREATE_ERR);

		// Call init_cube kernel to init the particles in a cube
		if (!enqueueInitCubeParticles())
			return false;
		std::cout << "OpenCL particles data initialized directly on GPU" << std::endl;
		return true;
	}
};
