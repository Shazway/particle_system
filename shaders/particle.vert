#version 430 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

uniform mat4 viewMatrix;

out vec3 frag_color;

void main()
{
    gl_Position = viewMatrix * vec4(in_pos, 1.0);
    frag_color = in_color;

    // Optional: point size based on distance or velocity
    gl_PointSize = 1.0;
}
