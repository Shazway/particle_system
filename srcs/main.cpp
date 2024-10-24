/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/25 01:21:47 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"
#include "camera.hpp"
#include "particle_system.hpp"

using namespace psys;

// Constants
const size_t particle_number = 3000000; 
const float mouse_sensitivity = 0.05f;

// Globals
bool ignoreMouseEvent = false;
bool hideCursor = false;
camera cam;
glm::mat4 projectionMatrix(1.0);
glm::mat4 viewMatrix(1.0);
bool keyStates[256] = {false};
particle_system particle_sys(particle_number);

// FPS counter
int frameCount = 0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;

void calculateFps()
{
	double fps = 0.0;
	frameCount++;
	currentFrameTime = glutGet(GLUT_ELAPSED_TIME);

	double timeInterval = currentFrameTime - lastFrameTime;

	if (timeInterval > 1000)
	{
		fps = frameCount / (timeInterval / 1000.0);

		lastFrameTime = currentFrameTime;
		frameCount = 0;

		std::stringstream title;
		title << "particle_system | FPS: " << fps;
		glutSetWindowTitle(title.str().c_str());
	}
}

void keyPress(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = true;
	if (key == 'm' && particle_sys.m.intensity)
		particle_sys.m.intensity = 0.0f;
	else if (key == 'm' && !particle_sys.m.intensity)
		particle_sys.m.intensity = 10.0f;
	if (key == '0')
	{
		particle_sys.reset_shape = particleShape::CUBE;
		particle_sys.resetSim = true;
	}
	else if (key == '1')
	{
		particle_sys.reset_shape = particleShape::SPHERE;
		particle_sys.resetSim = true;
	}
	else if (key == 'c')
		cam.mouseRotation = !cam.mouseRotation;
	else if (key == 't')
		particle_sys.m.rotationTangent = {0.0f, 1.0f, 0.0f};
	else if (key == 'f')
		particle_sys.massFollow = !particle_sys.massFollow;
	else if (key == 'p')
		particle_sys.massDisplay = !particle_sys.massDisplay;
	else if (key == 'h')
		std::cout << COMMANDS_LIST << std::endl;
	else if (key == 27)
		glutLeaveMainLoop();
}

void keyRelease(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = false;
}

void closeCallback()
{
	//Closing window callback to free all openCL related data
	particle_sys.freeCLdata(false);
	exit(0);
}

void mouseCallback(int x, int y)
{
	static bool firstMouse = true;
	static int lastX = 0, lastY = 0;

	particle_sys.mousePos.x = x;
	particle_sys.mousePos.y = y;
	// Get the current window size dynamically
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	// Update the size
	particle_sys.update_window_size(windowWidth, windowHeight);
	particle_sys.update_mouse_pos(x, y);

	// Hide cursor/Warp it to the center depending on cam.mouseRotation
	if (!cam.mouseRotation || (!cam.mouseRotation && hideCursor))
	{
		glutSetCursor(GLUT_CURSOR_INHERIT);
		hideCursor = false;
		return ;
	}
	if (!hideCursor)
	{
		hideCursor = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}

	int windowCenterX = windowWidth / 2;
	int windowCenterY = windowHeight / 2;

	if (firstMouse || ignoreMouseEvent) {
		lastX = windowCenterX;
		lastY = windowCenterY;
		firstMouse = false;
		ignoreMouseEvent = false;
		return ;
	}

	// Calculate the mouse movement offsets
	float xOffset = lastX - x;
	float yOffset = lastY - y; // Invert y-axis to match typical camera movement

	// Save the current mouse position for the next callback
	lastX = x;
	lastY = y;

	// Apply sensitivity to smooth the movement
	float sensitivity = 0.05f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Update the camera angles
	cam.xangle += xOffset * cam.rotationspeed;
	cam.yangle += yOffset * cam.rotationspeed;

	// Limit the pitch (yangle) to avoid flipping
	if (cam.yangle > 89.0f) cam.yangle = 89.0f;
	if (cam.yangle < -89.0f) cam.yangle = -89.0f;

	// Ignore movement for next call to move mouse from warpPointer function
	ignoreMouseEvent = true;

	// Optionally, recenter the mouse after each movement
	glutWarpPointer(windowCenterX, windowCenterY);
}

void renderParticles()
{
	glBindBuffer(GL_ARRAY_BUFFER, particle_sys.particleBufferGL);

	// Enables vertex array for position of particles
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(particle), (void *)offsetof(particle, pos));

	// Enables color array for colors
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, sizeof(particle), (void *)offsetof(particle, color));

	// Draw final result
	glDrawArrays(GL_POINTS, 0, particle_sys.nb_particles);

	// Release arrays and buffer
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render mass point as a sphere
	if (particle_sys.massDisplay)
	{
		glPushMatrix();
		glTranslatef(particle_sys.m.pos.x, particle_sys.m.pos.y, particle_sys.m.pos.z);
		glColor3f(0.0f, 0.0f, 0.0f);
		GLUquadric* quad = gluNewQuadric();
		gluSphere(quad, particle_sys.m.radius / 10, 20, 20);
		gluDeleteQuadric(quad);
		glPopMatrix();
	}
}

