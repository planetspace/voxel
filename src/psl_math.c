#include"psl_lib.h"
#include<stdio.h>
#include<math.h>

//simplex
//http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
int perm[512];

int grad3[12][3] = 
{
	{ 1, 1, 0 }, 	{ -1, 1, 0 }, 	{ 1, -1, 0 },	{ -1, -1, 0 }, 	
	{ 1, 0, 1 }, 	{ -1, 0, 1 },	{ 1, 0, -1 }, 	{ -1, 0, -1 },
	{ 0, 1, 1 }, 	{ 0, -1, 1 }, 	{ 0, 1, -1 },	{ 0, -1, -1 } 
};

int grad4[32][4] =
{
	{0,1,1,1},	{0,1,1,-1}, 	{0,1,-1,1}, 	{0,1,-1,-1},
	{0,-1,1,1}, {0,-1,1,-1}, 	{0,-1,-1,1}, 	{0,-1,-1,-1},
	{1,0,1,1}, 	{1,0,1,-1}, 	{1,0,-1,1}, 	{1,0,-1,-1},
	{-1,0,1,1}, {-1,0,1,-1}, 	{-1,0,-1,1}, 	{-1,0,-1,-1},
	{1,1,0,1}, 	{1,1,0,-1}, 	{1,-1,0,1}, 	{1,-1,0,-1},
	{-1,1,0,1}, {-1,1,0,-1}, 	{-1,-1,0,1}, 	{-1,-1,0,-1},
	{1,1,1,0}, 	{1,1,-1,0}, 	{1,-1,1,0}, 	{1,-1,-1,0},
	{-1,1,1,0}, {-1,1,-1,0}, 	{-1,-1,1,0}, 	{-1,-1,-1,0}
};

int p[] = 
{
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
	8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,
	117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,
	165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
	105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,
	208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,
	217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,
	58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,
	155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
	218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,
	14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,
	254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

void psl_init_simplex(int seed)
{
	srand(seed);
	for(int i = 0; i < 512; i++)
	{
		perm[i] = p[i & 255];
	}
}

int simplex[64][4] = 
{
	{0,1,2,3},	{0,1,3,2},	{0,0,0,0},	{0,2,3,1},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{1,2,3,0},
	{0,2,1,3},	{0,0,0,0},	{0,3,1,2},	{0,3,2,1},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{1,3,2,0},
	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},
	{1,2,0,3},	{0,0,0,0},	{1,3,0,2},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{2,3,0,1},	{2,3,1,0},
	{1,0,2,3},	{1,0,3,2},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{2,0,3,1},	{0,0,0,0},	{2,1,3,0},
	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},
	{2,0,1,3},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{3,0,1,2},	{3,0,2,1},	{0,0,0,0},	{3,1,2,0},
	{2,1,0,3},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{3,1,0,2},	{0,0,0,0},	{3,2,0,1},	{3,2,1,0}
};

double psl_simplex_2d(double x, double y)
{
	double n0, n1, n2;
	
	const double f2 = 0.5*(sqrt(3.0)-1.0);
	double s = (x+y)*f2;
	int i = psl_floor(x+s);
	int j = psl_floor(y+s);
	
	const double g2 = (3.0-sqrt(3.0))/6.0;
	double t = (i+j)*g2;
	double x0 = x - (i-t);
	double y0 = y - (j-t);
	
	int i1, j1;
	if(x0 > y0)	{	i1 = 1;	j1 = 0;	}
	else		{	i1 = 0;	j1 = 1;	}
	
	double x1 = x0 - i1 + g2;
	double y1 = y0 - j1 + g2;
	double x2 = x0 - 1.0 + 2.0 * g2;
	double y2 = y0 - 1.0 + 2.0 * g2;
	
	int ii = i & 255;
	int jj = j & 255;
	
	int gi0 = perm[ii+perm[jj]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1]] % 12;
	int gi2 = perm[ii+1+perm[jj+1]] % 12;
	
	double t0 = 0.5 - x0 * x0 - y0 * y0;
	if(t0 < 0)
		n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * psl_dot_2d(grad3[gi0], x0, y0);
	}
	
	double t1 = 0.5 - x1 * x1 - y1 * y1;
	if(t1 < 0)
		n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * psl_dot_2d(grad3[gi1], x1, y1);
	}
	
	double t2 = 0.5 - x2 * x2 - y2 * y2;
	if(t2 < 0)
		n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * psl_dot_2d(grad3[gi2], x2, y2);
	}
	
	return 70.0 * (n0 + n1 + n2);
}

