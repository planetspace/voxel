#ifndef CHUNK_H
#define CHUNK_H

#define CHUNK_SIZE_X 32
#define CHUNK_SIZE_Y 32
#define CHUNK_SIZE_Z 32

#define WORLD_SIZE_X 16
#define WORLD_SIZE_Y 16
#define WORLD_SIZE_Z 16

//always an even number
#define VIEW_DISTANCE 8

#include<GL/glew.h>
#include<time.h>

enum
{
	VOXEL_AIR = 0,
	VOXEL_STONE,
	VOXEL_DIRT,
	VOXEL_GRASS,
	VOXEL_GRASS_S,
	VOXEL_SAND,
	VOXEL_WOOD,
	VOXEL_WOOD_T,
	VOXEL_LEAVES,
	VOXEL_GEM,
	VOXEL_DIAMOND,
	VOXEL_WATER,
	VOXEL_NULL = 128
};

typedef struct
{
	GLbyte x, y, z, w;
} Byte4;

Byte4 make_byte(GLbyte x, GLbyte y, GLbyte z, GLbyte w);

typedef struct chunk
{
	unsigned char data[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
	struct chunk *left, *right, *up, *down, *front, *back; 
	GLuint vbo;
	int elements;
	char changed;
	int pos[3];
} Chunk;

//chunk functions
void create_chunk(Chunk* chunk, unsigned int buffer, int x, int y, int z);
void rebuild_chunk(Chunk* chunk);
void generate_chunk(Chunk* chunk);
void set_vox(Chunk* chunk, int x, int y, int z, int type);
void delete_chunk(Chunk* chunk);
void render_chunk(Chunk* chunk, GLint coord);

typedef struct
{
	Chunk* data[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];
	time_t seed;
} World;

//world functions
World create_world(void);
void delete_world(World* world);
void render_world(World* world, float cam[4][4], GLint transform, GLint coord);

unsigned char get_voxel(World* world, int x, int y, int z);
void set_voxel(World* world, int x, int y, int z, unsigned char type);

Chunk* get_chunk(World* world, int x, int y, int z);
void scroll_world(World* world, int pos[3], int cx, int cy, int cz);

#endif //CHUNK_H
