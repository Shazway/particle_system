/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:27:33 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/25 01:16:17 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "camera.hpp"

namespace psys
{
	/*
		Moving the camera around (first person view)
	*/
	void camera::move(float  forward, float  strafe, float up)
	{
		// Calculate the direction based on the current angles
		float radiansY = yangle * (M_PI / 180.0);
		float radiansX = xangle * (M_PI / 180.0);

		float scaleForward = forward * cos(radiansY);

		// Determine the forward movement vector
		float forwardX = cos(radiansX) * scaleForward;
		float forwardZ = sin(radiansX) * scaleForward;

		// Determine the strafe movement vector (perpendicular to forward)
		float strafeX = cos(radiansX + M_PI / 2) * strafe;
		float strafeZ = sin(radiansX + M_PI / 2) * strafe;

		// Update the camera position
		position.z += (forwardX + strafeX) * movementspeed;
		position.x += (forwardZ + strafeZ) * movementspeed;
		center.x = position.x;
		center.z = position.z -10.0f;
		position.y += movementspeed * up;
		center.y = position.y;
	}

	/*
		Reset the camera position
	*/
	void camera::reset()
	{
		position = vec3(0.0, 0.0, 0.0);
		xangle = 0.0;
		yangle = 0.0;
		rotationspeed = 1;
		movementspeed = 0.1;
	}
};
