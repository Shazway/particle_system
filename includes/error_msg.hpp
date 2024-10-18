/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_msg.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmoragli <tmoragli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 00:57:01 by tmoragli          #+#    #+#             */
/*   Updated: 2024/10/18 20:25:35 by tmoragli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define PLATFORM_ID_ERR "Couldn't get platform IDs"
#define PLATFORM_GET_ERR "Couldn't get any platforms"
#define DEVICE_ID_ERR "Couldn't get any device IDs"
#define DEVICE_GET_ERR "Couldn't find any suitable devices"
#define CONTEXT_CREATE_ERR "Couldn't create context"
#define QUEUE_CREATE_ERR "Couldn't create queue"
#define PROGRAM_CREATE_ERR "Couldn't create program: "
#define PROGRAM_BUILD_ERR "Couldn't build program: "
#define KERNEL_CREATE_ERR "Couldn't create kernel: "
#define BUFFER_CREATE_ERR "Couldn't create interoperable buffer"
#define KERNEL_ARGS_SET_ERR "Couldn't set args for kernel"
#define ENQUEUE_NDRANGE_KERNEL_ERR "Couldn't run kernel"
#define ENQUEUE_BUFFER_CL_GL_ERR "Failed to acquire OpenGL buffer for OpenCL"
#define RELEASE_BUFFER_CL_GL_ERR "Failed to release OpenGL buffer for OpenCL"
#define FETCH_CL_FILE_ERR "Failed to open .cl file"
#define NO_PARTICLES_ERR "0 particles detected, at least 1 required"
