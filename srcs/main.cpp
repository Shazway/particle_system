/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2026/01/06 16:57:23 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"

#include <cctype>
#include <cstdlib>

using namespace psys;

// Constants
const size_t particle_number = 1000000;
const size_t max_particles = 5000000;
const float mouse_sensitivity = 0.05f;

static bool is_digits_only(const char *str)
{
	if (!str || *str == '\0')
		return false;
	for (const unsigned char *p = reinterpret_cast<const unsigned char*>(str); *p; ++p)
	{
		if (!std::isdigit(*p))
			return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	size_t particle_count = particle_number;
	if (argc > 2)
	{
		std::cerr << "Usage: ./particle_system [nb]" << std::endl;
		return 1;
	}
	if (argc == 2)
	{
		if (!is_digits_only(argv[1]))
		{
			std::cerr << "Error: particle count must be numeric" << std::endl;
			return 1;
		}
		unsigned long long parsed = std::strtoull(argv[1], nullptr, 10);
		if (parsed == 0 || parsed > max_particles)
		{
			std::cerr << "Error: particle count must be > 0 and < 5000000" << std::endl;
			return 1;
		}
		particle_count = static_cast<size_t>(parsed);
	}
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	particle_system particle_sys(particle_count);
	if (!particle_sys.initCLdata())
		return 1;
	std::cout << std::endl;
	std::cout << "Welcome to particle_system" << std::endl;
	std::cout << "Press 'H' key to see the list of available commands" << std::endl;
	std::cout << "Press 'M' to start attracting particles to the mass" << std::endl;
	particle_sys.run();
	return 0;
}
