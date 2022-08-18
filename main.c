
/* ==========================================================================
                               MAIN_C
=============================================================================


============================================================================ */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <unistd.h>

#include "head.h"                 /* Local head data structure               */
#include "memory.h"

// <0 1.67><1 23.54><1 45.6><2 67.1>  channel 0 is in seconds * bufferlength/timescale = pointer
// then the channels will be filled accordingly with auto incrementing in necessary
// should automatically set the duration to 1 second, equivalent to D:0:1 


struct	input_frame // for real-time threaded serial input
{
	int		process_status;
	FILE	*fid;
	FILE	*outstream;
	int		echo;
	int		raw;
	float	time;
	int		rate;
};




double Glowr = 1.0, Glowg = 1.0, Glowb = 0.0, ScaleIntensity = 1.0;
double background_r = 0.0, background_g = 0.0, background_b = 0.0, complement = 0.0;
float limit = LIMIT;
float  sleepwake , sleep21 , sleepREM , sleepREMslope , awake, scale=0.5;
float start_scan = -1.0 , width_scan = 0.01, scan_incr = 0.0 ;
float xorig = DEF_ORG, yorig = DEF_ORG, zorig = DEF_ORG, xlimit = DEF_X, ylimit = DEF_Y , zlimit = DEF_Z, zaxis = DEF_ORG, yaxis = DEF_ORG, xaxis = DEF_ORG;
float grid_r = 1.0, grid_g = 1.0, grid_b =0.0; // grid colour
void print_mesg(void);

int DRAW_MODE = 2 ;

SURF *surfacePtr[NUM_SURF] ; // maximum number of surfaces
LINE *curvePtr[NUM_GRAPH] ; // maximum number of graphs
BALL *ballPtr[NUM_BALLS] ; // maximum no of balls
TXTLINE *textPtr[NUM_TXTLINES] ;  // maximum no of text lines
JUMP *jumpPtr[JUMP_SIZE];  // number of views

int maxJumpcntr=-1, Jumpcntr=-1;
int Num_curve = 0, Num_kurve = -1, Num_surface = 0, sleepStateFlag = 0, Num_texts = 0;
int Showlabels = 0, fullscreen = 0, simultrace = 0, flat = 0;
float Tracelight = 10.0;
float   trace = 0.0, tracex = -1.0, toggletrace = 0.0; 

float spinxlight = 0 ;
float spinylight = 0 ;
float spinxlight2 = 0 ;
float spinylight2 = 0 ;
float spinxlight3 = 0 ;
float spinylight3 = 0 ;
float spinxsurface = 0 ;
float spinysurface = 0 ;

float delay = 0.0, duration = -1;
int	realtime_rate = 0;

int FromFile = FALSE, newtitle = FALSE;
char *FileName = "/dev/stdin";
char title[TITLE_SIZE]; // any new title limited to TITLE_SIZE chars
int winSize_x = 800, winSize_y = 500 ; // default window size 
int winPos_x = -1 , winPos_y = -1; // default window position left to system to determine
int fixview = FALSE;

static struct input_frame frame;
static pthread_t serial_input_thread, display_graphics_thread;

/* ========================================================================= */
/* timing	                                                             */
/* ========================================================================= */  
/*
** returns number of lapsed seconds.
*/

GLdouble get_secs(void)
{
  return glutGet(GLUT_ELAPSED_TIME) / 1000.0;
}



/* ========================================================================= */
/* setview  orients axis to given angle	                                                             */
/* ========================================================================= */  
/*
** Rotate the surface and light about.
*/

void setview(int x_orientation, int y_orientation) {
	if ((x_orientation > -360) && (x_orientation < 360)) spinxsurface = (GLdouble) x_orientation; 
	else spinxsurface = 0;
	if ((y_orientation > -360) && (y_orientation < 360)) spinysurface = (GLdouble) y_orientation; 
	else spinysurface = 0;
}

void setlight_obj(int x_orientation, int y_orientation) {
	if ((x_orientation > -360) && (x_orientation < 360)) spinxlight = (GLdouble) x_orientation; 
	else spinxlight = 0;
	if ((y_orientation > -360) && (y_orientation < 360)) spinylight = (GLdouble) y_orientation; 
	else spinylight = 0;
}

void setlight_2(int x_orientation, int y_orientation) {
	if ((x_orientation > -360) && (x_orientation < 360)) spinxlight2 = (GLdouble) x_orientation; 
	else spinxlight2 = 0;
	if ((y_orientation > -360) && (y_orientation < 360)) spinylight2 = (GLdouble) y_orientation; 
	else spinylight2 = 0;
}

void setlight_3(int x_orientation, int y_orientation) {
	if ((x_orientation > -360) && (x_orientation < 360)) spinxlight3 = (GLdouble) x_orientation; 
	else spinxlight3 = 0;
	if ((y_orientation > -360) && (y_orientation < 360)) spinylight3 = (GLdouble) y_orientation; 
	else spinylight3 = 0;
}

/* ========================================================================= */
/* motion	                                                                 */
/* ========================================================================= */  
/*
** Rotate the surface and light about.
*/

int movestart = 0, rotate = 0, movelight = 0, moveangle = 0, origx, origy; 
GLdouble angle = 8.0;

void motion ( int x, int y )
{
  if (rotate) {
	movestart = movelight = 0;
    spinysurface = ( spinysurface + (GLdouble) (x - origx)/8.0 ) ;
		while (spinysurface > 360.0) spinysurface -= 360.0 ;
		while (spinysurface < 0.0) spinysurface += 360.0 ;
    spinxsurface = ( spinxsurface + (GLdouble) (y - origy)/4.0 ) ;
		while (spinxsurface > 360.0) spinxsurface -= 360.0 ;
		while (spinxsurface < 0.0) spinxsurface += 360.0 ;
    origx = x;
    origy = y;
    glutPostRedisplay();
  }
  if (movestart) {
	rotate = movelight = 0;
	start_scan = start_scan + (float) (x - origx )/600;
	if (start_scan + width_scan > 1.0) start_scan = 1.0 - width_scan;
	else if (start_scan < -1.0) start_scan = -1.0;
	origx = x;
    origy = y;
    glutPostRedisplay();
  }
  if (movelight) {
	rotate = movestart = 0;
	switch (movelight) {	
	  case 1:
	  		spinylight = ( spinylight + (GLdouble) (x - origx ) ) ;
				while (spinylight > 360.0) spinylight -= 360.0 ;
				while (spinylight < 0.0) spinylight += 360.0 ;
			spinxlight = ( spinxlight + (GLdouble) (y - origy ) ) ;
				while (spinxlight > 360.0) spinxlight -= 360.0 ;
				while (spinxlight < 0.0) spinxlight += 360.0 ;
		break;
	  case 2:
			spinylight2 = ( spinylight2 + (GLdouble) (x - origx ) ) ;
				while (spinylight2 > 360.0) spinylight2 -= 360.0 ;
				while (spinylight2 < 0.0) spinylight2 += 360.0 ;
			spinxlight2 = ( spinxlight2 + (GLdouble) (y - origy ) ) ;
				while (spinxlight2 > 360.0) spinxlight2 -= 360.0 ;
				while (spinxlight2 < 0.0) spinxlight2 += 360.0 ;
		break;
	  case 3:
			spinylight3 = ( spinylight3 + (GLdouble) (x - origx ) ) ;
				while (spinylight3 > 360.0) spinylight3 -= 360.0 ;
				while (spinylight3 < 0.0) spinylight3 += 360.0 ;
			spinxlight3 = ( spinxlight3 + (GLdouble) (y - origy ) ) ;
				while (spinxlight3 > 360.0) spinxlight3 -= 360.0 ;
				while (spinxlight3 < 0.0) spinxlight3 += 360.0 ;
		break;
	}
    origx = x;
    origy = y;
    glutPostRedisplay();
  }
  
  /* this was  test code.... need to get rid of this.
	if (moveangle) {
		moveangle = 0;
		angle = angle + (float) (x - origx );
		if (angle < 2.0) angle = (GLdouble) 2.0; if (angle > 60 ) angle = (GLdouble) 60.0;
		printf("%f\n", angle);
		origx = x;
    	origy = y;
		glutReshapeWindow ( winSize_x, winSize_y );
	}
	*/

}

/* ========================================================================= */
/* roll	                                                                 */
/* ========================================================================= */  
/*
** roll object.
*/

