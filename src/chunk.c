#include"chunk.h"
#include"texture.h"
#include"main.h"
#include"psl_lib.h"

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<time.h>

//generation defines
#define SEA_LEVEL 0

Byte4 make_byte(GLbyte x, GLbyte y, GLbyte z, GLbyte w)
{
	Byte4 byte;
	byte.x = x;
	byte.y = y;
	byte.z = z;
	byte.w = w;
	return byte;
}

void create_chunk(Chunk* chunk, unsigned int buffer, int pos_x, int pos_y, int pos_z)
{
	memset(chunk->data, 0, sizeof(chunk->data));
	chunk->left = chunk->right = chunk->up = chunk->down = chunk->front = chunk->back = NULL;
	chunk->elements = 0;
	chunk->pos[0] = pos_x;
	chunk->pos[1] = pos_y;
	chunk->pos[2] = pos_z;
	if(buffer == 0)
	{
		glGenBuffers(1, &chunk->vbo);
		chunk->changed |= (0x01 | 0x02);
	}
	else
	{
		chunk->vbo = buffer;
		chunk->changed |= (0x01 | 0x02);
	}
}

void generate_chunk(Chunk* chunk)
{
#if(DEBUG)
	printf("generate chunk(%i, %i, %i)\n", chunk->pos[0], chunk->pos[1], chunk->pos[2]);
#endif

	for(int x = 0; x < CHUNK_SIZE_X; x++)
	{
		for(int z = 0; z < CHUNK_SIZE_Z; z++)
		{
			//scale 0.5 * 4 = plane 
			//scale 0.6 * 8 = hill
			//scale 0.7 * 8 = mountain
			float n = psl_noise2d((x + chunk->pos[0] * CHUNK_SIZE_X) / 256.0,	(z + chunk->pos[2] * CHUNK_SIZE_Z) / 256.0,	5, 0.6) * 8;
			int h = n * 2;
			//height - depth = sea
			for(int y = 0; y < CHUNK_SIZE_Y; y++)
			{
				if(y + chunk->pos[1] * CHUNK_SIZE_Y >= h) //are we above ground?
				{
					if(y + chunk->pos[1] * CHUNK_SIZE_Y < SEA_LEVEL) // are we under sea level?
					{
						set_vox(chunk, x, y, z, VOXEL_WATER);
						continue;
					}
					else
					{
						//TODO: place trees
						int place = rand() % 255;
						if(place == 0)
						{
							if(y > 0 && chunk->data[x][y-1][z] == VOXEL_GRASS)
							{
								int tree_length = rand() % 5 + 3;
								int tree_leave_size = 5;
								for(int l = 0; l < tree_length + tree_leave_size; l++)
								{
									if(l < tree_length)
										set_vox(chunk, x, l+y, z, VOXEL_WOOD);
									else
									{
										for(int j = 0; j < tree_leave_size; j++)
										{
											for(int k = 0; k < tree_leave_size; k++)
											{	
												int temp = tree_leave_size / 2;
												set_vox(chunk, (j - temp) + x, l+y, (k - temp) + z, VOXEL_LEAVES);
											}
										}
									}
								}
							}
							else 
							{
								break;
							}
						}
						break;
					}
				}
				
				float r = psl_noise3d_abs((x + chunk->pos[0] * CHUNK_SIZE_X) / 16.0, (y + chunk->pos[1] * CHUNK_SIZE_Y) / 16.0, (z + chunk->pos[2] * CHUNK_SIZE_Z) / 16.0, 2, 0.2);
				
				//generate stone and ore
				if(y + chunk->pos[1] * CHUNK_SIZE_Y <= h - 8)
				{
					set_vox(chunk, x, y, z, VOXEL_STONE);
					if(r < 0.02)
						set_vox(chunk, x, y, z, VOXEL_DIAMOND);
					
				}
				else
				{
					if(n + r * 5 < 0.8f || n < 0)
						set_vox(chunk, x, y, z, VOXEL_SAND);
					else if(n + r * 5 < 12.0f)
					{
						if(h < SEA_LEVEL || y + chunk->pos[1] * CHUNK_SIZE_Y < h-1)
							set_vox(chunk, x, y, z, VOXEL_DIRT);
						else
							set_vox(chunk, x, y, z, VOXEL_GRASS);
					}
					else
						set_vox(chunk, x, y, z, VOXEL_STONE);
				}
			}
		}
		
	}
	
	//we no longer need to generate this chunk but we need to build it
	chunk->changed &= ~0x02;
	chunk->changed |= 0x01;
}
unsigned char get_vox(Chunk* chunk, int x, int y, int z)
{
	
	if(x < 0)
		return chunk->left ? get_vox(chunk->left, x + CHUNK_SIZE_X, y, z) : 0;
	if(x >= CHUNK_SIZE_X)
		return chunk->right ? get_vox(chunk->right, x - CHUNK_SIZE_X, y, z) : 0;
	if(y < 0)
		return chunk->down ? get_vox(chunk->down, x, y + CHUNK_SIZE_Y, z) : 0;
	if(y >= CHUNK_SIZE_Y)
		return chunk->up ? get_vox(chunk->up, x, y - CHUNK_SIZE_Y, z) : 0;
	if(z < 0)
		return chunk->front ? get_vox(chunk->front, x, y, z + CHUNK_SIZE_Z) : 0;
	if(z >= CHUNK_SIZE_Z)
		return chunk->back ? get_vox(chunk->back, x, y, z - CHUNK_SIZE_Z) : 0;
	return chunk->data[x][y][z];
}

