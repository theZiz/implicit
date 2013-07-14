/*
 The contents of this file are subject to the Mozilla Public License				
 Version 1.1 (the "License"); you may not use this file except in					 
 compliance with the License. You may obtain a copy of the License at			 
 http://www.mozilla.org/MPL/																								
																																						
 Software distributed under the License is distributed on an "AS IS"				
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the		
 License for the specific language governing rights and limitations				 
 under the License.																												 
																																						
 Alternatively, the contents of this file may be used under the terms			 
 of the GNU Lesser General Public license (the	"LGPL License"), in which case the	
 provisions of LGPL License are applicable instead of those									
 above.																																		 
																																						
 For feedback and questions about my Files and Projects please mail me,		 
 Alexander Matthes (Ziz) , zizsdl_at_googlemail.com												 
*/

#include <sparrow3d.h>

#define RESOLUTION spFloatToFixed(0.1f)
#define MIN spFloatToFixed(-2.5f)
#define MAX spFloatToFixed( 2.5f)

SDL_Surface* screen;
Sint32 rotation;
Sint32* raster;
typedef struct {
	Sint32 x,y,z;
} tPoint;
tPoint* rasterGrid;

Sint32 sphere(Sint32 x,Sint32 y,Sint32 z)
{
	return spSqrt(spSquare(x)+spSquare(y)+spSquare(z))-SP_ONE;
}

Sint32 ellipse(Sint32 x,Sint32 y,Sint32 z)
{
	return spSqrt(spSquare(x/2)+spSquare(y*2)+spSquare(z))-SP_ONE;
}

Sint32 cube(Sint32 x,Sint32 y,Sint32 z)
{
	return spMax(abs(x),spMax(abs(y),abs(z)))-SP_ONE;
}

Sint32 function_with_modelview(Sint32* model,Sint32 (*function)(Sint32 x,Sint32 y,Sint32 z),Sint32 x,Sint32 y,Sint32 z)
{
	x = x - model[12];
	y = y - model[13];
	z = z - model[14];
	/*Sint32 det = spMul(model[ 0],spMul(model[ 5],model[10]))
	           + spMul(model[ 4],spMul(model[ 9],model[ 2]))
	           + spMul(model[ 8],spMul(model[ 1],model[ 6]))
	           - spMul(model[ 8],spMul(model[ 5],model[ 2]))
	           - spMul(model[ 4],spMul(model[ 1],model[10]))
	           - spMul(model[ 0],spMul(model[ 9],model[ 6]));*/
	Sint32 det = SP_ONE;
	Sint32 invers[9];
	invers[0] = spMulHigh(model[ 5],model[10]) - spMulHigh(model[ 9],model[ 6]);
	invers[1] = spMulHigh(model[ 9],model[ 2]) - spMulHigh(model[ 1],model[10]);
	invers[2] = spMulHigh(model[ 1],model[ 6]) - spMulHigh(model[ 5],model[ 2]);
	invers[3] = spMulHigh(model[ 8],model[ 6]) - spMulHigh(model[ 4],model[10]);
	invers[4] = spMulHigh(model[ 0],model[10]) - spMulHigh(model[ 8],model[ 2]);
	invers[5] = spMulHigh(model[ 4],model[ 2]) - spMulHigh(model[ 0],model[ 6]);
	invers[6] = spMulHigh(model[ 4],model[ 9]) - spMulHigh(model[ 8],model[ 5]);
	invers[7] = spMulHigh(model[ 8],model[ 1]) - spMulHigh(model[ 0],model[ 9]);
	invers[8] = spMulHigh(model[ 0],model[ 5]) - spMulHigh(model[ 4],model[ 1]);
	Sint32 nx = spMulHigh(invers[0],x) + spMulHigh(invers[3],y) + spMulHigh(invers[6],z);
	Sint32 ny = spMulHigh(invers[1],x) + spMulHigh(invers[4],y) + spMulHigh(invers[7],z);
	Sint32 nz = spMulHigh(invers[2],x) + spMulHigh(invers[5],y) + spMulHigh(invers[8],z);
	return function(nx,ny,nz);
}

#define FUNCTION ellipse
#define VALUE(x,y,z) raster[(x)+(y)*size+(z)*size*size]
#define PIXEL(x,y,z) rasterGrid[(x)+(y)*size+(z)*size*size]

int count_in(Sint32* points)
{
	int i;
	int result = 0;
	for (i = 0; i < 8; i++)
		if (points[i] <= 0)
			result++;
	return result;
}

int get_the_one(Sint32* points)
{
	int i;
	int result = 0;
	for (i = 0; i < 8; i++)
		if (points[i] <= 0)
			return i;
}

int get_the_none(Sint32* points)
{
	int i;
	int result = 0;
	for (i = 0; i < 8; i++)
		if (points[i] > 0)
			return i;
}

