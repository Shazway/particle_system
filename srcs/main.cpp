/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 00:06:10 by tmoragli          #+#    #+#             */
/*   Updated: 2025/02/14 21:29:45 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "particle_system.hpp"

using namespace psys;

// Constants
const size_t particle_number = 3000000; 
const float mouse_sensitivity = 0.05f;

int main()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	particle_system particle_sys(particle_number);
	particle_sys.initSharedBufferData();
	if (!particle_sys.initCLdata())
		return 1;
	std::cout << std::endl;
	std::cout << "Welcome to particle_system" << std::endl;
	std::cout << "Press 'H' key to see the list of available commands" << std::endl;
	particle_sys.run();
	return 0;
}