void
 roll(void)
 {
    spinysurface = ( spinysurface + (GLdouble) trace ) ;
		while (spinysurface > 360.0) spinysurface -= 360.0 ;
		while (spinysurface < 0.0) spinysurface += 360.0 ;
		if (start_scan + width_scan >= 1) { scan_incr = -scan_incr; }
		else if (start_scan <= -1) { scan_incr = -scan_incr; }
   glutPostRedisplay();
 }

/* ========================================================================= */
/* timed_scope	                                                                 */
/* ========================================================================= */  
/*
** real-time replay  animation
*/
void
 timed_scope(void)
 {
 	GLdouble seconds;

	if ( duration < 0 ) glutIdleFunc(NULL);
	else {
		seconds = get_secs();
		if (seconds < delay)  start_scan = -1.0;
		else start_scan = -1.0 - width_scan/2 + 2*(seconds-delay)/duration;
		if (start_scan + width_scan > 1) { start_scan = 1.0 - width_scan ; scan_incr = 0.0; glutIdleFunc(NULL); }
		else if (start_scan < -1) { start_scan = -1.0;  }
	}
	glutPostRedisplay();
	
 }

/* ========================================================================= */
/* real_scope	                                                                 */
/* ========================================================================= */  
/*
** real-time scope animation
*/


void
 real_scope(void)
 {
	if ( duration < 0 ) glutIdleFunc(NULL);
	else {
		start_scan = (2*frame.time / duration - width_scan -.01)-1.0; 
//		if (start_scan < -1.0) start_scan = -1.0; // this happens only at the begining of the session
	}
	glutPostRedisplay();
	
 }


/* ========================================================================= */
/* scope	                                                                 */
/* ========================================================================= */  
/*
** variable-rate scope animation
*/

void
 scope(void)
 {
	if ( start_scan += scan_incr) {
		if (start_scan + width_scan > 1) { start_scan = 1.0 - width_scan ; scan_incr = 0.0; glutIdleFunc(NULL); }
		else if (start_scan < -1) { start_scan = -1.0; scan_incr = 0.0;  glutIdleFunc(NULL); }
	}
	else glutIdleFunc(NULL);
	glutPostRedisplay();
	
 }



/* ========================================================================= */
/* roll & scope	                                                             */
/* ========================================================================= */  
/*
** both roll and scope actions together
*/


void
 both(void)
 {
    spinysurface = ( spinysurface + (GLdouble) trace ) ;
		while (spinysurface > 360.0) spinysurface -= 360.0 ;
		while (spinysurface < 0.0) spinysurface += 360.0 ;
    spinxsurface = ( spinxsurface + (GLdouble) tracex ) ;
		while (spinxsurface > 360.0) spinxsurface -= 360.0 ;
		while (spinxsurface < 0.0) spinxsurface += 360.0 ;
	if ( start_scan += scan_incr) {
		if (start_scan + width_scan > 1) { start_scan = 1.0 - width_scan; scan_incr = 0.0; glutIdleFunc(NULL); }
		else if (start_scan < -1) { start_scan = -1.0; scan_incr = 0.0;  glutIdleFunc(NULL); }
	}
	else glutIdleFunc(NULL);
	glutPostRedisplay();	
 }


/* ========================================================================= */
/* mouse action	                                                                 */
/* ========================================================================= */  
/*
** 
*/


void
mouse(int button, int state, int x, int y)
{
  switch(button) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
        origx = x;
        origy = y;
	    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) movelight = 3;
        else {
        	movelight = 0;
      		rotate = 1;
      	}
    } else {
      rotate = 0; moveangle = 0;
    }
    break;
  case GLUT_MIDDLE_BUTTON: // option-
    if (state == GLUT_DOWN) {
        origx = x;
        origy = y;
	    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) movelight = 2;
        else movelight = 1;
    } else {
      movelight = 0; moveangle = 0;
    }
    break;
  case GLUT_RIGHT_BUTTON: // ctrl-
    if (state == GLUT_DOWN) {
      origx = x;
      origy = y;
      if (GLUT_ACTIVE_SHIFT != glutGetModifiers()) movestart = 1;
		else moveangle = 1; 
	} else {
		movestart = 0; moveangle = 0;
	}
    break;
  }
}


/* ========================================================================= */
/* myinit		                                                     */
/* ========================================================================= */  
/*
** Do the lighting thing.
*/

void myinit ( void )
{
	int i;
	glEnable    ( GL_LIGHTING   ) ;
	glEnable    ( GL_LIGHT0     ) ;
	glDepthFunc ( GL_LEQUAL     ) ;
	glEnable    ( GL_DEPTH_TEST ) ;
	glColorMaterial ( GL_FRONT_AND_BACK , GL_AMBIENT_AND_DIFFUSE ); // GL_EMISSION ); 
	glEnable(GL_BLEND);
//	glCullFace(GL_FRONT );
//	glEnable(GL_CULL_FACE); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable    ( GL_COLOR_MATERIAL );

	if (maxJumpcntr == -1) { // if no jump states loaded, it sets the load state to the default jump state
		Jumpcntr = 0; maxJumpcntr = 0;
		jumpPtr[Jumpcntr] = _new ( JUMP ) ;
				 
		jumpPtr[Jumpcntr]->spinxsurface = spinxsurface;
		jumpPtr[Jumpcntr]->spinysurface = spinysurface;
		jumpPtr[Jumpcntr]->spinxlight = spinxlight;
		jumpPtr[Jumpcntr]->spinylight = spinylight;
		jumpPtr[Jumpcntr]->spinxlight2 = spinxlight2;
		jumpPtr[Jumpcntr]->spinylight2 = spinylight2;
		jumpPtr[Jumpcntr]->spinxlight3 = spinxlight3;
		jumpPtr[Jumpcntr]->spinylight3 = spinylight3;
		
		jumpPtr[Jumpcntr]->Glowr = Glowr;
		jumpPtr[Jumpcntr]->Glowg = Glowg;
		jumpPtr[Jumpcntr]->Glowb = Glowb;
		jumpPtr[Jumpcntr]->ScaleIntensity = ScaleIntensity;
		jumpPtr[Jumpcntr]->background_r = background_r;
		jumpPtr[Jumpcntr]->background_g = background_g;
		jumpPtr[Jumpcntr]->background_b = background_b;
		
		
		jumpPtr[Jumpcntr]->start_scan = start_scan;
		jumpPtr[Jumpcntr]->scan_incr = scan_incr;
		jumpPtr[Jumpcntr]->width_scan = width_scan;
		jumpPtr[Jumpcntr]->trace = trace;
		jumpPtr[Jumpcntr]->tracex = tracex;
		
		jumpPtr[Jumpcntr]->xaxis = xaxis;
		jumpPtr[Jumpcntr]->yaxis = yaxis;
		jumpPtr[Jumpcntr]->zaxis = zaxis;
		jumpPtr[Jumpcntr]->xlimit = xlimit;
		jumpPtr[Jumpcntr]->ylimit = ylimit;
		jumpPtr[Jumpcntr]->zlimit = zlimit;
		jumpPtr[Jumpcntr]->xorig = xorig;
		jumpPtr[Jumpcntr]->yorig = yorig;
		jumpPtr[Jumpcntr]->zorig = zorig;
		jumpPtr[Jumpcntr]->grid_r = grid_r;
		jumpPtr[Jumpcntr]->grid_g = grid_g;
		jumpPtr[Jumpcntr]->grid_b = grid_b;
		
		jumpPtr[Jumpcntr]->simultrace = simultrace;
		jumpPtr[Jumpcntr]->flat = flat;
		jumpPtr[Jumpcntr]->Showlabels = Showlabels;
		jumpPtr[Jumpcntr]->Tracelight = Tracelight;
		
		jumpPtr[Jumpcntr]->awake = awake;
		jumpPtr[Jumpcntr]->sleepwake = sleepwake;
		jumpPtr[Jumpcntr]->sleep21 = sleep21;
		jumpPtr[Jumpcntr]->sleepREM = sleepREM;
		
		for (i=0; i < 10; i++) { // only look at the first 10 objects
			if ((surfacePtr[i]) && (surfacePtr[i]->npolylinenodes)) {
				jumpPtr[Jumpcntr]->object[i] = i+1;
				jumpPtr[Jumpcntr]->visibility[i] = surfacePtr[i]->visible ;
			}
			else jumpPtr[Jumpcntr]->object[i] = 0;

			if (curvePtr[i]) {
				jumpPtr[Jumpcntr]->object[i+10] = i+1;
				jumpPtr[Jumpcntr]->visibility[i+10] = curvePtr[i]->visible ;
			}
			else jumpPtr[Jumpcntr]->object[i+10] = 0;
		} 

	}
}



