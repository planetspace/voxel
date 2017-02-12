#include"psl_lib.h"
#include"psl_input.h"

#include<time.h>
#include<stdio.h>
#include<stdlib.h>

char* psl_load_file(const char* file_name)
{
	FILE* file = fopen(file_name, "rb");
	if(file == NULL)
	{
		printf("[PSL ERROR]\tFile: %s can not be opened!\n", file_name);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char* data = calloc(length +1, sizeof(char));
	fread(data, 1, length, file);
	fclose(file);
	return data;
}

GLuint psl_create_shader(const char* vertex, const char* fragment)
{
	//compile vertex shader
	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	char* source = psl_load_file(vertex);
	glShaderSource(vertex_id, 1, (const char**)&source, NULL);
	free(source);
	
	glCompileShader(vertex_id);
	//find error in compilation
	GLint is_compiled = 0;
	glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(vertex_id, GL_INFO_LOG_LENGTH, &length);
		GLchar* text = calloc(length, sizeof(GLchar));
		glGetShaderInfoLog(vertex_id, length, NULL, text);
		printf("shader: %s failed to compile:\t%s\n", vertex, text);
		free(text);
	}
	
	//compile fragment shader
	GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	source = psl_load_file(fragment);
	glShaderSource(fragment_id, 1, (const char**)&source, NULL);
	free(source);
	
	glCompileShader(fragment_id);
	//find error in compilation
	is_compiled = 0;
	glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(fragment_id, GL_INFO_LOG_LENGTH, &length);
		GLchar* text = calloc(length, sizeof(GLchar));
		glGetShaderInfoLog(fragment_id, length, NULL, text);
		printf("shader: %s failed to compile:\t%s\n", fragment, text);
		free(text);
	}
	
	//link shaders together
	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);
	//find errors in linking
	is_compiled = 0;
	glGetProgramiv(program_id, GL_LINK_STATUS, &is_compiled);
	if(is_compiled == GL_FALSE)
	{
		GLint length = 0;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
		GLchar* text = calloc(length, sizeof(GLchar));
		glGetProgramInfoLog(program_id, length, NULL, text);
		printf("Program failed to link:\t%s\n", text);
		free(text);
	}
	
	//delete shaders
	glDetachShader(program_id, vertex_id);
	glDetachShader(program_id, fragment_id);
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
	
	//print error
	psl_gl_error();
	
	return program_id;
}

GLint psl_get_attrib(GLuint program, const char* name)
{
	GLint attribute = glGetAttribLocation(program, name);
	if(attribute == -1)
		printf("[PSL ERROR]\tCould not bind attribute: %s\n", name);
	return attribute;
}

GLint psl_get_uniform(GLuint program, const char* name)
{
	GLint uniform = glGetUniformLocation(program, name);
	if(uniform == -1)
		printf("[PSL ERROR]\tCould not bind uniform: %s\n", name);
	return uniform;
}

void _psl_gl_error(const char* file, int line)
{
	GLenum error_code;
	while((error_code = glGetError()) != GL_NO_ERROR)
	{
		char* error;
		switch(error_code)
		{
			case GL_INVALID_ENUM:					error = "[PSL ERROR]\tinvalid enum"; break;
			case GL_INVALID_VALUE:					error = "[PSL ERROR]\tinvalid value"; break;
			case GL_INVALID_OPERATION:				error = "[PSL ERROR]\tinvalid operation"; break;
			case GL_STACK_OVERFLOW:					error = "[PSL ERROR]\tstack overflow"; break;
			case GL_STACK_UNDERFLOW:				error = "[PSL ERROR]\tstack underflow"; break;
			case GL_OUT_OF_MEMORY:					error = "[PSL ERROR]\tout of memory"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:	error = "[PSL ERROR]\tinvalid framebuffer operation"; break;
		}
		printf("%s: %s\t(%i)\n", error, file, line);
	}
}

PSL_Window psl_create_window(void)
{
	printf("[     PSL VERSION 0.0     ]\n");
	//initialize sdl2
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_ShowCursor(SDL_DISABLE);
	
	PSL_Window window;
	if(FULLSCREEN)
		window.screen = SDL_CreateWindow("title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
	else
		window.screen = SDL_CreateWindow("title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
	
	if(window.screen == NULL)
		printf("[PSL ERROR]\tWindow could not be created:\t%s\n", SDL_GetError());
	
	window.context = SDL_GL_CreateContext(window.screen);
	
	//initialize glew
	glewExperimental = GL_TRUE;
	GLenum error = glewInit();
	if(error != GLEW_OK)
		printf("[PSL ERROR]\tGlew could not be initialized:\t%s\n", glewGetString(error));
	else
		printf("[PSL]\tGlew version:\t%s\n", glewGetString(GLEW_VERSION));
	
	psl_gl_error();
	
	glClearColor(0.6, 0.8, 1.0, 1.0);
	
	return window;
}

void psl_destroy_window(PSL_Window* window)
{
	SDL_GL_DeleteContext(window->context);
	SDL_DestroyWindow(window->screen);
	SDL_Quit();
	
	psl_gl_error();
}

void psl_clear_screen(PSL_Window* window)
{
	SDL_GL_SwapWindow(window->screen);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

char psl_get_input(PSL_Input* input)
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				return 0;
				break;
			
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE)	return 0;
				if(event.key.keysym.sym == SDLK_w)	input->key |= MOVE_FORWARD;
				if(event.key.keysym.sym == SDLK_s)	input->key |= MOVE_BACK;
				if(event.key.keysym.sym == SDLK_a)	input->key |= MOVE_LEFT;
				if(event.key.keysym.sym == SDLK_d)	input->key |= MOVE_RIGHT;
				if(event.key.keysym.sym == SDLK_SPACE)	input->key |= MOVE_UP;
				if(event.key.keysym.sym == SDLK_LSHIFT)	input->key |= MOVE_DOWN;
				if(event.key.keysym.sym == SDLK_e)		input->key |= ACTION;
				break;
			case SDL_KEYUP:
				if(event.key.keysym.sym == SDLK_w)	input->key &= ~MOVE_FORWARD;
				if(event.key.keysym.sym == SDLK_s)	input->key &= ~MOVE_BACK;
				if(event.key.keysym.sym == SDLK_a)	input->key &= ~MOVE_LEFT;
				if(event.key.keysym.sym == SDLK_d)	input->key &= ~MOVE_RIGHT;
				if(event.key.keysym.sym == SDLK_SPACE)	input->key &= ~MOVE_UP;
				if(event.key.keysym.sym == SDLK_LSHIFT)	input->key &= ~MOVE_DOWN;
				if(event.key.keysym.sym == SDLK_e)		input->key &= ~ACTION;
				break;

		}
	}
	
	//capture mouse
	int x;
	int y;
	input->button = SDL_GetMouseState(&x, &y);
	input->x += ((SCREEN_WIDTH/2) - x);
	input->y += (y - (SCREEN_HEIGHT/2));	
	SDL_WarpMouseInWindow(NULL, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
	
	return 1;
}

double psl_get_time()
{
	return ((double)clock() / CLOCKS_PER_SEC) * 1000.0;
}
