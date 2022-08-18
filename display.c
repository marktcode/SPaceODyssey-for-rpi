/* ==========================================================================
                            DISPLAY_C
=============================================================================

 ============================================================================ */

#include <math.h>           /* C header for any math functions               */
#include <stdio.h>          /* C header for standard I/O                     */
#include <string.h>         /* For String compare                            */
#include <stdlib.h>
#include <GL/glut.h>	    /* OpenGl headers				     */

#include "head.h"	   /* local header for the face		             */

void calc_normal ( float *p1, float *p2, float *p3, float *norm );




 void
 output(float x, float y, float z, char *string)
 {
	int len, i;
		len = (int) strlen(string);
	
		glRasterPos3f(x, y, z);
		
		for (i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
		
		}
 }

/* void
 output(GLfloat x, GLfloat y, GLfloat z, char *text)
 {
   char *p;

   glPushMatrix();
   glColor3f ( 1,  1, 0 ) ;
   glTranslatef(x, y, z);
   for (p = text; *p; p++)
	 glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
   glPopMatrix();
 }
*/

/* ========================================================================= */  
/* draw_axis                                                                 */
/* ========================================================================= */  
/*
** Draws in the wire frame for the axis and base plane
**  
** 
** 
** 
*/

void draw_axis ( float zaxis, float yaxis, float xaxis, float grid_r, float grid_g, float grid_b)
{
	int i;
 	float x_red,x_green,x_blue,y_red,y_green,y_blue,z_red,z_green,z_blue;
	float luminance = sqrt (background_r*background_r + background_g*background_g + background_b*background_b);
	float mincolour;
	

    if (grid_r < grid_g) mincolour = grid_r; else mincolour = grid_g;
    if (mincolour > grid_b) mincolour = grid_b;
    luminance -= mincolour;
	
 	x_red = xaxis * grid_r;
 	x_green = xaxis * grid_g;
 	x_blue = xaxis * grid_b;
 	y_red = yaxis * grid_r;
 	y_green = yaxis * grid_g;
 	y_blue = yaxis * grid_b;
 	z_red = zaxis * grid_r;
 	z_green = zaxis * grid_g;
 	z_blue = zaxis * grid_b;
 	
 	if (complement) {  
 			x_red =  complement - x_red;  y_red =  complement - y_red;  z_red =  complement - z_red; 
 			x_green =  complement - x_green;  y_green =  complement - y_green;  z_green =  complement - z_green; 
 			x_blue =  complement - x_blue;  y_blue =  complement - y_blue;  z_blue =  complement - z_blue; 
 	}
    
 	x_red += luminance;
 	x_green += luminance;
 	x_blue += luminance;
 	y_red += luminance;
 	y_green += luminance;
 	y_blue += luminance;
 	z_red += luminance;
 	z_green += luminance;
 	z_blue += luminance;
 
	glLineWidth   ( 1.50 ) ;
	if (xaxis>.05) {
		 ;
		glColor3f	( x_red ,  x_green , x_blue) ;
		glBegin ( GL_LINES ) ; { // x-axis
		  glVertex3f (xorig - xlimit, yorig, zorig ) ;
		  glVertex3f (xorig + xlimit, yorig, zorig ) ;
		}  glEnd ( ) ;
		if (yaxis>.05) {
			glBegin ( GL_LINES ) ; { // x-axis
			  glVertex3f (xorig - xlimit, yorig, zlimit) ;
			  glVertex3f (xorig - xlimit, yorig, -zlimit ) ;
			}  glEnd ( ) ;
		}
		if (zaxis>.05) {
			glBegin ( GL_LINES ) ; { // x-axis
			  glVertex3f (xorig - xlimit,  ylimit , zorig) ;
			  glVertex3f (xorig - xlimit,  -ylimit , zorig) ;
			}  glEnd ( ) ;
		}
	}
	glLineWidth   ( 0.50 ) ;
	if (zaxis>.05) {
		glColor3f	( z_red ,  z_green , z_blue) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f ( xorig - xlimit,  0.0 , zorig) ;
				  glVertex3f ( xorig + xlimit,  0.0 , zorig ) ;
				}  glEnd ( ) ;
		for (i = 1; i < 2 * ((int) ylimit);i++) {
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f ( xorig - xlimit,  (float) i * 0.5 , zorig) ;
				  glVertex3f ( xorig + xlimit,  (float) i * 0.5 , zorig ) ;
				}  glEnd ( ) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f ( xorig - xlimit,  -(float) i * 0.5 , zorig) ;
				  glVertex3f ( xorig + xlimit,  -(float) i * 0.5 , zorig ) ;
				}  glEnd ( ) ;
		}
	}
	if ((zaxis>.05) || (yaxis>.05)) {
			if (zaxis > yaxis) glColor3f  ( z_red ,  z_green , z_blue) ;
			else  glColor3f	( y_red ,  y_green , y_blue) ;
			glBegin ( GL_LINES ) ; { // x-axis
		  glVertex3f ( xorig - xlimit, yorig, zorig) ;
		  glVertex3f ( xorig + xlimit, yorig, zorig ) ;
		}  glEnd ( ) ;
	}
	if (zaxis>.05) {
		glColor3f	( z_red ,  z_green , z_blue) ;
		glBegin ( GL_LINES ) ; { // x-axis
		  glVertex3f ( xorig ,  -(ylimit - 0.1) , zorig ) ;
		  glVertex3f ( xorig ,  ylimit - 0.1 , zorig ) ;
		}  glEnd ( ) ;
	}
	if (yaxis>.05) {
		glColor3f	( y_red ,  y_green , y_blue) ;
		glBegin ( GL_LINES ) ; { // x-axis
		  glVertex3f ( xorig , yorig, - (zlimit - 0.1)) ;
		  glVertex3f ( xorig , yorig, zlimit - 0.1 ) ;
		}  glEnd ( ) ;
	}
	for (i = 1; i < 2* ((int) xlimit);i++) {
		if (zaxis>.05) {
			glColor3f	( z_red ,  z_green , z_blue) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig + (float) i * 0.5,  -(ylimit - 0.1) , zorig) ;
				  glVertex3f (xorig + (float) i * 0.5,  ylimit - 0.1 , zorig ) ;
				}  glEnd ( ) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig - (float) i * 0.5,  -(ylimit - 0.1) , zorig) ;
				  glVertex3f (xorig - (float) i * 0.5,  ylimit - 0.1 , zorig ) ;
				}  glEnd ( ) ;
		}
		if (yaxis>.05) {
			glColor3f	( y_red ,  y_green , y_blue) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig + (float) i * 0.5, yorig,  - (zlimit - 0.1)) ;
				  glVertex3f (xorig + (float) i * 0.5, yorig,  zlimit - 0.1 ) ;
				}  glEnd ( ) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig - (float) i * 0.5, yorig,  -(zlimit - 0.1)) ;
				  glVertex3f (xorig - (float) i * 0.5, yorig,  zlimit - 0.1 ) ;
				}  glEnd ( ) ;
		}
	}
	if (yaxis>.05) {
		glColor3f	( y_red ,  y_green , y_blue) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig - xlimit, yorig,  0.0) ;
				  glVertex3f ( xorig + xlimit,  yorig, 0.0 ) ;
				}  glEnd ( ) ;
		for (i = 1; i < 2* ((int) zlimit) ;i++) {
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f (xorig - xlimit, yorig,  (float) i * 0.5) ;
				  glVertex3f ( xorig + xlimit,  yorig, (float) i * 0.5 ) ;
				}  glEnd ( ) ;
				glBegin ( GL_LINES ) ; { // x-axis
				  glVertex3f ( xorig - xlimit, yorig, -(float) i * 0.5) ;
				  glVertex3f ( xorig + xlimit,  yorig, -(float) i * 0.5 ) ;
				}  glEnd ( ) ;
		}
	}

  glLineWidth   ( 1.00 ) ;  

}
/* ========================================================================= */  
/* screentext                         		                                 */
/* ========================================================================= */  
/*
*/
void screentext (LINE **curvePtr, int Num_curve, TXTLINE **textPtr, int Num_texts)
{
  int plabel, p;
  char  scanpos[LABEL_SIZE], graphlabels[LABEL_SIZE], indextime[LABEL_SIZE];
  float left, middle, right, window, timestuff;
  int	hh, h, mm, m, ss, s, ps, pss;
  
    int pdivcpy = pdiv;
	if (sleepStateFlag == 1) { Num_curve++; pdivcpy++; } // raise count for the duration of the display call

	glDisable		( GL_LIGHTING ) ;
	glColor3f	( 0.5 ,  0.5 , 0.5) ;
	if (Showlabels) { // display the scan start/middle/finish   and optional time

		left = (1+ start_scan);
		middle = left + width_scan/2;
		right = left + width_scan;
		window = duration *  width_scan/2;
		if (left < 0) left = 0;
		if (middle < 0) middle = 0;
		if (right < 0) right = 0;


		if (duration > 0) { // I expect there is a c-library conversion available for this...
			timestuff = duration *  middle / 2.0 ;
			ps = (int) (timestuff * 100) % 100 ;
			pss = ps % 10 ;
			ps = ps/10;
			s = ((int) timestuff) % 60;
			ss = s % 10;
			s = s/10 ;
			m = ((int) timestuff/60) % 60;
			mm = m % 10;
			m = m/10 ;
			h = ((int) timestuff/3600) % 24;
			hh = h % 10;
			h = h/10;

			if (window < 1) 
				snprintf(&scanpos[0],LABEL_SIZE, "[  %0.4f  |  %0.3f  |  %d%d:%d%d:%d%d.%d%d  ]\000" , timestuff, window, h, hh , m, mm, s, ss, ps, pss); 
			else if (window < 10) 
				snprintf(&scanpos[0],LABEL_SIZE, "[  %0.3f  |  %0.2f  |  %d%d:%d%d:%d%d.%d%d  ]\000" , timestuff, window, h, hh , m, mm, s, ss, ps, pss); 
			else 
				snprintf(&scanpos[0],LABEL_SIZE, "[  %0.2f  |  %0.1f  |  %d%d:%d%d:%d%d.%d%d  ]\000" , timestuff, window, h, hh , m, mm, s, ss, ps, pss); 
			output(-2, -5.6 , 0.4, scanpos) ;
		}
		else {
			snprintf(&scanpos[0],LABEL_SIZE, "%f  |  %f  |  %f\000",left/2.0, middle/2.0, right/2.0); output(-1, -5.6 , 0.4, scanpos) ;
		}
		

		if (simultrace == 1) { // graph label now allows for LABEL_SIZE chars
			if (Num_curve-pdivcpy ) {
				plabel = pdivcpy + (int) ( (float) (Num_curve-pdivcpy) * (1+ start_scan + width_scan)/2);
				if (plabel > Num_curve-1) plabel = Num_curve-1;
				if ( plabel > pdivcpy) {snprintf(&graphlabels[0],LABEL_SIZE, "%s", curvePtr[plabel]->label );  output(-6.5, -5.0 , -.4, graphlabels);}
				// printf("NumCurve %d, pdiv  %d, plabel  %d\n", Num_curve, pdivcpy, plabel );
			}
		}
		for (p=0; ((p < Num_curve) && (p <= 6) && (simultrace == 0)) ; p++) {
			if ((curvePtr[p])->visible == GLUT_VISIBLE) {
				output(-6.0 + 2*p , -5.0   , .2, (curvePtr[p])->label) ; 
			}
		}
	}
	if (Num_texts) {
		for (p=0; p < Num_texts ; p++) {
			if ((textPtr[p])->visible == GLUT_VISIBLE) {
				output(textPtr[p]->xoffset, textPtr[p]->yoffset  , textPtr[p]->zoffset, (textPtr[p])->label) ; 
			}
		}
	}
	glEnable		( GL_LIGHTING ) ;

	
}