void set_vox(Chunk* chunk, int x, int y, int z, int type)
{
	if(x < 0)
	{
		if(chunk->left)
			set_vox(chunk->left, x + CHUNK_SIZE_X, y, z, type);
		return;	
	}
	if(x >= CHUNK_SIZE_X)
	{
		if(chunk->right)
			set_vox(chunk->right, x - CHUNK_SIZE_X, y, z, type);
		return;
	}
	if(y < 0)
	{
		if(chunk->down)
			set_vox(chunk->down, x, y + CHUNK_SIZE_Y, z, type);
		return;
	}
	if(y >= CHUNK_SIZE_Y)
	{
		if(chunk->up)
			set_vox(chunk->up, x, y - CHUNK_SIZE_Y, z, type);
		return;
	}
	if(z < 0)
	{
		if(chunk->front)
			set_vox(chunk->front, x, y, z + CHUNK_SIZE_Z, type);
		return;
	}
	if(z >= CHUNK_SIZE_Z)
	{
		if(chunk->back)
			set_vox(chunk->back, x, y, z - CHUNK_SIZE_Z, type);
		return;
	}
	
	chunk->data[x][y][z] = type;
	chunk->changed |= 0x01;
	
	//set neighbouring chunks to rebuild
	if(x == 0 && chunk->left)
		chunk->left->changed |= 0x01;
	if(x == CHUNK_SIZE_X-1 && chunk->right)
		chunk->right->changed |= 0x01;
	if(y == 0 && chunk->down)
		chunk->down->changed |= 0x01;
	if(y == CHUNK_SIZE_Y-1 && chunk->up)
		chunk->up->changed |= 0x01;
	if(z == 0 && chunk->front)
		chunk->front->changed |= 0x01;
	if(z == CHUNK_SIZE_Z-1 && chunk->back)
		chunk->back->changed |= 0x01;
}

void delete_chunk(Chunk* chunk)
{
	glDeleteBuffers(1, &chunk->vbo);
}

