#include"player.h"
#include<math.h>

#define MOVE_SPEED 30
#define ROTATE_SPEED 200
#define CLICK_DISTANCE 100

Player create_player(PSL_Input input)
{
	Player player;
	
	player.pos[0] = 9.0f;
	player.pos[1] = 10.0f;
	player.pos[2] = 9.0f;
	
	player.rot[0] = 0.0f;
	player.rot[1] = 0.0f;
	player.rot[2] = 1.0f;
	
	player.yaw = -90.0f;
	player.pitch = 0.0f;
	player.last_x = input.x;
	player.last_y = input.y;
	
	player.chunk[0] = 0;
	player.chunk[1] = 0;
	player.chunk[2] = 0;
	
	psl_perspective(player.view, 1.0f, (float)SCREEN_WIDTH/SCREEN_HEIGHT, 0.01f, 1000.0f);
	psl_identity(player.mvp);
	
	return player;
}

void update_player(Player* player, World* world, PSL_Input input, double delta)
{
	float offset_x = player->last_x - input.x;
	float offset_y = input.y - player->last_y;
	player->last_x = input.x;
	player->last_y = input.y;
	
	offset_x *= ROTATE_SPEED * delta;
	offset_y *= ROTATE_SPEED * delta;
	
	//calculate camera vector
	player->yaw += offset_x;
	player->pitch += offset_y;
	
	if(player->pitch > 89.0f)
		player->pitch = 89.0f;
	if(player->pitch < -89.0f)
		player->pitch = -89.0f;
	
	float rad_yaw = psl_radians(player->yaw);
	float rad_pitch = psl_radians(player->pitch);
	
	player->rot[0] = cos(rad_yaw) * cos(rad_pitch);
	player->rot[1] = sin(rad_pitch);
	player->rot[2] = sin(rad_yaw) * cos(rad_pitch);
	
	psl_normalize(player->rot);
	
	float up[3] = { 0.0, 1.0, 0.0 }; 
	psl_cross(player->right, player->rot, up);
	
	//get input and preform actions
	if(input.key & MOVE_FORWARD)
	{
		player->pos[0] -= player->rot[0] * MOVE_SPEED * delta;
		player->pos[1] -= player->rot[1] * MOVE_SPEED * delta;
		player->pos[2] -= player->rot[2] * MOVE_SPEED * delta;
	}
	if(input.key & MOVE_BACK)
	{
		player->pos[0] += player->rot[0] * MOVE_SPEED * delta;
		player->pos[1] += player->rot[1] * MOVE_SPEED * delta;
		player->pos[2] += player->rot[2] * MOVE_SPEED * delta;
	}
	if(input.key & MOVE_LEFT)
	{
		player->pos[0] += player->right[0] * MOVE_SPEED * delta;
		player->pos[1] += player->right[1] * MOVE_SPEED * delta;
		player->pos[2] += player->right[2] * MOVE_SPEED * delta;
	}
	if(input.key & MOVE_RIGHT)
	{
		player->pos[0] -= player->right[0] * MOVE_SPEED * delta;
		player->pos[1] -= player->right[1] * MOVE_SPEED * delta;
		player->pos[2] -= player->right[2] * MOVE_SPEED * delta;
	}
	if(input.key & MOVE_UP)
	{
		player->pos[1] += MOVE_SPEED * delta;
	}
	if(input.key & MOVE_DOWN)
	{
		
		player->pos[1] -= MOVE_SPEED * delta;
	}
	
	//cast ray
	float temp[3];
	float prev[3];
	
	temp[0] = prev[0] = player->pos[0];
	temp[1] = prev[1] = player->pos[1];
	temp[2] = prev[2] = player->pos[2];
	
	for(int i = 0; i < 200; i++)
	{
		prev[0] = temp[0];
		prev[1] = temp[1];
		prev[2] = temp[2];
		
		temp[0] -= player->rot[0] * 0.1f;
		temp[1] -= player->rot[1] * 0.1f;
		temp[2] -= player->rot[2] * 0.1f;
		
		player->select[0] = psl_floor(temp[0]);
		player->select[1] = psl_floor(temp[1]);
		player->select[2] = psl_floor(temp[2]);
		
		if(get_voxel(world, (int)player->select[0], (int)player->select[1], (int)player->select[2]) != VOXEL_AIR)
			break;
	}
	
	if(input.button & BUTTON_LEFT)
	{
		player->mine_time += delta;
		//0.2 time to mine
		if(player->mine_time >= 0.2 )
		{
			set_voxel(world, (int)player->select[0], (int)player->select[1], (int)player->select[2], VOXEL_AIR);
			player->mine_time = 0.0;
		}
	}
	
	if(input.button & BUTTON_RIGHT && get_voxel(world, (int)player->select[0], (int)player->select[1], (int)player->select[2]) != VOXEL_AIR)
	{
		player->mine_time += delta;
		if(player->mine_time >= 0.1)
		{
			int mx = psl_floor(prev[0]);
			int my = psl_floor(prev[1]);
			int mz = psl_floor(prev[2]);
		
			int xx = player->select[0];
			int yy = player->select[1];
			int zz = player->select[2];
		
			if(mx > player->select[0])
				xx++;
			else if(mx < player->select[0])
				xx--;
			else if(my > player->select[1])
				yy++;
			else if(my < player->select[1])
				yy--;
			else if(mz > player->select[2])
				zz++;
			else if(mz < player->select[2])
				zz--;
		
			set_voxel(world, xx, yy, zz, VOXEL_STONE);
			player->mine_time = 0.0;
		}
	}
	
	//did we leave the chunk already?
	if(player->chunk[0] > (int)player->pos[0] / CHUNK_SIZE_X)
	{
		scroll_world(world, player->chunk, -1, 0, 0);
		player->chunk[0]--;
	}
	else if(player->chunk[0] < (int)player->pos[0] / CHUNK_SIZE_X)
	{
		scroll_world(world, player->chunk, 1, 0, 0);
		player->chunk[0]++;
	}
	if(player->chunk[1] > (int)player->pos[1] / CHUNK_SIZE_Y)
	{
		scroll_world(world, player->chunk, 0, -1, 0);
		player->chunk[1]--;
	}
	else if(player->chunk[1] < (int)player->pos[1] / CHUNK_SIZE_Y)
	{
		scroll_world(world, player->chunk, 0, 1, 0);
		player->chunk[1]++;
	}
	if(player->chunk[2] > (int)player->pos[2] / CHUNK_SIZE_Z)
	{
		scroll_world(world, player->chunk, 0, 0, -1);
		player->chunk[2]--;
	}
	else if(player->chunk[2] < (int)player->pos[2] / CHUNK_SIZE_Z)
	{
		scroll_world(world, player->chunk, 0, 0, 1);
		player->chunk[2]++;
	}
}