double psl_simplex_3d(double x, double y, double z)
{
	double n0, n1, n2, n3;
	
	const double f3 = 1.0/3.0;
	double s = (x + y + z) * f3;
	int i = psl_floor(x + s);
	int j = psl_floor(y + s);
	int k = psl_floor(z + s);
	
	const double g3 = 1.0/6.0;
	double t = (i + j + k) * g3;
	double x0 = x - (i - t);
	double y0 = y - (j - t);
	double z0 = z - (k - t);
	
	int i1, j1, k1;
	int i2, j2, k2;
	
	if(x0 >= y0)
	{
		if(y0 >= z0)		{ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1, k2 = 0; }
		else if(x0 >= z0)	{ i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
		else				{ i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
	}
	else
	{
		if(y0 < z0)			{ i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
		else if(x0 < z0)	{ i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
		else				{ i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
	}
	
	double x1 = x0 - i1 + g3;
	double y1 = y0 - j1 + g3;
	double z1 = z0 - k1 + g3;
	double x2 = x0 - i2 + 2.0 * g3;
	double y2 = y0 - j2 + 2.0 * g3;
	double z2 = z0 - k2 + 2.0 * g3;
	double x3 = x0 - 1.0 + 3.0 * g3;
	double y3 = y0 - 1.0 + 3.0 * g3;
	double z3 = z0 - 1.0 + 3.0 * g3;
	
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	
	int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
	int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
	int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
	int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;
	
	double t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0;
	if(t0 < 0)
		n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * psl_dot_3d(grad3[gi0], x0, y0, z0);	
	}
	
	double t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
	if(t1 < 0)
		n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * psl_dot_3d(grad3[gi1], x1, y1, z1);	
	}
	
	double t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
	if(t2 < 0)
		n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * psl_dot_3d(grad3[gi2], x2, y2, z2);
	}
	
	double t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
	if(t3 < 0)
		n3 = 0.0;
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * psl_dot_3d(grad3[gi3], x3, y3, z3);	
	}
	
	return 32.0 * (n0 + n1 + n2 + n3);
}

double psl_simplex_4d(double x, double y, double z, double w)
{
	//TODO: implement this: //http://webstaff.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
	return x + y + z + w;
}

//perlin

void psl_seed_perlin(int seed)
{
	srand(seed);
	int iter = rand() % 255;
	int permutation[] = 
	{
		151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 			252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 			122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 			159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 			183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 			228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 			127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
	};
 
	for(int i = 0; i < 256; i++)
	{
		perm[256 + i] = perm[i] = permutation[i];
	}
	for(int j = 0; j < iter; j++)
	{
		int place = rand() % 512;
		int temp = perm[place];
		perm[place] = perm[j];
		perm[j] = temp;
	}
}

float psl_grad(int hash, float x, float y, float z)
{
	switch(hash & 0xF)
	{
		case 0x0: 	return  x + y;
		case 0x1:	return -x + y;
		case 0x2:	return  x - y;
		case 0x3:	return -x - y;
		case 0x4:	return  x + x;
		case 0x5:	return -x + x;
		case 0x6:	return  x - x;
		case 0x7:	return -x - x;
		case 0x8:	return  y + x;
		case 0x9:	return -y + x;
		case 0xA:	return  y - x;
		case 0xB:	return -y - x;
		case 0xC:	return  y + z;
		case 0xD:	return -y + x;
		case 0xE:	return  y - x;
		case 0xF:	return -y - z;
		default:	return 0;
	}
}

float psl_noise(float x, float y, float z)
{
	long ix, iy, iz, gx, gy, gz;
	long a0, b0, aa, ab, ba, bb;
	
	float aa0, ab0, ba0, bb0;
	float aa1, ab1, ba1, bb1;
	float a1, a2, a3, a4, a5, a6, a7, a8;
	float u, v, w, a8_5, a4_1;
	
	ix = (long)x;	x-=ix;
	iy = (long)y;	y-=iy;
	iz = (long)z;	z-=iz;
	
	gx = ix & 0xFF;
	gy = iy & 0xFF;
	gz = iz & 0xFF;
	
	a0 = gy + perm[gx];
	b0 = gy + perm[gx+1];
	aa = gz + perm[a0];
	ab = gz + perm[a0+1];
	ba = gz + perm[b0];
	bb = gz + perm[b0+1];
	
	aa0 = perm[aa]; aa1 = perm[aa+1];
	ab0 = perm[ab]; ab1 = perm[ab+1];
	ba0 = perm[ba]; ba1 = perm[ba+1];
	bb0 = perm[bb]; bb1 = perm[bb+1];
	
	a1 = psl_grad(bb1, x-1, y-1, z-1);
	a2 = psl_grad(ab1, x  , y-1, z-1);
	a3 = psl_grad(ba1, x-1, y  , z-1);
	a4 = psl_grad(aa1, x  , y  , z-1);
	a5 = psl_grad(bb0, x-1, y-1, z  );
	a6 = psl_grad(ab0, x  , y-1, z  );
	a7 = psl_grad(ba0, x-1, y  , z  );
	a8 = psl_grad(aa0, x  , y  , z  );
	
	u = psl_fade(x);
	v = psl_fade(y);
	w = psl_fade(z);
	
	a8_5 = psl_lerp(v, psl_lerp(u, a8, a7), psl_lerp(u, a6, a5));
	a4_1 = psl_lerp(v, psl_lerp(u, a4, a3), psl_lerp(u, a2, a1));
	return psl_lerp(w, a8_5, a4_1);
}

float psl_noise2d(float x, float y, int octaves, float persistence)
{
	float sum = 0.0f;
	float strength = 1.0f;
	float scale = 1.0f;
	for(int i = 0; i < octaves; i++)
	{
		sum += strength * psl_simplex_2d(x * scale, y * scale);
		scale *= 2.0;
		strength *= persistence;
	}
	return sum;
}

float psl_noise3d_abs(float x, float y, float z, int octaves, float persistence)
{
	float sum = 0.0f;
	float strength = 1.0f;
	float scale = 1.0f;
	
	for(int i = 0; i < octaves; i++)
	{
		sum += strength * fabs(psl_simplex_3d(x * scale, y * scale, z * scale));
		scale *= 2.0;
		strength *= persistence;
	}
	return sum;
}

float psl_normalize_f(float a, float min, float max)	{	return a - min / max - min;	}
float psl_radians(float a)	{	return (PSL_PI * a) / 180.0f;	}
float psl_degrees(float a)	{	return (180.0f * a) / PSL_PI;	}
float psl_fade(float t)		{	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);	}
float psl_lerp(float t, float a, float b)	{	return a + t * (b - a);	}
double psl_floor(double x) 	{	return x > 0 ? (int)x : (int)x-1;	}

