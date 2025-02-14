/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:26:25 by tmoragli          #+#    #+#             */
/*   Updated: 2025/02/14 20:52:29 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace psys
{
	class Camera
	{
		public:
			Camera() : position{0.0, 0.0, 0.0}, center{0.0f, 0.0f, -10.0f} {};
			~Camera() {};
			void move(float forward, float strafe, float up);
			void reset();
			glm::vec3 getPosition();
			glm::vec3 getCenter();
			glm::vec2 getAngles();
			void rotate(float xAngle, float yAngle, double rotationSpeed);
			void updateMousePos(int x, int y);

			glm::vec3 position;
			glm::vec3 center;
			glm::vec2 angle;
			glm::vec2 mousePos;
			float xangle = 0.0;
			float yangle = 0.0;
			float rotationspeed = 1;
			float movementspeed = 0.25;
			bool mouseRotation = false;
	};
};
