#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 in_tex_coord;

uniform mat4 model;
uniform mat4 proj;

out vec2 tex_coord;

void main(void) {
	tex_coord = in_tex_coord;
	gl_Position = proj * model * vec4(pos, 0.0, 1.0);
}