/* ========================================================================= */  
/* paint_polygons_lines                                                      */
/* ========================================================================= */  
/*
** Paints the polygons of the surface. 
** Type indicates if they are to be 
** drawn         (type=0), 
** flat shaded   (type=1),
** smooth shaded (type=2).
*/

void paint_polygons ( SURF **surfacePtr, int Num_surface, long type, long normals, LINE **curvePtr, int Num_curve, BALL **ballPtr, int spinx, int spiny)
{
  int p, q, s, curv, pos, pstart, pinc, pdone, plabel, reversescandir;
  long    i, j, k, ii,	nxtii, ring_ii;
  float  v1[3], v2[3], v3[3] ;
  float  norm1[3], norm2[3], norm3[3] ;
  float  vn1[3], vn2[3], vn3[3], AvgColour , intensity, ydepth, zdepth;
  char  scanpos[LABEL_SIZE], graphlabels[LABEL_SIZE];
  float	ring_scan, shift, coeff;


	int pdivcpy = pdiv; // pass in pdiv as an argument in future
	if (sleepStateFlag == 1) { Num_curve++; pdivcpy++; }// raise count for the duration of the display call
		
	glEnable		( GL_LIGHTING ) ;
	
	pstart = 0; pinc = 1; pdone = Num_curve;  
	for (p=pstart; p != pdone; p += pinc) { 
//		printf ("curve = %d, visible = %d, kind %d, pdiv %d \n",p, curvePtr[p]->visible, curvePtr[p]->kind, pdiv); 

		if (curvePtr[p]->kind == COLOUR_FUNC) { // treat the colour functions as invisible and static
			curvePtr[p]->visible = GLUT_NOT_VISIBLE;
			pos = (int) ((float) curvePtr[p]->npolylinenodes * (1.0 + start_scan + width_scan/2.0)/2.0);
			pos = 3*pos;

			curvePtr[p]->colour_red = curvePtr[p]->polyline[pos];
			curvePtr[p]->colour_green = curvePtr[p]->polyline[pos + 1];
			curvePtr[p]->colour_blue = curvePtr[p]->polyline[pos + 2];
			
		} else if (curvePtr[p]->visible == GLUT_VISIBLE) {
//		   printf ("simultrace %d, kind %d, pdiv %d\n",simultrace, curvePtr[p]->kind, pdivcpy); 
			if ( ((simultrace == 0) &&  (curvePtr[p]->kind == DYNAMIC) && (p > pdivcpy)) || ((p <= pdivcpy) &&  (curvePtr[p]->kind == STATIC)) || (curvePtr[p]->colour == ' ') ) {	// static graphs or Simultrace == 0 or colour is black
//					printf("got to lc %c \n", curvePtr[p]->colour);
				glLineWidth   ( 1.10 ) ;  
			   ii=0;
			   if (curvePtr[p]->polyline) {

					glColor3f  ( 0.0 ,  0.0 , 0.0 ) ; // black default
					if (curvePtr[p]->colour == 'p') { // only does line segments for a 'plane' intersected curve
						if (curvePtr[p]->linetype == '-') {
							glLineWidth   ( curvePtr[p]->width ) ;  

							for (i=0; i < (curvePtr[p])->npolylinenodes - 1 ; i++ )  // draw the (curvePtr[p])
							   {
									if (  ( curvePtr[p]->polyline[ii+2] < awake) || (curvePtr[p]->polyline[ii+5] < awake) ) glColor3f  (10.0 ,  10.0 , 10.0 ) ; // disconnected
									else if (sleepwake -((curvePtr[p])->polyline[ii+2] + (curvePtr[p])->polyline[ii+5])/2 < 0) glColor3f  (0.0 ,  1.0 , 15.0 ) ; // set blue for the awake
									else if (sleep21 + ((curvePtr[p])->polyline[ii+1] + (curvePtr[p])->polyline[ii+4])/2 - ((curvePtr[p])->polyline[ii+2] + (curvePtr[p])->polyline[ii+5])/2  < 0) {
										if (sleepREM  + ((curvePtr[p])->polyline[ii+1] + (curvePtr[p])->polyline[ii+4])/2 - sleepREMslope *((curvePtr[p])->polyline[ii+2] + (curvePtr[p])->polyline[ii+5])/2  < 0) glColor3f  (15.0 ,  0.0 , 0.0 ) ;
										else glColor3f  (0.0 ,  15.0 , 0.0 ) ;
									}
									else glColor3f  (15.0 ,  15.0 , 0.0 ) ; // yellow

								glBegin ( GL_LINES ) ; { //GL_LINE_STRIP
								// draw the lines
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								  glVertex3f ( (curvePtr[p])->polyline[ii+3] ,    (curvePtr[p])->polyline[ii+4],    (curvePtr[p])->polyline[ii+5]     ) ;
								}  glEnd ( ) ;

							ii+=3;
							}
						}
						else if (curvePtr[p]->linetype == '.') {
							glPointSize (curvePtr[p]->width);
							for (i=0; i < (curvePtr[p])->npolylinenodes; i++ )  // draw the (curvePtr[p])
							   {
									if (curvePtr[p]->polyline[ii+2] < awake ) glColor3f  (10.0 ,  10.0 , 10.0 ) ; // disconnected
									else if (sleepwake -(curvePtr[p])->polyline[ii+2] < 0) glColor3f  (0.0 ,  1.0 , 15.0 ) ; // set blue for the awake
									else if (sleep21 + (curvePtr[p])->polyline[ii+1]  - (curvePtr[p])->polyline[ii+2]  < 0) {
										if (sleepREM  + (curvePtr[p])->polyline[ii+1]  - sleepREMslope * curvePtr[p]->polyline[ii+2]   < 0) glColor3f  (15.0 ,  0.0 , 0.0 ) ;
										else glColor3f  (0.0 ,  15.0 , 0.0 ) ;
									}
									else glColor3f  (15.0 ,  15.0 , 0.0 ) ; // yellow

								glBegin ( GL_POINTS ) ; { //points
								// draw the point
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								}  glEnd ( ) ;

							ii+=3;
							}
						}
						else if (curvePtr[p]->linetype == '|') {
							glPointSize (curvePtr[p]->width);
							for (i=0; i < (curvePtr[p])->npolylinenodes; i++ )  // draw the (curvePtr[p])
							   {
									if (curvePtr[p]->polyline[ii+2] < awake ) glColor3f  (10.0 ,  10.0 , 10.0 ) ; // disconnected
									else if (sleepwake -(curvePtr[p])->polyline[ii+2] < 0) glColor3f  (0.0 ,  1.0 , 15.0 ) ; // set blue for the awake
									else if (sleep21 + (curvePtr[p])->polyline[ii+1]  - (curvePtr[p])->polyline[ii+2]  < 0) {
										if (sleepREM  + (curvePtr[p])->polyline[ii+1]  - sleepREMslope * curvePtr[p]->polyline[ii+2]   < 0) glColor3f  (15.0 ,  0.0 , 0.0 ) ;
										else glColor3f  (0.0 ,  15.0 , 0.0 ) ;
									}
									else glColor3f  (15.0 ,  15.0 , 0.0 ) ; // yellow

								glBegin ( GL_LINES ) ; { // virtical line to x-axis
								// draw a vertical line
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    0,    0     ) ; // origin
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								}  glEnd ( ) ;

							ii+=3;
							}
						}
						

					} // done 'p's
					else 
					{ // do the other colours
						glColor3f  (Tracelight * curvePtr[p]->colour_red , Tracelight * curvePtr[p]->colour_green , Tracelight * curvePtr[p]->colour_blue ) ;

						if (curvePtr[p]->linetype == '-') {
							glLineWidth   ( curvePtr[p]->width ) ; 
							for (i=0; i < (curvePtr[p])->npolylinenodes - 1 ; i++ ) { // draw the (curvePtr[p])
								glBegin ( GL_LINES ) ; { //GL_LINE_STRIP
								// draw the lines
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								  glVertex3f ( (curvePtr[p])->polyline[ii+3] ,    (curvePtr[p])->polyline[ii+4],    (curvePtr[p])->polyline[ii+5]     ) ;
								}  glEnd ( ) ;
	
							ii+=3;
							}
						} // end of linetype
						else if (curvePtr[p]->linetype == '.') { // point type
							glPointSize (curvePtr[p]->width);
	
							for (i=0; i < (curvePtr[p])->npolylinenodes ; i++ )  { // draw the (curvePtr[p])
								glBegin ( GL_POINTS ) ; { //points
								// draw the point
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								}  glEnd ( ) ;
								ii+=3;
							}
						} // end points
						else if (curvePtr[p]->linetype == '|') { // virtical type
							glPointSize (curvePtr[p]->width);

							for (i=0; i < (curvePtr[p])->npolylinenodes ; i++ )  { // draw the (curvePtr[p])
								glBegin ( GL_LINES ) ; { //points
								// draw the point
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    0,  0 ) ;
								  glVertex3f ( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								}  glEnd ( ) ;
								ii+=3;
							}
						} // end virtical lines
						else if (curvePtr[p]->linetype == 'b') { //  balls
							for (i=0; i < (curvePtr[p])->npolylinenodes ; i++ )  { // draw the (curvePtr[p])
								// draw the ball

								glPushMatrix ( ) ;
								glTranslated( (curvePtr[p])->polyline[ii] ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
								glutSolidSphere ((GLdouble) 0.1 * curvePtr[p]->width , 20, 20) ;
								glPopMatrix ( ) ;
								
								ii+=3;
							}
						} // end balls
					
					} // end of other colours
				
				} // end if (curvePtr[p]->polyline)

			} // end of LC colour attribute (static curves)
			else if (curvePtr[p]->kind == DYNAMIC) // polygraph (DYNAMIC traces and graphs)
			{
				if (p <= pdivcpy) { // for all polygraph traces ...  toggle between polygraph trace and multi-static scan
				   if ((curvePtr[p])->polyline) { // polygraph trace
						glColor3f  (15.0 * curvePtr[p]->colour_red , 15.0 * curvePtr[p]->colour_green , 15.0 * curvePtr[p]->colour_blue ) ;  // to do tag: look into this saturation factor of 15...
						
						ring_scan = (start_scan + 1.0)/2.0 ; // + ring buffer pointer ; // allow ring buffer handling
						if (ring_scan >=0) ring_scan = (ring_scan - ((int) ring_scan)) * 2.0 -1.0;
				//		else {};
				//		while (ring_scan > 1.0 ) ring_scan -= 2.0;
				//		while (ring_scan < -1.0 ) ring_scan += 2.0; // modulo the scaning range
						
						ii = 3 * (int) ((float) curvePtr[p]->npolylinenodes * (1.0 + ring_scan )/2.0) ; // -1.0 <= ring_scan <= 1.0
						nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes); // precalculate the next, to cut down on processing
						ring_ii = 0 ; // index for x scale only references always from the start of the array, instead of with respect to the data pointer
						
						shift = curvePtr[p]->xoffset -curvePtr[p]->xscale;
						coeff = 2*(curvePtr[p]->xscale) / width_scan;
						if (curvePtr[p]->linetype == '-') { // trace
							glLineWidth   ( curvePtr[p]->width ) ; 
//printf("p %d, colour %c, width %f, type %c, x %f, y %f, z %f \n", p, curvePtr[p]->colour, curvePtr[p]->width, curvePtr[p]->linetype, curvePtr[p]->polyline[ring_ii], curvePtr[p]->polyline[ii + 1], curvePtr[p]->polyline[ii+ 2] );											
							while (curvePtr[p]->polyline[ring_ii] <= -1 + width_scan)  {
								if (curvePtr[p]->polyline[ring_ii+3] > -1) {
									glBegin ( GL_LINES ) ; { // GL_LINE_STRIP				
									// draw the lines
									  glVertex3f (shift + coeff *  (curvePtr[p]->polyline[ring_ii] + 1) ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
									  glVertex3f (shift + coeff * (curvePtr[p]->polyline[ring_ii+=3] + 1) ,    (curvePtr[p])->polyline[nxtii+1],    (curvePtr[p])->polyline[nxtii+2]     ) ;
									}  glEnd ( ) ;
								}
								ii = nxtii; nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes); // precalculate next
							} // end while
						} // end of trace
						else if (curvePtr[p]->linetype == '.') { // points
						glPointSize (curvePtr[p]->width);
							while (curvePtr[p]->polyline[ring_ii] <= -1 + width_scan)  {
								if (curvePtr[p]->polyline[ring_ii+3] > -1) {
									glBegin ( GL_POINTS ) ; {	// GL_LINE_STRIP				
									// draw the point
									//  glVertex3f (shift + coeff *  (curvePtr[p]->polyline[ring_ii] +1) ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
									  glVertex3f (shift + coeff * (curvePtr[p]->polyline[ring_ii+=3] + 1) ,    (curvePtr[p])->polyline[nxtii+1],    (curvePtr[p])->polyline[nxtii+2]     ) ;
									}  glEnd ( ) ;
								}
								ii = nxtii; nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes);
							} // end while
						} // end of points
						else if (curvePtr[p]->linetype == '|') { // virtical lines
						glPointSize (curvePtr[p]->width);
							while (curvePtr[p]->polyline[ring_ii] <= -1 + width_scan)  {
								if (curvePtr[p]->polyline[ring_ii+3] > -1) {
									glBegin ( GL_LINES ) ; {	// GL_LINE_STRIP				
									// draw the drop line
									  glVertex3f (shift + coeff * (curvePtr[p]->polyline[ring_ii+=3] + 1) ,   0,  0  ) ;
									  glVertex3f (shift + coeff * (curvePtr[p]->polyline[ring_ii] + 1) ,    (curvePtr[p])->polyline[nxtii+1],    (curvePtr[p])->polyline[nxtii+2]     ) ;
									}  glEnd ( ) ;
								}
								ii = nxtii; nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes);
							} // end while
						} // end of virtical lines
/* would be nice to sort this out ... placement of balls at all the points
						else if (curvePtr[p]->linetype == 'b') { //  balls
							while (curvePtr[p]->polyline[ring_ii] <= -1 + width_scan)  {
								if (curvePtr[p]->polyline[ring_ii+3] > -1) {
								// draw the ball

								glPushMatrix ( ) ;
								glTranslated( shift + coeff * (curvePtr[p]->polyline[ring_ii] + 1) ,    (curvePtr[p])->polyline[nxtii+1],   (curvePtr[p])->polyline[nxtii+2]     ) ;
								glutSolidSphere ((GLdouble) 0.1 * curvePtr[p]->width , 20, 20) ;
								glPopMatrix ( ) ;
								}
								ii = nxtii; nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes);
							} // end while
						} // end balls
*/				
					} // end polygraph trace
				} // end simultrace == 0
				else if ((simultrace != 0) && (p > pdivcpy)) // and p > pdivcpy then display selected traces simultaneously
				{
					intensity = 2.0 * ((float) (p-pdivcpy)/ (float) (Num_curve-pdivcpy)  - (start_scan + 1.0)/2.0) / width_scan ; // in the range of 0.0 - 1.0 
					if ((intensity >= 0 )  && ( intensity <=  1.0 ) ) { // polyline is in the window so display it

//						if (scan_incr < 0) intensity = 1.0 - intensity; // if travelling back in time reverse persistence
//						else 
//						if (scan_incr == 0) intensity = 1.0; // if stopped give all curves equal brightness

						zdepth = ydepth = 0;
						if ( flat == 1 )  ydepth = 10.0* intensity - 5;
						else if ( flat == 2 ) zdepth = 10.0* intensity - 5;
						else if (scan_incr == 0) intensity = 1.0;

						intensity = sqrt(sqrt(intensity)); // make the fade out more sudden... compress the rest

						glLineWidth   ( 1.10 ) ; 
						 
					
						glColor3f  (intensity * curvePtr[p]->colour_red , intensity * curvePtr[p]->colour_green , intensity * curvePtr[p]->colour_blue ) ;

						ii=0;
						if (curvePtr[p]->linetype == '-') {
							glLineWidth   ( curvePtr[p]->width ) ; 
							if (curvePtr[p]->polyline) for (i=0; i < curvePtr[p]->npolylinenodes - 1 ; i++ ) { // draw the (curvePtr[p])
								glBegin ( GL_LINES ) ; { // GL_LINE_STRIP
								// draw the lines
								  glVertex3f ( curvePtr[p]->polyline[ii] ,    curvePtr[p]->polyline[ii+1] - ydepth ,   curvePtr[p]->polyline[ii+2] + zdepth    ) ;
								  glVertex3f ( curvePtr[p]->polyline[ii+3] ,    curvePtr[p]->polyline[ii+4] - ydepth,   curvePtr[p]->polyline[ii+5] + zdepth    ) ;
								}  glEnd ( ) ;
							ii+=3;
							}
						}
						else if (curvePtr[p]->linetype == '.') {
						glPointSize (curvePtr[p]->width);
							if (curvePtr[p]->polyline) for (i=0; i < curvePtr[p]->npolylinenodes ; i++ ) { // draw the (curvePtr[p])
								glBegin ( GL_POINTS ) ; { // points
								// draw the point
								  glVertex3f ( curvePtr[p]->polyline[ii] ,    curvePtr[p]->polyline[ii+1] - ydepth ,   curvePtr[p]->polyline[ii+2] + zdepth    ) ;
								}  glEnd ( ) ;
							ii+=3;
							}
						}
						else if (curvePtr[p]->linetype == '|') { // virtical line type
						glPointSize (curvePtr[p]->width);
							if (curvePtr[p]->polyline) for (i=0; i < curvePtr[p]->npolylinenodes ; i++ ) { // draw the (curvePtr[p])
								glBegin ( GL_LINES ) ; { // points
								// draw the line
								  glVertex3f ( curvePtr[p]->polyline[ii] ,  0 ,  0    ) ;
								  glVertex3f ( curvePtr[p]->polyline[ii] ,    curvePtr[p]->polyline[ii+1] - ydepth ,   curvePtr[p]->polyline[ii+2] + zdepth    ) ;
								}  glEnd ( ) ;
							ii+=3;
							}
						}

				

					} // end of polyline window.
				} // end of simul == 1
			} // end else
		} // end while (visble)
	} // end of curve treatment


	glLineWidth   ( 1.00 ) ;  

