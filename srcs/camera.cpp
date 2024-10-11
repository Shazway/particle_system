/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:27:33 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/12 01:04:39 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "camera.hpp"

namespace psys
{
	void camera::move(double forward, double strafe)
	{
		// Calculate the direction based on the current angles
		double radiansY = yangle * (M_PI / 180.0);
		double radiansX = xangle * (M_PI / 180.0);

		double scaleForward = forward * cos(radiansY);

		// Determine the forward movement vector
		double forwardX = cos(radiansX) * scaleForward;
		double forwardZ = sin(radiansX) * scaleForward;

		// Determine the strafe movement vector (perpendicular to forward)
		double strafeX = cos(radiansX + M_PI / 2) * strafe;
		double strafeZ = sin(radiansX + M_PI / 2) * strafe;

		// Update the camera position
		position.z += (forwardX + strafeX) * movementspeed;
		position.x += (forwardZ + strafeZ) * movementspeed;
	}

	void camera::reset()
	{
		position = vec3(0.0, 0.0, 0.0);
		xangle = 0.0;
		yangle = 0.0;
		rotationspeed = 1;
		movementspeed = 0.1;
	}
};
