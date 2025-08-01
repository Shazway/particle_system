/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 15:14:39 by tmoragli          #+#    #+#             */
/*   Updated: 2025/08/01 02:37:40 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"
#include "cl_ext_loader.hpp"

namespace psys
{
	particle_system::particle_system(const size_t &nbParticles) : nb_particles(nbParticles)
	{
		std::cout << "Starting particle system with: " << nb_particles << " particles" << std::endl;

		initSimData();
		reset_shape = particleShape::CUBE;
		initGLFW();
		initGlew();
		reshapeAction(windowWidth, windowHeight);
		initSharedBufferData();
		initShaders();
	}

	particle_system::~particle_system()
	{
		freeCLdata(false);
	}

	bool particle_system::initGlew()
	{
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			std::cerr << "Error initializing GLEW" << glewGetErrorString(err) << std::endl;
			return false;
		}
		return true;
	}

	void particle_system::initData()
	{
		// Keys states and runtime booleans
		bzero(keyStates, sizeof(keyStates));
		ignoreMouseEvent	= IGNORE_MOUSE;
		mouseCaptureToggle	= CAPTURE_MOUSE;

		// Window size
		windowHeight	= W_HEIGHT;
		windowWidth		= W_WIDTH;

		// FPS counter
		frameCount			= 0;
		lastFrameTime		= 0.0;
		currentFrameTime	= 0.0;
		fps					= 0.0;

		// Player data
		moveSpeed		= 0.0;
		rotationSpeed	= 0.0;
	}

	void particle_system::run()
	{
		// Main loop
		while (!glfwWindowShouldClose(_window))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			update();
			glfwPollEvents();
		}
	}

	void particle_system::findMoveRotationSpeed()
	{
		// Calculate delta time
		static auto lastTime = std::chrono::steady_clock::now();
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsedTime = currentTime - lastTime;
		float deltaTime = std::min(elapsedTime.count(), 0.1f);
		lastTime = currentTime;


		// Apply delta to rotation and movespeed
		if (keyStates[GLFW_KEY_LEFT_CONTROL])
			moveSpeed = (MOVEMENT_SPEED * 2.0) * deltaTime;
		else
			moveSpeed = MOVEMENT_SPEED * deltaTime;
		rotationSpeed = (ROTATION_SPEED - 1.5) * deltaTime;
		start = std::chrono::steady_clock::now();
	}

	void particle_system::updateMovement()
	{
		// Camera movement with keys
		if (keyStates[GLFW_KEY_W]) camera.move(moveSpeed, 0.0, 0.0);
		if (keyStates[GLFW_KEY_A]) camera.move(0.0, moveSpeed, 0.0);
		if (keyStates[GLFW_KEY_S]) camera.move(-moveSpeed, 0.0, 0.0);
		if (keyStates[GLFW_KEY_D]) camera.move(0.0, -moveSpeed, 0.0);
		if (keyStates[GLFW_KEY_SPACE]) camera.move(0.0, 0.0, -moveSpeed);
		if (keyStates[GLFW_KEY_LEFT_SHIFT]) camera.move(0.0, 0.0, moveSpeed);

		// Camera rotation with keys
		if (keyStates[GLFW_KEY_UP]) camera.rotate(0.0f, -1.0f, rotationSpeed * 150.0f);
		if (keyStates[GLFW_KEY_DOWN]) camera.rotate(0.0f, 1.0f, rotationSpeed * 150.0f);
		if (keyStates[GLFW_KEY_RIGHT]) camera.rotate(1.0f, 0.0f, rotationSpeed * 150.0f);
		if (keyStates[GLFW_KEY_LEFT]) camera.rotate(-1.0f, 0.0f, rotationSpeed * 150.0f);
	}

	void particle_system::updateParticles()
	{
		if (resetSim) {
			resetSimulation();
			camera.reset();
		}
		else
			enqueueUpdateParticles();
		return ;
	}
	void particle_system::renderParticles(glm::mat4& viewMatrix)
	{
		// Activate shader
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint activeShader = spaghettiMode == true ? spaghettiShaderProgram : shaderProgram;
		glUseProgram(activeShader);

		// Set the view-projection matrix uniform
		glm::mat4 viewProj = projectionMatrix * viewMatrix;
		GLint vpLoc = glGetUniformLocation(activeShader, "u_viewProj");
		glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(viewProj));

		// Bind the VAO
		glBindVertexArray(vao);

		if (spaghettiMode && nb_particles >= 1024)
		{
			glDrawArrays(GL_LINE_STRIP, 0, nb_particles);
		}
		else
		{
			// Draw each particle as a point
			glDrawArrays(GL_POINTS, 0, nb_particles);
		}
		
		// Clean up
		glBindVertexArray(0);
		glUseProgram(0);

		// Render the mass point
		if (massDisplay)
		{
			glPushMatrix();
			glTranslatef(m.pos.x, m.pos.y, m.pos.z);
			glColor3f(0.0f, 0.0f, 0.0f);
			GLUquadric* quad = gluNewQuadric();
			gluSphere(quad, m.radius*0.3, 80, 80);
			gluDeleteQuadric(quad);
			glPopMatrix();
		}
	}
	

	void particle_system::calculateFps()
	{
		frameCount++;
		currentFrameTime = glfwGetTime();

		double timeInterval = currentFrameTime - lastFrameTime;

		if (timeInterval > 1)
		{
			fps = frameCount / timeInterval;

			lastFrameTime = currentFrameTime;
			frameCount = 0;

			std::stringstream title;
			title << "Not ft_minecraft | FPS: " << fps;
			glfwSetWindowTitle(_window, title.str().c_str());
		}
	}

	void particle_system::display()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		// glClearColor(1.0, 1.0, 1.0, 1.0);

		float radY, radX;
		radX = camera.getAngles().x * (M_PI / 180.0);
		radY = camera.getAngles().y * (M_PI / 180.0);

		glm::mat4 viewMatrix = glm::lookAt(
			camera.getPosition(),		// cam position
			camera.getCenter(),			// Look-at point
			glm::vec3(0.0f, 1.0f, 0.0f)	// Up direction
		);

		viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::rotate(viewMatrix, radY, glm::vec3(-1.0f, 0.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, radX, glm::vec3(0.0f, -1.0f, 0.0f));
		viewMatrix = glm::translate(viewMatrix, glm::vec3(camera.getPosition()));
		glLoadMatrixf(glm::value_ptr(viewMatrix));

		// Update the size
		update_window_size(windowWidth, windowHeight);

		// Update the mass position to the cursor in space
		if (massFollow)
			update_mass_position(projectionMatrix, viewMatrix);

		// Call update kernel
		updateParticles();
		
		// Draw particles
		renderParticles(viewMatrix);
		calculateFps();
		glfwSwapBuffers(_window);
	}

	void particle_system::update()
	{
		// Check for delta and apply to move and rotation speeds
		findMoveRotationSpeed();

		// Update player position and orientation
		updateMovement();

		// Mass control
		// Increase tangent vector rotation in x, y z respectively
		if (keyStates[GLFW_KEY_U])
			update_mass_tangent(0.1f, 0.0f, 0.0f);
		if (keyStates[GLFW_KEY_I])
			update_mass_tangent(0.0f, 0.1f, 0.0f);
		if (keyStates[GLFW_KEY_O])
			update_mass_tangent(0.0f, 0.0f, 0.1f);
		// Decrease tangent vector rotation in x, y z respectively
		if (keyStates[GLFW_KEY_J])
			update_mass_tangent(-0.1f, 0.0f, 0.0f);
		if (keyStates[GLFW_KEY_K])
			update_mass_tangent(0.0f, -0.1f, 0.0f);
		if (keyStates[GLFW_KEY_L])
			update_mass_tangent(0.0f, 0.0f, -0.1f);
		if (keyStates[GLFW_KEY_KP_ADD]) m.intensity += 0.1f;
		if (keyStates[GLFW_KEY_KP_SUBTRACT]) m.intensity -= 0.1f;

		display();

		// Register end of frame for the next delta
		end = std::chrono::steady_clock::now(); 
		delta = std::chrono::duration_cast<std::chrono::milliseconds>(start - end);
	}

	int particle_system::initGLFW()
	{
		_window = glfwCreateWindow(windowWidth, windowHeight, "particle_system | FPS: 0", NULL, NULL);
		if (!_window)
		{
			std::cerr << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return 0;
		}
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, reshape);
		glfwSetKeyCallback(_window, keyPress);
		glfwSetCursorPosCallback(_window, mouseCallback);
		glfwMakeContextCurrent(_window);
		glfwSwapInterval(0);
		return 1;
	}

	void particle_system::keyAction(int key, int scancode, int action, int mods)
	{
		(void)scancode;
		(void)mods;

		if (action == GLFW_PRESS)
			keyStates[key] = true;
		else if (action == GLFW_RELEASE)
			keyStates[key] = false;

		if (action == GLFW_PRESS && ((key == GLFW_KEY_M || key == GLFW_KEY_SEMICOLON) && m.intensity))
			m.intensity = 0.0f;
		else if (action == GLFW_PRESS && ((key == GLFW_KEY_M || key == GLFW_KEY_SEMICOLON) && !m.intensity))
			m.intensity = 10.0f;
		if (action == GLFW_PRESS && key == GLFW_KEY_KP_0)
		{
			reset_shape = particleShape::CUBE;
			resetSim = true;
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_KP_1)
		{
			reset_shape = particleShape::SPHERE;
			resetSim = true;
		}
		else if (action == GLFW_PRESS && key == GLFW_KEY_T)
			m.rotationTangent = {0.0f, 1.0f, 0.0f};
		else if (action == GLFW_PRESS && key == GLFW_KEY_F)
			massFollow = !massFollow;
		else if (action == GLFW_PRESS && key == GLFW_KEY_P)
			massDisplay = !massDisplay;
		else if (action == GLFW_PRESS && key == GLFW_KEY_H)
			std::cout << COMMANDS_LIST << std::endl;
		else if (action == GLFW_PRESS && key == GLFW_KEY_C)
			mouseCaptureToggle = !mouseCaptureToggle;
		else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(_window, GL_TRUE);
		else if (action == GLFW_PRESS && key == GLFW_KEY_G)
			spaghettiMode = !spaghettiMode;
	}

	void particle_system::keyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		particle_system *engine = static_cast<particle_system*>(glfwGetWindowUserPointer(window));

		if (engine) engine->keyAction(key, scancode, action, mods);
	}

	void particle_system::reshapeAction(int width, int height)
	{
		glViewport(0, 0, width, height);
		windowHeight = height;
		windowWidth = width;
		// Apply projection matrix operations
		glMatrixMode(GL_PROJECTION);

		// Load identity matrix
		projectionMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 1000.0f);
		glLoadMatrixf(glm::value_ptr(projectionMatrix));
	}

	void particle_system::reshape(GLFWwindow* window, int width, int height)
	{
		particle_system *engine = static_cast<particle_system*>(glfwGetWindowUserPointer(window));

		if (engine) engine->reshapeAction(width, height);
	}


	void particle_system::mouseAction(double x, double y)
	{
		if (!mouseCaptureToggle)
		{
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			camera.updateMousePos(x, y);
			return ;
		}
		static bool firstMouse = true;
		static double lastX = 0, lastY = 0;

		// Get the current window size dynamically
		int windowWidth, windowHeight;
		glfwGetWindowSize(_window, &windowWidth, &windowHeight);
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		camera.updateMousePos(x, y);

		int windowCenterX = windowWidth / 2;
		int windowCenterY = windowHeight / 2;

		if (firstMouse || ignoreMouseEvent)
		{
			lastX = windowCenterX;
			lastY = windowCenterY;
			firstMouse = false;
			ignoreMouseEvent = false;
			return;
		}

		float xOffset = lastX - x;
		float yOffset = lastY - y;

		lastX = x;
		lastY = y;

		float sensitivity = 0.05f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		camera.rotate(1.0f, 0.0f, xOffset * ROTATION_SPEED);
		camera.rotate(0.0f, 1.0f, yOffset * ROTATION_SPEED);
		ignoreMouseEvent = true;
		glfwSetCursorPos(_window, windowCenterX, windowCenterY);
	}

	void particle_system::mouseCallback(GLFWwindow* window, double x, double y)
	{
		particle_system *engine = static_cast<particle_system*>(glfwGetWindowUserPointer(window));

		if (engine) engine->mouseAction(x, y);
	}


	/*
		Initialises simulation data
	*/
	void particle_system::initSimData()
	{
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
		m.pos.x = 5.0f;
		m.pos.y = -20.0f;
		m.pos.z = -10.0f;

		// Default rotation angle for particles around the mass
		m.rotationTangent.x = 0.0f;
		m.rotationTangent.y = 1.0f;
		m.rotationTangent.z = 0.0f;

		// Window data
		windowHeight = W_HEIGHT;
		windowWidth = W_WIDTH;

		// Reset data
		resetSim = false;

		// Mass toggles
		massFollow = false;
		spaghettiMode = false;
		massDisplay = true;

		// Keys states and runtime booleans()
		bzero(keyStates, sizeof(keyStates));
		ignoreMouseEvent	= IGNORE_MOUSE;
		mouseCaptureToggle	= CAPTURE_MOUSE;

		// FPS counter
		frameCount			= 0;
		lastFrameTime		= 0.0;
		currentFrameTime	= 0.0;
		fps					= 0.0;

		// Player data
		moveSpeed		= 0.0;
		rotationSpeed	= 0.0;
	}

	/*
		Updates the window size
	*/
	void particle_system::update_window_size(int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
	}

	/*
		Updates the mass tangent,
		The particles at which angle they rotate around the mass depends on these parameters
	*/
	void particle_system::update_mass_tangent(float x, float y, float z)
	{
		m.rotationTangent.x = std::clamp(m.rotationTangent.x + x, 0.0f, 1.0f);
		m.rotationTangent.y = std::clamp(m.rotationTangent.y + y, 0.0f, 1.0f);
		m.rotationTangent.z = std::clamp(m.rotationTangent.z + z, 0.0f, 1.0f);
	}

	/*
		Updates the mass position,
		The mass is updated at the position of the cursor translated in 3D
	*/
	void particle_system::update_mass_position(glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
	{
		// Normalized Dimension coordinates
		float xNDC = (2.0f * camera.mousePos.x) / windowWidth - 1.0f;
		float yNDC = 1.0f - (2.0f * camera.mousePos.y) / windowHeight;
		float zNDC = 0.75f;

		glm::vec4 mouseClipCoords = glm::vec4(xNDC, yNDC, zNDC, 1.0f);
		glm::vec4 pointingWorldCoords = glm::inverse(projectionMatrix * viewMatrix) * mouseClipCoords;
		pointingWorldCoords /= pointingWorldCoords.w;

		m.pos = {pointingWorldCoords.x, pointingWorldCoords.y, pointingWorldCoords.z};
	}

	/*
		Resets the simulation,
		Releases all previous data and sets it back to the selected shape
	*/
	void particle_system::resetSimulation() {
		if (reset_shape == particleShape::CUBE)
			std::cout << "Resetting the simulation back to a cube of size: " << cubeSize << std::endl;
		else if (reset_shape == particleShape::SPHERE)
			std::cout << "Resetting the simulation back to a sphere of radius: " << sphereRadius << std::endl;
		freeCLdata(false);
		initSimData();
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
		depending on sphere radius and number of particles
	*/
	bool particle_system::enqueueInitSphereParticles() {
		//Acquiring buffer
		err = clEnqueueAcquireGLObjects(queue, 1, &particleBufferCL, 0, nullptr, nullptr);
		if (err != CL_SUCCESS)
			return freeCLdata(true, ENQUEUE_BUFFER_CL_GL_ERR);

		// Set kernel arguments
		err = clSetKernelArg(init_particles_sphere, 0, sizeof(cl_mem), &particleBufferCL);
		if (err != CL_SUCCESS)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);
		err = clSetKernelArg(init_particles_sphere, 1, sizeof(float), &sphereRadius);
		if (err != CL_SUCCESS)
			return freeCLdata(true, KERNEL_ARGS_SET_ERR);

		// Execute the init kernel
		size_t globalWorkSize = nb_particles;
		err = clEnqueueNDRangeKernel(queue, init_particles_sphere, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);
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
		Initialises vertex array and vertex buffer objects
		Initialises the vertex and fragment shaders
	*/
	void particle_system::initShaders()
	{
		// OpenGL VAO/VBO setup
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, particleBufferGL);

		// Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)offsetof(particle, pos));

		// Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)offsetof(particle, color));

		// Old position
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)offsetof(particle, pos_prev));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// Vertex and Fragment shader setup
		shaderProgram = createShaderProgram("shaders/particle.vert", "shaders/particle.frag", "shaders/particle.gs");

		// Vertex and Fragment shader setup for spaghetti mode
		spaghettiShaderProgram = createShaderProgram("shaders/spaghetti.vert", "shaders/spaghetti.frag", "");
	}

	/*
		Initialises and allocates the CL/GL shared buffer
		on the VRAM and checks for errors
	*/
	bool particle_system::initSharedBufferData() {
		std::cout << "Initialising OpenGL/OpenCL shared buffer" << std::endl;
		// Generate OpenGL buffer
		std::cout << glGetString(GL_VERSION) << std::endl;
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
		if (!resetSim)
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
				std::cerr << "No GPU on this platform " << i + 1 << std::endl;
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
					if (!resetSim)
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
		err = clGetGLContextInfoKHR_safe(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(selected_platform), &selected_platform, nullptr);
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

		// OpenCL kernel source for initializing particles in a cube
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

		// OpenCL kernel source for initializing particles in a sphere
		const char *initParticlesSphereSrc = get_CL_program("kernel_srcs/init_particles_sphere.cl");
		if (!initParticlesSphereSrc)
			return freeCLdata(true, FETCH_CL_FILE_ERR);

		//Init cube_init program
		// Create program for initializing particles
		init_sphere_program = clCreateProgramWithSource(context, 1, &initParticlesSphereSrc, nullptr, &err);
		if (err != CL_SUCCESS || !init_sphere_program)
			return freeCLdata(true, std::string(PROGRAM_CREATE_ERR) + " init_particles_sphere");

		// Build the init program
		err = clBuildProgram(init_sphere_program, 1, &selected_device, nullptr, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			size_t len;
			char buffer[2048];
			bzero(buffer, sizeof(buffer));
			clGetProgramBuildInfo(init_sphere_program, selected_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			std::cerr << buffer << std::endl;
			return freeCLdata(true, std::string(PROGRAM_BUILD_ERR) + " init_sphere_program");
		}
		return true;
	}

	/*
		Initialises openCL kernels with the cl programs
	*/
	bool particle_system::initKernels() {
		// Create init cube particles kernel
		init_particles_cube = clCreateKernel(init_cube_program, "init_particles_cube", &err);
		if (err != CL_SUCCESS || !init_particles_cube)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " init_cube_program");

		// Create init sphere particles kernel
		init_particles_sphere = clCreateKernel(init_sphere_program, "init_particles_sphere", &err);
		if (err != CL_SUCCESS || !init_particles_sphere)
			return freeCLdata(true, std::string(KERNEL_CREATE_ERR) + " init_sphere_program");

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
		if (reset_shape == particleShape::CUBE && !enqueueInitCubeParticles())
			return false;
		// Call init_sphere kernel to init the particles in a sphere
		else if (reset_shape == particleShape::SPHERE && !enqueueInitSphereParticles())
			return false;
		if (!resetSim)
			std::cout << "OpenCL particles data initialized directly on GPU" << std::endl;
		return true;
	}
};
