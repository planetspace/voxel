#ifndef PLAYER_H
#define PLAYER_H

#include"psl_lib.h"
#include"chunk.h"

typedef struct
{
	float pos[3];
	float rot[3];
	float right[3];
	
	float yaw, pitch, last_x, last_y;
	
	int select[3];
	unsigned char voxel;
	double mine_time;
	
	int chunk[3];
	
	float view[4][4];
	float mvp[4][4];
} Player;

Player create_player(PSL_Input input);
void update_player(Player* player, World* world, PSL_Input input, double delta);
void render_player(Player* player, GLint camera);
void render_ui(Player player, GLint camera, GLint transform, GLint coord, GLuint cursor);


#endif //PLAYER_H
