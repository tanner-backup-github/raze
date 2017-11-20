#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <curl/curl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "gl.h"
#include "gl_math.h"
#include "dumb_string.h"
#include "tokenizer.h"
#include "parser.h"

void get_request(CURL *curl, const char *url, dumb_string *recv) {
	size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
		dumb_string *ds = ((dumb_string **) userdata)[0];
		append_dumb_string(ds, ptr);
		return size * nmemb;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *[]) { recv });
	assert(curl_easy_perform(curl) == CURLE_OK);
}

// @TODO: memcpy array
// @TODO: assert strdup
// @TODO: safe_malloc, safe_realloc, NULL_FREE
// @TODO: Custom allocator

typedef double (* Math)(double, double);

double my_add(double x, double y) {
	return x + y;
}

double my_sub(double x, double y) {
	return x - y;
}

double my_mul(double x, double y) {
	return x * y;
}

double my_div(double x, double y) {
	return x / y;
}

Math lookup(const char *buf) {
	if (strcmp(buf, "+") == 0) {
		return my_add;
	} else if (strcmp(buf, "-") == 0) {
		return my_sub;
	} else if (strcmp(buf, "*") == 0) {
		return my_mul;
	} else if (strcmp(buf, "/") == 0) {
		return my_div;
	}
	printf("Undefined function!!!\n");
	assert(false); // @TODO: panic
	return NULL;
}

void eval(parse_node *root, array *stack) {
	if (!root->children) {
		double *i = malloc(sizeof(*i));
		*i = strtof(root->token.buf, NULL);
		add_array(stack, i);
	} else {
		for (size_t i = 0; i < root->children->size; ++i) {
			eval(GET_ARRAY(root->children, i, parse_node *), stack);
		}

		if (strcmp(root->token.buf, "#ROOT#") == 0 && root->token.type == PUNCTUATION) {
			return;
		}

		Math op = lookup(root->token.buf);
	}
}

int main(void) {
	
	/* curl_global_init(CURL_GLOBAL_ALL); */
	
	/* CURL *curl = curl_easy_init(); */
	
	dumb_string s;
        init_dumb_string(&s, read_entire_file("local_code"), 512);
	/* get_request(curl, "https://razefiles.herokuapp.com/code", &s); */
	array *tokens = tokenize(s.data, s.len);
	free_dumb_string(&s);

	parse_node *root = parse(tokens);
	free_array(tokens);
	free(tokens);

	array stack;
	INIT_ARRAY(&stack, 32, sizeof(double *));
	eval(root, &stack);
	
	/* printf("%f\n", * (double *) pop_array(&stack)); */
	
	/* free_array(&stack); */

	free_parse_nodes(root);
	
	/* curl_global_cleanup(); */
	
	assert(glfwInit());
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const int WINDOW_WIDTH  = 800;
	const int WINDOW_HEIGHT = 600;
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raze",
					      NULL, NULL);
	assert(window);
	
	const GLFWvidmode *const MODE = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(window, MODE->width / 2 - WINDOW_WIDTH / 2,
			 MODE->height / 2 - WINDOW_HEIGHT / 2);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	assert(!glewInit());

	uint32_t VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
#define X 0.5
	GLfloat vertices[] = {
		X,  X,  1.0, 1.0,
		X,  -X, 1.0, 0.0,
		-X, -X, 0.0, 0.0,
		-X, X,  0.0, 1.0
	};
        GLuint indices[] = {
		0, 1, 3,
		1, 2, 3,
	};
	GLuint VBO = make_buffer(GL_ARRAY_BUFFER, vertices, sizeof(vertices));
	GLuint IBO = make_buffer(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices));
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
			      (void *) (2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	GLuint shader = make_shader_program("vs.glsl", "fs.glsl");
	GLuint text_shader = make_shader_program("vs.glsl", "text_fs.glsl");

	GLfloat projm[] = IDENTITY_MATRIX;
	ortho(projm, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	
	glClearColor(0.92, 0.92, 0.92, 1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	/* glfwSwapInterval(-1); */

	FT_Library ft;
	FT_Init_FreeType(&ft);

	Font *f = new_font(ft, "OpenSans-Regular.ttf", 64);

	int32_t w = 0, h = 0;
	GLuint tex_id = generate_text("The Web Sucks.", f, &w, &h);

	int32_t w2 = 0, h2 = 0;
	GLuint tex_id2 = generate_text("Let's Make It Better.", f, &w2, &h2);
	
	free_font(f);
	
	FT_Done_FreeType(ft);
	
	while (!glfwWindowShouldClose(window)) {
	        float s = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			break;
		}

		glBindVertexArray(VAO);
		glUseProgram(text_shader);

		{
			GLfloat modelm[] = IDENTITY_MATRIX;
			model_matrix(modelm, WINDOW_WIDTH / 2 - w / 2, 0, w, h, 0);
			
			glBindTexture(GL_TEXTURE_2D, tex_id);
		
			glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, projm);
			glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, modelm);
		
			glUniform4f(glGetUniformLocation(shader, "color"), 0.7, 0.3, 0.8, 1.0);
		
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		}

		{
			GLfloat modelm[] = IDENTITY_MATRIX;
			model_matrix(modelm, WINDOW_WIDTH / 2 - w2 / 2, h, w2, h2, 0);
		
			glBindTexture(GL_TEXTURE_2D, tex_id2);
		
			glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, projm);
			glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, modelm);
		
			glUniform4f(glGetUniformLocation(shader, "color"), 0.9, 0.3, 0.1, 1.0);
		
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		}
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		float e = glfwGetTime();
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
			printf("Frame took: %fms\n", (e - s) * 1000);
		}
	}

	glDeleteTextures(1, &tex_id);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteProgram(text_shader);
	glDeleteProgram(shader);

	glfwTerminate();
	
	return 0;
}