/*   surface treatment */


	if (( ((spinx >= 0) && (spinx <= 90)) || ((spinx > 270) && (spinx < 360)) ) && ((spiny >= 0) && (spiny <= 180))) {reversescandir = TRUE;}
	else {reversescandir = FALSE;}
	
	for (q=0; q < Num_surface; q++) {
		if (surfacePtr[q]->visible == GLUT_VISIBLE) {
			if ((surfacePtr[q]->kind == STATIC) || (surfacePtr[q]->kind == FULLYSPECIFIED)) {
				for (k=0; k<(surfacePtr[q])->npolygons; k++ ) {
				    if (reversescandir==TRUE) {i = (surfacePtr[q])->npolygons - 1 - k;}
				    else  {i = k;}

//				for (i=0; i<(surfacePtr[q])->npolygons; i++ ) {

				  	for (j=0; j<3; j++) {
						v1[j] = (surfacePtr[q])->polygon[i]->vertex[0]->xyz[j] ;
						v2[j] = (surfacePtr[q])->polygon[i]->vertex[1]->xyz[j] ;
						v3[j] = (surfacePtr[q])->polygon[i]->vertex[2]->xyz[j] ;
					}

		// rendering only if not all 3 polygon vertices in the zero plane
					if (( v1[2] != surfacePtr[q]->zoffset) || ( v2[2] != surfacePtr[q]->zoffset) || ( v3[2] != surfacePtr[q]->zoffset)) {
	
						if ( type ) { 
							if (surfacePtr[q]->kind == STATIC) { 
						// averaging z-values to estimate a colour for the polygon							
								if (v1[0] == v2[0])
									AvgColour =  v3[2]/2.0  + (v1[2] + v2[2])/4.0 - surfacePtr[q]->zoffset;
								
								else if (v2[0] == v3[0])
									AvgColour = v1[2]/2.0  + (v2[2] + v3[2])/4.0 - surfacePtr[q]->zoffset;
								
								else if (v3[0] == v1[0])
									AvgColour = v2[2]/2.0  + (v1[2] + v3[2])/4.0 - surfacePtr[q]->zoffset;
								glColor3f	( Glowr + ScaleIntensity*AvgColour ,  Glowg - ScaleIntensity*AvgColour , Glowb) ; // set the colour
							}
							else if (surfacePtr[q]->kind == FULLYSPECIFIED) { 
	//							glDepthMask(GL_FALSE) ;
								// basically i runs through the polygons... we could use this to call up the colour functions directly
								if (surfacePtr[q]->colour == 'p') glColor3f ((surfacePtr[q])->polygon[i]->rgb[0], (surfacePtr[q])->polygon[i]->rgb[1], (surfacePtr[q])->polygon[i]->rgb[2]);
								else glColor3f  (surfacePtr[q]->colour_red, surfacePtr[q]->colour_green, surfacePtr[q]->colour_blue);
							}
	
						} /* endif painted */
						else { 
							glColor3f  (1.0 ,  1.0 , 1.0) ; // colour of tesselation map
						}
		
		
						if ( type == 0 ) {
					
							for (j=0; j<3; j++) {
							  norm1[j] = (surfacePtr[q])->polygon[i]->vertex[0]->norm[j] ;
							  norm2[j] = (surfacePtr[q])->polygon[i]->vertex[1]->norm[j] ;
							  norm3[j] = (surfacePtr[q])->polygon[i]->vertex[2]->norm[j] ;
							}
							glBegin ( GL_LINE_LOOP ) ; {
							  glNormal3f ( norm1[0], norm1[1], norm1[2] ) ;
							  glVertex3f ( v1[0],    v1[1],    v1[2]    ) ;
							  glNormal3f ( norm2[0], norm2[1], norm2[2] ) ;
							  glVertex3f ( v2[0],    v2[1],    v2[2]    ) ;
							  glNormal3f ( norm3[0], norm3[1], norm3[2] ) ;
							  glVertex3f ( v3[0],    v3[1],    v3[2]    ) ;
							}  glEnd ( ) ;
		
						} /* end if drawn */
		
	
	
						if ( type == 1 ) {
					
							for (j=0; j<3; j++) {
							  norm1[j] = (surfacePtr[q])->polygon[i]->vertex[0]->norm[j] ;
							  norm2[j] = (surfacePtr[q])->polygon[i]->vertex[1]->norm[j] ;
							  norm3[j] = (surfacePtr[q])->polygon[i]->vertex[2]->norm[j] ;
							}
							glBegin ( GL_TRIANGLES ) ; {
							  glNormal3f ( norm1[0], norm1[1], norm1[2] ) ;
							  glVertex3f ( v1[0],    v1[1],    v1[2]    ) ;
							  glNormal3f ( norm2[0], norm2[1], norm2[2] ) ;
							  glVertex3f ( v2[0],    v2[1],    v2[2]    ) ;
							  glNormal3f ( norm3[0], norm3[1], norm3[2] ) ;
							  glVertex3f ( v3[0],    v3[1],    v3[2]    ) ;
							}  glEnd ( ) ;
						} /* end if drawn */
		
						if ( type == 2 ) { // smoothed
		
							averaged_vertex_normals ( surfacePtr[q], i, norm1, norm2, norm3 ) ;
							glBegin ( GL_TRIANGLES ) ; { 
							
							  glNormal3f ( norm1[0], norm1[1], norm1[2] ) ;
							  glVertex3f ( v1[0],    v1[1],    v1[2]    ) ;
							  glNormal3f ( norm2[0], norm2[1], norm2[2] ) ;
							  glVertex3f ( v2[0],    v2[1],    v2[2]    ) ;
							  glNormal3f ( norm3[0], norm3[1], norm3[2] ) ;
							  glVertex3f ( v3[0],    v3[1],    v3[2]    ) ;
							}  glEnd ( ) ;
		
						} /* end if smoothed */
		
					  
						if ( normals ) {
							for (j=0; j<3; j++) {
								vn1[j] = (surfacePtr[q])->polygon[i]->vertex[0]->xyz[j] + norm1[j] ;
								vn2[j] = (surfacePtr[q])->polygon[i]->vertex[1]->xyz[j] + norm2[j] ;
								vn3[j] = (surfacePtr[q])->polygon[i]->vertex[2]->xyz[j] + norm3[j] ;
							}
		
							glBegin ( GL_LINE_STRIP  ) ; { // GL_LINE_STRIP GL_LINES
								glVertex3f ( v1[0],    v1[1],    v1[2]     ) ;
								glVertex3f ( vn1[0],  vn1[1],    vn1[2]    ) ;
							}  glEnd ( ) ;
		
		
							glBegin ( GL_LINES ) ; {
								glVertex3f ( v2[0],    v2[1],    v2[2]     ) ;
								glVertex3f ( vn2[0],  vn2[1],    vn2[2]    ) ;
							}  glEnd ( ) ;
		
		
							glBegin ( GL_LINES ) ; {
								glVertex3f ( v3[0],    v3[1],    v3[2]     ) ;
								glVertex3f ( vn3[0],  vn3[1],    vn3[2]    ) ;
							}  glEnd ( ) ;
							
		
						} // if (normals)
					} // rendering if at least one polygon vertices not in the zero plane
				} // for (i=0; i<(surfacePtr[q])->npolygons; i++ ) 
			} // if ((surfacePtr[q])->kind == STATIC) 

/*						ring_scan = start_scan ; // + ring buffer pointer ; // allow ring buffer handling
						if (ring_scan > 1.0 ) ring_scan -= 2.0; else if (ring_scan < -1.0 ) ring_scan += 2.0 ; // modulo the scaning range
						ii = 3 * (int) ((float) curvePtr[p]->npolylinenodes * (1.0 + ring_scan )/2.0) ; // -1.0 <= ring_scan <= 1.0
						nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes); // precalculate the next, to cut down on processing
						ring_ii = 0 ; // temporary index references always from the start of the array, instead of with respect to the pointers
						shift = curvePtr[p]->xoffset -curvePtr[p]->xscale;
						coeff = 2*(curvePtr[p]->xscale) / width_scan;
						if (curvePtr[p]->linetype == '-') { // trace
							glLineWidth   ( curvePtr[p]->width ) ; 
							while ((curvePtr[p]->polyline[ring_ii] < -1 + width_scan) )  {
								if (curvePtr[p]->polyline[ring_ii+3] > -1) {
									glBegin ( GL_LINES ) ; { // GL_LINE_STRIP				
									// draw the lines
									  glVertex3f (shift + coeff *  (curvePtr[p]->polyline[ring_ii] + 1) ,    (curvePtr[p])->polyline[ii+1],    (curvePtr[p])->polyline[ii+2]     ) ;
									  glVertex3f (shift + coeff * (curvePtr[p]->polyline[ring_ii+=3] + 1) ,    (curvePtr[p])->polyline[nxtii+1],    (curvePtr[p])->polyline[nxtii+2]     ) ;
									}  glEnd ( ) ;
								}
								ii = nxtii; nxtii = (ii + 3) % (3 * curvePtr[p]->npolylinenodes); // precalculate next
											
							} // end while
						} // end of trace
*/
		} // VISIBLE
	} // for (q=0; q < Num_surface; q++) {
  glLineWidth   ( 1.0 ) ;  // reset line size
} 

