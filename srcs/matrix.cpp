/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   matrix.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/29 19:15:49 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/13 01:41:02 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "matrix.hpp"

namespace psys
{
	mat4::mat4()
	{
		data = {{
			{1.0, 0.0, 0.0, 0.0},
			{0.0, 1.0, 0.0, 0.0},
			{0.0, 0.0, 1.0, 0.0},
			{0.0, 0.0, 0.0, 1.0}
		}};
	}
	mat4::mat4(const std::array<std::array<float, 4>, 4> &cpy) : data(cpy) { }

	// Set as identity matrix
	mat4 mat4::identity()
	{
		mat4 result;
		result.data = {{
			{1.0, 0.0, 0.0, 0.0},
			{0.0, 1.0, 0.0, 0.0},
			{0.0, 0.0, 1.0, 0.0},
			{0.0, 0.0, 0.0, 1.0}
		}};
		return result;
	}

	mat4 mat4::translate(float x, float y, float z)
	{
		mat4 result;

		result.data[3][0] = x;
		result.data[3][1] = y;
		result.data[3][2] = z;
		return result;
	}

	mat4 mat4::rotate(float angle, float x, float y, float z)
	{
		mat4 result;
		float c = cos(angle);
		float s = sin(angle);
		float invert_c = 1.0 - c;

		result.data[0][0] = x * x * invert_c + c;
		result.data[1][0] = y * x * invert_c + z * s;
		result.data[2][0] = z * x * invert_c - y * s;

		// Second column
		result.data[0][1] = x * y * invert_c - z * s;
		result.data[1][1] = y * y * invert_c + c;
		result.data[2][1] = z * y * invert_c + x * s;

		// Third column
		result.data[0][2] = x * z * invert_c + y * s;
		result.data[1][2] = y * z * invert_c - x * s;
		result.data[2][2] = z * z * invert_c + c;

		return result;
	}

	mat4 mat4::perspective(float fov, float aspect, float near, float far)
	{
		mat4 result;
		float tanHalfFov = tan(fov / 2.0);
		float range = near - far;

		result.data[0][0] = 1.0 / (aspect * tanHalfFov);
		result.data[1][1] = 1.0 / tanHalfFov;
		result.data[2][2] = -(near + far) / range;
		result.data[2][3] = (2.0 * near * far) / range;
		result.data[3][2] = -1.0;
		result.data[3][3] = 0.0;

		return result;
	}

	mat4 mat4::operator*(const mat4 &other) const
	{
		mat4 result;
		for (int col = 0; col < 4; ++col) {
			for (int row = 0; row < 4; ++row) {
				result.data[row][col] = 0.0;
				for (int i = 0; i < 4; ++i) {
					result.data[row][col] += data[i][col] * other.data[row][i];
				}
			}
		}
		return result;
	}

	mat4& mat4::operator*=(const mat4 &other)
	{
		*this = *this * other;
		return *this;
	}

	std::ostream& operator<<(std::ostream &os, const mat4& matrix)
	{
		os << std::endl;
		for (const auto& row : matrix.data) {
			os << "{ ";
			for (float value : row) {
				os << value << " ";
			}
			os << "}";
			os << std::endl;
		}
		return os;
	}
}