/* ========================================================================= */
/* display                                                                   */
/* ========================================================================= */  
/*
** Here's were all the display action takes place.
*/

void display ( void )
{
  GLfloat position [] = { 30.0, 70.0, 10.0, 1.0 }  ;			// position of light ?
//	GLfloat Light_Ambient[]=  { 0.15f, 0.15f, 0.15f, 1.0f };
//	GLfloat Light_Diffuse[]=  { 1.0f, 1.0f, 1.0f, 1.0f }; 
    

  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  glPushMatrix ( ) ; 
  
	glTranslatef ( 0.0, 0.0, -90.0 ) ;							// position of viewer ? 

    glRotated	( (GLdouble) spinxsurface, 1.0, 0.0, 0.0 ) ;
    glRotated	( (GLdouble) spinysurface, 0.0, 1.0, 0.0 ) ;
  
    glPushMatrix ( ) ; 
      glRotated		( (GLdouble) spinxlight, 1.0, 0.0, 0.0 ) ;
      glRotated		( (GLdouble) spinylight, 0.0, 1.0, 0.0 ) ;
//  glShadeModel(GL_SMOOTH);
      glLightfv		( GL_LIGHT0, GL_POSITION, position ) ;
//   glLightfv(GL_LIGHT0, GL_AMBIENT,  Light_Ambient);
//   glLightfv(GL_LIGHT0, GL_DIFFUSE,  Light_Diffuse); 
   glPopMatrix ( ) ;


  calculate_polygon_vertex_normal ( surfacePtr, Num_surface ) ;

  paint_polygons ( surfacePtr, Num_surface, DRAW_MODE, 0, curvePtr, Num_curve, ballPtr, (int) spinxsurface, (int)  spinysurface) ;	


   glPushMatrix ( ) ; 
      glRotated		( (GLdouble) spinxlight2, 1.0, 0.0, 0.0 ) ;
      glRotated		( (GLdouble) spinylight2, 0.0, 1.0, 0.0 ) ;
//   glShadeModel(GL_SMOOTH);
     glLightfv		( GL_LIGHT0, GL_POSITION, position ) ;
//   glLightfv(GL_LIGHT0, GL_AMBIENT,  Light_Ambient);
//   glLightfv(GL_LIGHT0, GL_DIFFUSE,  Light_Diffuse); 
   glPopMatrix ( ) ; 

  paint_ball ( curvePtr, ballPtr) ;	


   glPushMatrix ( ) ; 
      glRotated		( (GLdouble) spinxlight3, 1.0, 0.0, 0.0 ) ;
      glRotated		( (GLdouble) spinylight3, 0.0, 1.0, 0.0 ) ;
      glLightfv		( GL_LIGHT0, GL_POSITION, position ) ;
   glPopMatrix ( ) ; 
   
  draw_axis ( zaxis, yaxis, xaxis, grid_r, grid_g, grid_b  );	

 
 
  screentext (curvePtr, Num_curve, textPtr, Num_texts);
 

  
  glPopMatrix();




  glutSwapBuffers();
}


/* ========================================================================= */
/* myReshape		                                                     */
/* ========================================================================= */  
/*
** What to do if the window is modified.
*/

void myReshape ( int w, int h )
{
  glViewport	( 0,0,w,h ) ;
  glMatrixMode  ( GL_PROJECTION ) ;
  glLoadIdentity( ) ;
//  gluPerspective( 8.0, (GLfloat) w/(GLfloat) h, 1.0, 200.0 ) ;
  gluPerspective( angle, (GLfloat) w/(GLfloat) h, 1.0, 200.0 ) ;
  glMatrixMode  ( GL_MODELVIEW ) ;
}


/* ========================================================================= */
/* error_exit	                                                             */
/* ========================================================================= */  
/*
** Problems!
*/

void error_exit( char *error_message )
{
    fprintf ( stderr, "%s\n", error_message ) ;
    exit( 1 ) ;
}

/* ========================================================================= */
/* generate the sleepstates from first trace								 */
/* ========================================================================= */  
/*
**
*/

#define ycoord -5
void generate_states(LINE **curvePtr, int from, int to)
{

int i, ii=0;

		   if ((curvePtr[from])->polyline) 
		   {
		   for (i=0; i < (curvePtr[from])->npolylinenodes ; i++ )  {// create the states
		   
/*				if (  ( curvePtr[from]->polyline[ii+2] < awake) || (curvePtr[from]->polyline[ii+5] < awake) )  { // isolate the disconnects

					(curvePtr[to])->polyline[ii] = (curvePtr[from])->polyline[ii]; (curvePtr[to])->polyline[ii+2] = -1*scale; (curvePtr[to])->polyline[ii+1] = ycoord;
				}
				else {
					if (sleepwake -((curvePtr[from])->polyline[ii+2] + (curvePtr[from])->polyline[ii+5])/2 < 0) {

						(curvePtr[to])->polyline[ii] = (curvePtr[from])->polyline[ii]; (curvePtr[to])->polyline[ii+2] = 0*scale; (curvePtr[to])->polyline[ii+1] = ycoord;
					}
					else if (sleep21 + ((curvePtr[from])->polyline[ii+1] + (curvePtr[from])->polyline[ii+4])/2 - ((curvePtr[from])->polyline[ii+2] + (curvePtr[from])->polyline[ii+5])/2  < 0) {
						if (sleepREM  + ((curvePtr[from])->polyline[ii+1] + (curvePtr[from])->polyline[ii+4])/2 - sleepREMslope *((curvePtr[from])->polyline[ii+2] + (curvePtr[from])->polyline[ii+5])/2  < 0) {

							(curvePtr[to])->polyline[ii] = (curvePtr[from])->polyline[ii]; (curvePtr[to])->polyline[ii+2] = 5*scale; (curvePtr[to])->polyline[ii+1] = ycoord;
						}
						else {

							(curvePtr[to])->polyline[ii] = (curvePtr[from])->polyline[ii]; (curvePtr[to])->polyline[ii+2] = 1*scale; (curvePtr[to])->polyline[ii+1] = ycoord;
						}
					}
					else {

						(curvePtr[to])->polyline[ii] = (curvePtr[from])->polyline[ii]; (curvePtr[to])->polyline[ii+2] = 2*scale; (curvePtr[to])->polyline[ii+1] = ycoord;
					}
				} */
				if (curvePtr[from]->polyline[ii+2] < awake ) {
					curvePtr[to]->polyline[ii] = curvePtr[from]->polyline[ii]; curvePtr[to]->polyline[ii+2] = -1*scale; curvePtr[to]->polyline[ii+1] = ycoord;
				}
				else if (sleepwake -(curvePtr[from])->polyline[ii+2] < 0) {
					curvePtr[to]->polyline[ii] = curvePtr[from]->polyline[ii]; curvePtr[to]->polyline[ii+2] = 0*scale; curvePtr[to]->polyline[ii+1] = ycoord;

				} // set blue for the awake
				else if (sleep21 + (curvePtr[from])->polyline[ii+1]  - (curvePtr[from])->polyline[ii+2]  < 0) {
					if (sleepREM  + (curvePtr[from])->polyline[ii+1]  - sleepREMslope * curvePtr[from]->polyline[ii+2]   < 0) {
						curvePtr[to]->polyline[ii] = curvePtr[from]->polyline[ii]; curvePtr[to]->polyline[ii+2] = 5*scale; curvePtr[to]->polyline[ii+1] = ycoord;
					}
					else {
						curvePtr[to]->polyline[ii] = curvePtr[from]->polyline[ii]; curvePtr[to]->polyline[ii+2] = 1*scale; curvePtr[to]->polyline[ii+1] = ycoord;
					}
				}
				else {
					curvePtr[to]->polyline[ii] = curvePtr[from]->polyline[ii]; curvePtr[to]->polyline[ii+2] = 2*scale; curvePtr[to]->polyline[ii+1] = ycoord;
				}

				ii+=3;

			}
			ii = 3;
			for (i=1; i < (curvePtr[from])->npolylinenodes ; i++ )  // change disconnect prior value to a disconnect
				{
				if (curvePtr[to]->polyline[ii+2] == -1*scale) curvePtr[to]->polyline[ii+2-3] = -1*scale;
				ii+=3;
				}
			ii = 3* (curvePtr[from]->npolylinenodes);
			for (i=curvePtr[from]->npolylinenodes; i > 0 ; i-- )  // change disconnect follower value to a disconnect
				{
				ii-=3;
				if (curvePtr[to]->polyline[ii+2-3] == -1*scale) curvePtr[to]->polyline[ii+2] = -1*scale;
				}

		   }
}