void render_player(Player* player, GLint camera)
{
	//render player
	float matrix[4][4];
	
	float temp[3];
	temp[0] = player->pos[0] + player->rot[0];
	temp[1] = player->pos[1] + player->rot[1];
	temp[2] = player->pos[2] + player->rot[2];
	psl_look_at(matrix, player->pos, temp);
	
	psl_multiply(player->mvp, matrix, player->view);
	glUniformMatrix4fv(camera, 1, 0, &player->mvp[0][0]);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);
	
}

void render_ui(Player player, GLint camera, GLint transform, GLint coord, GLuint cursor)
{
	float bx = player.select[0];
	float by = player.select[1];
	float bz = player.select[2];
	
	float mat[4][4] = 
	{
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 }
	};
	
	float box[24][4] = 
	{
		{bx + 0, by + 0, bz + 0, 14},
		{bx + 1, by + 0, bz + 0, 14},
		{bx + 0, by + 1, bz + 0, 14},
		{bx + 1, by + 1, bz + 0, 14},
		{bx + 0, by + 0, bz + 1, 14},
		{bx + 1, by + 0, bz + 1, 14},
		{bx + 0, by + 1, bz + 1, 14},
		{bx + 1, by + 1, bz + 1, 14},
		
		{bx + 0, by + 0, bz + 0, 14},
		{bx + 0, by + 1, bz + 0, 14},
		{bx + 1, by + 0, bz + 0, 14},
		{bx + 1, by + 1, bz + 0, 14},
		{bx + 0, by + 0, bz + 1, 14},
		{bx + 0, by + 1, bz + 1, 14},
		{bx + 1, by + 0, bz + 1, 14},
		{bx + 1, by + 1, bz + 1, 14},
		
		{bx + 0, by + 0, bz + 0, 14},
		{bx + 0, by + 0, bz + 1, 14},
		{bx + 1, by + 0, bz + 0, 14},
		{bx + 1, by + 0, bz + 1, 14},
		{bx + 0, by + 1, bz + 0, 14},
		{bx + 0, by + 1, bz + 1, 14},
		{bx + 1, by + 1, bz + 0, 14},
		{bx + 1, by + 1, bz + 1, 14}
		
	};
		
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_CULL_FACE);
	glLineWidth(3);

	glBindBuffer(GL_ARRAY_BUFFER, cursor);
		
	glUniformMatrix4fv(transform, 1, GL_FALSE, &mat[0][0]);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, 24);
	
	//render cross in centre of the screen
	float cross[4][4] = 
	{
		{ -0.05, 0.0, 0.0, 13.0 },
		{ +0.05, 0.0, 0.0, 13.0 },
		{ 0.0, -0.05, 0.0, 13.0 },
		{ 0.0, +0.05, 0.0, 13.0 }
	};

	glUniformMatrix4fv(transform, 1, GL_FALSE, &mat[0][0]);
	glUniformMatrix4fv(camera, 1, GL_FALSE, &mat[0][0]);

	glDisable(GL_DEPTH_TEST);

	glBufferData(GL_ARRAY_BUFFER, sizeof cross, cross, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINES, 0, 4);

}
