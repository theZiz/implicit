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
#define COLOR spGetRGB(255,255,0)
#define CORRECTION 0
#define FUNCTION ellipse
#define VALUE(x,y,z) raster[(x)+(y)*size+(z)*size*size]
#define PIXEL(x,y,z) rasterGrid[(x)+(y)*size+(z)*size*size]
#define FIRST_TRANSFORMATION

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


void draw_spTriangle3D(Sint32 x1,Sint32 y1,Sint32 z1,
	Sint32 x2,Sint32 y2,Sint32 z2,
	Sint32 x3,Sint32 y3,Sint32 z3,Uint16 color,int backwards)
{
	//if (!backwards)
		spTriangle3D(x1,y1,z1,x2,y2,z2,x3,y3,z3,color);
	//else
		spTriangle3D(x3,y3,z3,x2,y2,z2,x1,y1,z1,color);
}

void draw_spQuad3D(Sint32 x1,Sint32 y1,Sint32 z1,
	Sint32 x2,Sint32 y2,Sint32 z2,
	Sint32 x3,Sint32 y3,Sint32 z3,
	Sint32 x4,Sint32 y4,Sint32 z4,Uint16 color,int backwards)
{
	//if (!backwards)
		spQuad3D(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,color);
	//else
		spQuad3D(x4,y4,z4,x3,y3,z3,x2,y2,z2,x1,y1,z1,color);
}


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

void get_edges(int *edge,int the_one)
{
	switch (the_one)
	{
		case 0:
			edge[0] = 1;
			edge[1] = 3;
			edge[2] = 4;
			break;
		case 1:
			edge[0] = 0;
			edge[1] = 5;
			edge[2] = 2;
			break;
		case 2:
			edge[0] = 1;
			edge[1] = 6;
			edge[2] = 3;
			break;
		case 3:
			edge[0] = 0;
			edge[1] = 2;
			edge[2] = 7;
			break;
		case 4:
			edge[0] = 0;
			edge[1] = 7;
			edge[2] = 5;
			break;
		case 5:
			edge[0] = 1;
			edge[1] = 4;
			edge[2] = 6;
			break;
		case 6:
			edge[0] = 2;
			edge[1] = 5;
			edge[2] = 7;
			break;
		case 7:
			edge[0] = 3;
			edge[1] = 6;
			edge[2] = 4;
			break;
	}
}

int add_point(tPoint* point,tPoint in,tPoint out,Sint32 in_value,Sint32 out_value,int correction)
{
	Sint32 distance = out_value-in_value;
	if (distance == 0)
		return 1;
	Sint32 factor = spDiv(-in_value,distance);
	if (factor < 0 || factor > SP_ONE)
		return 1;
	point->x = spMul(in.x,factor) + spMul(out.x,SP_ONE-factor);
	point->y = spMul(in.y,factor) + spMul(out.y,SP_ONE-factor);
	point->z = spMul(in.z,factor) + spMul(out.z,SP_ONE-factor);
	if (correction >= 0)
	{
		Sint32 value = FUNCTION(point->x,point->y,point->z);
		//printf("%i: %i %i %i\n",correction,in_value,value,out_value);
		if ((value < 0 && in_value < 0) || (value > 0 && in_value > 0))
			return add_point(point,*point,out,value,out_value,correction-1);
		else
			return add_point(point,in,*point,in_value,value,correction-1);
	}		
	return 0;
}

void draw_one(Sint32* point,tPoint* position,int the_one,int backwards)
{
	int edge[3];
	get_edges(edge,the_one);
	tPoint triangle[3];
	int i;
	for (i = 0; i < 3; i++)
		if (add_point(&(triangle[i]),position[the_one],position[edge[i]],point[the_one],point[edge[i]],CORRECTION))
			return;
	draw_spTriangle3D(triangle[0].x,triangle[0].y,triangle[0].z,
					 triangle[1].x,triangle[1].y,triangle[1].z,
					 triangle[2].x,triangle[2].y,triangle[2].z,COLOR,backwards);
}

