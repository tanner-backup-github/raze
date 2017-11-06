#ifndef GL_H__
#define GL_H__

#include <inttypes.h>
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "file.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

GLuint make_buffer(GLenum target, const void *data, GLsizei size) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(target, buf);
	glBufferData(target, size, data, GL_STATIC_DRAW);
	return buf;
}

// @TODO: unicode
// @TODO: kerning
// @TODO: compare sdl_ttf to this to see what else we need
GLuint generate_text_texture(const char *text, FT_Face face, size_t *w, size_t *h) {
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        size_t text_len = strlen(text);
	FT_GlyphSlot glyph = face->glyph;
        size_t width = 0;
        size_t base = 0;
	for (size_t i = 0; i < text_len; ++i) {
		FT_Load_Char(face, text[i], FT_LOAD_RENDER); // @TODO: !cache

		width += glyph->advance.x >> 6;
	        base = MAX(base, glyph->bitmap.rows);
	}
	
	// @NOTE: I am proud of this bad name, but it's really just a little extra bit
	// of a gap so that glyphs with 'tails' are rendered in full
	size_t mind_the_gap = (face->ascender >> 6) - (face->descender >> 6);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, base + mind_the_gap, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	
	int32_t advx = 0;
	for (size_t i = 0; i < text_len; ++i) {
		FT_Load_Char(face, text[i], FT_LOAD_RENDER); // @TODO: !cache

		glTexSubImage2D(GL_TEXTURE_2D, 0, advx, base - glyph->bitmap_top, glyph->bitmap.width, glyph->bitmap.rows,
				GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
		advx += glyph->advance.x >> 6;
	}
	*w = width;
	*h = base + mind_the_gap;

	return tex_id;
}

GLuint load_texture(const char *path, int *w, int *h) {
        assert(w);
        assert(h);
	int n;
	uint8_t *data = stbi_load(path, w, h, &n, 4);
	
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *w, *h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	stbi_image_free(data);
	
	return tex_id;
}

GLuint make_shader_program(const char *vs_path, const char *fs_path) {
	char *vs_code = read_entire_file(vs_path);
	char *fs_code = read_entire_file(fs_path);

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *const) &vs_code, NULL);
	glShaderSource(fragment_shader, 1, (const char *const *const) &fs_code, NULL);
	glCompileShader(vertex_shader);
	/* CHECK_SHADER("Vertex", vertex_shader); */
	glCompileShader(fragment_shader);
	/* CHECK_SHADER("Fragment", fragment_shader); */
	
	free(vs_code);
	free(fs_code);
	
	GLuint shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	return shader;
}

#endif