void update_chunk(Chunk* chunk)
{
	//we no longer need to build this chunk
	chunk->changed &= ~0x01;
	
	Byte4 vertex[CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 6 * 6];
	int i = 0;
	
	for(int x = 0; x < CHUNK_SIZE_X; x++)
	{
		for(int y = 0; y < CHUNK_SIZE_Y; y++)
		{
			for(int z = 0; z < CHUNK_SIZE_Z; z++)
			{
				unsigned char side = chunk->data[x][y][z];
				unsigned char top = chunk->data[x][y][z];
				unsigned char bottom = chunk->data[x][y][z];
				
				if(side == VOXEL_AIR)
					continue;
				
				if(top == VOXEL_GRASS)
				{
					side = VOXEL_GRASS_S;
					bottom = VOXEL_DIRT;
				}
				if(side == VOXEL_WOOD)
				{
					top = VOXEL_WOOD_T;
					bottom = VOXEL_WOOD_T;			
				}
				
				//TODO: merge faces
				
				if(z == 0 || (z > 0 && !chunk->data[x][y][z - 1]))
				{
					if(z == 0 && chunk->front && chunk->front->data[x][y][CHUNK_SIZE_Z-1])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//-z
						vertex[i++] = make_byte(x,		y,		z,		side);
						vertex[i++] = make_byte(x,		y+1,	z,		side);
						vertex[i++] = make_byte(x+1,	y,		z,		side);
						vertex[i++] = make_byte(x,		y+1,	z,		side);
						vertex[i++] = make_byte(x+1,	y+1,	z,		side);
						vertex[i++] = make_byte(x+1,	y,		z,		side);
					}
				}
				if(z == CHUNK_SIZE_Z-1 || (z < CHUNK_SIZE_Z && !chunk->data[x][y][z + 1]))
				{
					if(z == CHUNK_SIZE_Z-1 && chunk->back && chunk->back->data[x][y][0])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//+z
						vertex[i++] = make_byte(x,		y,		z+1,	side);
						vertex[i++] = make_byte(x+1,	y,		z+1,	side);
						vertex[i++] = make_byte(x,		y+1,	z+1,	side);
						vertex[i++] = make_byte(x,		y+1,	z+1,	side);
						vertex[i++] = make_byte(x+1,	y,		z+1,	side);
						vertex[i++] = make_byte(x+1,	y+1,	z+1,	side);
					}
				}
				if(x == 0 || (x > 0 && !chunk->data[x - 1][y][z]))
				{
					if(x == 0 && chunk->left && chunk->left->data[CHUNK_SIZE_X-1][y][z])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//-x
						vertex[i++] = make_byte(x,		y,		z,		side);
						vertex[i++] = make_byte(x,		y,		z+1,	side);
						vertex[i++] = make_byte(x,		y+1,	z,		side);
						vertex[i++] = make_byte(x,		y+1,	z,		side);
						vertex[i++] = make_byte(x,		y,		z+1,	side);
						vertex[i++] = make_byte(x,		y+1,	z+1,	side);
					}
				}
				if(x == CHUNK_SIZE_X-1 || (x < CHUNK_SIZE_X && !chunk->data[x + 1][y][z]))
				{
					if(x == CHUNK_SIZE_X-1 && chunk->right && chunk->right->data[0][y][z])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//+x
						vertex[i++] = make_byte(x+1,	y,		z,		side);
						vertex[i++] = make_byte(x+1,	y+1,	z,		side);
						vertex[i++] = make_byte(x+1,	y,		z+1,	side);
						vertex[i++] = make_byte(x+1,	y+1,	z,		side);
						vertex[i++] = make_byte(x+1,	y+1,	z+1,	side);
						vertex[i++] = make_byte(x+1,	y,		z+1,	side);
					}
				}
				if(y == 0 || (y > 0 && !chunk->data[x][y - 1][z]))
				{
					if(y == 0 && chunk->down && chunk->down->data[x][CHUNK_SIZE_Y-1][z])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//-y
						vertex[i++] = make_byte(x,		y,		z,		bottom + 128);
						vertex[i++] = make_byte(x+1,	y,		z,		bottom + 128);
						vertex[i++] = make_byte(x,		y,		z+1,	bottom + 128);
						vertex[i++] = make_byte(x+1,	y,		z,		bottom + 128);
						vertex[i++] = make_byte(x+1,	y,		z+1,	bottom + 128);
						vertex[i++] = make_byte(x,		y,		z+1,	bottom + 128);
					}
				}
				if(y == CHUNK_SIZE_Y-1 || (y < CHUNK_SIZE_Y && !chunk->data[x][y + 1][z]))
				{
					if(y == CHUNK_SIZE_Y-1 && chunk->up && chunk->up->data[x][0][z])
					{ /* hide faces near edges of the chunk */ }
					else
					{
						//+y
						vertex[i++] = make_byte(x,		y+1,	z,		top + 128);
						vertex[i++] = make_byte(x,		y+1,	z+1,	top + 128);
						vertex[i++] = make_byte(x+1,	y+1,	z,		top + 128);
						vertex[i++] = make_byte(x+1,	y+1,	z,		top + 128);
						vertex[i++] = make_byte(x,		y+1,	z+1,	top + 128);
						vertex[i++] = make_byte(x+1,	y+1,	z+1,	top + 128);
					}
				}
			}
		}
	}
	chunk->elements = i;
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
	glBufferData(GL_ARRAY_BUFFER, chunk->elements * sizeof *vertex, vertex, GL_STATIC_DRAW);
}

void rebuild_chunk(Chunk* chunk)
{
	if(chunk->changed & 0x02)
		generate_chunk(chunk);
	if(chunk->changed & 0x01)
		update_chunk(chunk);
}

void render_chunk(Chunk* chunk, GLint coord)
{
	if(chunk->changed & 0x01 || chunk->changed & 0x02)
		rebuild_chunk(chunk);
					
	if(!chunk->elements)
		return;
	
	glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
	glVertexAttribPointer(coord, 4, GL_BYTE, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, chunk->elements);
}