const int x_plus[8] = {0,1,1,0,0,1,1,0};
const int y_plus[8] = {0,0,1,1,0,0,1,1};
const int z_plus[8] = {0,0,0,0,1,1,1,1};
const int nr_lookup[2][2][2] = {{{0,4},{3,7}},{{1,5},{2,6}}};

int get_distance(int one,int two)
{
	int count;
	if (x_plus[one] != x_plus[two])
		count++;
	if (y_plus[one] != y_plus[two])
		count++;
	if (z_plus[one] != z_plus[two])
		count++;
	return count;
}

void draw_two_near(Sint32* point,tPoint* position,int one,int two,int backwards)
{
	int edge_one[3];
	get_edges(edge_one,one);
	int edge_two[3];
	get_edges(edge_two,two);
	//Searching one in edge_one & two in edge_two
	int pos_one;
	for (pos_one = 0; pos_one < 3; pos_one++)
		if (edge_two[pos_one] == one)
			break;
	int pos_two;
	for (pos_two = 0; pos_two < 3; pos_two++)
		if (edge_one[pos_two] == two)
			break;
	tPoint quad[4];
	//0
	int pos = pos_two+1;
	if (pos >= 3)
		pos = 0;
	if (add_point(&(quad[0]),position[one],position[edge_one[pos]],point[one],point[edge_one[pos]],CORRECTION))
		return;
	//1
	pos++;
	if (pos >= 3)
		pos = 0;
	if (add_point(&(quad[1]),position[one],position[edge_one[pos]],point[one],point[edge_one[pos]],CORRECTION))
		return;
	//2
	pos = pos_one+1;
	if (pos >= 3)
		pos = 0;
	if (add_point(&(quad[2]),position[two],position[edge_two[pos]],point[two],point[edge_two[pos]],CORRECTION))
		return;
	//3
	pos++;
	if (pos >= 3)
		pos = 0;
	if (add_point(&(quad[3]),position[two],position[edge_two[pos]],point[two],point[edge_two[pos]],CORRECTION))
		return;
	
	draw_spQuad3D(quad[0].x,quad[0].y,quad[0].z,
		 		  quad[1].x,quad[1].y,quad[1].z,
				  quad[2].x,quad[2].y,quad[2].z,
				  quad[3].x,quad[3].y,quad[3].z,COLOR,backwards);	
}

void draw_two(Sint32* point,tPoint* position,int backwards)
{
	//Finding the two:
	int i;
	int one = -1;
	int two;
	if (!backwards)
	{
		for (i = 0; i < 8; i++)
			if (point[i] <= 0)
			{
				if (one == -1)
					one = i;
				else
					two = i;
			}
	}
	else
	{
		for (i = 0; i < 8; i++)
			if (point[i] > 0)
			{
				if (one == -1)
					one = i;
				else
					two = i;
			}
	}
	//Getting the distance
	int distance = get_distance(one,two);
	switch (distance)
	{
		case 2: case 3:
			draw_one(point,position,one,backwards);
			draw_one(point,position,two,backwards);
			break;
		case 1:
			draw_two_near(point,position,one,two,backwards);
			break;
	}
}