void updateParticles()
{
	if (particle_sys.resetSim)
		particle_sys.resetSimulation();
	else
		particle_sys.enqueueUpdateParticles();
	return ;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	// Matrix operations for the camera
	float radY;
	float radX;

	// Radians conversion
	radX = cam.xangle * (M_PI / 180.0);
	radY = cam.yangle * (M_PI / 180.0);

	viewMatrix = glm::mat4(1.0f);
	viewMatrix = glm::rotate(viewMatrix, radY, glm::vec3(-1.0f, 0.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, radX, glm::vec3(0.0f, -1.0f, 0.0f));
	viewMatrix = glm::translate(viewMatrix, glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	// Get the current window size dynamically
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	// Update the size
	particle_sys.update_window_size(windowWidth, windowHeight);

	// Update the mass position to the cursor in space
	if (particle_sys.massFollow)
		particle_sys.update_mass_position(projectionMatrix, viewMatrix);

	// Call update kernel
	updateParticles();
	
	// Draw particles
	renderParticles();
	glutSwapBuffers();
	calculateFps();
	glutPostRedisplay();
}

void update(int value)
{
	(void)value;
	// Camera movement
	if (keyStates['z']) cam.move(1.0, 0.0, 0.0);
	if (keyStates['w']) cam.move(1.0, 0.0, 0.0);
	if (keyStates['q']) cam.move(0.0, 1.0, 0.0);
	if (keyStates['a']) cam.move(0.0, 1.0, 0.0);
	if (keyStates['s']) cam.move(-1.0, 0.0, 0.0);
	if (keyStates['d']) cam.move(0.0, -1.0, 0.0);
	if (keyStates[' ']) cam.move(0.0, 0.0, -1.0);
	if (keyStates['v']) cam.move(0.0, 0.0, 1.0);
	if (keyStates['+']) particle_sys.m.intensity += 0.1f;
	if (keyStates['-']) particle_sys.m.intensity -= 0.1f;

	// Camera rotations
	if (cam.xangle > 360.0)
		cam.xangle = 0.0;
	else if (cam.xangle < 0.0)
		cam.xangle = 360.0;

	// Mass control
	// Increase tangent vector rotation in x, y z respectively
	if (keyStates['u'])
		particle_sys.update_mass_tangent(0.1f, 0.0f, 0.0f);
	if (keyStates['i'])
		particle_sys.update_mass_tangent(0.0f, 0.1f, 0.0f);
	if (keyStates['o'])
		particle_sys.update_mass_tangent(0.0f, 0.0f, 0.1f);
	// Decrease tangent vector rotation in x, y z respectively
	if (keyStates['j'])
		particle_sys.update_mass_tangent(-0.1f, 0.0f, 0.0f);
	if (keyStates['k'])
		particle_sys.update_mass_tangent(0.0f, -0.1f, 0.0f);
	if (keyStates['l'])
		particle_sys.update_mass_tangent(0.0f, 0.0f, -0.1f);

	glutPostRedisplay();
	// Call update every 8 milliseconds (~120 FPS)
	glutTimerFunc(8, update, 0);
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	// Apply projection matrix operations
	glMatrixMode(GL_PROJECTION);

	// Load identity matrix
	projectionMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 1.0f, 1000.0f);
	glLoadMatrixf(glm::value_ptr(projectionMatrix));
}

void initGlutWindow(int ac, char **av)
{
	glutInit(&ac, av);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(W_WIDTH, W_HEIGHT);
	glutCreateWindow("particle_system");
	glEnable(GL_DEPTH_TEST);
	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
}

void initGlutEvents()
{
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(8, update, 0); // 8 ticks per second update, 120 fps~
	glutKeyboardFunc(keyPress);
	glutKeyboardUpFunc(keyRelease);
	glutCloseFunc(closeCallback);
	glutPassiveMotionFunc(mouseCallback);
}

bool initGlew()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Error initializing GLEW" << glewGetErrorString(err) << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	if (particle_sys.err != CL_SUCCESS)
	{
		std::cerr << "Could not initialize openCL properly" << std::endl;
		return 1;
	}

	initGlutWindow(argc, argv);
	if (!initGlew())
		return 1;
	initGlutEvents();
	particle_sys.initSharedBufferData();
	if (!particle_sys.initCLdata())
		return 1;
	std::cout << std::endl;
	std::cout << "Welcome to particle_system" << std::endl;
	std::cout << "Press 'H' key to see the list of available commands" << std::endl;
	glutMainLoop();
	return 0;
}