World create_world(void)
{
	World world;
	memset(world.data, 0, sizeof(world.data));

	world.seed = time(NULL);
	psl_init_simplex(world.seed);

	
	for(int x = 0; x < WORLD_SIZE_X; x++)
	{
		for(int y = 0; y < WORLD_SIZE_Y; y++)
		{
			for(int z = 0; z < WORLD_SIZE_Z; z++)
			{
				world.data[x][y][z] = (Chunk*)malloc(sizeof(Chunk));
				create_chunk(world.data[x][y][z], 0, x - WORLD_SIZE_X/2, y - WORLD_SIZE_Y/2, z - WORLD_SIZE_Z/2);
			}
		}
	}
	for(int x = 0; x < WORLD_SIZE_X; x++)
	{
		for(int y = 0; y < WORLD_SIZE_Y; y++)
		{
			for(int z = 0; z < WORLD_SIZE_Z; z++)
			{
				if(x > 0)
					world.data[x][y][z]->left = world.data[x-1][y][z];
				if(y > 0)
					world.data[x][y][z]->down = world.data[x][y-1][z];
				if(z > 0)
					world.data[x][y][z]->front = world.data[x][y][z-1];
				if(x < WORLD_SIZE_X-1)
					world.data[x][y][z]->right = world.data[x+1][y][z];
				if(y < WORLD_SIZE_Y-1)
					world.data[x][y][z]->up = world.data[x][y+1][z];
				if(z < WORLD_SIZE_Z-1)
					world.data[x][y][z]->back = world.data[x][y][z+1];
			}
		}
	}

	return world;
}

void delete_world(World* world)
{
	for(int x = 0; x < WORLD_SIZE_X; x++)
	{
		for(int y = 0; y < WORLD_SIZE_Y; y++)
		{
			for(int z = 0; z < WORLD_SIZE_Z; z++)
			{
				if(world->data[x][y][z])
				{
					delete_chunk(world->data[x][y][z]);
					free(world->data[x][y][z]);
				}
			}
		}
	}
}

void render_world(World* world, float cam[4][4], GLint transform, GLint coord)
{
	for(int x = 0; x < WORLD_SIZE_X; x++)
	{
		for(int y = 0; y < WORLD_SIZE_Y; y++)
		{
			for(int z = 0; z < WORLD_SIZE_Z; z++)
			{
				if(world->data[x][y][z])
				{
					float trans[4][4];
					float pos[3] = 
					{
						(float)world->data[x][y][z]->pos[0] * CHUNK_SIZE_X, (float)world->data[x][y][z]->pos[1] * CHUNK_SIZE_Y, (float)world->data[x][y][z]->pos[2] * CHUNK_SIZE_Z
					};
				
					psl_translate(trans, pos);
					float mvp[4][4];
					psl_multiply(mvp, trans, cam);
/*					float center[4];*/
/*					psl_multiply_vec4(center, mvp, (float)CHUNK_SIZE_X / 2, (float)CHUNK_SIZE_Y / 2, (float)CHUNK_SIZE_Z / 2, 1.0f);*/
/*					*/
/*					center[0] /= center[3];*/
/*					center[1] /= center[3];*/
/*					*/
/*					if(center[2] < -CHUNK_SIZE_Y / 2)*/
/*						continue;*/
/*					*/
/*					if(fabsf(center[0]) > 1 + fabsf(CHUNK_SIZE_Y * 2 / center[3]) || fabsf(center[1]) > 1 + fabsf(CHUNK_SIZE_Y * 2 / center[3]))*/
/*						continue;*/
/*					*/
					glUniformMatrix4fv(transform, 1, GL_FALSE, &mvp[0][0]);
			
					render_chunk(world->data[x][y][z], coord);
				}
			}
		}
	}

}


unsigned char get_voxel(World* world, int x, int y, int z)
{
	int xx = x / CHUNK_SIZE_X;
	int yy = y / CHUNK_SIZE_Y;
	int zz = z / CHUNK_SIZE_Z;
	
	x %= CHUNK_SIZE_X;
	y %= CHUNK_SIZE_Y;
	z %= CHUNK_SIZE_Z;
	
	for(int i = 0; i < WORLD_SIZE_X; i++)
	{
		for(int j = 0; j < WORLD_SIZE_Y; j++)
		{
			for(int k = 0; k < WORLD_SIZE_Z; k++)
			{
				if(world->data[i][j][k] && world->data[i][j][k]->pos[0] == xx && world->data[i][j][k]->pos[1] == yy && world->data[i][j][k]->pos[2] == zz)
				{
					return get_vox(world->data[i][j][k], x, y, z); 
				}
			}
		}
	}
	
	return 128;
}

