/*
---------------------
This is the source of etram/Collapse, a 4k intro released at breakpoint'05
Copyright (C) 2005 by Bernat Muñoz García (aka shash/Collapse)
Linux port (C) 2005 by Jorge Gorbe Moya (aka slack/Necrostudios)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
---------------------
*/

#include <GL/glu.h>
#include <SDL.h>

#include "defines.h"
#include "particles.h"
#include "player.h"

#define __FULLSCREEN__

// Rand seed
s32		seed = 1;

float	resta, added = 0.0f, mult=0.0f, particules[64][2];

GLUquadricObj *obj;

// Rand copied from the crt
/*
s32 rand(void) 
{
	__asm {
			mov		eax, dword ptr [seed]
			imul	eax, 0x000343FD
			add		eax, 0x00269EC3
			mov		dword ptr [seed], eax
			sar		eax, 10
			and		eax, 0x00007FFF
	};
}
*/

// Setup of the Projection Matrix and return 
// to the ModelView matrix
static void __attribute__((regparm(1))) matriuProjeccio (float fov)
{
	glMatrixMode	(GL_PROJECTION);
	glLoadIdentity	();
	gluPerspective	(fov, (float)w_screen/(float)h_screen, 1.0f, 100.0f);
	glMatrixMode	(GL_MODELVIEW);
}