void draw_one(Sint32* point,tPoint* position,int the_one,int backwards)
{
	int edge[3];
	switch (the_one)
	{
		case 0:
			edge[0] = 1;
			edge[1] = 4;
			edge[2] = 3;
			break;
		case 1:
			edge[0] = 0;
			edge[1] = 2;
			edge[2] = 5;
			break;
		case 2:
			edge[0] = 1;
			edge[1] = 3;
			edge[2] = 6;
			break;
		case 3:
			edge[0] = 0;
			edge[1] = 7;
			edge[2] = 2;
			break;
		case 4:
			edge[0] = 0;
			edge[1] = 5;
			edge[2] = 7;
			break;
		case 5:
			edge[0] = 1;
			edge[1] = 6;
			edge[2] = 4;
			break;
		case 6:
			edge[0] = 2;
			edge[1] = 7;
			edge[2] = 5;
			break;
		case 7:
			edge[0] = 3;
			edge[1] = 4;
			edge[2] = 6;
			break;
	}
	if (backwards)
	{
		int left = edge[0];
		edge[0] = edge[2];
		edge[2] = left;
	}
	tPoint triangle[3];
	int i;
	for (i = 0; i < 3; i++)
	{
		Sint32 distance = point[edge[i]]-point[the_one];
		triangle[i].x = spDiv(spMul(position[the_one].x,-point[the_one]) + spMul(position[edge[i]].x,point[edge[i]]),distance);
		triangle[i].y = spDiv(spMul(position[the_one].y,-point[the_one]) + spMul(position[edge[i]].y,point[edge[i]]),distance);
		triangle[i].z = spDiv(spMul(position[the_one].z,-point[the_one]) + spMul(position[edge[i]].z,point[edge[i]]),distance);
	}
	spTriangle3D(triangle[0].x,triangle[0].y,triangle[0].z,
	             triangle[1].x,triangle[1].y,triangle[1].z,
	             triangle[2].x,triangle[2].y,triangle[2].z,65535);
}

void draw( void )
{
	spResetZBuffer();
	spIdentity();
	spClearTarget( 12345 );
	//spSetZSet(0);
	//spSetZTest(0);
	
	Sint32 zShift = spFloatToFixed(-3.0f);

	spTranslate(0,0,zShift);
	
	spRotateX(rotation);
	spRotateY(rotation/2);
	spRotateZ(rotation/4);
	
	int size = (MAX-MIN)/RESOLUTION+1;
	
	Sint32 x,y,z;
	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
			for (z = 0; z < size; z++)
			{
				Sint32 X = MIN+x*RESOLUTION;
				Sint32 Y = MIN+y*RESOLUTION;
				Sint32 Z = MIN+z*RESOLUTION+zShift;
				VALUE(x,y,z) = function_with_modelview(spGetMatrix(),FUNCTION,X,Y,Z);
				PIXEL(x,y,z).x = X;
				PIXEL(x,y,z).y = Y;
				PIXEL(x,y,z).z = Z;
				Z-=zShift;
				Sint32 value = FUNCTION(X,Y,Z);
				if (value <= 0)
				{
					Sint32 h = spFixedToInt(-value * 256);
					if (h > 255)
						h = 255;
					h = 255-h;
					//spEllipse3D(X,Y,Z,RESOLUTION/2,RESOLUTION/2,spGetFastRGB(0,h,0));
				}
			}
	spIdentity();
	//spSetBlending(spFloatToFixed(0.5f));
	int countcount[8];
	memset(countcount,0,32);
	for (x = 0; x < size-1; x++)
		for (y = 0; y < size-1; y++)
			for (z = 0; z < size-1; z++)
			{
				Sint32 points[8];
				tPoint position[8];
				points[0] = VALUE(x  ,y  ,z  ); //front left top
				points[1] = VALUE(x+1,y  ,z  ); //front right top
				points[2] = VALUE(x+1,y+1,z  ); //front right bottom
				points[3] = VALUE(x  ,y+1,z  ); //front left top
				points[4] = VALUE(x  ,y  ,z+1); //back left top
				points[5] = VALUE(x+1,y  ,z+1); //back right top
				points[6] = VALUE(x+1,y+1,z+1); //back right bottom
				points[7] = VALUE(x  ,y+1,z+1); //back left top
				position[0] = PIXEL(x  ,y  ,z  ); //front left top
				position[1] = PIXEL(x+1,y  ,z  ); //front right top
				position[2] = PIXEL(x+1,y+1,z  ); //front right bottom
				position[3] = PIXEL(x  ,y+1,z  ); //front left top
				position[4] = PIXEL(x  ,y  ,z+1); //back left top
				position[5] = PIXEL(x+1,y  ,z+1); //back right top
				position[6] = PIXEL(x+1,y+1,z+1); //back right bottom
				position[7] = PIXEL(x  ,y+1,z+1); //back left top
				int count = count_in(points);
				countcount[count]++;
				switch (count)
				{
					case 1: draw_one(points,position,get_the_one(points),0); break;
					case 7: draw_one(points,position,get_the_none(points),1); break;
				}
				if (count > 0 && count < 8)
					spEllipse3D(PIXEL(x,y,z).x,PIXEL(x,y,z).y,PIXEL(x,y,z).z,RESOLUTION/8,RESOLUTION/8,54321);
			}	spSetBlending(SP_ONE);
	int i;
	printf("----- Draw these count so often:\n");
	for (i = 0; i < 8; i++)
	{
		printf("%i: %i\n",i,countcount[i]);
	}
	spFlip();
}

int calc( Uint32 steps )
{
	rotation += steps*8;
	if ( spGetInput()->button[SP_BUTTON_START] )
		return 1;
	return 0;
}

void resize(Uint16 w,Uint16 h)
{
  //Setup of the new/resized window
  spSelectRenderTarget(spGetWindowSurface());
  spSetPerspective(50.0,(float)spGetWindowSurface()->w/(float)spGetWindowSurface()->h,1.0,100);
}

int main(int argc, char **argv)
{
	spSetDefaultWindowSize( 800, 480 );
	spInitCore();
	spSetAffineTextureHack(0); //We don't need it :)

	//Setup
	screen = spCreateDefaultWindow();
	resize(screen->w,screen->h);
	
	int size = (MAX-MIN)/RESOLUTION+1;
	raster = (Sint32*) malloc(sizeof(Sint32)*size*size*size);
	rasterGrid = (tPoint*) malloc(sizeof(tPoint)*size*size*size);
	spSetLight(1);
	spLoop( draw, calc, 10, resize, NULL);
	free(raster);
	free(rasterGrid);
	spQuitCore();
	return 0;
}
