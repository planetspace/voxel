#include"texture.h"
#include"../deps/stb_image/stb_image.h"

#include<stdio.h>


GLuint texture_atlas;
GLuint font_atlas;


void load_texture_atlas(const char* name)
{
	int width, height, comp;
	
	unsigned char* image = stbi_load(name, &width, &height, &comp, STBI_rgb_alpha);
	
	if(image == NULL)
		printf("texture: %s could not be loaded!\n", name);
		
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_atlas);
	glBindTexture(GL_TEXTURE_2D, texture_atlas);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	stbi_image_free(image);
}