void set_voxel(World* world, int x, int y, int z, unsigned char type)
{
	int xx = x / CHUNK_SIZE_X;
	int yy = y / CHUNK_SIZE_Y;
	int zz = z / CHUNK_SIZE_Z;
	
	x %= CHUNK_SIZE_X;
	y %= CHUNK_SIZE_Y;
	z %= CHUNK_SIZE_Z;
	
	for(int i = 0; i < WORLD_SIZE_X; i++)
	{
		for(int j = 0; j < WORLD_SIZE_Y; j++)
		{
			for(int k = 0; k < WORLD_SIZE_Z; k++)
			{
				if(world->data[i][j][k] && world->data[i][j][k]->pos[0] == xx && world->data[i][j][k]->pos[1] == yy && world->data[i][j][k]->pos[2] == zz)
				{
					set_vox(world->data[i][j][k], x, y, z, type);
				}
			}
		}
	}
}

Chunk* get_chunk(World* world, int x, int y, int z)
{
	for(int i = 0; i < WORLD_SIZE_X; i++)
	{
		for(int j = 0; j < WORLD_SIZE_Y; j++)
		{
			for(int k = 0; k < WORLD_SIZE_Z; k++)
			{
				if(world->data[i][j][k] && world->data[i][j][k]->pos[0] == x && world->data[i][j][k]->pos[1] == y && world->data[i][j][k]->pos[2] == z)
				{
					return world->data[i][j][k];
				}
			}
		}
	}
	
	return NULL;
}

void scroll_world(World* world, int pos[3], int cx, int cy, int cz)
{
	if(cx != 0)
	{
		int new_chunk = (WORLD_SIZE_X / 2) * cx + pos[0];
		int old_chunk = (WORLD_SIZE_X / 2) * -cx + pos[0];
		
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && world->data[x][y][z]->pos[0] == old_chunk)
					{
						create_chunk(world->data[x][y][z], world->data[x][y][z]->vbo, new_chunk, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
					}
				}
			}
		}
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && (world->data[x][y][z]->pos[0] == new_chunk || world->data[x][y][z]->pos[0] == new_chunk - cx))
					{
						world->data[x][y][z]->left = get_chunk(world, world->data[x][y][z]->pos[0] -1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->right = get_chunk(world, world->data[x][y][z]->pos[0] +1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->up = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] +1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->down = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] -1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->front = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] -1);
						world->data[x][y][z]->back = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] +1);
					}
				}
			}
		}
	}
	else if(cy != 0)
	{
		int new_chunk = (WORLD_SIZE_Y / 2) * cy + pos[1];
		int old_chunk = (WORLD_SIZE_Y / 2) * -cy + pos[1];
		
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && world->data[x][y][z]->pos[1] == old_chunk)
					{
						create_chunk(world->data[x][y][z], world->data[x][y][z]->vbo, world->data[x][y][z]->pos[0], new_chunk, world->data[x][y][z]->pos[2]);
					}
				}
			}
		}
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && (world->data[x][y][z]->pos[1] == new_chunk || world->data[x][y][z]->pos[1] == new_chunk - cy))
					{
						world->data[x][y][z]->left = get_chunk(world, world->data[x][y][z]->pos[0] -1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->right = get_chunk(world, world->data[x][y][z]->pos[0] +1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->up = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] +1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->down = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] -1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->front = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] -1);
						world->data[x][y][z]->back = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] +1);
					}
				}
			}
		}
	}
	else if(cz != 0)
	{
		int new_chunk = (WORLD_SIZE_Z / 2) * cz + pos[2];
		int old_chunk = (WORLD_SIZE_Z / 2) * -cz + pos[2];
		
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && world->data[x][y][z]->pos[2] == old_chunk)
					{
						create_chunk(world->data[x][y][z], world->data[x][y][z]->vbo, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], new_chunk);
					}
				}
			}
		}
		for(int x = 0; x < WORLD_SIZE_X; x++)
		{
			for(int y = 0; y < WORLD_SIZE_Y; y++)
			{
				for(int z = 0; z < WORLD_SIZE_Z; z++)
				{
					if(world->data[x][y][z] && (world->data[x][y][z]->pos[2] == new_chunk || world->data[x][y][z]->pos[2] == new_chunk - cz))
					{
						world->data[x][y][z]->left = get_chunk(world, world->data[x][y][z]->pos[0] -1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->right = get_chunk(world, world->data[x][y][z]->pos[0] +1, world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->up = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] +1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->down = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1] -1, world->data[x][y][z]->pos[2]);
						world->data[x][y][z]->front = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] -1);
						world->data[x][y][z]->back = get_chunk(world, world->data[x][y][z]->pos[0], world->data[x][y][z]->pos[1], world->data[x][y][z]->pos[2] +1);
					}
				}
			}
		}
	}
}
