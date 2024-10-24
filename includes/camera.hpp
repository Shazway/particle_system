/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:26:25 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/25 00:10:37 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "particle_system.hpp"

namespace psys {
	class camera {
		public:
			camera() : position{0.0, 0.0, 0.0}, center{0.0f, 0.0f, -10.0f} {};
			~camera() {};
			void move(float forward, float strafe, float up);
			void reset();

			vec3 position;
			vec3 center;
			float xangle = 0.0;
			float yangle = 0.0;
			float rotationspeed = 1;
			float movementspeed = 0.25;
			bool mouseRotation = false;
	};
};