void draw_three_near(Sint32* point,tPoint* position,int one,int two,int three,int backwards)
{
	int edge_one[3];
	get_edges(edge_one,one);
	int edge_two[3];
	get_edges(edge_two,two);
	int edge_three[3];
	get_edges(edge_three,three);
	//Drawing the three triangles.
	//Seaching the points of 1 and 3:
	int common = -1;
	int above_one = -1;
	int above_two = -1;
	int above_three = -1;
	int i,j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			if (edge_one[i] == edge_three[j] && edge_one[i] != two)
			{
				common = edge_one[i];
				break;
			}
	for (i = 0; i < 3; i++)
		if (edge_one[i] != common && edge_one[i] != two)
		{
			above_one = edge_one[i];
			break;
		}
	for (i = 0; i < 3; i++)
		if (edge_two[i] != one && edge_two[i] != three)
		{
			above_two = edge_two[i];
			break;
		}
	for (i = 0; i < 3; i++)
		if (edge_three[i] != common && edge_three[i] != two)
		{
			above_three = edge_three[i];
			break;
		}
	tPoint triangle[3];
	int do_not_draw = 0;
	do_not_draw |= add_point(&(triangle[2]),position[  one],position[     common],point[  one],point[     common],CORRECTION);
	do_not_draw |= add_point(&(triangle[1]),position[three],position[above_three],point[three],point[above_three],CORRECTION);
	do_not_draw |= add_point(&(triangle[0]),position[three],position[     common],point[three],point[     common],CORRECTION);
	if (!do_not_draw)
		draw_spTriangle3D(triangle[0].x,triangle[0].y,triangle[0].z,
					      triangle[1].x,triangle[1].y,triangle[1].z,
					      triangle[2].x,triangle[2].y,triangle[2].z,COLOR,backwards);
	do_not_draw = 0;
	do_not_draw |= add_point(&(triangle[2]),position[  one],position[     common],point[  one],point[     common],CORRECTION);
	do_not_draw |= add_point(&(triangle[1]),position[  one],position[  above_one],point[  one],point[  above_one],CORRECTION);
	do_not_draw |= add_point(&(triangle[0]),position[three],position[above_three],point[three],point[above_three],CORRECTION);
	if (!do_not_draw)
		draw_spTriangle3D(triangle[0].x,triangle[0].y,triangle[0].z,
					      triangle[1].x,triangle[1].y,triangle[1].z,
					      triangle[2].x,triangle[2].y,triangle[2].z,COLOR,backwards);
	do_not_draw = 0;
	do_not_draw |= add_point(&(triangle[2]),position[  one],position[  above_one],point[  one],point[  above_one],CORRECTION);
	do_not_draw |= add_point(&(triangle[1]),position[  two],position[  above_two],point[  two],point[  above_two],CORRECTION);
	do_not_draw |= add_point(&(triangle[0]),position[three],position[above_three],point[three],point[above_three],CORRECTION);
	if (!do_not_draw)
		draw_spTriangle3D(triangle[0].x,triangle[0].y,triangle[0].z,
					      triangle[1].x,triangle[1].y,triangle[1].z,
					      triangle[2].x,triangle[2].y,triangle[2].z,COLOR,backwards);
}

void draw_three(Sint32* point,tPoint* position,int backwards)
{
	//Finding the three:
	int i;
	int one = -1;
	int two = -1;
	int three;
	if (!backwards)
	{
		for (i = 0; i < 8; i++)
			if (point[i] <= 0)
			{
				if (one == -1)
					one = i;
				else
				if (two == -1)
					two = i;
				else
					three = i;
			}
	}
	else
	{
		for (i = 0; i < 8; i++)
			if (point[i] > 0)
			{
				if (one == -1)
					one = i;
				else
				if (two == -1)
					two = i;
				else
					three = i;
			}
	}
	int distance12 = get_distance(one,two);	
	int distance23 = get_distance(two,three);	
	int distance31 = get_distance(three,one);
	//Sort distances. Shortest first.
	int temp;
	if (distance12 > distance23)
	{
		temp = distance12;
		distance12 = distance23;
		distance23 = temp;
		temp = one;
		one = three;
		three = temp;
	}
	if (distance12 > distance31)
	{
		temp = distance12;
		distance12 = distance31;
		distance31 = temp;
		temp = two;
		two = three;
		three = temp;
	}
	if (distance23 > distance31)
	{
		temp = distance23;
		distance23 = distance31;
		distance31 = temp;
		temp = two;
		two = one;
		one = temp;
	}
	if (distance12 == 1 && distance23 == 1 && distance31 == 2)
	{
		draw_three_near(point,position,one,two,three,backwards);
	}
	else
	if (distance12 == 1 && distance23 == 2 && distance31 == 3)
	{
			draw_two_near(point,position,one,two,backwards);
			draw_one(point,position,three,backwards);
	}
	else
	if (distance12 == 2 && distance23 == 2 && distance31 == 2)
	{
			draw_one(point,position,one,backwards);
			draw_one(point,position,two,backwards);
			draw_one(point,position,three,backwards);
	}
	else
		printf("This shouldn't be possible\n");
}

