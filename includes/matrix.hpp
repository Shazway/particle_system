/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   matrix.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 00:32:35 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/13 01:39:57 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "particle_system.hpp"

namespace psys {
	class mat4 {
		public:
			std::array<std::array<float, 4>, 4> data;
			mat4();
			mat4(const std::array<std::array<float, 4>, 4> &cpy);

			// Set as identity matrix
			static mat4 identity();
			static mat4 translate(float x, float y, float z);
			static mat4 rotate(float angle, float x, float y, float z);
			static mat4 perspective(float fov, float aspect, float near, float far);

			mat4 operator*(const mat4 &other) const;
			mat4 &operator*=(const mat4 &other);
	};
	std::ostream& operator<<(std::ostream &os, const mat4& matrix);
};