/* ========================================================================= */
/* usage		                                                     */
/* ========================================================================= */  
/*
** At startup provide usage modes.
*/

/*void usage( char *name )
{
	fprintf( stderr, "\n%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		"usage: ", name, " [options]\n\n",
		"  Options:\n",
		"    -display  displayname  specify an X server connection\n",
		"    -geometry geometry     specify window geometry in pixels\n",
		"    -rgba                  ask for rgba visual\n",
		"    -index                 ask for color index visual\n",
		"    -doublebuffer          ask for double buffered visual\n",
		"    -singlebuffer          ask for single buffered visual\n",
		"    -accum                 ask for accumulation buffer\n",
		"    -alpha                 ask for alpha buffer\n",
		"    -depth                 ask for depth buffer\n",
		"    -stencil               ask for stencil buffer\n",
		"    -aux nauxbuf           specify number of aux buffers\n",
		"    -level planes          specify planes (0=main,>0=overlay,<0=underlay\n",
		"    -transparent           ask for transparent overlay\n",
		"    -opaque                ask for opaque overlay\n"
	);
	
	exit( 1);
}
*/
/* ========================================================================= */
/* Key			                                                     */
/* ========================================================================= */
/*
** Actions on a key press.
*/

// static int m = 0, e = 0;

/* ARGSUSED1 */
static void Key ( unsigned char key, int x, int y )
{
//  char title[512];
	int i, rc;
	
    switch ( key ) {
      case 27 :

	exit (0) ;

//      case 'r' :
//      case 'R' :
//	  printf("Restore surface %ld \n", sizeof ( POLYGON ));
//		Glowr = 1.0 ; Glowg = 1.0 ; Glowb = 0.0 ;
//		glutPostRedisplay();
//	break;

    case '0' : // toggle surface
    case '9' : 	
    case '8' : 	
    case '7' :
		if ((int) key == (int) '0') i = 0;
		else i =  (int) '9' + 1 - (int) key ;	
		if ((surfacePtr[i]) && (surfacePtr[i]->npolylinenodes)) {
			if ((surfacePtr[i])->visible == GLUT_NOT_VISIBLE ) (surfacePtr[i])->visible = GLUT_VISIBLE ;
			else (surfacePtr[i])->visible = GLUT_NOT_VISIBLE ;
		}
		else {
			i = (int) key - (int) '1' ;
			if (i>=6) {
				if (curvePtr[i]) {
					if ((curvePtr[i])->visible == GLUT_NOT_VISIBLE ) (curvePtr[i])->visible = GLUT_VISIBLE ;
					else (curvePtr[i])->visible = GLUT_NOT_VISIBLE ;
				}
			}
		}
		glutPostRedisplay();
		break;
    case '1' : // toggle point/line objects
    case '2' : //
    case '3' : //
    case '4' : //
    case '5' : //
    case '6' : //
		i = (int) key - (int) '1' ;
		if (curvePtr[i]) {
			if ((curvePtr[i])->visible == GLUT_NOT_VISIBLE ) (curvePtr[i])->visible = GLUT_VISIBLE ;
			else (curvePtr[i])->visible = GLUT_NOT_VISIBLE ;
		}
		glutPostRedisplay();
		break;


	case 'w' : // sleep /wake
		if (-maxlimitz > (sleepwake -= 0.005)) {}//sleepwake = -maxlimitz;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'W' : // sleep /wake
		if (-maxlimitz > (sleepwake -= 0.1)) {}//sleepwake = -maxlimitz;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 's' : // sleep /wake
		if (maxlimitz < (sleepwake += 0.005)) {}//sleepwake = maxlimitz;
		sleepwake += 0.01;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'S' : // sleep /wake
		if (maxlimitz < (sleepwake += 0.1)) {}//sleepwake = maxlimitz;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'e' : // sleep12	  
		if (-maxlimity > (sleep21 -= 0.005)) {} //sleep21 = -maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'E' : // sleep12  
		if (-maxlimity > (sleep21 -= 0.1)) {}//sleep21 = -maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'd' : // sleep12
		if (maxlimity < (sleep21 += 0.005)) {}//sleep21 = maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'D' : // sleep12
		if (maxlimity < (sleep21 += 0.1)) {}//sleep21 = maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
		break;
	case 'r' : // sleep /wake	  
		if (-maxlimity > (sleepREM -= 0.005)) {}//sleepREM = -maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'R' : // sleep /wake	  
		if (-maxlimity > (sleepREM -= 0.1)) {}//sleepREM = -maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'f' : // sleep /wake
		if (maxlimity < (sleepREM += 0.005)) {}//sleepREM = maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'F' : // sleep /wake
		if (maxlimity < (sleepREM += 0.1)) {}//sleepREM = maxlimity;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'Q' : // quick set
		sleepwake = 3.2, sleep21 = 2.0, sleepREM = 0.45, sleepREMslope = 0.2;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'p' : // toggle background colour
		background_r = 1.0 - background_r;
		background_g = 1.0 - background_g;
		background_b = 1.0 - background_b;
		glClearColor( background_r, background_g, background_b, 1.0);
		glutPostRedisplay();
	break;
      case 'P' : // toggle background colour
		complement = 1.0 - complement;
		glutPostRedisplay();
	break;
      case 'l' : // show labels
		if (Showlabels) Showlabels = 0;
		else Showlabels = 1;
		glutPostRedisplay();
	break;
      case 'L' : // allow/disallow Light control of graphs and traces
		if (Tracelight==1.0 ) Tracelight = 10.0;
		else Tracelight = 1.0;
		glutPostRedisplay();
	break;
      case 'I' : // trace/simul
		if (simultrace == 0) simultrace = 1; 
		else  simultrace = 0; 
		glutPostRedisplay();
	break;
      case 'j' : // jump
		Jumpcntr++;
		if (Jumpcntr > maxJumpcntr) Jumpcntr = 0;			

		spinxsurface = jumpPtr[Jumpcntr]->spinxsurface;
		spinysurface = jumpPtr[Jumpcntr]->spinysurface;
		
		spinxlight = jumpPtr[Jumpcntr]->spinxlight;
		spinylight = jumpPtr[Jumpcntr]->spinylight;
		spinxlight2 = jumpPtr[Jumpcntr]->spinxlight2;
		spinylight2 = jumpPtr[Jumpcntr]->spinylight2;
		spinxlight3 = jumpPtr[Jumpcntr]->spinxlight3;
		spinylight3 = jumpPtr[Jumpcntr]->spinylight3;

		
		Glowr = jumpPtr[Jumpcntr]->Glowr;
		Glowg = jumpPtr[Jumpcntr]->Glowg;
		Glowb = jumpPtr[Jumpcntr]->Glowb;
		ScaleIntensity = jumpPtr[Jumpcntr]->ScaleIntensity;
		background_r = jumpPtr[Jumpcntr]->background_r;
		background_g = jumpPtr[Jumpcntr]->background_g;
		background_b = jumpPtr[Jumpcntr]->background_b;

		start_scan = jumpPtr[Jumpcntr]->start_scan;
		scan_incr = jumpPtr[Jumpcntr]->scan_incr;
		width_scan = jumpPtr[Jumpcntr]->width_scan;
		trace = jumpPtr[Jumpcntr]->trace;
		tracex = jumpPtr[Jumpcntr]->tracex;
		
	  	xaxis = jumpPtr[Jumpcntr]->xaxis;     
		yaxis = jumpPtr[Jumpcntr]->yaxis;     
		zaxis  = jumpPtr[Jumpcntr]->zaxis;     
		xlimit = jumpPtr[Jumpcntr]->xlimit;     
		ylimit = jumpPtr[Jumpcntr]->ylimit;     
		zlimit = jumpPtr[Jumpcntr]->zlimit;     
		xorig  = jumpPtr[Jumpcntr]->xorig;     
		yorig  = jumpPtr[Jumpcntr]->yorig;     
		zorig  = jumpPtr[Jumpcntr]->zorig;     
		grid_r = jumpPtr[Jumpcntr]->grid_r;     
		grid_g = jumpPtr[Jumpcntr]->grid_g;     
		grid_b = jumpPtr[Jumpcntr]->grid_b;     

		simultrace = jumpPtr[Jumpcntr]->simultrace;     
		flat = 	jumpPtr[Jumpcntr]->flat;  
		Showlabels = jumpPtr[Jumpcntr]->Showlabels;     
		Tracelight = jumpPtr[Jumpcntr]->Tracelight;
       
		awake = jumpPtr[Jumpcntr]->awake;     
		sleepwake = jumpPtr[Jumpcntr]->sleepwake;     
		sleep21 = jumpPtr[Jumpcntr]->sleep21;     
		sleepREM = jumpPtr[Jumpcntr]->sleepREM;
		
		for (i=0; i < 10; i++) { // only look at first ten objects
			if (jumpPtr[Jumpcntr]->object[i]) {
				if ((surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]) && (surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]->npolylinenodes)) 
					surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]->visible =  jumpPtr[Jumpcntr]->visibility[i];
			}
			if (jumpPtr[Jumpcntr]->object[i+10]) {
				if (curvePtr[jumpPtr[Jumpcntr]->object[i+10] - 1]) 
					curvePtr[jumpPtr[Jumpcntr]->object[i+10] - 1]->visible = jumpPtr[Jumpcntr]->visibility[i+10];
			}
		} 


		glutPostRedisplay();
	break;
      case 'J' : // jump
		Jumpcntr--;
		if (Jumpcntr < 0) Jumpcntr = maxJumpcntr;			

		spinxsurface = jumpPtr[Jumpcntr]->spinxsurface;
		spinysurface = jumpPtr[Jumpcntr]->spinysurface;
		
		spinxlight = jumpPtr[Jumpcntr]->spinxlight;
		spinylight = jumpPtr[Jumpcntr]->spinylight;
		spinxlight2 = jumpPtr[Jumpcntr]->spinxlight2;
		spinylight2 = jumpPtr[Jumpcntr]->spinylight2;
		spinxlight3 = jumpPtr[Jumpcntr]->spinxlight3;
		spinylight3 = jumpPtr[Jumpcntr]->spinylight3;

		
		Glowr = jumpPtr[Jumpcntr]->Glowr;
		Glowg = jumpPtr[Jumpcntr]->Glowg;
		Glowb = jumpPtr[Jumpcntr]->Glowb;
		ScaleIntensity = jumpPtr[Jumpcntr]->ScaleIntensity;
		background_r = jumpPtr[Jumpcntr]->background_r;
		background_g = jumpPtr[Jumpcntr]->background_g;
		background_b = jumpPtr[Jumpcntr]->background_b;

		start_scan = jumpPtr[Jumpcntr]->start_scan;
		scan_incr = jumpPtr[Jumpcntr]->scan_incr;
		width_scan = jumpPtr[Jumpcntr]->width_scan;
		trace = jumpPtr[Jumpcntr]->trace;
		tracex = jumpPtr[Jumpcntr]->tracex;
		
	  	xaxis = jumpPtr[Jumpcntr]->xaxis;     
		yaxis = jumpPtr[Jumpcntr]->yaxis;     
		zaxis  = jumpPtr[Jumpcntr]->zaxis;     
		xlimit = jumpPtr[Jumpcntr]->xlimit;     
		ylimit = jumpPtr[Jumpcntr]->ylimit;     
		zlimit = jumpPtr[Jumpcntr]->zlimit;     
		xorig  = jumpPtr[Jumpcntr]->xorig;     
		yorig  = jumpPtr[Jumpcntr]->yorig;     
		zorig  = jumpPtr[Jumpcntr]->zorig;     
		grid_r = jumpPtr[Jumpcntr]->grid_r;     
		grid_g = jumpPtr[Jumpcntr]->grid_g;     
		grid_b = jumpPtr[Jumpcntr]->grid_b;     

		simultrace = jumpPtr[Jumpcntr]->simultrace;     
		flat = 	jumpPtr[Jumpcntr]->flat;  
		Showlabels = jumpPtr[Jumpcntr]->Showlabels;     
		Tracelight = jumpPtr[Jumpcntr]->Tracelight;
       
		awake = jumpPtr[Jumpcntr]->awake;     
		sleepwake = jumpPtr[Jumpcntr]->sleepwake;     
		sleep21 = jumpPtr[Jumpcntr]->sleep21;     
		sleepREM = jumpPtr[Jumpcntr]->sleepREM;     

		for (i=0; i < 10; i++) { // only look at frst ten objects
			if (jumpPtr[Jumpcntr]->object[i]) {
				if ((surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]) && (surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]->npolylinenodes)) 
					surfacePtr[jumpPtr[Jumpcntr]->object[i] - 1]->visible =  jumpPtr[Jumpcntr]->visibility[i];
			}
			if (jumpPtr[Jumpcntr]->object[i+10]) {
				if (curvePtr[jumpPtr[Jumpcntr]->object[i+10] - 1]) 
					curvePtr[jumpPtr[Jumpcntr]->object[i+10] - 1]->visible = jumpPtr[Jumpcntr]->visibility[i+10];
			}
		} 

		glutPostRedisplay();
	break;


      case 'i' : // depth
		 flat++; if (flat >=3) {flat = 0;}
		glutPostRedisplay();
	break;


      case 'g' : // scope 
		if (width_scan < .0025) { // expanded scale
			if (scan_incr > .00001) { scan_incr = .000009; glutIdleFunc(scope);}
			else if (scan_incr < -.00001) {scan_incr = -.000009; glutIdleFunc(scope);}
			else {
				if (scan_incr += .000001) glutIdleFunc(scope);
				else glutIdleFunc(NULL); 
			}
		}
		else if (scan_incr += .00001) glutIdleFunc(scope); 
		else glutIdleFunc(NULL); 
	break;
      case 'G' : // scope
		if (width_scan < .0025) { // expanded scale
			if (scan_incr > .00001) { scan_incr = .000009; glutIdleFunc(scope);}
			else if (scan_incr < -.00001) {scan_incr = -.000009; glutIdleFunc(scope);}
			else {
				if (scan_incr -= .000001) glutIdleFunc(scope);
				else glutIdleFunc(NULL); 
			}
		}
		else if (scan_incr -= .00001) glutIdleFunc(scope); 
		else  glutIdleFunc(NULL); 
	break;


	  case 'Y' : // timed_scope 
			glutIdleFunc(timed_scope);
	break;	  
      case 'y' : // scope
			glutIdleFunc(NULL); 
	break;


      case 'h' : // scope
      	start_scan += width_scan/2.0;  // shift reference point to middle of screen 
		if (width_scan >= .0025) width_scan += .0025 ;
		else if  (width_scan >= .00025) width_scan += .00025 ;
		else if  (width_scan >= .000025) width_scan += .000025 ;
		start_scan -= width_scan/2.0; // shift back to left hand side of the screen... this keeps the middle stationary in respect of the window changes
		if (start_scan < -1.0) start_scan = -1.0; else if (start_scan + width_scan  > 1.0) start_scan = 1.0 - width_scan;
		glutPostRedisplay();
		break;
      case 'H' : // scope
      	start_scan += width_scan/2.0;
		if (width_scan > .005) width_scan -= .0025 ;
		else if (width_scan > .0005) width_scan -= .00025 ;
		else if (width_scan > .00005) width_scan -= .000025;
		start_scan -= width_scan/2.0; 
		if (start_scan < -1.0) start_scan = -1.0; else if (start_scan + width_scan  > 1.0) start_scan = 1.0 - width_scan;
		glutPostRedisplay();
	break;

      case 't' : // tumble
		toggletrace = 0.0;
		if (trace += .05) glutIdleFunc(roll); 
		else glutIdleFunc(NULL); 
	break;
      case 'T' : // tumble
		toggletrace = 0.0;
		if (trace -= .05) glutIdleFunc(roll); 
		else  glutIdleFunc(NULL); 
	break;

      case ' ' : // tumble off
		if (trace == 0.0) trace = toggletrace; 
		else if ((trace  > 0.0) && (toggletrace > 0.0 )) {trace = 0.0;}
		else  {toggletrace = trace;  trace = 0.0;}
		tracex = -1.0;
		glutIdleFunc(NULL);
	break;
      case 'B' : // both tumble and trace
		if (tracex < 0) tracex = 0.0;
		else tracex += 0.04;
		glutIdleFunc(both); 
	break;
	// the screen grid
      case '!' : // increase zaxis brightness
		if (zaxis <= .9) zaxis+=.1 ;
		glutPostRedisplay();
	break;
      case '#' : // increase yaxis brightness
		if (yaxis <= .9) yaxis+=.1 ;
		glutPostRedisplay();
	break;
      case '@' : // decrease zaxis brightness
		if (zaxis > 0.0 ) zaxis-=.1 ;
		glutPostRedisplay();
	break;
      case '$' : // decrease yaxis brightness
		if (yaxis > 0.0 ) yaxis-=.1 ;
		glutPostRedisplay();
	break;
      case '%' : // toggle main axis 
		if (xaxis == 0.0 ) xaxis =.7; 
		else xaxis =0.0;
		glutPostRedisplay();
	break;

	
      case 'o' :
	  // toggle sleepstate
