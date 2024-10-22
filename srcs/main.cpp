/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/22 03:05:17 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"
#include "matrix.hpp"
#include "camera.hpp"
#include "particle_system.hpp"

using namespace psys;

// Constants
const size_t particle_number = 1000000; 
const float mouse_sensitivity = 0.05f;

// Globals
bool resetSim = false;
bool ignoreMouseEvent = false;
bool hideCursor = false;
bool cameraToggle = false;
camera cam;
mat4 projectionMatrix;
mat4 viewMatrix;
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
particle_system particle_sys(particle_number);
cl_event kernel_event;

// FPS counter
int frameCount = 0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double fps = 0.0;

void calculateFps()
{
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

void specialKeyPress(int key, int x, int y)
{
	(void)x;
	(void)y;
	specialKeyStates[key] = true;
}

void specialKeyRelease(int key, int x, int y)
{
	(void)x;
	(void)y;
	specialKeyStates[key] = false;
}

void keyPress(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = true;
	if (key == 'm' && particle_sys.m.intensity)
		particle_sys.m.intensity = 0.0f;
	else if (key == 'm' && !particle_sys.m.intensity)
		particle_sys.m.intensity = 50.0f;
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
		cameraToggle = !cameraToggle;
	else if (key == 't')
		particle_sys.m.rotationTangent = {0.0f, 1.0f, 0.0f};
	else if (key == 27)
		glutLeaveMainLoop();
}

void keyRelease(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = false;
}

void closeCallback() {
	//Closing window callback to free all openCL related data
	particle_sys.freeCLdata(false);
	exit(0);
}

void mouseCallback(int x, int y) {
	static bool firstMouse = true;
	static int lastX = 0, lastY = 0;

	particle_sys.mousePos.x = x;
	particle_sys.mousePos.y = y;
	//TODO: process mouse position in 3D space
	if (!cameraToggle)
	{
		if (hideCursor == true) 
		{
			glutSetCursor(GLUT_CURSOR_INHERIT);
			hideCursor = false;
		}
		return ;
	}
	if (!hideCursor)
	{
		hideCursor = true;
		glutSetCursor(GLUT_CURSOR_NONE);
	}

	// Get the current window size dynamically
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

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

void renderParticles() {
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
	glPushMatrix();
	// Get the mass for its position
	mass m = particle_sys.m;
	glTranslatef(m.pos.x, m.pos.y, m.pos.z);
	glColor3f(0.0f, 0.0f, 0.0f);
	GLUquadric* quad = gluNewQuadric();
	gluSphere(quad, particle_sys.m.radius / 2, 20, 20);
	gluDeleteQuadric(quad);

	glPopMatrix();

	// Reset color to default
	glColor3f(1.0f, 1.0f, 1.0f);
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

	// Matrix operations for the camera
	float radX;
	float radY;
	glMatrixMode(GL_MODELVIEW);

	viewMatrix = mat4::identity();
	radX = cam.xangle * (M_PI / 180.0);
	radY = cam.yangle * (M_PI / 180.0);
	viewMatrix *= mat4::rotate(radY, 1.0, 0.0, 0.0);
	viewMatrix *= mat4::rotate(radX, 0.0, 1.0, 0.0);
	viewMatrix *= mat4::translate(cam.position.x, cam.position.y, cam.position.z);
	glLoadMatrixf((viewMatrix).data[0].data());

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
	if (keyStates['z']) cam.move(1.0, 0.0);
	if (keyStates['w']) cam.move(1.0, 0.0);
	if (keyStates['q']) cam.move(0.0, 1.0);
	if (keyStates['a']) cam.move(0.0, 1.0);
	if (keyStates['s']) cam.move(-1.0, 0.0);
	if (keyStates['d']) cam.move(0.0, -1.0);
	if (keyStates[' ']) {
		cam.position.y -= cam.movementspeed;
		cam.center.y = cam.position.y;
	}
	if (keyStates['v']) {
		cam.position.y += cam.movementspeed;
		cam.center.y = cam.position.y;
	}

	// Camera rotations
	if (specialKeyStates[GLUT_KEY_LEFT]) cam.xangle += cam.rotationspeed;
	if (specialKeyStates[GLUT_KEY_RIGHT]) cam.xangle -= cam.rotationspeed;
	if (specialKeyStates[GLUT_KEY_UP] && cam.yangle < 45.0) cam.yangle += cam.rotationspeed;
	if (specialKeyStates[GLUT_KEY_DOWN] && cam.yangle > -45.0) cam.yangle -= cam.rotationspeed;
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
	projectionMatrix = mat4::identity();
	projectionMatrix *= mat4::perspective(45.0 / 180.0, (double)width / (double)height, 1.0, 100.0);
	glLoadMatrixf((projectionMatrix).data[0].data());
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
	glutSpecialFunc(specialKeyPress);
	glutSpecialUpFunc(specialKeyRelease);
	glutCloseFunc(closeCallback);
	glutPassiveMotionFunc(mouseCallback);
}

void initGlew()
{
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Error initializing GLEW" << glewGetErrorString(err) << std::endl;
	}
}

int main(int argc, char **argv)
{
	if (particle_sys.err != CL_SUCCESS) {
		std::cerr << "Could not initialize openCL properly" << std::endl;
		return 1;
	}

	initGlutWindow(argc, argv);
	initGlew();
	initGlutEvents();
	particle_sys.initSharedBufferData();
	if (!particle_sys.initCLdata())
		return 1;
	glutMainLoop();
	return 0;
}