// Init of 4k
static void  init4K (void)
{
	// Lights position
	GLfloat		light1Pos[4]	 = { 10.0f,  5.0f, 0.0f, 0.0f };
	GLfloat		light2Pos[4]	 = { 0.0f,  5.0f, 10.0f, 0.0f };

	// Lights colors
	GLfloat		diffuseLight1[4] = { 1.5f, 0.0f, 0.0f, 0.0f };
	GLfloat		diffuseLight2[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	
	u8 			*flare = (u8*)malloc(256*256);
	
	// We init the flares "explode" vectors
	for (s32 x = 0; x < 64; x++)
	{
		particules[x][0] = ((rand()%64)-32)/256.0f;
		particules[x][1] = ((rand()%64)-32)/256.0f;
	}

	glLightfv		(GL_LIGHT0,GL_DIFFUSE,diffuseLight1);
	glLightfv		(GL_LIGHT1,GL_DIFFUSE,diffuseLight2);

	glLightfv		(GL_LIGHT0,GL_POSITION,light1Pos);
	glLightfv		(GL_LIGHT1,GL_POSITION,light2Pos);

	glEnable		(GL_LIGHT0);
	glEnable		(GL_LIGHT1);
	glEnable		(GL_LIGHTING);
	glEnable		(GL_COLOR_MATERIAL);
	glEnable		(GL_TEXTURE_2D);
	glEnable		(GL_DEPTH_TEST);

	// We create the flare and upload to oGL
	createFlare		(flare);
	glTexParameteri	(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D	(GL_TEXTURE_2D, 0, 1, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, flare );

	obj = gluNewQuadric ();

    // White clear color for the "flashes"
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
}


// Cylinder with top and bottom pieces, drawn with glu*
void drawCylinder (float radius, float height, u32 slices)
{
	glPushMatrix();

	// We want texture coords to be generated
	gluQuadricTexture (obj, GL_TRUE);

	// We rotate the whole mesh, innecesary
	// with proper camera setup
	glRotatef   (90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef   (45.0f, 0.0f, 0.0f, 1.0f);

	// Bottom piece
	glTranslatef(0.0f, 0.0f, height * -0.5f);
	glRotatef	(180.0f, 1.0f, 0.0f, 0.0f);
	gluDisk	(obj, 0.0f, radius, slices, 4);

	glRotatef	(180.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder	(obj,
                 radius,
                 radius,
                 height,
                 slices,
		 4);

	// Top piece
	glTranslatef(0.0f, 0.0f, height);
	gluDisk	(obj, 0.0f, radius, slices, 4);

	glPopMatrix ();
}

// Sort of torus drawn with two cylinders and two
// partial disks
void drawTorus (float innerRadius, float outerRadius, float height)
{
	glPushMatrix ();

	// Exterior cylinder
	glRotatef		(90.0f, 1.0f, 0.0f, 0.0f);
	gluCylinder		(obj, outerRadius, outerRadius, height, 32, 2);

	// Interior cylinder
	gluQuadricOrientation(obj, GLU_INSIDE);
	gluCylinder		(obj, innerRadius, innerRadius, height, 32, 2);

	// Bottom piece
	gluDisk		(obj, innerRadius, outerRadius, 32, 2);

	// Top piece
	gluQuadricOrientation(obj, GLU_OUTSIDE);
	glTranslatef		(0.0f, 0.0f, height);
	gluDisk		(obj, innerRadius, outerRadius, 32, 2);

	glPopMatrix ();
}

// Room drawing routine
// Optimitzable if everything was a bit more sorted :)
static void drawRoom (s32 etime)
{
	glPushMatrix ();
	glPushMatrix ();
	glPushMatrix ();
	glPushMatrix ();

	// Draw the blocks of the walls and the "railing"
	for (s32 i = 0; i < 20; i++)
	{
		glPushMatrix ();

		glRotatef (18.0f*i, 0.0f, 1.0f, 0.0f);

		glTranslatef (0.0f,-1.0f, 8.0f);
		drawCylinder (0.25f, 1.9f,4);

		glTranslatef (0.0f, 0.0f, 6.0f);
		drawCylinder (0.75f, 12.0f,4);

		glTranslatef (0.0f, 1.0f, -14.0f);

		glRotatef (9.0f, 0.0f, 1.0f, 0.0f);
		glTranslatef (0.0f, -1.0f, 15.5f);

		drawCylinder (2.5f, 2.5f,4);

		glPopMatrix ();
	}

	// Draw the railing "top"
	drawTorus (7.7f, 8.3f, 0.2f);


	// Draw the ceiling
	glTranslatef (0.0f, 5.0f, 0.0f);
	
	/*
	glRotatef	 (90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef	 (90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef	 (90.0f, 1.0f, 0.0f, 0.0f);
	*/
	glRotatef	(270.0f, 1.0f, 0.0f, 0.0f);

	gluDisk	 (obj, 0.0f, 14.0f, 32, 4);

	glPopMatrix ();	

	// Draw the top level floor
	glTranslatef (0.0f, -1.5f, 0.0f);
	
	/*
	glRotatef	(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef	(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef	(90.0f, 1.0f, 0.0f, 0.0f);
	*/
	glRotatef	(270.0f, 1.0f, 0.0f, 0.0f);

	gluDisk	(obj, 7.0f, 14.0f, 32, 4);

	// Draw the top wall
	gluQuadricOrientation (obj, GLU_INSIDE);
	gluCylinder	(obj, 14.0f, 14.0f, 7.0f, 32, 4);

	// Draw the bottom wall
	glTranslatef (0.0f,  0.0f, -3.0f);
	gluCylinder	(obj, 7.0f, 7.0f, 3.0f, 32, 4);

	// Draw the bottom lever floor
	gluQuadricOrientation (obj, GLU_OUTSIDE);
	gluDisk		(obj, 0.0f, 7.0f, 32, 4);

	glPopMatrix ();

	// Draw the bottom level torus (for the "flare column")
	glTranslatef	(0.0f, 4.0f, 0.0f);
	drawTorus		(13.f, 14.5f, 1.0f);
	glTranslatef	(0.0f, 1.4f, 0.0f);

	// Draw the ceiling cilynders
	drawTorus		(4.5f, 5.0f, 0.5f);
	drawTorus		(7.5f, 8.0f, 0.5f);
	drawTorus		(10.5f, 11.0f, 0.5f);

	glTranslatef	(0.0f,-0.3f, 0.0f);
	glRotatef		(90.0f, 1.0f, 0.0f, 0.0f);

	// Draw the cilinder at the ceiling (creating the star like shape)
	for (s32 i = 0; i < 10; i++)
	{
		glRotatef	(18.0f, 0.0f, 0.0f, 1.0f);
		drawCylinder (0.2f, 50.0f, 4);
	}

	glPopMatrix ();

	// Draw the "flare column" control disks when needed
	if (etime > 8000)
	{
		float barresControl = 1.0f;

		// The start movement of the control disks
		if (etime < 13000)
		{
			barresControl = ((etime-7500) / 6500.0f);
		}

		for (s32 i = 0; i < 10; i++)
		{
			float randX = 0.0f;

			glPushMatrix();

			// Control disks random movements
			if (etime > 27500)
			{
				randX = ((rand()%32) / 256.0f);
			}

			// Disks moving randomly / rotating
			if (etime < 34000)
			{
				glRotatef	(etime/10.0f, 0.0f, 1.0f, 0.0f);
				glTranslatef(	((i*3)%5)/8.25f + randX, 
								(5.0f - i)*barresControl, 
								((i*2)%4)/8.25f + randX );
			}

			// Disks flying away from it's position
			else
			{
				glTranslatef(	((i*3)%5)/8.25f + sinf(i*10.0f)*(etime-34000)/500.0f, 
								(5.0f - i)*barresControl, 
								((i*2)%4)/8.25f + cosf(i*10.0f)*(etime-34000)/500.0f );
			}

			// Disks drawing with variable size
			drawTorus (	1.0f+fabsf(sinf((float)i))*fabsf(sinf(etime/1000.0f + i)), 
						1.1f+fabsf(sinf((float)i)), 
						0.4f);

			glPopMatrix();
		}
	}

	glRotatef	 (90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef (0.0f, 0.0f, 4.5f);

	// Bottom cilinders drawing (creating the star like shape)
	for (s32 i = 0; i < 12; i++)
	{
		glRotatef	(15.0f, 0.0f, 0.0f, 1.0f);
		drawCylinder	(.1f, 15.0f, 32);
	}

	glPopMatrix ();
}

// Reduced version of the gluLookAt
void gluLookAt2 (float eyex, float eyey, float eyez, float centery)
{
	gluLookAt (eyex, eyey, eyez, 0.0f, centery, 0.0f, 0.0f, 1.0f, 0.0f);
}

void quit_intro()
{
	// Kill the 4k
	SDL_Quit();
/*
	__asm__("xorl %eax, %eax\n\t"
		"incl %eax\n\t"
		"int $0x80\n\t");
*/
	exit(0);
}

// Synch and render of the whole intro
static void drawMain (s32 etime) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	glPushMatrix();

	// Start rotating and fade from black
	if (etime < 5000)
	{
		glColor3f (etime/5000.0f, etime/5000.0f, etime/5000.0f);

		matriuProjeccio (90.0f - (45.0f*(etime/5000.0f)));

		gluLookAt2 ( sinf(etime/1500.0f)*12.0f, 5.0f - (etime/1000.0f), cosf(etime/1500.0f)*12.0f,
					0.0f );
	}
	// Camera 1
	else if (etime < 8000)
	{
		etime -= 5000;

		gluLookAt2 ( 2.0f + etime/500.0f + sinf(etime/500.0f)*4.0f, cosf(etime/500.0f)*2.0f, etime/500.0f + cosf(etime/500.0f)*4.0f,
					0.0f );

		etime += 5000;
	}
	// Camera 2
	else if (etime < 15000)
	{
		etime -= 8000;
		
		gluLookAt2 ( sinf(2.0f)*12.0f - sinf(etime/4000.0f)*8.0f, sinf(etime/3500.0f)*3.0f+1.0f, cosf(2.0f)*12.0f + cosf((etime/5000.0f)),
					sinf(etime/1000.0f)*3.0f );

		etime += 8000;
	}
	// Camera 3 (with fov animation)
	else if (etime < 18000)
	{
		etime -= 15000;

		matriuProjeccio (sinf(etime/1000.0f)*40.0f + 45.0f);

		gluLookAt2 ( sinf(etime/1500.0f)*13.0f, sinf(etime/1500.0f)*3.0f, cosf(etime/1500.0f)*13.0f,
					0.0f );

		etime += 15000;
	}
	// Camera 3
	else if (etime < 28000)
	{
		etime -= 18000;

		gluLookAt2 ( sinf(etime/1500.0f)*(4.0f+etime/3000.0f), sinf(etime/1500.0f)*4.0f, cosf(etime/1500.0f)*(4.0f+etime/3000.0f),
					cosf(etime/1500.0f)*4.0f );

		etime += 18000;
	}

	// Fast cameras for the random moving control disks
	else if (etime < 47000)
	{
		etime -= 28000;

		matriuProjeccio (45.0f);

		if (etime < 500)
		{
			gluLookAt2 ( 4.0f, 5.0f, 4.0f, -3.0f);
		}
		else if (etime < 1000)
		{
			gluLookAt2 ( 4.0f, -3.0f, -4.0f, 1.0f);
		}
		else if (etime < 1500)
		{
			gluLookAt2 ( -4.0f, 0.0f, 4.0f, 1.0f);
		}
		else
		{
			gluLookAt2 ( 5.0f+etime/2000.0f, 3.0f, 5.0f-etime/2000.0f, 1.0f);
		}

		etime += 28000;
	}

	// We kill the 4k at the end
	else
	{
		quit_intro();	
	}

	// Flare column position
	resta = (68.0f - (etime/300.0f));
	if (resta < 5.0f) resta = 5.0f;

	// Draw the whole room
	drawRoom		(etime);

	for (int x = 0; x < 64; x ++)
	{
		// Flare particles grow
		if (etime > 25000 && etime < 27500)
		{
			added = sinf (((etime-25000) / 2500.0f)*(M_PI/2.0f)*4.0f)*3.0f;
		}

		// Flare particles growing and "exploding"
		if (etime >= 35500)
		{
			mult  = (etime-35500)/100.0f;
			added = ((etime-35500)/1000.0f);
		}

		// Flare drawer
		drawFlare(	particules[x][0]*mult, 
					((x/5.0f) - resta), 
					particules[x][1]*mult, 
					1.5f+sinf((etime+x*500)/1000.0f)*0.5f+added);
	}

	// Flashes
	if ((etime > 5000  && etime < 5100) || 
		(etime > 8000  && etime < 8100) || 
		(etime > 15000 && etime < 15100) || 
		(etime > 18000 && etime < 18100) ||
		(etime > 28000 && etime < 28100) ||
		(etime > 28500 && etime < 28600) ||
		(etime > 29000 && etime < 29100) ||
		(etime > 29500 && etime < 29600) ||
		(etime > 34000 && etime < 34100) ||
		(etime > 45000 && etime < 50000))
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	glPopMatrix();
}


#define BUFFER_SIZE 4096 

static int audio_pos=0;

void audio_callback(void *userdata, Uint8 *stream, int len)
{
	short *buf=(short *) stream;
	for (int i=0; i < len>>1; ++i)
	{
		buf[i]=datawave[audio_pos++];
	}
	
}

SDL_AudioSpec desired={PLAYFREQ,AUDIO_S16SYS, 1,0,BUFFER_SIZE,0,0,audio_callback,NULL};

// Entry point of all Windows programs
int main()
{
	s32 etime, time0;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

#ifndef __FULLSCREEN__
	Uint32 sdl_video_flags = SDL_HWSURFACE | SDL_OPENGL;
#else
	Uint32 sdl_video_flags = SDL_HWSURFACE | SDL_OPENGL | SDL_FULLSCREEN;
#endif
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	SDL_SetVideoMode(w_screen, h_screen, w_bpp, sdl_video_flags);
	SDL_ShowCursor(0);
	// Init 4k data
	init4K ();

	// Init and play sound
	initPlayer ();

	SDL_OpenAudio(&desired, NULL);
	SDL_PauseAudio(0);
		
	time0 = SDL_GetTicks();

	SDL_Event ev;
	while(1)
	{
		while (SDL_PollEvent(&ev))
		{
			if (ev.type==SDL_KEYDOWN)
				goto fin;
		}
							
		etime = SDL_GetTicks() - time0;

		// Draw everything at "slow" pace
		drawMain (etime>>1);

		SDL_GL_SwapBuffers();
	}
	fin:
	quit_intro();
}