float psl_length(float a[3])		{	return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);	}
float psl_length_vec4(float a[4])	{	return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2] + a[3] * a[3]);	}

void psl_normalize(float out[3])
{
	float length = sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
	out[0] /= length;
	out[1] /=length;
	out[2] /= length;
}

void psl_cross(float out[3], float a[3], float b[3])
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}

float psl_dot(float a[3], float b[3])		{	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];		}
float psl_dot_inv(float a[3], float b[3])	{	return a[0] * -b[0] + a[1] * -b[1] + a[2] * -b[2];	}

double psl_dot_2d(int a[], double x, double y)						{	return a[0] * x + a[1] * y;	}
double psl_dot_3d(int a[], double x, double y, double z)			{	return a[0] * x + a[1] * y + a[2] * z;	}
double psl_dot_4d(int a[], double x, double y, double z, double w)	{	return a[0] * x + a[1] * y + a[2] * z + a[3] * w;	}

void psl_identity(float out[4][4])
{
	out[0][0] = 1.0f;	out[1][0] = 0.0f;	out[2][0] = 0.0f;	out[3][0] = 0.0f;
	out[0][1] = 0.0f;	out[1][1] = 1.0f;	out[2][1] = 0.0f;	out[3][1] = 0.0f;
	out[0][2] = 0.0f;	out[1][2] = 0.0f;	out[2][2] = 1.0f;	out[3][2] = 0.0f;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}

void psl_print_matrix(float a[4][4])
{
	printf("%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n",
				a[0][0], a[0][1], a[0][2], a[0][3],
				a[1][0], a[1][1], a[1][2], a[1][3],
				a[2][0], a[2][1], a[2][2], a[2][3],
				a[3][0], a[3][1], a[3][2], a[3][3]);
}