/* ========================================================================= */  
/* paint ball.					     */
/* ========================================================================= */
/*
** As it says.
*/
void paint_ball ( LINE **curvePtr, BALL **ballPtr)
{
  int  	s, curv, pos;
 // long  ii,	nxtii, ring_ii;
  float	ring_scan, red, green, blue;
  char	label[LABEL_SIZE];


	s = 0;
	while ( ballPtr[s]->ball > -1 ) {
		curv = ballPtr[s]->ball;
		ring_scan = (start_scan  + 1.0 + width_scan/2.0)/2.0; 
		if (ring_scan >=0) ring_scan = (ring_scan - ((int) ring_scan)) * 2.0 -1.0;

		pos = (int) ((float) curvePtr[curv]->npolylinenodes * (1.0 + ring_scan )/2.0);
		pos = 3*pos;


		if (curvePtr[curv]->kind == COLOUR_FUNC) { //test ball placed at origion
			ballPtr[s]->xval = 0; ballPtr[s]->yval = 0; ballPtr[s]->zval = 0;
				glEnable		( GL_LIGHTING ) ;
				glPushMatrix ( ) ;
					
					glTranslated (ballPtr[s]->txt_xoffset,  ballPtr[s]->txt_yoffset, ballPtr[s]->txt_zoffset ) ;
						red = ballPtr[s]->xscale * curvePtr[curv]->polyline[pos] + ballPtr[s]->xoffset; 
						green = ballPtr[s]->yscale * curvePtr[curv]->polyline[pos+1] + ballPtr[s]->yoffset; 
						blue = ballPtr[s]->zscale * curvePtr[curv]->polyline[pos+2] + ballPtr[s]->zoffset; 
					//	if (red > 1.0) red = 1.0;																//need to think about this further
					//	else 
						if (red < 0.0) red = 0.0;
					//	if (green > 1.0) green = 1.0;
					//	else 
						if (green < 0.0) green = 0.0;
					//	if (blue > 1.0) blue = 1.0;
					//	else 
						if (blue < 0.0) blue = 0.0;
						glColor3f ( red, green, blue); // use values to give colour

					switch (ballPtr[s]->type) {
						case 's' :
							glutSolidSphere ((GLdouble) ballPtr[s]->size , 50, 50) ; // ball
						break;
						case 't' :
							glutSolidTeapot ((GLdouble) ballPtr[s]->size); //teaPot
						break;
						case 'b' :
							glutSolidCube ((GLdouble) ballPtr[s]->size);  //cube
						break;
						case 'c' :
							glutSolidCone  ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
						case 'd' :
							glutSolidTorus ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
					}  
			   glPopMatrix ( ) ;
			
		}
		else if (curvePtr[curv]->visible == GLUT_VISIBLE) {

			// caculate scaled/offset values to ball structure 
			ballPtr[s]->xval = ballPtr[s]->xscale * curvePtr[curv]->polyline[pos] + ballPtr[s]->xoffset;   // compute scaled / offset value x
			ballPtr[s]->yval = ballPtr[s]->yscale * curvePtr[curv]->polyline[pos+1] + ballPtr[s]->yoffset; // compute scaled / offset value y
			ballPtr[s]->zval = ballPtr[s]->zscale * curvePtr[curv]->polyline[pos+2] + ballPtr[s]->zoffset; // compute scaled / offset value z
			
			// what combination of outputs
			if ((ballPtr[s]->xscale) && (ballPtr[s]->yscale) && (ballPtr[s]->zscale)) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f][%0.3f][%0.3f]", ballPtr[s]->label, ballPtr[s]->xval, ballPtr[s]->yval, ballPtr[s]->zval );
			else if ((ballPtr[s]->xscale) && (ballPtr[s]->yscale)) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f][%0.3f]", ballPtr[s]->label, ballPtr[s]->xval, ballPtr[s]->yval );
			else if ((ballPtr[s]->xscale) && (ballPtr[s]->zscale)) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f][%0.3f]", ballPtr[s]->label, ballPtr[s]->xval, ballPtr[s]->zval );
			else if ((ballPtr[s]->yscale) && (ballPtr[s]->zscale)) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f][%0.3f]", ballPtr[s]->label, ballPtr[s]->yval, ballPtr[s]->zval );
			else if (ballPtr[s]->xscale) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f]", ballPtr[s]->label, ballPtr[s]->xval );
			else if (ballPtr[s]->yscale) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f]", ballPtr[s]->label, ballPtr[s]->yval );
			else if (ballPtr[s]->zscale) snprintf(&label[0], LABEL_SIZE, "%s [%0.3f]", ballPtr[s]->label, ballPtr[s]->zval );
			else  snprintf(&label[0], LABEL_SIZE, "%s", ballPtr[s]->label);
			
			
			if ((ballPtr[s]->txt_xoffset == 0) && (ballPtr[s]->txt_yoffset == 0) && (ballPtr[s]->txt_zoffset == 0) ) { // track the ball
					glColor3f ( 0.5, 0.5, 0.5);
					glDisable		( GL_LIGHTING ) ;

					if (curvePtr[curv]->kind == STATIC) {
						output(curvePtr[curv]->polyline[pos] + ballPtr[s]->size, curvePtr[curv]->polyline[pos+1] + ballPtr[s]->size, curvePtr[curv]->polyline[pos+2] + ballPtr[s]->size , label) ; 
					}
					else if (curvePtr[curv]->kind == DYNAMIC) {
						output( ballPtr[s]->size, curvePtr[curv]->polyline[pos+1] + ballPtr[s]->size, curvePtr[curv]->polyline[pos+2] + ballPtr[s]->size , label) ; 
					}
					glEnable		( GL_LIGHTING ) ;

			}
			else {
					glColor3f ( 0.5, 0.5, 0.5);
					glDisable		( GL_LIGHTING ) ;
				output(ballPtr[s]->txt_xoffset, ballPtr[s]->txt_yoffset, ballPtr[s]->txt_zoffset, label) ; // output at preset position
					glEnable		( GL_LIGHTING ) ;

				if ( ballPtr[s]->visible == GLUT_VISIBLE) { //  show if ball-visible is specifically set

				glPushMatrix ( ) ; 
					glTranslated 	(ballPtr[s]->txt_xoffset, ballPtr[s]->txt_yoffset, ballPtr[s]->txt_zoffset ) ;
					glColor3f ( ballPtr[s]->colour_red, ballPtr[s]->colour_green, ballPtr[s]->colour_blue);
					switch (ballPtr[s]->type) {
						case 's' :
							glutSolidSphere ((GLdouble) ballPtr[s]->size , 50, 50) ; // ball
						break;
						case 't' :
							glutSolidTeapot ((GLdouble) ballPtr[s]->size); //teaPot
						break;
						case 'b' :
							glutSolidCube ((GLdouble) ballPtr[s]->size);  //cube
						break;
						case 'c' :
							glutSolidCone  ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
						case 'd' :
							glutSolidTorus ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
					}
				glPopMatrix ( ) ;
				}
			}
				
		
				
			if ( ballPtr[s]->visible == GLUT_VISIBLE) { //  show if associated curve is visible or ball-visible has been specifically set
		
				glPushMatrix ( ) ; 
					if (curvePtr[curv]->kind == STATIC) glTranslated 	(curvePtr[curv]->polyline[pos],  curvePtr[curv]->polyline[pos+1], curvePtr[curv]->polyline[pos+2] ) ;
					else if (curvePtr[curv]->kind == DYNAMIC) glTranslated 	(0,  curvePtr[curv]->polyline[pos+1], curvePtr[curv]->polyline[pos+2] ) ;
					glColor3f ( ballPtr[s]->colour_red, ballPtr[s]->colour_green, ballPtr[s]->colour_blue);
					switch (ballPtr[s]->type) {
						case 's' :
							glutSolidSphere ((GLdouble) ballPtr[s]->size , 50, 50) ; // ball
						break;
						case 't' :
							glutSolidTeapot ((GLdouble) ballPtr[s]->size); //teaPot
						break;
						case 'b' :
							glutSolidCube ((GLdouble) ballPtr[s]->size);  //cube
						break;
						case 'c' :
							glutSolidCone  ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
						case 'd' :
							glutSolidTorus ((GLdouble) (ballPtr[s]->size)/2 , (GLdouble) ballPtr[s]->size, 50, 50) ;
						break;
					}  
			   glPopMatrix ( ) ;
			} //end if ball visible
		} // end if graph visible
		s++;
	} // while


}

