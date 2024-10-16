/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 20:26:25 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/13 01:39:27 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "particle_system.hpp"

namespace psys {
	class camera {
		public:
			camera() : position{0.0, 0.0, 0.0} {};
			~camera() {};
			void move(float forward, float strafe);
			void reset();

			vec3 position;
			float xangle = 0.0;
			float yangle = 0.0;
			float rotationspeed = 1;
			float movementspeed = 0.1;
	};
};