void psl_multiply(float out[4][4], float a[4][4], float b[4][4])
{
	for(unsigned int i = 0; i < 4; i++)
	{
		for(unsigned int j = 0; j < 4; j++)
		{
			out[i][j] = 0.0f;
			for(unsigned int k = 0; k < 4; k++)
			{
				out[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

void psl_multiply_vec4(float out[4], float a[4][4], float x, float y, float z, float w)
{
	out[0] = a[0][0] * x + a[1][0] * y + a[2][0] * z + a[3][0] * w;
	out[1] = a[0][1] * x + a[1][1] * y + a[2][1] * z + a[3][1] * w;
	out[2] = a[0][2] * x + a[1][2] * y + a[2][2] * z + a[3][2] * w;
	out[3] = a[0][3] * x + a[1][3] * y + a[2][3] * z + a[3][3] * w;
}

void psl_perspective(float out[4][4], float fov, float aspect, float z_near, float z_far)
{
	float range = z_far - z_near;
	float tanfov = tan(fov/2.0f);
	
	out[0][0] = 1.0f/(tanfov*aspect);	out[0][1] = 0.0f;			out[0][2] = 0.0f;						out[0][3] = 0.0f;
	out[1][0] = 0.0f;					out[1][1] = 1.0f/tanfov;	out[1][2] = 0.0f;						out[1][3] = 0.0f;
	out[2][0] = 0.0f;					out[2][1] = 0.0f;			out[2][2] = -(z_far+z_near)/range;		out[2][3] = -1.0f;
	out[3][0] = 0.0f;					out[3][1] = 0.0f;			out[3][2] = -(2.0f*z_far*z_near)/range;	out[3][3] = 0.0f;
}

void psl_look_at(float out[4][4], float pos[3], float target[3])
{
	float z_axis[3];
	z_axis[0] = target[0] - pos[0];
	z_axis[1] = target[1] - pos[1];
	z_axis[2] = target[2] - pos[2];
	psl_normalize(z_axis);
	
	float x_axis[3];
	float up_vector[3] = { 0.0f, 1.0f, 0.0f };
	psl_cross(x_axis, up_vector, z_axis);
	psl_normalize(x_axis);
	
	float y_axis[3];
	psl_cross(y_axis, z_axis, x_axis);
	psl_normalize(y_axis);
	
	out[0][0] = x_axis[0];	out[1][0] = x_axis[1];	out[2][0] = x_axis[2];	out[3][0] = psl_dot_inv(x_axis, pos);
	out[0][1] = y_axis[0];	out[1][1] = y_axis[1];	out[2][1] = y_axis[2];	out[3][1] = psl_dot_inv(y_axis, pos);
	out[0][2] = z_axis[0];	out[1][2] = z_axis[1];	out[2][2] = z_axis[2];	out[3][2] = psl_dot_inv(z_axis, pos);
	out[0][3] = 0.0f;		out[1][3] = 0.0f;		out[2][3] = 0.0f;		out[3][3] = 1.0f;
}

void psl_translate(float out[4][4], float vec[3])
{
	out[0][0] = 1.0f;	out[1][0] = 0.0f;	out[2][0] = 0.0f;	out[3][0] = vec[0];
	out[0][1] = 0.0f;	out[1][1] = 1.0f;	out[2][1] = 0.0f;	out[3][1] = vec[1];
	out[0][2] = 0.0f;	out[1][2] = 0.0f;	out[2][2] = 1.0f;	out[3][2] = vec[2];
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
	
}

void psl_scale(float out[4][4], float vec[3])
{
	out[0][0] = vec[0];	out[1][0] = 0.0f;	out[2][0] = 0.0f;	out[3][0] = 0.0f;
	out[0][1] = 0.0f;	out[1][1] = vec[1];	out[2][1] = 0.0f;	out[3][1] = 0.0f;
	out[0][2] = 0.0f;	out[1][2] = 0.0f;	out[2][2] = vec[2];	out[3][2] = 0.0f;
	out[0][3] = 0.0f;	out[1][3] = 0.0f;	out[2][3] = 0.0f;	out[3][3] = 1.0f;
}

void psl_rotate(float out[4][4], float vec[3])
{
	float x[4][4];
	x[0][0] = 1.0f;	x[1][0] = 0.0f;			x[2][0] = 0.0f;			x[3][0] = 0.0f;
	x[0][1] = 0.0f;	x[1][1] = cos(vec[0]);	x[2][1] = -sin(vec[0]);	x[3][1] = 0.0f;
	x[0][2] = 0.0f;	x[1][2] = sin(vec[0]);	x[2][2] = cos(vec[0]);	x[3][2] = 0.0f;
	x[0][3] = 0.0f;	x[1][3] = 0.0f;			x[2][3] = 0.0f;			x[3][3] = 1.0f;
	float y[4][4];
	y[0][0] = cos(vec[1]);	y[1][0] = 0.0f;	y[2][0] = -sin(vec[1]);	y[3][0] = 0.0f;
	y[0][1] = 0.0f;			y[1][1] = 1.0f;	y[2][1] = 0.0f;			y[3][1] = 0.0f;
	y[0][2] = sin(vec[1]);	y[1][2] = 0.0f;	y[2][2] = cos(vec[1]);	y[3][2] = 0.0f;
	y[0][3] = 0.0f;			y[1][3] = 0.0f;	y[2][3] = 0.0f;			y[3][3] = 1.0f;
	float z[4][4];
	z[0][0] = cos(vec[2]);	z[1][0] = -sin(vec[2]);	z[2][0] = 0.0f;	z[3][0] = 0.0f;
	z[0][1] = sin(vec[2]);	z[1][1] = cos(vec[0]);	z[2][1] = 0.0f;	z[3][1] = 0.0f;
	z[0][2] = 0.0f;			z[1][2] = 0.0f;			z[2][2] = 1.0f;	z[3][2] = 0.0f;
	z[0][3] = 0.0f;			z[1][3] = 0.0f;			z[2][3] = 0.0f;	z[3][3] = 1.0f;
	float temp[4][4];
	psl_multiply(temp, x, y);
	psl_multiply(out, temp, z);
}
