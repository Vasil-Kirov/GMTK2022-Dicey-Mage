#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texture;

out vec2 texture_coords;

void main()
{
    gl_Position = vec4(position.x, position.y, 1, 1);
    texture_coords = texture;
}

