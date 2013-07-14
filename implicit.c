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
#define MIN spFloatToFixed(-2.0f)
#define MAX spFloatToFixed( 2.0f)

SDL_Surface* screen;
Sint32 rotation;

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
	invers[0] = spMul(model[ 5],model[10]) - spMul(model[ 9],model[ 6]);
	invers[1] = spMul(model[ 9],model[ 2]) - spMul(model[ 1],model[10]);
	invers[2] = spMul(model[ 1],model[ 6]) - spMul(model[ 5],model[ 2]);
	invers[3] = spMul(model[ 8],model[ 6]) - spMul(model[ 4],model[10]);
	invers[4] = spMul(model[ 0],model[10]) - spMul(model[ 8],model[ 2]);
	invers[5] = spMul(model[ 4],model[ 2]) - spMul(model[ 0],model[ 6]);
	invers[6] = spMul(model[ 4],model[ 9]) - spMul(model[ 8],model[ 5]);
	invers[7] = spMul(model[ 8],model[ 1]) - spMul(model[ 0],model[ 9]);
	invers[8] = spMul(model[ 0],model[ 5]) - spMul(model[ 4],model[ 1]);
	Sint32 nx = spMul(invers[0],x) + spMul(invers[3],y) + spMul(invers[6],z);
	Sint32 ny = spMul(invers[1],x) + spMul(invers[4],y) + spMul(invers[7],z);
	Sint32 nz = spMul(invers[2],x) + spMul(invers[5],y) + spMul(invers[8],z);
	return function(nx,ny,nz);
}

void draw( void )
{
	spResetZBuffer();
	spIdentity();
	spClearTarget( 12345 );
	spSetZSet(0);
	spSetZTest(0);
	Sint32 Z = spFloatToFixed(-3.0f);
	spTranslate(0,0,Z);
	
	spRotateX(rotation);
	spRotateY(rotation/2);
	spRotateZ(rotation/4);
	
	//Marching cubes!
	Sint32 x,y,z;
	spSetBlending(spFloatToFixed(0.5f));
	for (x = MIN; x <= MAX; x+=RESOLUTION)
		for (y = MIN; y <= MAX; y+=RESOLUTION)
			for (z = MIN; z <= MAX; z+=RESOLUTION)
			{
				Sint32 value = ellipse(x,y,z);
				if (value <= 0)
				{
					Sint32 h = spFixedToInt(-value * 256);
					if (h > 255)
						h = 255;
					h = 255-h;
					spEllipse3D(x,y,z,RESOLUTION/2,RESOLUTION/2,spGetFastRGB(h,0,0));
				}
			}
	Sint32 matrix[16];
	memcpy( matrix, spGetMatrix(), 16 * sizeof( Sint32 ) ); //Saving the matrix
	spIdentity();
	for (x = MIN; x <= MAX; x+=RESOLUTION)
		for (y = MIN; y <= MAX; y+=RESOLUTION)
			for (z = MIN+Z; z <= MAX+Z; z+=RESOLUTION)
			{
				Sint32 value = function_with_modelview(matrix,ellipse,x,y,z);
				if (value <= 0)
				{
					Sint32 h = spFixedToInt(-value * 256);
					if (h > 255)
						h = 255;
					h = 255-h;
					spEllipse3D(x,y,z,RESOLUTION/2,RESOLUTION/2,spGetFastRGB(0,h,0));
				}
			}	spSetBlending(SP_ONE);
	spFlip();
}

int calc( Uint32 steps )
{
	rotation += steps*32;
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
	
	spLoop( draw, calc, 10, resize, NULL);
	
	spQuitCore();
	return 0;
}