//	  printf("%f %f %f %f\n", sleepwake, sleep21, sleepREM , sleepREMslope);
		if (Num_kurve != -1) { // a kurve has been loaded then
			switch (sleepStateFlag) {
				case 0 :
					(curvePtr[Num_curve])->polyline = ( float * ) malloc ( (curvePtr[Num_kurve])->npolylinenodes*3 * sizeof ( float )) ; //creating space first time only, same size as first line
					sleepStateFlag = 1; // then allows it to toggle between 2 and 1
					(curvePtr[Num_curve])->npolylinenodes = (curvePtr[Num_kurve])->npolylinenodes;
					(curvePtr[Num_curve])->linetype = '-';
					(curvePtr[Num_curve])->colour = 'w';
					(curvePtr[Num_curve])->kind = STATIC;
					(curvePtr[Num_curve])->visible = GLUT_VISIBLE ;
					strcpy((char *)  &(curvePtr[Num_curve])->label, (char *) "entropy-markup") ;
					generate_states(curvePtr, Num_kurve, Num_curve);
				break;
				case 1 :
					sleepStateFlag = 2;
				break ;
				case 2 :
					sleepStateFlag = 1;
				break;
			}
			glutPostRedisplay();
		}
		else sleepStateFlag = 0; // confirm that no sleepstates can be set. 				
	break;
      case 'O' : 
	  // output screen states
		if (sleepStateFlag) {
			generate_states(curvePtr, Num_kurve, Num_curve); //generate_states(curvePtr, 0, Num_curve);
			printf("z:w:entropyderivedstates\n%ld 1\n%f %f %f\n0.0 0.0 0.0\n", (curvePtr[Num_curve])->npolylinenodes, xlimit, scale, scale);
			for (i = 0; i < (curvePtr[Num_curve])->npolylinenodes; i++) printf("%d ", (int) ((curvePtr[Num_curve])->polyline[3*i+2]/scale));
			printf("\n");
		}