void count_distance(int* d1,int* d2, int* d3,int distance)
{
	switch (distance)
	{
		case 1: (*d1)++; return;
		case 2: (*d2)++; return;
		case 3: (*d3)++; return;
	}
}

void draw_four_case1(Sint32* point,tPoint* position,int one,int two,int three,int four)
{
	//Sorting, that the distance between consecutive points is one
	if (get_distance(one,two) != 1)
	{
		int temp = two;
		two = three;
		three = temp;
	}
	if (get_distance(two,three) != 1)
	{
		int temp = three;
		three = four;
		four = temp;
	}
	
	int edge_one[3];
	get_edges(edge_one,one);
	int edge_two[3];
	get_edges(edge_two,two);
	int edge_three[3];
	get_edges(edge_three,three);
	int edge_four[3];
	get_edges(edge_four,four);
	//Drawing the quad.
	//Seaching the points of 1 and 3:
	int above_one = -1;
	int above_two = -1;
	int above_three = -1;
	int above_four = -1;
	int i,j;
	for (i = 0; i < 3; i++)
		if (edge_one[i] != four && edge_one[i] != two)
		{
			above_one = edge_one[i];
			break;
		}
	for (i = 0; i < 3; i++)
		if (edge_two[i] != one && edge_two[i] != three)
		{
			above_two = edge_two[i];
			break;
		}
	for (i = 0; i < 3; i++)
		if (edge_three[i] != four && edge_three[i] != two)
		{
			above_three = edge_three[i];
			break;
		}
	for (i = 0; i < 3; i++)
		if (edge_four[i] != three && edge_four[i] != one)
		{
			above_four = edge_four[i];
			break;
		}
	tPoint quad[3];
	int do_not_draw = 0;
	if (add_point(&(quad[0]),position[  one],position[  above_one],point[  one],point[  above_one],CORRECTION))
		return;
	if (add_point(&(quad[1]),position[  two],position[  above_two],point[  two],point[  above_two],CORRECTION))
		return;
	if (add_point(&(quad[2]),position[three],position[above_three],point[three],point[above_three],CORRECTION))
		return;
	if (add_point(&(quad[3]),position[ four],position[ above_four],point[ four],point[ above_four],CORRECTION))
		return;

	draw_spQuad3D(quad[0].x,quad[0].y,quad[0].z,
	         quad[1].x,quad[1].y,quad[1].z,
	         quad[2].x,quad[2].y,quad[2].z,
	         quad[3].x,quad[3].y,quad[3].z,COLOR,0);
}
void draw_four(Sint32* point,tPoint* position)
{
	//Finding the three:
	int i;
	int one = -1;
	int two = -1;
	int three = -1;
	int four;
	for (i = 0; i < 8; i++)
		if (point[i] <= 0)
		{
			if (one == -1)
				one = i;
			else
			if (two == -1)
				two = i;
			else
			if (three == -1)
				three = i;
			else
				four = i;
		}
	int distance12 = get_distance(one,two);
	int distance13 = get_distance(one,three);
	int distance14 = get_distance(one,four);
	int distance23 = get_distance(two,three);
	int distance24 = get_distance(two,four);
	int distance34 = get_distance(three,four);
	
	int d1 = 0, d2 = 0, d3 = 0;
	count_distance(&d1,&d2,&d3,distance12);
	count_distance(&d1,&d2,&d3,distance13);
	count_distance(&d1,&d2,&d3,distance14);
	count_distance(&d1,&d2,&d3,distance23);
	count_distance(&d1,&d2,&d3,distance24);
	count_distance(&d1,&d2,&d3,distance34);
	
	if (d1 == 4) //Case 1
	{
		draw_four_case1(point,position,one,two,three,four);
	}
	else
	if (d1 == 2 && d2 == 3 && d3 == 1) //Case 2
	{
		//Searching the one without distance 1
		int the_one;
		int other[3];
		if (distance12!=1 && distance13!=1 && distance14!=1)
		{
			the_one = one;
			other[0] = two;
			other[1] = three;
			other[2] = four;
		}
		else
		if (distance12!=1 && distance23!=1 && distance24!=1)
		{
			the_one = two;
			other[0] = one;
			other[1] = three;
			other[2] = four;
		}
		else
		if (distance13!=1 && distance23!=1 && distance34!=1)
		{
			the_one = three;
			other[0] = one;
			other[1] = two;
			other[2] = four;
		}
		else
		if (distance14!=1 && distance24!=1 && distance34!=1)
		{
			the_one = four;
			other[0] = one;
			other[1] = two;
			other[2] = three;
		}
		draw_one(point,position,the_one,0);
		draw_three_near(point,position,other[0],other[1],other[2],0);
	}
	else
	if (d2 == 6) //Case 3
	{
		draw_one(point,position,one,0);
		draw_one(point,position,two,0);
		draw_one(point,position,three,0);		
		draw_one(point,position,four,0);		
	}
	else
	if (d1 == 3 && d2 == 3) //Case 4
	{
		//TO DO
	}
	else
	if (d1 == 3 && d2 == 2 && d3 == 1) //Case 5 / 7
	{
		//TO DO
	}
	else
	if (d1 == 2 && d2 == 4) //Case 6
	{
		//TO DO
	}
	else
		printf("This shouldn't be possible\n");
}

