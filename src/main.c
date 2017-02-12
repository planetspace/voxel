#include"psl_lib.h"
#include"main.h"
#include"player.h"
#include"texture.h"
#include"chunk.h"

#include<time.h>
#include<stdlib.h>


int main()
{
	PSL_Window window = psl_create_window();
	PSL_Input input = {0};
	
	GLuint mesh_shader = psl_create_shader("shaders/mesh.vert", "shaders/mesh.frag");
	GLint mesh_coord = psl_get_attrib(mesh_shader, "coord");
	GLint mesh_camera = psl_get_uniform(mesh_shader, "camera");
	//GLint mesh_transform = psl_get_uniform(mesh_shader, "transform");
	GLint mesh_texture = psl_get_uniform(mesh_shader, "tex");
	
	GLuint cursor_vbo;
	glGenBuffers(1, &cursor_vbo);
	
	Player player = create_player(input);
		
	load_texture_atlas("resources/texture_atlas.png");
	
	World world = create_world();
	
	//send non changing data to the shader
	glPolygonOffset(1, 1);
	glUseProgram(mesh_shader);
	glEnableVertexAttribArray(mesh_coord);
	glUniform1i(mesh_texture, 0);
	
	psl_gl_error();
	

	double last_time = psl_get_time();
	double current = 0.0;
	double delta_t = 0.0;
		
#if DEBUG
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
	//TODO: calculate fps + print

	while(psl_get_input(&input))
	{
		current = psl_get_time();
		
		delta_t = (current - last_time) / 1000.0;
		//printf("%f S.\n", delta_t);
		
		update_player(&player, &world, input, delta_t);
		
		psl_clear_screen(&window);

		render_player(&player, mesh_camera);
		render_world(&world, player.mvp, mesh_camera, mesh_coord);
		
		render_ui(player, mesh_camera, mesh_camera, mesh_coord, cursor_vbo);
		
		last_time = current;
	}
	
	delete_world(&world);
	psl_destroy_window(&window);
	
	return 0;	
}
