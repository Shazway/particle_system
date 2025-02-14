/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:27:33 by tmoragli          #+#    #+#             */
/*   Updated: 2025/02/14 22:27:54 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "camera.hpp"

namespace psys
{
	/*
		Moving the camera around (first person view)
	*/
	void Camera::move(float  forward, float  strafe, float up)
	{
		float radiansX = angle.x * (M_PI / 180.0);

		//float scaleForward = forward * cos(radiansY);

		// Determine the forward movement vector
		float forwardX = cos(radiansX) * forward;
		float forwardZ = sin(radiansX) * forward;

		// Determine the strafe movement vector (perpendicular to forward)
		float strafeX = cos(radiansX + M_PI / 2) * strafe;
		float strafeZ = sin(radiansX + M_PI / 2) * strafe;

		// Update theCcamera position
		position.z += (forwardX + strafeX) * movementspeed;
		position.x += (forwardZ + strafeZ) * movementspeed;
		center.x = position.x;
		center.z = position.z -10.0f;
		position.y += movementspeed * up;
		center.y = position.y;
	}

	void Camera::rotate(float xAngle, float yAngle, double rotationSpeed)
	{
		//std::lock_guard<std::mutex> lock(angleMutex);
		angle.x += xAngle * rotationSpeed;
		angle.y += yAngle * rotationSpeed;
		angle.y = std::clamp(angle.y, -90.0f, 90.0f);
		if (angle.x < 0)
			angle.x = 360;
		else if (angle.x > 360)
			angle.x = 0;
	}

	void Camera::updateMousePos(int x, int y)
	{
		mousePos.x = static_cast<float>(x);
		mousePos.y = static_cast<float>(y);
	}

	/*
		Reset the camera position
	*/
	void Camera::reset()
	{
		position = glm::vec3(0.0, 0.0, 0.0);
		xangle = 0.0;
		yangle = 0.0;
		rotationspeed = 1;
		movementspeed = 0.1;
	}

	glm::vec3 Camera::getPosition()
	{
		return position;
	}

	glm::vec3 Camera::getCenter()
	{
		return center;
	}

	glm::vec2 Camera::getAngles()
	{
		return angle;
	}
};