void draw( void )
{
	spResetZBuffer();
	spIdentity();
	spClearTarget( 0 );
	//spSetZSet(0);
	//spSetZTest(0);
	//spSetCulling(0);
	Sint32 zShift = spFloatToFixed(-4.0f);


	#ifdef FIRST_TRANSFORMATION
	spTranslate(0,0,zShift);	
	spRotateX(rotation);
	spRotateY(rotation/2);
	spRotateZ(rotation/4);
	#endif
	
	int size = (MAX-MIN)/RESOLUTION+1;
	
	Sint32 x,y,z;
	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
			for (z = 0; z < size; z++)
			{
				Sint32 X = MIN+x*RESOLUTION;
				Sint32 Y = MIN+y*RESOLUTION;
				#ifdef FIRST_TRANSFORMATION
				Sint32 Z = MIN+z*RESOLUTION+zShift;
				#else
				Sint32 Z = MIN+z*RESOLUTION;
				#endif
				VALUE(x,y,z) = function_with_modelview(spGetMatrix(),FUNCTION,X,Y,Z);
				PIXEL(x,y,z).x = X;
				PIXEL(x,y,z).y = Y;
				PIXEL(x,y,z).z = Z;
				#ifdef FIRST_TRANSFORMATION
				Z-=zShift;
				#endif
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
	#ifndef FIRST_TRANSFORMATION
	spTranslate(0,0,zShift);	
	spRotateX(rotation);
	spRotateY(rotation/2);
	spRotateZ(rotation/4);
	#endif
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
					case 2: draw_two(points,position,0); break;
					case 3: draw_three(points,position,0); break;
					case 4: draw_four(points,position); break;
					case 5: draw_three(points,position,1); break;
					case 6: draw_two(points,position,1); break;
					case 7: draw_one(points,position,get_the_none(points),1); break;
				}
				if (count > 0 && count < 8)
					spEllipse3D(PIXEL(x,y,z).x+RESOLUTION/2,PIXEL(x,y,z).y+RESOLUTION/2,PIXEL(x,y,z).z+RESOLUTION/2,RESOLUTION/4,RESOLUTION/4,65535);
			}
	spSetBlending(SP_ONE);
	int i;
	printf("----- Draw these count so often:\n");
	for (i = 0; i < 8; i++)
		printf("%i: %i\n",i,countcount[i]);
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
	//spSetDefaultWindowSize( 800, 480 );
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
