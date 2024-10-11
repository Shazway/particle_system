/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   particle_system.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/12 01:04:07 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/12 01:10:18 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// Useful includes
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

#define COMMANDS_LIST								\
	"Model Commands:\n"								\
	"Moving the model relative to the origin:\n"	\
	"'C': Toggle camera mode\n"						\
	"'Z', 'W': Move the model up\n"					\
	"'Q', 'A': Move the model left\n"				\
	"'S': Move the model down\n"					\
	"'D': Move the model right\n"					\
	"'+': Move the model further away\n"			\
	"'-': Move the model closer\n"					\
	"'O': Increase the model speed\n"				\
	"'P': Decrease the model speed\n"				\
	"\nRotating the model:\n"						\
	"'X': Toggle X axis rotation\n"					\
	"'Y': Toggle Y axis rotation\n"					\
	"'R': Stop all rotations\n"						\
	"'J': Accelerate rotations\n"					\
	"'K': Decelerate rotations\n"					\
	"\nCamera commands:\n"							\
	"'Z', 'W': Move forward\n"						\
	"'Q', 'A': Move left\n"							\
	"'S': Move back\n"								\
	"'D': Move right\n"								\
	"' ': Move up\n"								\
	"'V': Move down\n"								\
	"'LEFT_ARROW': Look left\n"						\
	"'RIGHT_ARROW': Look right\n"					\
	"'DOWN_ARROW': Look down\n"						\
	"'UP_ARROW': Look up\n"							\
	"\nOther commands:\n"							\
	"'0': Reset simulation\n"						\
	"'Esc': Exit program"


#define W_WIDTH 800
#define W_HEIGHT 600

namespace psys {
	struct vec3 {
		double x, y, z;

		vec3(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}
	};

	struct Color {
		//red, green, blue, opacity
		double r;
		double g;
		double b;
		double o;
		Color(): r(0), g(0), b(0), o(0) {}
		Color(double r, double g, double b, double o): r(r), g(g), b(b), o(o) {}
		Color(double r, double g, double b): r(r), g(g), b(b), o(1) {}
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

	const double movespeed = 0.1;
};
