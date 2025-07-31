/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2025/07/31 21:56:59 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"

using namespace psys;

// Constants
const size_t particle_number = 1000000; 
const float mouse_sensitivity = 0.05f;

int main()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	particle_system particle_sys(particle_number);
	if (!particle_sys.initCLdata())
		return 1;
	std::cout << std::endl;
	std::cout << "Welcome to particle_system" << std::endl;
	std::cout << "Press 'H' key to see the list of available commands" << std::endl;
	particle_sys.run();
	std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
	return 0;
}