//		printf("W %d %d %d %d\n", winPos_x, winPos_y, winSize_x, winSize_y ); hasn't got hold of window data, tbd
		printf("\nO: %d:%d\n",(int) spinxsurface, (int) spinysurface);
		printf("L: %d:%d: %d:%d: %d:%d\n", (int) spinxlight, (int) spinylight, (int) spinxlight2, (int) spinylight2, (int) spinxlight3, (int) spinylight3);
		printf("C: %f:%f:%f: %f:%f:%f: %f\n",Glowr, Glowg, Glowb, ScaleIntensity, background_r, background_g, background_b);
		printf("G: %f:%f:%f: %f:%f:%f: %f:%f:%f: %f:%f:%f\n",xaxis, yaxis, zaxis, xlimit, ylimit, zlimit, xorig, yorig, zorig, grid_r, grid_g, grid_b);
		printf("M: %d:%d:%d: %f\nP %f %f %f %f\n", simultrace, flat, Showlabels, Tracelight, awake, sleepwake, sleep21, sleepREM  );
		printf("o: ");
		for (i=0; i < 10; i++) { // only first ten objects
			if ((surfacePtr[i]) && (surfacePtr[i]->npolylinenodes)) printf ("%d:%d ",i+1,(1==surfacePtr[i]->visible) );
			else printf ("0:0 ");
		} 
		for (i=0; i < 10; i++) {
			if (curvePtr[i]) printf ("%d:%d ",i+1,curvePtr[i]->visible );
			else printf ("0:0 ");
		} 
		printf("\n");

		printf("I: %f:%f:%f:%f:%f\n", start_scan, scan_incr, width_scan, trace, tracex); fflush(stdout);
		FILE *f; 
	  	f = fopen("/tmp/pos.spd", "w");
		fprintf(f,"I %f %f %f %f %f\n", start_scan, scan_incr, width_scan, trace, tracex);
	  	fclose(f);
		
	break;
	
      case 'a' :
		awake+=0.1;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;
      case 'A' :
		awake-=0.02;
		if (sleepStateFlag) generate_states(curvePtr, Num_kurve, Num_curve);
		glutPostRedisplay();
	break;

      case 'b' :
//      	DRAW_MODE = 0;
		DRAW_MODE++ ;
		if ( DRAW_MODE >= 3 ) DRAW_MODE = 0 ;
//	printf ("draw mode: %d\n", DRAW_MODE ) ; debug
	glutPostRedisplay();
	break;

      case '?' :
      
      fprintf ( stderr,"Mouse actions:\n");
      fprintf ( stderr,"  left btn :  rotate field of view\n");
      fprintf ( stderr,"  midl btn :  light source 1 angle\n");
      fprintf ( stderr,"  rght btn :  animation counter set\n");
      fprintf ( stderr,"  shft mdl :  light source 2 angle\n");
      fprintf ( stderr,"  shft lft :  light source 3 angle\n\n");
      
      fprintf ( stderr," Keyboard actions:\n");
      fprintf ( stderr,"   Object selection:\n");
      fprintf ( stderr,"    1-6 [7-0]  :  toggle select/deselect graph object\n");
      fprintf ( stderr,"    0-7 [6-0]  :  toggle select/deselect  surface object\n\n");

      fprintf ( stderr,"   animation:\n");
      fprintf ( stderr,"   <sp>    :  freeze animation display, reset rates.\n");
      fprintf ( stderr,"     g     :  play animation sequence forward rate increase \n");
      fprintf ( stderr,"     G     :  play animation sequence reverse rate increase \n");
      fprintf ( stderr,"     t     :  tumble forward rate increase\n");
      fprintf ( stderr,"     T     :  tumble forward rate decrease\n");
      fprintf ( stderr,"     B     :  combine tumble, roll and animation \n");
      fprintf ( stderr,"           :   (may be used in conjunction with t/T and g/G)\n");
      fprintf ( stderr,"     h     :  window width increase\n\n");

      fprintf ( stderr,"   Viewing options:\n");
      fprintf ( stderr,"     l     :  toggle labels display/inhibit\n");
      fprintf ( stderr,"     i     :  toggle scope mode,\n");
      fprintf ( stderr,"              i) without out motion ii) z-motion, iii) y-motion\n");
      fprintf ( stderr,"     b     :  toggle surface display mode:\n");
      fprintf ( stderr,"              i) triangular tessellations, \n");
      fprintf ( stderr,"              ii) colour gradiant, iii) smoothed (default) \n");
      fprintf ( stderr,"     H     :  window width decrease\n");
      fprintf ( stderr,"     I     :   i) display of X-graphs toggle simultaneous \n");
      fprintf ( stderr,"              ii) display  iii) animated display\n");
      fprintf ( stderr,"     L     :  toggle static graphs intensity \n");
      fprintf ( stderr,"              controlled/disabled, by light source 2 angle \n\n");
      fprintf ( stderr,"     p     :  toggle  background colour setting to complement\n");

      fprintf ( stderr,"   Grid viewing:\n");
      fprintf ( stderr,"     !     :  illuminated scale zx plane, brightness incr.\n");
      fprintf ( stderr,"     @     :  illuminated scale zx plane, brightness dec.\n");
      fprintf ( stderr,"     #     :  illuminated scale yx plane, brightness incr.\n");
      fprintf ( stderr,"     $     :  illuminated scale yx plane, brightness dec.\n");
      fprintf ( stderr,"     %%     :  toggle major scale axis\n\n");
      fprintf ( stderr,"     P     :  toggle  grid colour setting to complement\n");

      fprintf ( stderr,"   Surface colouring options:\n");
      fprintf ( stderr,"   up arow :  light intensity offset  increase\n");
      fprintf ( stderr,"   dn arow :  light intensity offset  decrease\n");
      fprintf ( stderr,"   rht  -> :  surface colour, red/green shift\n");
      fprintf ( stderr,"   lft  <- :  surface colour, green/red shift\n");
      fprintf ( stderr,"   shft -> :  blue light increase\n");
      fprintf ( stderr,"   shft <- :  blue light decrease\n\n");


      fprintf ( stderr,"   Coding options:\n");
      fprintf ( stderr,"     o     :  coding-markup computed/displayed\n");
      fprintf ( stderr,"    W/w    :  blue/yellow\n");
      fprintf ( stderr,"    S/s    :  yellow/blue\n");
      fprintf ( stderr,"    E/e    :  green/yellow\n");
      fprintf ( stderr,"    D/d    :  yellow/green\n");
      fprintf ( stderr,"    R/r    :  red/green\n");
      fprintf ( stderr,"    F/f    :  green/red\n");
      fprintf ( stderr,"     A     :  coarse white increase\n");
      fprintf ( stderr,"     a     :  fine while decrease white\n\n");


      fprintf ( stderr,"   Outout options:\n");
      fprintf ( stderr,"     O     :  display sleep staging \n\n");

      fprintf ( stderr,"   <esc>   :  exit display\n");
