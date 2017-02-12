#ifndef PSL_LIB_H
#define PSL_LIB_H

#define SCREEN_HEIGHT 1080
#define SCREEN_WIDTH 1920
#define FULLSCREEN 1

#include"psl_input.h"
#include<SDL2/SDL.h>
#include<GL/glew.h>

#define PSL_PI 3.14

//noise functions
void psl_init_simplex(int seed);

double psl_simplex_2d(double x, double y);
double psl_simplex_3d(double x, double y, double z);
double psl_simplex_4d(double x, double y, double z, double w);

void psl_seed_perlin(int seed);
float psl_grad(int hash, float x, float y, float z);

float psl_noise(float x, float y, float z);
float psl_noise2d(float x, float y, int octaves, float persistence);
float psl_noise3d_abs(float x, float y, float z, int octaves, float persistence);

//vector math
float psl_normalize_f(float a, float min, float max);
float psl_radians(float a);
float psl_degrees(float a);
float psl_fade(float t);
float psl_lerp(float t, float a, float b);
double psl_floor(double x);

float psl_length(float a[3]);
float psl_length_vec4(float a[4]);
void psl_normalize(float out[3]);
void psl_cross(float out[3], float a[3], float b[3]);

float psl_dot(float a[3], float b[3]);
float psl_dot_inv(float a[3], float b[3]);
double psl_dot_2d(int a[], double x, double y);
double psl_dot_3d(int a[], double x, double y, double z);
double psl_dot_4d(int a[], double x, double y, double z, double w);

//matrix math
void psl_identity(float out[4][4]);
void psl_print_matrix(float a[4][4]);
void psl_multiply(float out[4][4], float a[4][4], float b[4][4]);
void psl_multiply_vec4(float out[4], float a[4][4], float x, float y, float z, float w);

void psl_perspective(float out[4][4], float fov, float aspect, float z_near, float z_far);
void psl_look_at(float out[4][4], float pos[3], float target[3]);

void psl_translate(float out[4][4], float vec[3]);
void psl_scale(float out[4][4], float vec[3]);
void psl_rotate(float out[4][4], float vec[3]);

//window
typedef struct
{
	SDL_Window* screen;
	SDL_GLContext context;
} PSL_Window;

typedef struct
{
	int key; //keyboard input
	unsigned char button;
	int x, y; //mouse position
} PSL_Input;

PSL_Window psl_create_window(void);
void psl_destroy_window(PSL_Window* window);
void psl_clear_screen(PSL_Window* window);
char psl_get_input(PSL_Input* input);
double psl_get_time();

//shader functions
char* psl_load_file(const char* file_name);	//NOTE: after this function free() is needed.
GLuint psl_create_shader(const char* vertex, const char* fragment);
GLint psl_get_attrib(GLuint program, const char* name);
GLint psl_get_uniform(GLuint program, const char* name);

//opengl util function
void _psl_gl_error(const char* file, int line); //dont call this function but call psl_check_error()
#define psl_gl_error() _psl_gl_error(__FILE__, __LINE__)

#endif //PSL_LIB_H
