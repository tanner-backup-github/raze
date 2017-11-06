#version 330 core

in vec2 tex_coord;

out vec4 out_color;

uniform sampler2D my_texture;
uniform vec4 color;

void main(void) {
	out_color = color * texture(my_texture, tex_coord);
}
