/*
---------------------
This is the source of etram/Collapse, a 4k intro released at breakpoint'05
Copyright (C) 2005 by Bernat Mu�oz Garc�a (aka shash/Collapse)
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
// Flare creation in 8 bit depth
static void createFlare ( u8 *buffer )
{
	s32		sy, sx,	dist;

	for (sy = -128; sy < 128; sy ++)
	{
		for (sx = -128; sx < 128; sx ++)
		{
			dist = SQR(sx) + SQR(sy);

/*
			__asm__(
				"fildl	%0\n\t"
				"fsqrt\n\t"
				"fistpl	%0\n\t"
				: "=m"(dist)
				: "0"(dist)
			       );
*/
			
			
			dist = (int)sqrt(dist);
			
			if (dist < 128)
			{
				*buffer = (127 - dist)<<1;
			}

			buffer ++;			
        }
    }
}

// Flare Quad drawer
static void drawFlareInternal (void)
{
	// Blending and disable writing to the zbuffer
	glEnable			(GL_BLEND);
	glDepthMask			(GL_FALSE);
	glBlendFunc			(GL_ONE, GL_ONE);	

	glBegin (GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f (-.5f, -.5f);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f (0.5f, -.5f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f (0.5f, 0.5f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f (-.5f, 0.5f);		
	glEnd ();

	// Disable blending and reenable zbuffer writing
	glDepthMask	(GL_TRUE);	
	glDisable	(GL_BLEND);
}


// Draw billboar aligned flare
void drawFlare (float x2, float y2, float z2, float dens)
{
	glPushMatrix();

	glTranslatef (x2, y2, z2);

	float modelview[16]={0};

	// Get the current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

	// Undo all rotations
	for( int i=0; i<3; i++ ) 
		for( int j=0; j<3; j++ ) {
			if ( i==j )
				modelview[i*4+j] = 1.0;
			else
			modelview[i*4+j] = 0.0;
		}

	// Set the modelview with no rotations and scaling
	glLoadMatrixf(modelview);

	// Scale the flare as needed
	glScalef (dens, dens, dens);
	drawFlareInternal ();

	// Restores the modelview matrix
	glPopMatrix();
}
