/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/12 01:11:56 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"
#include "matrix.hpp"
#include "camera.hpp"

using namespace psys;

// Globals
camera cam;
mat4 projectionMatrix;
mat4 viewMatrix;
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};

// Constants
const size_t particle_number = 1; 

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
	if (key == 27)
		glutLeaveMainLoop();
}

void keyRelease(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;
	keyStates[key] = false;
}

void renderParticles()
{
	return ;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Matrix operations
	double radX;
	double radY;
	glMatrixMode(GL_MODELVIEW);
	viewMatrix = mat4::identity();
	radX = cam.xangle * (M_PI / 180.0);
	radY = cam.yangle * (M_PI / 180.0);
	viewMatrix *= mat4::rotate(radY, 1.0, 0.0, 0.0);
	viewMatrix *= mat4::rotate(radX, 0.0, 1.0, 0.0);
	viewMatrix *= mat4::translate(cam.position.x, cam.position.y, cam.position.z);
	glLoadMatrixd((viewMatrix).data[0].data());

	// Draw object
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
	if (keyStates[' ']) cam.position.y -= cam.movementspeed;
	if (keyStates['v']) cam.position.y += cam.movementspeed;

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
	glLoadMatrixd((projectionMatrix).data[0].data());
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
}

int main(int argc, char **argv)
{
	initGlutWindow(argc, argv);
	initGlutEvents();
	glutMainLoop();
	return 0;
}
