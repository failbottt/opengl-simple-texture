// The purpose of this demo is to load a sprite sheet into memory
// and then render subsections of the sheet
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "GLFW/glfw3.h"

#include "glextloader.c"
#include "file.h"
#include "gl_compile_errors.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SCREEN_WIDTH  1024 
#define SCREEN_HEIGHT 1024 

GLuint VBO, VAO, EBO;
GLuint vertex_shader;
GLuint frag_shader;
GLuint texture_program;

const char *texture_vert_shader = "shaders/texture.vert";
const char *texture_frag_shader = "shaders/texture.frag";

unsigned int indices[] = {  
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

int main() {
	// INIT WINDOW
	//
	//
	//
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ed", NULL, NULL);
	glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
	glfwSetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	load_gl_extensions();
	//
	//
	//
	//

	// COMPILE SHADERS
	//
	//
	//
	const char *vert_shader_source = read_file(texture_vert_shader);
	const char *frag_shader_source = read_file(texture_frag_shader);
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vert_shader_source, NULL);
	glCompileShader(vertex_shader);
	checkCompileErrors(vertex_shader, "VERTEX");
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
	glCompileShader(frag_shader);
	checkCompileErrors(frag_shader, "FRAG");
	texture_program = glCreateProgram();
	glAttachShader(texture_program, vertex_shader);
	glAttachShader(texture_program, frag_shader);
	glLinkProgram(texture_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(frag_shader);
	free(vert_shader_source);
	free(frag_shader_source);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//
	//
	//
	//
	//

	// SETUP UP PROJECTION
	//
	// This changes coordinates of the screen from 
	// perspective to orthographic (ortho is useful for 2D)
	//
	//
	float pj[4][4] = {{0.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 0.0f},  
		{0.0f, 0.0f, 0.0f, 0.0f},                    \
		{0.0f, 0.0f, 0.0f, 0.0f}};

	float rl, tb, fn;
	float left = 0.0f;
	float right = (float)SCREEN_WIDTH;
	float bottom = 0.0f;
	float top = (float)SCREEN_HEIGHT;
	float nearZ = -1.0f;
	float farZ = 1.0f;

	rl = 1.0f / (right  - left);
	tb = 1.0f / (top    - bottom);
	fn =-1.0f / (farZ - nearZ);

	pj[0][0] = 2.0f * rl;
	pj[1][1] = 2.0f * tb;
	pj[2][2] = 2.0f * fn;
	pj[3][0] =-(right  + left)    * rl;
	pj[3][1] =-(top    + bottom)  * tb;
	pj[3][2] = (farZ + nearZ) * fn;
	pj[3][3] = 1.0f;
	// ORTHO END

	glUseProgram(texture_program);
	glUniformMatrix4fv(glGetUniformLocation(texture_program, "projection"), 1, GL_FALSE, (float *)pj);

	// TEXTURE
	//
	//
	//
	// ------------------------------------------------------------------
	// load and create a texture 
	// -------------------------
	int w, h, bits;

	stbi_set_flip_vertically_on_load(1);
	unsigned char *pixels = stbi_load("./images/tiles.png", &w, &h, &bits, STBI_rgb_alpha);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	stbi_image_free(pixels);
	//
	//
	//
	// TEXTURE END

	int quit = 0;
	while (!quit) {
		glfwPollEvents();

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.21f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind Texture
		glBindTexture(GL_TEXTURE_2D, texture);

		// render container
		glUseProgram(texture_program);
		glBindVertexArray(VAO);

		float img_height = 64.0f;
		float img_width = 64.0f;
		float xpos = img_width;
		float ypos = img_height;
		float spritewidth = 16.0f;
		float spriteheight = 16.0f;

		// these are the (x,y) coordinates for the 
		// sprite sheet
		int x = 0;
		int y = 10;

		int rc = (int)(SCREEN_HEIGHT / img_height);
		int cc = (int)(SCREEN_WIDTH / img_width);

		for (int rows = 0; rows < rc/2; rows++)
		{
			if (rows % 3 == 0) {
				y = 9;
			} else {
				y = 10;
			}
			ypos = ((rows)*img_height)+img_height;

			for (int cols = 0; cols < cc; cols++) 
			{
				xpos = (cols*img_width)+img_width;
				float x1 = ((x * spritewidth) / w);
				float y1 = ((y * spriteheight) / h);
				float x2 = (((1+x) * spritewidth) / w);
				float y2 = ((y * spriteheight) / h);
				float x3 = (((1+x) * spritewidth) / w);
				float y3 = (((1+y) * spriteheight) / h);
				float x4 = ((x * spritewidth) / w);
				float y4 = (((1+y) * spriteheight) / h);

				float vertices[] = {
					// positions							// colors				 // texture coords
					xpos,			ypos,				0.0f, 1.0f, 0.0f, 0.0f,  x3, y3, // top right
					xpos,			ypos-img_height,	0.0f, 0.0f, 1.0f, 0.0f,  x2, y2, // bottom right
					xpos-img_width,	ypos-img_height,	0.0f, 0.0f, 0.0f, 0.0f,  x1, y1, // bottom left
					xpos-img_width,	ypos,				0.0f, 1.0f, 1.0f, 0.0f,  x4, y4  // top left 
				};

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

				// position attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				// color attribute
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);

				// texture coord attribute
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);

				glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);
			}
		}

		glfwSwapBuffers(window);
	}

	glDeleteProgram(texture_program);
	glfwTerminate();
}
