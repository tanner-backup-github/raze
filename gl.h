#pragma once

#include <assert.h>
#include <inttypes.h>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define STB_IMAGE_IMPLEMENTATION
#include "not_mine/stb_image.h"
#include "array.h"
#include "file.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

GLuint make_buffer(GLenum target, const void *data, GLsizei size) {
	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(target, buf);
	glBufferData(target, size, data, GL_STATIC_DRAW);
	return buf;
}

typedef struct {
	uint8_t *buffer;
        int32_t w, h;
	int32_t bearingX, bearingY;
	int32_t advanceX;
} MyGlyph;

typedef struct {
        array glyphs;
	size_t size;
	// @NOTE: Prevent tails from getting cut off
	int32_t tail_gap;
} Font;

void free_myglyph(MyGlyph *my_glyph) {
	free(my_glyph->buffer);
	free(my_glyph);
}

Font *new_font(FT_Library ft, const char *path, size_t size) {
	FT_Face face;
	assert(!FT_New_Face(ft, path, 0, &face));
	// @TODO: Invalid font sizes?
	FT_Set_Pixel_Sizes(face, 0, size);

	Font *font = malloc(sizeof(*font));
	font->size = size;
	font->tail_gap = (face->ascender >> 6) + (face->descender >> 6);

	// @TODO: unicode
	init_array_f(&font->glyphs, 128, sizeof(MyGlyph *), (void *) free_myglyph);
	FT_GlyphSlot glyph = face->glyph;
	for (int i = 0; i < 128; ++i) {
		FT_Load_Char(face, i, FT_LOAD_RENDER);

		MyGlyph *mg = malloc(sizeof(*mg));
		mg->w = glyph->bitmap.width;
		mg->h = glyph->bitmap.rows;

		mg->buffer = malloc(mg->w * mg->h);
		memcpy(mg->buffer, glyph->bitmap.buffer, mg->w * mg->h);

		mg->bearingX = glyph->bitmap_left;
		mg->bearingY = glyph->bitmap_top;
		mg->advanceX = glyph->advance.x >> 6;

		add_array(&font->glyphs, mg);
	}

	FT_Done_Face(face);

	return font;
}

void free_font(Font *f) {
	free_array(&f->glyphs);
	free(f);
}

// @TODO: unicode
// @TODO: kerning
GLuint generate_text(const char *text, Font *font, int32_t *w, int32_t *h) {
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	int32_t width = 0;
	int32_t height = 0;
	int32_t base = 0;
	for (size_t i = 0; text[i] != '\0'; ++i) {
		MyGlyph *glyph = GET_ARRAY(&font->glyphs, (int) text[i], MyGlyph *);

		width += glyph->advanceX;
		height = MAX(height, glyph->h + (glyph->h - glyph->bearingY)); /* */
		base = MAX(base, glyph->h);
	}
	height += font->tail_gap;

	uint8_t *blank_for_artifacts = calloc(width * height, sizeof(uint8_t));

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height,
		     0, GL_RED, GL_UNSIGNED_BYTE, blank_for_artifacts);

	*w = width;
	*h = height;

	int advx = 0;
	for (size_t i = 0; text[i] != '\0'; ++i) {
		MyGlyph *glyph = GET_ARRAY(&font->glyphs, (int) text[i], MyGlyph *);

		glTexSubImage2D(GL_TEXTURE_2D, 0, advx + MAX(glyph->bearingX, 0), base - glyph->bearingY, glyph->w, glyph->h,
				GL_RED, GL_UNSIGNED_BYTE, glyph->buffer);
		advx += glyph->advanceX;
	}

	return tex_id;
}

GLuint load_texture(const char *path, int32_t *w, int32_t *h) {
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