//% left btn: \n");
//% right btn (option + mouse) : \n");
//% middle btn (ctrl + mouse) : \n");

	break;

//      case 'c' :
//	glutPostRedisplay();
//	break;
	
      case 'n' : // re-new file load
        if (FromFile != TRUE) {
        	fprintf ( stderr,"\nNo file specified for input\n");
        } else {
        	fprintf ( stderr,"\nReloading data\n");
        	glutIdleFunc(NULL);
//        	for (i=Num_surface; i<=0; i--) 	surfacePtr[i]=0; // initialise	
//        	for (i=Num_curve; i<=0; i--) 	curvePtr[i]=0; // initialise	
//        	for (i=NUM_BALLS-1; i<=0; i--) 	ballPtr[i]=0; // initialise	
//        	Num_curve = 0; Num_kurve = -1; Num_surface = 0; sleepStateFlag = 0;

//		    read_array ( FileName, surfacePtr, curvePtr, ballPtr ) ; // read in the data

//		    if (newtitle) glutSetWindowTitle(title);  // 
		
		
//		    make_surface ( surfacePtr, Num_surface ) ; // out of mesh data
//			if (tracex >= 0.00) glutIdleFunc(both);
//			else if (trace != 0.00) glutIdleFunc(roll);
//			else if (scan_incr != 0.00) glutIdleFunc(scope);
//			else glutIdleFunc(NULL);

        
        }
	break;

//      case 'h' :
//	break;
		
    }
}

void
special(int key, int x, int y)
{

  switch(key) {
  case GLUT_KEY_RIGHT:
    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) {
    	Glowb += 0.1 ; Glowg -= 0.1 ;
    } else {
		Glowr -= 0.05 ; Glowg += 0.05 ;
	}
	glutPostRedisplay();
    break;
  case GLUT_KEY_LEFT:
    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) {
    	Glowb -= 0.1 ; Glowg += 0.1 ;
    } else {
		Glowr += 0.05 ; Glowg -= 0.05 ;
	}
	glutPostRedisplay();
    break;
  case GLUT_KEY_UP:
    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) {
    	ScaleIntensity += 0.05;
    } else {
		Glowr += 0.1 ; Glowg += 0.1 ;
	}
	glutPostRedisplay();
    break;
  case GLUT_KEY_DOWN:
    if (GLUT_ACTIVE_SHIFT == glutGetModifiers()) {
    	ScaleIntensity -= 0.05;
    } else {
		Glowr -= 0.1 ; Glowg -= 0.1 ;
	}
	glutPostRedisplay();
    break;
	
  case GLUT_KEY_PAGE_UP:
  	ScaleIntensity += 0.05;
	glutPostRedisplay();
    break;
	
  case GLUT_KEY_PAGE_DOWN:
  	ScaleIntensity -= 0.05;
	glutPostRedisplay();
    break;

  case GLUT_KEY_HOME:
	Glowb -= 0.1 ; Glowg += 0.1 ;
	glutPostRedisplay();
    break;

  case GLUT_KEY_END:
	Glowb += 0.1 ; Glowg -= 0.1 ;
	glutPostRedisplay();
    break;
  }
}


void
main_menu_select(int value)
{
  char title[512];

  switch(value) {
  case 1:
    glutPostRedisplay();
    break;
  case 2:
    break;
  case 3:
    glutPostRedisplay();
    break;
  case 4:
    glutPostRedisplay();
    break;
  case 5:
    glutSetWindowTitle(title);
    break;
  case 666:
    exit(0);
    break;
  }
}

void
draw_mode_select(int value)
{
  DRAW_MODE = value;
  glutPostRedisplay();
}

/*void
make_menus(void)
{
//  int i, j; //, my_menu, 
  int draw_mode_menu;
//  char *entry;

//  colour_menu = glutCreateMenu(colour_select);
//  for (i=0; i<surface->nmuscles; i++) {
//    entry = surface->muscle[i]->name;
//    for(j=(int) strlen(entry)-1; j>=0; j--) {
//      if (entry[j] == '_') entry[j] = ' ';
//    }
//    glutAddMenuEntry(entry, i);
//  }


  draw_mode_menu = glutCreateMenu(draw_mode_select);
  glutAddMenuEntry("first", 0);
  glutAddMenuEntry("second", 1);
  glutAddMenuEntry("third", 2);

//  glutAddSubMenu("Color", colour_menu);
  glutAddSubMenu("Draw mode - b", draw_mode_menu);
//  glutAddSubMenu("Display mode - shft-i", display_mode_menu); // dynamic display mode using i key
//  glutAddSubMenu("Labels - l", labels_mode_menu); // dynamic display mode using i key
//  glutAddSubMenu("Grid", labels_mode_menu); // dynamic display mode using i key
//  glutAddSubMenu("Output parameters", parameters_menu); // dynamic display mode using i key
  glutAddMenuEntry("print help", 2);
  glutAddMenuEntry("Quit", 666);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}
*/       



/****************************************************************************/
/*																			*/
/*  serial input (stdin) channel monitor									*/
/*																			*/
/****************************************************************************/
// one thing to do is to modify for variety of sampling rates... and to handle these in relation to the first channel's rate.
void *serial_input_monitor(void *threadarg)
{
	struct input_frame *my_frame;
	int	channels = 0, channel, done = 0, ii = 0;
	int	count=0, rate;
	float value; 
	char line[80];
     
	my_frame = (struct input_frame *) threadarg;
	
	while ((my_frame->process_status == TRUE) && (done != EOF )) { 
		done = fscanf(my_frame->fid, "%s", &line); 
		if ( !strcmp(line, "#channels")) break;
	}
	done = fscanf(my_frame->fid, "%d", &channels);
	
	
	while ((my_frame->process_status == TRUE) && (done != EOF )) { 
		done = fscanf(my_frame->fid, "%s", &line); 
		if ( !strcmp(line, "#stream")) break;
	}
	if (my_frame->echo) fprintf(my_frame->outstream, "\n#channels %d \n#stream", channels) ;
	
//	done = fgetc(my_frame->fid))); // get the following \n
//	if (done == \n) { // proceed only if 
		if (my_frame->raw == FALSE) {
			while ((my_frame->process_status == TRUE) && (done != EOF )) { 
				for (channel = 0; channel <= channels; channel++) {
					if (channel == 0) {
						done = fscanf(my_frame->fid, " %d",&count);
						if (realtime_rate) usleep(realtime_rate);
						if (my_frame->echo) { 
							fprintf(my_frame->outstream, "\n%d ", count); 
						}
					}
					else {
						done = fscanf(my_frame->fid, " %f",&value);
						if (my_frame->echo) {
							if (value ==0) fprintf(my_frame->outstream, "0 ");
							else if ( value - ((int) value) ) fprintf(my_frame->outstream, "%f ", value);
							else fprintf(my_frame->outstream, "%d ", (int) value);
						}
						if (channel <= Num_curve) { //insert data in the curve
							if (channel == 1) { 
								my_frame->rate = curvePtr[channel-1]->npolylinenodes/duration; // get the sample rate
								my_frame->time = ((float) count)/my_frame->rate; // convert the frame count to the time 
							}
							ii = 3 * ( count  % (int) curvePtr[channel-1]->npolylinenodes ) ;
							curvePtr[ channel-1 ]->polyline[ii+1]= curvePtr[ channel-1 ]->yoffset + value * curvePtr[channel-1]->yscale;
							curvePtr[ channel-1 ]->polyline[ii+2]= curvePtr[ channel-1 ]->zoffset + value * curvePtr[channel-1]->zscale; // in both dimensions... scale determines which 
						}
					}
					if (my_frame->echo) fflush(my_frame->outstream);
				}
			}
		} else if (my_frame->raw == TRUE) {
			while ((my_frame->process_status == TRUE) && (done != EOF )) { 
				done = fread( &channel , sizeof ( char ) , 1 ,my_frame->fid);
				done = fread( &value , sizeof ( float ) , 1 ,my_frame->fid);
				if (done==1) { 																// make sure we are in the range
					if (channel == 0) {
						count = (int) value; 
						if (realtime_rate) usleep(realtime_rate);
						if (my_frame->echo) { fprintf(my_frame->outstream, "\r %d ", channel, value);  fflush(my_frame->outstream); }
		
					}
					else if (channel <= Num_curve) {
						if (channel==1) { //insert data in the curve
							my_frame->rate = curvePtr[channel-1]->npolylinenodes/duration; // get the sample rate
							my_frame->time = ((float) count)/my_frame->rate; // convert the frame count to the time 
						}
						ii = 3 * ( count  % (int) curvePtr[channel-1]->npolylinenodes ) ;
						curvePtr[ channel-1 ]->polyline[ii+1]= curvePtr[ channel-1 ]->yoffset + value * curvePtr[channel-1]->yscale;
						curvePtr[ channel-1 ]->polyline[ii+2]= curvePtr[ channel-1 ]->zoffset + value * curvePtr[channel-1]->zscale; // in both dimensions... scale determines which 
						if (my_frame->echo) { fprintf(my_frame->outstream, "%d %f ", channel, value);  fflush(my_frame->outstream); }
					}
				}
		
			}
			
		}
		
//	}

	if (my_frame->echo) fprintf(my_frame->outstream, "\n"); 
	
	pthread_exit(NULL);
}



