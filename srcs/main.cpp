/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/18 20:12:27 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"
#include "matrix.hpp"
#include "camera.hpp"
#include "particle_system.hpp"

using namespace psys;

// Constants
const size_t particle_number = 3000000; 
bool resetSim = false;

// Globals
camera cam;
mat4 projectionMatrix;
mat4 viewMatrix;
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
particle_system particle_sys(particle_number);
cl_event kernel_event;

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
	if (key == 'm')
		particle_sys.m.intensity = 50.0f;
	if (key == '0')
		particle_sys.resetSim = true;
	if (key == 27)
		glutLeaveMainLoop();
}

void keyRelease(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = false;
	if (key == 'm')
		particle_sys.m.intensity = 0.0f;
}

void closeCallback() {
	//Closing window callback to free all openCL related data
	particle_sys.freeCLdata(false);
	exit(0);
}

void mouseCallback(int x, int y) {
	float w_width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
	float w_height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));

	particle_sys.mousePos.x = 2.0f * static_cast<float>(x) / w_width - 1.0f;
	particle_sys.mousePos.y = 1.0f - 2.0f * static_cast<float>(y) / w_height;
}

void renderParticles() {
	glBindBuffer(GL_ARRAY_BUFFER, particle_sys.particleBufferGL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(particle), 0);
	glDrawArrays(GL_POINTS, 0, particle_sys.nb_particles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