/* ========================================================================= */  
/* calculate_polygon_vertex_normal.					     */
/* ========================================================================= */
/*
** As it says.
*/

void
calculate_polygon_vertex_normal ( SURF **surfacePtr, int Num_surface ) 
{
  long i,j,k ;
  float p1[3], p2[3], p3[3] ;
  float norm[3] ;
  int p;

for (p=0; p < Num_surface; p++) {
	if (surfacePtr[p]->visible == GLUT_VISIBLE) {
		  for (i=0; i<surfacePtr[p]->npolygons; i++ )
			{
			  for (j=0; j<3; j++) 
			p1[j] = surfacePtr[p]->polygon[i]->vertex[0]->xyz[j] ;
			  for (j=0; j<3; j++) 
			p2[j] = surfacePtr[p]->polygon[i]->vertex[1]->xyz[j] ;
			  for (j=0; j<3; j++) 
			p3[j] = surfacePtr[p]->polygon[i]->vertex[2]->xyz[j] ;

			  calc_normal ( p1, p2, p3, norm ) ;

			  for (j=0; j<3; j++) 
			for (k=0; k<3; k++)
			  surfacePtr[p]->polygon[i]->vertex[j]->norm[k] = norm[k] ;
			}
		}
	}
}

/* ========================================================================= */  
/* calc_normal.					     			     */
/* ========================================================================= */
/*
** Calculates the normal vector from three vertices.
*/
void
calc_normal ( float *p1, float *p2, float *p3, float *norm )
{
  float coa, cob, coc ;
  float px1, py1, pz1 ;
  float px2, py2, pz2 ;
  float px3, py3, pz3 ;
  
  float absvec ;
  
  px1 = p1[0] ;
  py1 = p1[1] ;
  pz1 = p1[2] ;
  
  px2 = p2[0] ;
  py2 = p2[1] ;
  pz2 = p2[2] ;
  
  px3 = p3[0] ;
  py3 = p3[1] ;
  pz3 = p3[2] ;
  
  coa = -(py1 * (pz2-pz3) + py2*(pz3-pz1) + py3*(pz1-pz2)) ;
  cob = -(pz1 * (px2-px3) + pz2*(px3-px1) + pz3*(px1-px2)) ;
  coc = -(px1 * (py2-py3) + px2*(py3-py1) + px3*(py1-py2)) ;
  
  absvec = sqrt ((double) ((coa*coa) + (cob*cob) + (coc*coc))) ;
  
  norm[0] = coa/absvec ;
  norm[1] = cob/absvec ;
  norm[2] = coc/absvec ;
}