/****************************************************************************/
/*																			*/
/*  initiate the graphical display stuff									*/
/*																			*/
/****************************************************************************/

void	display_setup() {

	  glutInitWindowPosition ( winPos_x, winPos_y );
	  glutInitWindowSize	( winSize_x, winSize_y );
	  glutInitDisplayMode	(GLUT_ACCUM | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE );
	  
	  glutCreateWindow	( "2007 Space Odyssey:- by MRT" ) ;
	  
	  myinit		    ( ) ;
	  glClearColor( background_r, background_g, background_b, 1.0);

	
	  if (fullscreen) glutFullScreen(); // set by S:F object
	  else if (newtitle) glutSetWindowTitle(title);  // 
	
	
	  make_surface ( surfacePtr, Num_surface ) ; // out of mesh data
	
	  data_struct       ( surfacePtr, Num_surface ) ;	
	  glutMouseFunc     ( mouse );
	  glutMotionFunc    ( motion );
	  glutKeyboardFunc  ( Key ) ;	
	  glutSpecialFunc   ( special );
	  glutReshapeFunc 	( myReshape ) ;
	  glutDisplayFunc   ( display );

//		  make_menus();

	if (tracex >= 0.00) glutIdleFunc(both);
	else if (trace != 0.00) glutIdleFunc(roll);
	else if (scan_incr != 0.00) glutIdleFunc(scope);
	else glutIdleFunc(NULL);
}

/* ========================================================================= */
/* main			                                                    */
/* ========================================================================= */
/*
** All the initialization and action takes place here.
*/

int main ( int argc, char** argv )
{
  int i, echo = FALSE, raw = FALSE, textin = TRUE, supress = FALSE, mode = 0;
  char  Search_label[100];
  int	Search_count = 0, rc;
  FILE *InFile = NULL, *OutFile;

		// init the partition
		sleepwake = limit;
		sleep21 = limit;
		sleepREM = limit;
		sleepREMslope = 0.2; 
		awake = 0.0;

		frame.process_status=FALSE;
		
		glutInit(&argc, argv);
//		printf ("%d arg %s\n ",argc, argv[1]);

		strcpy(Search_label,"");

		for (i=0; i< NUM_SURF; i++) 	surfacePtr[i]=0; // initialise	...... why am I doing this ?????

		if (argc == 2) {
			FromFile = TRUE; // assume true until otherwise proven, any switch will reset this
			FileName = argv[1];
		}


		for(i=1; i<argc; i++) {
			if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "-file")) {
				FromFile = TRUE;
			  if (argc > (i+1)) {      
				FileName = argv[i + 1];
				i++;
			  }
			  else {
				printf("No filenname specified!\n");
				return -1;
			  }
		
			}
			else if ( !strcmp(argv[i], "-s") || !strcmp(argv[i], "-stream") ){ //  stream in
				mode = 1; // streaming mode
				if (argc == 2) FromFile = FALSE;
			}
			else if ( !strcmp(argv[i], "-txt") || !strcmp(argv[i], "-textout") ){ // textout
				raw = FALSE;
				if ((strlen(Search_label)==0) && (Search_count == 0)) echo = TRUE; // if not doing a search then echo all
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-raw") || !strcmp(argv[i], "-rawout") ){ //  raw out
				raw = TRUE;
				if ((strlen(Search_label)==0) && (Search_count == 0)) echo = TRUE; // if not doing a search then echo all
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-ri") || !strcmp(argv[i], "-rawin") ){ // text in
				textin = FALSE; // default is TRUE
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-q") || !strcmp(argv[i], "-quiet") ) { // quiet the display 
				supress = TRUE; // default is FALSE
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-g") || !strcmp(argv[i], "-graph") ) { // search for a graph 
      			sscanf(argv[i + 1], "%d", &Search_count);
      			i++;
				echo = FALSE; 	// selective echo
				supress = TRUE; // suppress graphics
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-l") || !strcmp(argv[i], "-label") ) { // search for a graph by label
				sscanf(argv[i + 1], "%s", &Search_label);
				i++;
				echo = FALSE; 	// selective echo
				supress = TRUE; // suppress graphics
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-rate") ) { // realtime rate
				sscanf(argv[i + 1], "%d", &realtime_rate);
				if (realtime_rate < 0) realtime_rate = 0;
				else realtime_rate = 1000000/realtime_rate; // microseconds delay
				i++;
				if (argc == 2) FromFile = FALSE;

			}
			else if ( !strcmp(argv[i], "-fixview") ) { // realtime rate
				fixview = TRUE; // default is TRUE

			}
		}

	switch (mode) {
	
		case 0: // non-streaming
	// reading in data
	
		if (FromFile = FALSE) strcpy ( FileName, "/dev/stdin");
		
		if (( InFile = fopen ( FileName, "r" )) == 0 ) {
			fprintf ( stderr, "can't open input file: %s\n", FileName ) ;
			exit (-1) ;
		}
	
		read_array ( InFile, surfacePtr, curvePtr, ballPtr, textPtr, textin, echo, raw, Search_label , Search_count, NULL) ; // read in the objects
		fclose ( InFile ) ;
	
	// display stuff
			if ( supress == FALSE ) { // may use spod for simply processing file formats... in whih case we suppress display
				display_setup();
				glutMainLoop() ;
	
			}

			break;
		
		case 1: // streaming
				if (( InFile = fopen ( "/dev/stdin", "r" )) == 0 ) {
					fprintf ( stderr, "> can't open input stream:\n");
					exit (-1) ;
				}

		  		read_array ( InFile, surfacePtr, curvePtr, ballPtr, textPtr, textin, echo, raw, Search_label , Search_count, NULL) ; // read in the objects


				if (echo) {
					OutFile = fopen("/dev/stdout","w");  //open standard out 
    				if (!OutFile) exit (-1) ;
    				fprintf ( OutFile, "Q\n"); fflush(OutFile);
    			}
    			else OutFile = NULL;

				frame.echo = echo;
				frame.raw = raw;
				frame.outstream = OutFile;
				
				frame.process_status = TRUE;
				frame.fid = InFile;
				rc = pthread_create(&serial_input_thread, NULL, serial_input_monitor, (void *) &frame);
				supress = TRUE; //suppress the normal display.... do it in a thread instead
				if (rc) {
					fprintf(stderr, "> Return code from pthread_create() is %d\n", rc);
					exit(-1);
				}

				
				display_setup();
				glutIdleFunc(real_scope);
				glutMainLoop() ;
				frame.process_status = FALSE;
				sleep(2); // slight delay to give the thread a chance to quite before...
				fclose(InFile);
				fclose(OutFile);


			break;
	}

		// need to free memory  .... is this really good enough?
	
	for (i = 1; i < Num_surface; i++) if (surfacePtr[i] != NULL) free(surfacePtr[i]) ; 
	for (i = 1; i < Num_curve; i++) if (curvePtr[i] != NULL) free(curvePtr[i]) ; 
	for (i = 1; i < NUM_BALLS; i++) if (ballPtr[i] != NULL) free(ballPtr[i]) ; 
	for (i = 1; i < NUM_TXTLINES; i++) if (textPtr[i] != NULL) free(textPtr[i]) ; 
	for (i = 1; i < JUMP_SIZE; i++) if (jumpPtr[i] != NULL) free(jumpPtr[i]) ; 
	
	exit(0); // exiting		
}


