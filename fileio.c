/*==========================================================================
                               FILEIO_C
============================================================================*/

#include <math.h>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#include <GL/glut.h>

/* 
 * from /usr/include/sys/types.h
 * Just in case TRUE and FALSE are not defined
 */

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif



#include "head.h"		/* local header for the data structures  */
//#include "memory.h"

float scalex=DEF_X, scaley=1.0, scalez=1.0, coords;
float maxlimity, maxlimitz;

int pdiv = -1; // counts number of curves loaded




/* Select_count alternatives */
#define HEADER -1
#define ENVIRONMENT -2

/*========================================================================== 
 print label with escapes													
============================================================================*/  

static void print_label(FILE * file, char * prefix, char *label, char * suffix)
{
	int i;
	if (strlen(prefix)) fprintf(file, "%s", prefix );
	for (i=0; i < strlen(label); i++) { 
		if ((label[i] == ' ') || (label[i] == ':') || (label[i] == '"')) 
			fputc('\\', file); fputc(label[i], file);
	}
	if (strlen(suffix)) fprintf(file, "%s", suffix);
}


/*========================================================================== 
 read label	 
============================================================================*/  


static char read_label( FILE *InFile, char * label) 
{
	int i = 0, done; 
	
	label[0] = 0;
	while (((done = fgetc(InFile)) != EOF)  && (done != '"')  && (' ' != done) && ('\r' != done)  && ('\n' != done ) && (':' != done )) { 
		if (done == '\\') { //escape, get another character
			if (((done = fgetc(InFile)) == EOF ) || ('\n' == done )) {
				label[i] = 0;   
				return (done);
			}
		} 
		label[i++] = done; // store character
		label[i] = 0; // terminate string
	}
	return (done);
}


/*==========================================================================
  print spod header
============================================================================*/  
static int print_header(FILE *OutStream, char key, char colour, char *label, int index , int part , float scalex, float scaley, float scalez, float offsetx, float offsety, float offsetz)
{
	if (OutStream) {
		if (colour != 0) fprintf(OutStream,"%c:%c", key, colour); 
		else fprintf(OutStream,"%c", key);

		print_label(OutStream, ":", label,":");
		fprintf(OutStream,"%d:%d:%g:%g:%g:%g:%g:%g\n", index , part , scalex, scaley, scalez, offsetx, offsety, offsetz);
		return (TRUE);
	}
	else return (FALSE);
}


/*==========================================================================
  read till terminating character in list
============================================================================*/  
static int read_till(FILE * input, char *characters)
{
	int i;
	int inputc; 
	do {
		if (EOF == (inputc = fgetc(input))) { 
			return( inputc );
 			}
		if (inputc == '\\') { //escape character  skip to next 
			if (EOF == (inputc = fgetc(input))) {
				return( inputc );
			}
			i = strlen(characters); // force loop back
		}
		else {
			i = 0;
			while ((i < strlen(characters)) && (inputc != characters[i])) {i++;}
		}
	} while  ( i == strlen(characters) );
	
	return (inputc);
}


/*=========================================================================
  set raw switch	
===========================================================================*/  

static int Set_raw_switch(FILE *OutStream, int echo, int raw)
{
	if (OutStream) {
		if (echo) {
			if (raw) fprintf(OutStream,"r:n:raw_data_on\n");  //  set raw data switch
			else fprintf(OutStream,"r:f:text\n");  //  set raw data switch
		}
		return (TRUE);
	}
	else return (FALSE);
}



static int getint(FILE *InFile, long *valuePtr) {
	int done;
	
	done = fscanf ( InFile, " %d", valuePtr ) ;
	if ( done == 1) return(done);

	done = read_till(InFile, " :\r\n") ;
	if (done == EOF) return(done);
	done = fscanf ( InFile, " %d", valuePtr ) ;
	return (done);
}

static int getfloat(FILE *InFile, float *valuePtr) {
	int done;
	
	done = fscanf ( InFile, " %f", valuePtr ) ;
	if ( done == 1) return(done);

	done = read_till(InFile, " :\r\n") ;
	if (done == EOF) return(done);
	done = fscanf ( InFile, " %f", valuePtr ) ;
	return (done);

}

/*==========================================================================
  read graph
============================================================================*/  
/* if buffer pointer == NULL then don't siphon data off 					*/
/* else leave a copy of the data in buffer pointed to by BuffPtr 			*/

static char read_graph ( char key, int p, FILE *InFile, LINE **curvePtr, int Object_count, int textin, FILE *OutStream, int echo, int raw, char *Search_labelPtr, int Search_count, LINE **BufferPtrPtr )
{


int   done;
int i, j;
long x, y, ii,  part = 0, index = 0;
float offsetx, offsety, offsetz, xmin, ymin, zmin, xmax, ymax, zmax;




	if(Search_count > 0) {
		if (Search_count == Object_count) Set_raw_switch(OutStream, echo = TRUE, raw); //  output a raw data switch to reflect the settings
		else echo = FALSE;
	}
	else if (Search_count < 0) { // just collecting headers 
		echo = TRUE;
	}
	
	
	done = read_till(InFile, ":");
	if (done != EOF) { 								// got a valid separator
		if ( EOF != (done = fgetc(InFile))) {
			if ((done == 'r') || (done == 'g') || (done == 'b') || (done == 'm') || (done == 'c') || (done == 'y') || (done == 'B') || (done == 'W') || (done == 'w') || (done == 'p') ) { // got a valid colour 

				curvePtr[p]->colour = done;
				
				// here is a todo tag:   make this in to a routine....
				switch (done) {
					case 'r': //red
						curvePtr[p]->colour_red = 1.0; curvePtr[p]->colour_green = 0.0; curvePtr[p]->colour_blue = 0.0;
						break;
					case 'g': //green
						curvePtr[p]->colour_red = 0.0; curvePtr[p]->colour_green = 1.0; curvePtr[p]->colour_blue = 0.0;
						break;
					case 'b': //blue
						curvePtr[p]->colour_red = 0.0; curvePtr[p]->colour_green = 0.0; curvePtr[p]->colour_blue = 1.0;
						break;
					case 'm': //magenta
						curvePtr[p]->colour_red = 1.0; curvePtr[p]->colour_green = 0.0; curvePtr[p]->colour_blue = 1.0;
						break;
					case 'c': //cyan
						curvePtr[p]->colour_red = 0.0; curvePtr[p]->colour_green = 1.0; curvePtr[p]->colour_blue = 1.0;
						break;
					case 'y': //yellow
						curvePtr[p]->colour_red = 1.0; curvePtr[p]->colour_green = 1.0; curvePtr[p]->colour_blue = 0.0;
						break;
					case 'B': //black
						curvePtr[p]->colour_red = 0.0; curvePtr[p]->colour_green = 0.0; curvePtr[p]->colour_blue = 0.0;
						break;
					case 'W': //white
					case 'w': //white
						curvePtr[p]->colour_red = 1.0; curvePtr[p]->colour_green = 1.0; curvePtr[p]->colour_blue = 1.0;
						break;
				}
				
				if ((key == 'X') || (key == 'Y') || (key == 'Z') || (key == 'K') || (key == 'F')) { curvePtr[p]->kind = DYNAMIC;}  
				else if ((key == 'x') || (key == 'y') || (key == 'z') || (key == 'k') || (key == 'f')) { curvePtr[p]->kind = STATIC;}
//	 fprintf (stderr,"graph type %c, kind %d\n", key, curvePtr[p]->kind); // debug to find why 'p' and 'P' were not recognised
				
				done = read_till(InFile, " :");  // get second :
				if (done == ' ' ) while ((done !=EOF) && ('"' != (done = fgetc(InFile)))) {}  // if ' ' search for opening '"':
				if ((done == '"' ) || (done == ':' )) { 	// for either opening

					done = read_label(InFile, curvePtr[p]->label); // read in label
					
					if (strlen(Search_labelPtr) ) { // search by label ?
						echo = FALSE; // preset
						if (strlen(Search_labelPtr) == strlen(curvePtr[p]->label)) { // if lengths same, check content
							if ( strcmp(Search_labelPtr, curvePtr[p]->label ) == 0) {
								echo=TRUE;
								Set_raw_switch(OutStream, echo, raw); //  output a raw data switch to reflect the settings
							}
						}
					}
					
					if (done != EOF) {  // now get the index part scale and offsets

						if (EOF != getint(InFile, &index)) {
							if (EOF != getint(InFile, &part)) {
								if (EOF != getfloat(InFile, &scalex )) { 
									if (EOF != getfloat(InFile,  &scaley )) {
										if (EOF != getfloat(InFile,  &scalez )) {
											if (EOF != getfloat(InFile,  &offsetx )) {
												if (EOF != getfloat(InFile,  &offsety )) {
													if (EOF != getfloat(InFile,  &offsetz )) {


														xmin = ymin = zmin = 1000; xmax = ymax = zmax = -1000; // start large

														curvePtr[p]->xscale = scalex ;  
														curvePtr[p]->yscale = scaley ;  
														curvePtr[p]->zscale = scalez ;  
														curvePtr[p]->xoffset = offsetx ;  
														curvePtr[p]->yoffset = offsety ;  
														curvePtr[p]->zoffset = offsetz ;  
													
														curvePtr[p]->part = part ;  // should always be 1 !
														curvePtr[p]->index = index ;  


														if ((p < 6) && (p > 0)) curvePtr[p]->visible = GLUT_NOT_VISIBLE ;
														else curvePtr[p]->visible = GLUT_VISIBLE ; 
														if ((key == 'x') ||(key == 'X')) curvePtr[p]->visible = GLUT_VISIBLE ;  // default all x: and X: to be visible

														(curvePtr[p])->npolylinenodes = index ;	// assumed that part should always be 1 //
														(curvePtr[p])->polyline = ( float * ) malloc ( (curvePtr[p])->npolylinenodes*3 * sizeof ( float )) ; //creating space



														if (echo) { 
															if ((BufferPtrPtr != NULL)  && ((Search_count > 0 ) || ( strlen(Search_labelPtr)  > 0 ))) *BufferPtrPtr =  curvePtr[p] ;
															print_header(OutStream, key, curvePtr[p]->colour, curvePtr[p]->label, index , 1 , scalex, scaley, scalez, offsetx, offsety, offsetz);
														}

														switch (key) {
															case 'h' : // colour function  series [x,y,z] three rows [red_i],[green_i],[blue_i]
																curvePtr[p]->visible = GLUT_NOT_VISIBLE; 
																curvePtr[p]->kind = COLOUR_FUNC;
															case 'x' : // graph static curve from series [x,y,z] three rows [x_i],[y_i],[z_i]
															case 'X' : // only difference is in the display.... uses the scan features to display a subspace of the curves
															
															
																// reading
																if (textin==TRUE)  { // read ascii floats three rows x[], y[], z[]
															
																	for ( j=ii=0; j<index; j++, ii+=3) { // get the x values
																		done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii] ) ; 				// get the data in
																		if ( curvePtr[p]->polyline[ii] < xmin ) xmin = curvePtr[p]->polyline[ii]; // get min
																		if ( curvePtr[p]->polyline[ii] > xmax ) xmax = curvePtr[p]->polyline[ii]; // get max
																	}
																	for ( j=ii=0; j<index; j++, ii+=3) { // get the y values
																		done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii+1] ) ;
																		if ( curvePtr[p]->polyline[ii+1] < ymin ) ymin = curvePtr[p]->polyline[ii+1]; // get min
																		if ( curvePtr[p]->polyline[ii+1] > ymax ) ymax = curvePtr[p]->polyline[ii+1]; // get max
																	}
																	for (j=ii=0; j<index; j++, ii+=3) { // get the z values
																		done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii+2] ) ;
																		if ( curvePtr[p]->polyline[ii+2] < zmin ) zmin = curvePtr[p]->polyline[ii+2]; // get min
																		if ( curvePtr[p]->polyline[ii+2] > zmax ) zmax = curvePtr[p]->polyline[ii+2]; // get max
																	}
																}
																else { // read raw  data
																	done = read_till(InFile, "\r\n"); //look for new line
																	if (done != EOF) fread( (curvePtr[p])->polyline , 3 * sizeof ( float ) , (curvePtr[p])->npolylinenodes ,InFile);
																	
																	for ( j=ii=0; j<index; j++, ii+=3) { // derive min max 
																		if ( curvePtr[p]->polyline[ii] < xmin ) xmin = curvePtr[p]->polyline[ii]; // get min
																		if ( curvePtr[p]->polyline[ii] > xmax ) xmax = curvePtr[p]->polyline[ii]; // get max
																		if ( curvePtr[p]->polyline[ii+1] < ymin ) ymin = curvePtr[p]->polyline[ii+1]; // get min
																		if ( curvePtr[p]->polyline[ii+1] > ymax ) ymax = curvePtr[p]->polyline[ii+1]; // get max
																		if ( curvePtr[p]->polyline[ii+2] < zmin ) zmin = curvePtr[p]->polyline[ii+2]; // get min
																		if ( curvePtr[p]->polyline[ii+2] > zmax ) zmax = curvePtr[p]->polyline[ii+2]; // get max
																	}
																}
																
																curvePtr[p]->xmax = xmax; curvePtr[p]->xmin = xmin ;
																curvePtr[p]->ymax = ymax; curvePtr[p]->ymin = ymin;
																curvePtr[p]->zmax = zmax; curvePtr[p]->zmin = zmin ;

																
																if ((echo) && (Search_count >= 0)) {
																	if (raw==FALSE) {
																		for ( j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii] ) ; //  echo x values
																		fputc('\n',OutStream);		// echo retn
																		for ( j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii+1] ) ; // echo y
																		fputc('\n',OutStream);		// echo retn
																		for (j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii+2] ) ; // echo z
																		fputc('\n',OutStream);		// echo retn
																	}
																	else {
																		fwrite( (curvePtr[p])->polyline, 3 * sizeof ( float ) , (curvePtr[p])->npolylinenodes, OutStream);
																		fputc('\n',OutStream);		// echo retn
																	}

																}

																//scaling and offsets

																if (BufferPtrPtr == NULL) { // don't bother with this if  buffer enabled or if have colour function
																	for ( j=ii=0; j<index; j++, ii+=3) {
																		(curvePtr[p])->polyline[ii]= offsetx + (curvePtr[p])->polyline[ii]*scalex; 
																		(curvePtr[p])->polyline[ii+1]= offsety + (curvePtr[p])->polyline[ii+1]*scaley; 
																		(curvePtr[p])->polyline[ii+2]= offsetz + (curvePtr[p])->polyline[ii+2]*scalez;
																	}
																}
																	
																break;

															case 'f' : // formated space for real-time functions
															case 'F' :
																curvePtr[p]->xmax = 0.0 ;
																curvePtr[p]->ymax = 0.0 ;
																curvePtr[p]->zmax = 0.0 ;

																if (BufferPtrPtr == NULL) { // don't bother with this if buffer enabled
																	for ( y=ii=0; y<part; y++ ) {
																		for ( x=0; x<index; x++, ii+=3) {
																			if (key == 'f') (curvePtr[p])->polyline[ii] = offsetx +  scalex*(-1.0 + 2.0 * ((float) x)/ ((float) (index -1))) ; // index -1 resolves the fence post problem
																			else if (key == 'F') (curvePtr[p])->polyline[ii] =   (-1.0 + 2.0 * ((float) (x + y * index))/ ((float) (index * part ) )) ; // in range -1 to 1 ... part should be 1 ! why multiply by 1 ?
																			(curvePtr[p])->polyline[ii+1]= offsety ; // init to offset
																			(curvePtr[p])->polyline[ii+2] = offsetz; // + scalez*(0.0) ;
																		}
																	}
																}
																break;
															
															case 'Y' :  //
															case 'y' : // graph y versus x

																// read into last third of the available space
																if (textin==TRUE)  { 

																	for ( y=2 * (curvePtr[p])->npolylinenodes; y < 3 * (curvePtr[p])->npolylinenodes; y++) 
																			done = fscanf ( InFile," %f", &(curvePtr[p])->polyline[y] ) ; // read ascii floats  y[]
																}
																else { 
																	done = read_till(InFile, "\r\n"); //look for new line
																	if (done != EOF) fread( &(curvePtr[p])->polyline[2* (curvePtr[p])->npolylinenodes], sizeof ( float ), (curvePtr[p])->npolylinenodes, InFile);
																}


																// echo data while series is contiguous and provided Search_count >= 0, -ve suppresses data out
																if ((echo) && (Search_count >= 0)) { 
																	if (raw==FALSE) {
																		for ( y = 2 * curvePtr[p]->npolylinenodes ; y < 3 * curvePtr[p]->npolylinenodes; y++)  fprintf(OutStream,"%g ", curvePtr[p]->polyline[y] ) ; // echo y
																	}
																	else 
																		fwrite( &(curvePtr[p])->polyline[2* (curvePtr[p])->npolylinenodes], sizeof ( float ) , (curvePtr[p])->npolylinenodes, OutStream);
																		
																	fputc('\n',OutStream);		// echo retn
																}



																//  rearrange, and log range
																for ( y = 2 * (curvePtr[p])->npolylinenodes, ii = 0; y < 3*(curvePtr[p])->npolylinenodes; y++, ii+=3 ) { 
																	curvePtr[p]->polyline[ii+1] = curvePtr[p]->polyline[y];
																	if ( curvePtr[p]->polyline[y] < ymin ) ymin = curvePtr[p]->polyline[y]; 
																	if ( curvePtr[p]->polyline[y] > ymax ) ymax = curvePtr[p]->polyline[y];
																}

																curvePtr[p]->xmax = curvePtr[p]->xmin = 0.0 ;
																curvePtr[p]->ymax = ymax; curvePtr[p]->ymin = ymin ;
																curvePtr[p]->zmax = curvePtr[p]->zmin = 0.0 ;


																//scaling and offsets
																if (BufferPtrPtr == NULL) { // don't bother with this if buffer enabled
																	for ( y=ii=0; y<part; y++ ) {
																		for ( x=0; x<index; x++, ii+=3) {
																			if (key == 'y') (curvePtr[p])->polyline[ii] = offsetx +  scalex*(-1.0 + 2.0 * ((float) x)/ ((float) (index -1))) ; // index -1 resolves the fence post problem
																			else if (key == 'Y') (curvePtr[p])->polyline[ii] =   (-1.0 + 2.0 * ((float) (x + y * index))/ ((float) (index * part ) )) ; // in range -1 to 1 ... part should be 1 ! why multiply by 1 ?
																			(curvePtr[p])->polyline[ii+1]= offsety + (curvePtr[p])->polyline[ii+1]*scaley; // scale & offset
																			(curvePtr[p])->polyline[ii+2] =  offsetz; // + scalez*(0.0) ;
																		}
																	}
																}
																break;
															
																if (echo) fprintf(OutStream, "## %g %g\n", curvePtr[p]->polyline[1], curvePtr[p]->polyline[4]);
															
															case 'z' :  // graph z versus x
															case 'Z' :  //

																if (textin==TRUE)  { // read into last third of the available space

																	for ( y = 2 * (curvePtr[p])->npolylinenodes; y < 3 * (curvePtr[p])->npolylinenodes; y++) 
																			done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[y] ) ; // read ascii floats  y[]
																}
																else { 
																	done = read_till(InFile, "\r\n"); //look for new line
																	if (done != EOF) fread( &(curvePtr[p])->polyline[2* (curvePtr[p])->npolylinenodes] , sizeof ( float ) , (curvePtr[p])->npolylinenodes ,InFile);
																}


																if ((echo) && (Search_count >= 0)) { // do this while  series is contiguous
																	if (raw==FALSE) {
																		for ( y = 2 * (curvePtr[p])->npolylinenodes; y < 3 * (curvePtr[p])->npolylinenodes; y++)  fprintf(OutStream,"%g ", (curvePtr[p])->polyline[y] ) ; // echo y
																	}
																	else 
																		fwrite( &(curvePtr[p])->polyline[2* (curvePtr[p])->npolylinenodes], sizeof ( float ) , (curvePtr[p])->npolylinenodes, OutStream);
																		
																	fputc('\n',OutStream);		// echo retn
																}



																// then rearrange, and log range
																for ( y = 2 * (curvePtr[p])->npolylinenodes, ii = 0; y < 3*(curvePtr[p])->npolylinenodes; y++, ii+=3 ) { 
																	curvePtr[p]->polyline[ii+2] = curvePtr[p]->polyline[y];
																	if ( curvePtr[p]->polyline[y] < zmin ) zmin = curvePtr[p]->polyline[y]; 
																	if ( curvePtr[p]->polyline[y] > zmax ) zmax = curvePtr[p]->polyline[y];
																}

																curvePtr[p]->xmax = curvePtr[p]->xmin = 0.0 ;
																curvePtr[p]->ymax = curvePtr[p]->ymin = 0.0 ;
																curvePtr[p]->zmax = zmax; curvePtr[p]->zmin = zmin;


																//set up remaining with scaling and offsets
																if (BufferPtrPtr == NULL) { // don't bother with this if  buffer enabled
																	for ( y=ii=0; y<part; y++ ) {
																		for ( x=0; x<index; x++, ii+=3) {
																			if (key == 'z') (curvePtr[p])->polyline[ii] = offsetx +  scalex*(-1.0 + 2.0 * ((float) x)/ ((float) (index -1 ))) ; // index -1 resolves the fence post problem
																			else if (key == 'Z') (curvePtr[p])->polyline[ii] =  (-1.0 + 2.0 * ((float) (x + y * index))/ ((float) (index * part ) )) ; // in range -1 to 1... part should be 1 ! why multiply by 1 ?
	
																			(curvePtr[p])->polyline[ii+1] =  offsety ;// + scaley*(0.0) ;
																			(curvePtr[p])->polyline[ii+2]= offsetz + (curvePtr[p])->polyline[ii+2]*scalez;
																		}
																	}
																}
																
															break;

															case 'k' :
															case 'K' :
																Num_kurve = p; // record this for the region colour rendering later

																// reading
																if ( textin==TRUE )  { // read ascii floats three rows y[], z[]

																	for ( j = (curvePtr[p])->npolylinenodes; j < 2 * (curvePtr[p])->npolylinenodes; j++) { // get the y values into the middle third 
																		done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[j] ) ;
																		if ( curvePtr[p]->polyline[j] < ymin ) ymin = curvePtr[p]->polyline[j]; // get min
																		if ( curvePtr[p]->polyline[j] > ymax ) ymax = curvePtr[p]->polyline[j]; // get max

																	}
																	for (j = 2 * (curvePtr[p])->npolylinenodes; j < 3 * (curvePtr[p])->npolylinenodes; j++) { // get the z values into the last third
																		done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[j] ) ;
																		if ( curvePtr[p]->polyline[j] < zmin ) zmin = curvePtr[p]->polyline[j]; // get min
																		if ( curvePtr[p]->polyline[j] > zmax ) zmax = curvePtr[p]->polyline[j]; // get max
																	}
																}
																else { // read raw data
																	done = read_till(InFile, "\r\n"); //look for new line
																	if (done != EOF) fread( &(curvePtr[p])->polyline[(curvePtr[p])->npolylinenodes]  , 2 * sizeof ( float ) , (curvePtr[p])->npolylinenodes ,InFile);
																
																	for ( j=(curvePtr[p])->npolylinenodes; j < 2*(curvePtr[p])->npolylinenodes; j++) { // derive min max 
																		if ( curvePtr[p]->polyline[j] < ymin ) ymin = curvePtr[p]->polyline[j]; // get min
																		if ( curvePtr[p]->polyline[j] > ymax ) ymax = curvePtr[p]->polyline[j]; // get max
																	}
																	for ( j = 2*(curvePtr[p])->npolylinenodes; j < 3*(curvePtr[p])->npolylinenodes; j++) {
																		if ( curvePtr[p]->polyline[j] < zmin ) zmin = curvePtr[p]->polyline[j]; // get min
																		if ( curvePtr[p]->polyline[j] > zmax ) zmax = curvePtr[p]->polyline[j]; // get max
																	}
																}

																curvePtr[p]->xmax = curvePtr[p]->xmin = 0.0 ;
																maxlimity = offsety + scaley * ( curvePtr[p]->ymax = ymax ) ; curvePtr[p]->ymin = ymin;
																maxlimitz = offsetz + scalez * ( curvePtr[p]->zmax = zmax ) ; curvePtr[p]->zmin = zmin;


																if ((echo) && (Search_count >= 0)) { // do this while  series is contiguous
																	if (raw==FALSE) {
																		for ( y=(curvePtr[p])->npolylinenodes; y < 2*(curvePtr[p])->npolylinenodes; y++) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[y] ) ; // echo y
																		fputc('\n',OutStream);
																		for ( y = 2 * (curvePtr[p])->npolylinenodes; y < 3 * (curvePtr[p])->npolylinenodes; y++) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[y] ) ; // echo z
																		fputc('\n',OutStream);
																	}
																	else {
																		fwrite( &(curvePtr[p])->polyline[(curvePtr[p])->npolylinenodes], 2 * sizeof ( float ) ,  (curvePtr[p])->npolylinenodes, OutStream);
																		fputc('\n',OutStream);		// echo retn
																	}
																}


																// then rearrange ... first the [y]... (more tricky)
																ii = 1; x =(curvePtr[p])->npolylinenodes;
																while (ii <= x) { // moves [y] out 
																	curvePtr[p]->polyline[ii] = curvePtr[p]->polyline[x];
																	ii+=3; x++;
																}
																int saveii = ii;
																ii = 0; 
																while (ii < 2 * (curvePtr[p])->npolylinenodes) { // a temporary move for half or [y]
																	curvePtr[p]->polyline[ii] = curvePtr[p]->polyline[x];
																	ii+=3; x++;
																}
																
																ii = 2; x = 2 * (curvePtr[p])->npolylinenodes;
																while (x < 3 * (curvePtr[p])->npolylinenodes) { // this distributes  [z]
																	curvePtr[p]->polyline[ii] = curvePtr[p]->polyline[x];
																	ii+=3; x++;
																}
																
																ii = saveii ; x = 0;
																while (ii < 3* (curvePtr[p])->npolylinenodes) { // removes  half [y]
																	curvePtr[p]->polyline[ii] = curvePtr[p]->polyline[x];
																	ii+=3; x+=3;
																}
																
																
																//set up remaining with scaling and offsets
																if (BufferPtrPtr == NULL) { // don't bother with this if  buffer enabled
																	for ( x=ii=0; x < (curvePtr[p])->npolylinenodes; x++, ii+=3) {
																		if (key == 'k')  (curvePtr[p])->polyline[ii] = offsetx +  scalex*(-1.0 + 2.0 * ((float) x)/ ((float) (index -1 ))) ; // index -1 resolves the fence post problem
																		else if (key == 'K') (curvePtr[p])->polyline[ii] =  (-1.0 + 2.0 * ((float) (x))/ ((float) (index  ))) ; // in range -1 to 1
	
																		(curvePtr[p])->polyline[ii+1]= offsety + (curvePtr[p])->polyline[ii+1]*scaley;
																		(curvePtr[p])->polyline[ii+2]= offsetz + (curvePtr[p])->polyline[ii+2]*scalez;
																	}
																}
																
															break;
															
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				} // end if
			} // end if
		} // end if
	}			

	if ((Search_count != 0 ) || ( strlen(Search_labelPtr)  > 0 )) echo = FALSE; // reset echo on exit if search is in progress
	return (done);
}


/* ========================================================================= */  
/* read array, with a few other features thrown in                           */
/* ========================================================================= */  
/*
/*
   's'  object: partial surface mesh
   'S'  object: full surface mesh
   'k'  object: kurve 3D
   'b'  qualifier: ballmarker e.g. b:r:redball or b:b:blueball
   'x'  object: static graph 3D
   'y'  object: static graph 2D in yx plane
   'z'  object: static graph 2D in zx plane
   'K'  object: animation Kurve sequence 3D
   'X'  object: animation graph sequence 3D
   'Y'  object: animated trace 2D in yx plane
   'Z'  object: animated trace 2D in zx plane
   'O'  specifier: Orientation  e.g. O 316 9
   'o'  specifier: object visibility
   'L'  specifier: Light position e.g. L 288 1
   'C'  specifier: surface Colour gradient  / could use G for gradient
   'f'  qualifier: special hidden colour function? 
   'c'  qualifier: to be completed colour specifier 
   'I'  specifier: Index animation rate and time initiialisation
   'V'  qualifier: View Screen managment e.g. V:f:full screen
   'v'  qualifier: object Visible  e.g. v:n:on  or v:f:off
   'w'  qualifier: line width e.g. w 0.5
   'W'  specifier: Window position and dimensions xorig yorig xdimen ydimen
   'p'  qualifier: points (line type) e.g. p:.:points or p:-:line
   'G'  specifier: grid brightness 
   'B'  specifier: grid bounds
   'E'  specifier: grid origin
   'M'  qualifier: Mode of animation
   'P'  qualifier: Plane Partition parameters for kurves
   'D'     delta-T timed_scan
   'a'  qualifier: simultrace set or reset t:n:on or t:f:off
   'A'  qualifier: axis intensity
   'l'  qualifier: label display l:[n]/[f]:label   the label is discarded
   't'  qualifier: text e.g. t:c:"my title"\nsize\nx y z
   'T'  qualifier: window Title e.g. T "my title"
   '#'  escape: comment line
   'Q'  escape: quit object descriptors
   'r'  raw data switch r:f:raw_off  or r:n:raw_data
*/


int read_array ( FILE *InFile,  SURF **surfacePtr, LINE **curvePtr, BALL **ballPtr, TXTLINE **textPtr, int textin, int echo, int raw, char *Search_labelPtr, int Search_count, LINE **BufferPtrPtr )
{
  FILE *OutStream ;
  int key ;
  char label[LABEL_SIZE]; 
  int *lastVisPtr = NULL;
  char *lastlinetypePtr = NULL;
  float *lastWidthPtr = NULL;
  float *lastcolour_redPtr = NULL;
  float *lastcolour_greenPtr = NULL;
  float *lastcolour_bluePtr = NULL;
  char *lastcolourPtr = NULL;
  int   done, p=0, q=0, s=0, t=0, x_orientation, y_orientation,j;
  int whichobject, howvisible, i;
  long x, y, ii,  part = 0, index = 0;
  float offsetx, offsety, offsetz, xmin, ymin, zmin, xmax, ymax, zmax, axis_intensity;
  float red, green, blue, intensity, width;
  char tempchar;
  float tempfloat;
  float *Buffptr = NULL;
  
  
  int	Object_count = 0, Surface_count = 0 , Graph_count = 0; // allows one to identify  a graph by number
  
	
  	maxlimitz = MAX_Z ;
	maxlimity = MAX_Y ;

    
  
   
	if ((echo) || ( strlen(Search_labelPtr) > 0 ) || (Search_count != 0)) { // going to be outputting 
		OutStream=fopen("/dev/stdout","w");  //open standard out 
    	if (!OutStream) exit (-1) ;
    }
    else OutStream = NULL;



	// start off the arrays

	surfacePtr[q] = malloc( sizeof( SURF )) ;
	surfacePtr[q]->npolygons		= 0 ;
	surfacePtr[q]->npolylinenodes	= 0 ;
	surfacePtr[q]->polygon = 0;
	surfacePtr[q]->polyline =  (float *) -1;
	surfacePtr[q]->index = (surfacePtr[q])->part = 0;
	surfacePtr[q]->visible = GLUT_NOT_VISIBLE;

	curvePtr[p] = malloc( sizeof( LINE )) ;
	curvePtr[p]->npolylinenodes = 0;
	curvePtr[p]->polyline = 0;
	curvePtr[p]->linetype = '-';
	curvePtr[p]->width = 1.4; /* default width */
	curvePtr[p]->visible = GLUT_NOT_VISIBLE; /*  */

	ballPtr[s] = malloc( sizeof( BALL )) ;
	ballPtr[s]->ball = -1 ;

	textPtr[t] = malloc( sizeof( TXTLINE )) ;
	textPtr[t]->label[0]=0; // mark a null label
		
	key = done = 0;


	Set_raw_switch(OutStream, echo, raw); //  output a raw data switch to reflect the settings

	do {
		// look for a key character
		done = key = read_till(InFile, "#aAbBCDEfFGIkKlLMoOpPQrsStTvVwWxXhyYzZ");

// debug		fprintf(stdout," %c", key); fflush(stdout);
	
		switch (key) {

			case '#' : //  #comment line may only occur outside of an object,   the comment is ignored, provides one method to overloading a graphics file
				if (echo) fputc('#', OutStream);
				while (((done = fgetc(InFile)) !=EOF) && ('\n' != done)) {if (echo) fputc(done, OutStream); } // skip to end-of-line, conditional echo
				if (echo) fputc('\n', OutStream); // terminate with a line feed
				break;
				
			case 'T' : //  window title
				done = read_till(InFile, ":\"\r\n"); 
	
				if ((done == '"') || (done == ':')) {
					read_label(InFile, title);
					newtitle = TRUE;
					if ((echo) || (Search_count == ENVIRONMENT)) print_label(OutStream, "T:", title, ":\n"); 								
				}
				break;

			case 'r' : // raw data switch indicates what type of graphics data follows 
				done = read_till(InFile, ":");
				if (':' == done) { // continue
					if (EOF != (done = fgetc(InFile))) {
						if ((done == 'f') || (done == 'F'))  { 
							if ((textin == FALSE) && ((echo) || (Search_count == ENVIRONMENT)))  fprintf(OutStream,"r:n:raw_data_on\n", tempchar);  //  set raw data switch off
							textin = TRUE;
						}
						else if ((done == 'n') || (done == 'N')) {
							if ((textin == TRUE) && ((echo) || (Search_count == ENVIRONMENT))) fprintf(OutStream,"r:f:text\n", tempchar);  //  set raw data switch on
							textin = FALSE;
						}
					}
					read_till(InFile, "\n\r");
				}
				break;


			case 'c' : // set RGB colours 
				done = read_till(InFile, ":");
				if ((done == ' ') || (done == ':')) {
				
					fscanf ( InFile, "%f:%f:%f", &red, &green, &blue );
					if (red < 0.0) red =0.0;
					if (red > 1.0) red =1.0;
					if (red < 0.0) green =0.0;
					if (red > 1.0) green =1.0;
					if (red < 0.0) blue =0.0;
					if (red > 1.0) blue =1.0;
					if (lastcolour_redPtr != NULL) lastcolour_redPtr[0] = red ;
					if (lastcolour_greenPtr != NULL) lastcolour_greenPtr[0] = green ;
					if (lastcolour_bluePtr != NULL) lastcolour_bluePtr[0] = blue ;
					if (lastcolourPtr != NULL) lastcolourPtr[0] = 'w';
				}
				break;

			case 'v' : // set visible 
				done = read_till(InFile, ":");
				if (':' == done) { // continue
					if (EOF != (done = fgetc(InFile))) tempchar = done;
					if ((tempchar == '+') || (tempchar == '-')) { // expand  to v:[+-][sc]number:label for general objects this needs thinking about in relation to jump features
					
					
					}
					else {  //  flag  o[n] or o[f]f  format v:[n]/[f]:label   the label is discarded 
						while (((done = fgetc(InFile)) !=EOF) && (':' != done )) { if ((echo) || (Search_count == ENVIRONMENT)) fputc(done, OutStream);}
						if (':' == done) { // continue to read label
							fscanf ( InFile, "%s", &label[0]);
							if (lastVisPtr != NULL) { 
								if ((tempchar == 'f') || (tempchar == 'F'))  lastVisPtr[0] = GLUT_NOT_VISIBLE ;
								else if ((tempchar == 'n') || (tempchar == 'N')) lastVisPtr[0] = GLUT_VISIBLE ;
							}
						}
						if ((echo) || (Search_count == ENVIRONMENT)){ //  output line + label
							fprintf(OutStream,"v:%c", tempchar); 
							print_label(OutStream, ":", label, "\n");
						}
					}
				}
				break;
				
			case 'w' : // set width of line w:1.0 or w 1.0		
				done = read_till(InFile, " :");
				if ((done == ' ') || (done == ':')) {
					fscanf ( InFile, "%f", &width );
					if (width < 0.5) width =0.5;
					else if (width > 3.0) width = 3.0;
					if (lastWidthPtr) lastWidthPtr[0] = width;
					if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"w:%.2f\n", width);  // finish outputting title
				}
				break;
	
	
			case 'p' : // Points
				done = read_till(InFile, ":");
				if (':' == done) { // continue
					done = read_till(InFile, ".-|b \r\n:");
					if (((done != '.') || (done != '-') || (done != '|') || (done != 'b')  || (done != ' ')) && (lastlinetypePtr != NULL)) {
						tempchar = lastlinetypePtr[0] = done;
						done = read_till(InFile, ":");
						if (done == ':') {
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"p:%c:",tempchar);
							while (((done = fgetc(InFile)) !=EOF) && ('\r' != done) && ('\n' != done)) { // finish outputting title
								if ((echo) || (Search_count == ENVIRONMENT)) {
									if ((done == ' ') || (done == ':') || (done == '"')) fputc('\\', OutStream); 
									fputc(done, OutStream);
								}
							}   
							if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
						}
					}
				}
				break;

			case 'l' : // label display  format l:[n]/[f]:label   the label is discarded
				done = read_till(InFile, "fFnN\r\n");
				if ((done !=EOF) && (done != '\n')) {
					// think this must be a mistake if (lastVisPtr != NULL) 
					if (lastVisPtr != NULL) { 
						if ((done == 'f') || (done == 'F')) Showlabels = 0 ;
						else if ((done == 'n') || (done != 'N')) Showlabels = 1 ;
					}
					if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"l:%c:", done);
					done = read_till(InFile, ":");
					while (((done = fgetc(InFile)) !=EOF) && ('\r' != done) && ('\n' != done)) {if ((echo) || (Search_count == ENVIRONMENT)) fputc(done, OutStream);} // echo label
					if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
				}
				break;
	
			case 'a' : // simultrace/animation is set or reset   format a:[n]/[f]:label
				done = read_till(InFile, "fFnN\r\n");
				if ((done !=EOF) && (done != '\n')) {
					if (lastVisPtr != NULL) { 
						if ((done == 'f') || (done == 'F')) simultrace = 0 ;
						else if ((done == 'n') || (done != 'N')) simultrace = 1 ;
					}
					if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"a:%c:", done);
					done = read_till(InFile, ":");
					while (((done = fgetc(InFile)) !=EOF) && ('\r' != done) && ('\n' != done)) {if ((echo) || (Search_count == ENVIRONMENT)) fputc(done, OutStream);} // echo label
					if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
				}
				break;
			
			case 'b' : // assign a ball and colour to a curve   b:n:on  b:w:weight  b:t:type   b:r:label  b:T:labelx,labely,labelz,xscale,yscale,zscale,xoffset,yoffset,zoffset
				done = read_till(InFile, ":");
				done = fgetc(InFile); // get the next character should be one of the following
					switch (done) {
						case 'n': //visible 																	{why control this...? well this now makes sense because you might want a text display but not the ball}
							ballPtr[s-1]->visible = GLUT_VISIBLE; // presumes to be a postscript to a ball set specifier
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"b:n:");
							done = read_till(InFile, ":");
							while (((done = fgetc(InFile)) !=EOF) && ('\n' != done )) {if ((echo) || (Search_count == ENVIRONMENT)) fputc(done, OutStream);} // echo label till end of line
							if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream); // add line feed			
							break;
						case 'f': //not visible 																	{why control this...? well this now makes sense because you might want a text display but not the ball}
							ballPtr[s-1]->visible = GLUT_NOT_VISIBLE; // presumes to be a postscript 
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"b:f:");
							done = read_till(InFile, ":");
							while (((done = fgetc(InFile)) !=EOF) && ('\n' != done )) {if ((echo) || (Search_count == ENVIRONMENT)) fputc(done, OutStream);} // echo label till end of line
							if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream); // add line feed			
							break;
	
						case 'w': //weight/size
							done = read_till(InFile, ":");
							if (done == ':') {
								done = fscanf ( InFile, " %f", &(ballPtr[s-1]->size) ); //radius
								if (ballPtr[s-1]->size < .01) ballPtr[s-1]->size = .01;
								else if (ballPtr[s-1]->size  > 10) ballPtr[s-1]->size =10;
								if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"b:w:%g\n",ballPtr[s-1]->size);
							}
							break;
	
						case 't': //reset type  need to check s value is reasonable
							fscanf ( InFile, ":%c:%s", &(ballPtr[s-1]->type), &(ballPtr[s-1]->label[0]) ); //applies to last ball_set shape s=sphere (default), t=teapot, b=box, c=cone, d=donut
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream, "b:t:%c:%s\n",ballPtr[s-1]->size, ballPtr[s-1]->label[0]);
							break;

						case 'T': //set up text field, uses the ball label content to indicate the fixed text in the field
							fscanf ( InFile, ":%f:%f:%f:%f:%f:%f:%f:%f:%f", &(ballPtr[s-1]->txt_xoffset), &(ballPtr[s-1]->txt_yoffset), &(ballPtr[s-1]->txt_zoffset),
								&(ballPtr[s-1]->xscale), &(ballPtr[s-1]->yscale), &(ballPtr[s-1]->zscale),
								&(ballPtr[s-1]->xoffset), &(ballPtr[s-1]->yoffset), &(ballPtr[s-1]->zoffset)); 
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream, "b:T:%g:%g:%g:%g:%g:%g:%g:%g:%g\n",ballPtr[s-1]->txt_xoffset, ballPtr[s-1]->txt_yoffset, ballPtr[s-1]->txt_zoffset,
								ballPtr[s-1]->xscale, ballPtr[s-1]->yscale, ballPtr[s-1]->zscale,
								ballPtr[s-1]->xoffset, ballPtr[s-1]->yoffset, ballPtr[s-1]->zoffset);
							
							break;

						case 'C': //set up RGB directly
							fscanf ( InFile, ":%f:%f:%f", &(ballPtr[s-1]->colour_red), &(ballPtr[s-1]->colour_green), &(ballPtr[s-1]->colour_blue));
							if ((echo) || (Search_count == ENVIRONMENT))  fprintf(OutStream, "b:C:%g:%g:%g\n",ballPtr[s-1]->colour_red, ballPtr[s-1]->colour_green, ballPtr[s-1]->colour_blue);
							break;
	
						case 'r': //red
						case 'g': //green
						case 'b': //blue
						case 'm': //magenta
						case 'c': //cyan
						case 'y': //yellow
	//					case 'w': //white has been used above
		//				case ' ': //? would this serve any purpose ... maybe for a text field  follower
							switch (done) {
								case 'r': //red
									ballPtr[s]->colour_red = 1.0; ballPtr[s]->colour_green = 0.0; ballPtr[s]->colour_blue = 0.0;
									break;
								case 'g': //green
									ballPtr[s]->colour_red = 0.0; ballPtr[s]->colour_green = 1.0; ballPtr[s]->colour_blue = 0.0;
									break;
								case 'b': //blue
									ballPtr[s]->colour_red = 0.0; ballPtr[s]->colour_green = 0.0; ballPtr[s]->colour_blue = 1.0;
									break;
								case 'm': //magenta
									ballPtr[s]->colour_red = 1.0; ballPtr[s]->colour_green = 0.0; ballPtr[s]->colour_blue = 1.0;
									break;
								case 'c': //cyan
									ballPtr[s]->colour_red = 0.0; ballPtr[s]->colour_green = 1.0; ballPtr[s]->colour_blue = 1.0;
									break;
								case 'y': //yellow
									ballPtr[s]->colour_red = 1.0; ballPtr[s]->colour_green = 1.0; ballPtr[s]->colour_blue = 0.0;
									break;
							}

							ballPtr[s]->ball = p-1; 									// assign to last defined curve ... assignment must immediately follow a graph defn
							ballPtr[s]->visible = GLUT_VISIBLE;
							ballPtr[s]->colour = (done & 95) + 32; 
							ballPtr[s]->type = 's'; // default to sphere 
							ballPtr[s]->size = .075; // default size


							ballPtr[s]->xval = 0; 				// default values ... these get updated whenever the ball is drawn
							ballPtr[s]->yval = 0;
							ballPtr[s]->zval = 0;

																// these next parameters used to convert values in display... if set to zero then corresponding value is not displayed
							ballPtr[s]->xscale = 0;  				/* x-scale for converting xval to displayed text */
							ballPtr[s]->yscale = 0;
							ballPtr[s]->zscale = 0;
							ballPtr[s]->xoffset = 0;				/* offset may be required in conversion of the values */
							ballPtr[s]->yoffset = 0;  				/* y- */
							ballPtr[s]->zoffset = 0;  				/* z- */
							
							ballPtr[s]->txt_xoffset = 0;				/* x-offset for fixed text label */
							ballPtr[s]->txt_yoffset = 0;  				/* y- */
							ballPtr[s]->txt_zoffset = 0;  				/* z- */

							done = read_till(InFile, ":");
							
							read_label(InFile, ballPtr[s]->label);

							if ((echo) || (Search_count == ENVIRONMENT)) {
								fprintf (OutStream,"b:%c",ballPtr[s]->colour); 
								print_label(OutStream, ":", ballPtr[s]->label, "\n") ;  // finish outputting title
							}	
							s++ ; 							// set up for new ball
							ballPtr[s] = malloc( sizeof( BALL )) ;	// set up for new ball
							ballPtr[s]->ball = -1; 			// set up for new ball
							ballPtr[s]->visible = GLUT_NOT_VISIBLE;
							break;
							
						default: 
							done = read_till(InFile, "\n");
							break;  // discard line?
					}
				break;
	
			case 'O' : // Orientation of objects on screen  going to allow for a : separator or a space also allow for floating point input
				done = read_till(InFile, " :");
				if ((done == ' ' ) || (done == ':' )) {
					done = fscanf ( InFile, " %f", &tempfloat ) ;
					x_orientation = (int) tempfloat ;
					while (x_orientation >= 360) x_orientation -= 360;
					while (x_orientation < 0) x_orientation += 360;
					while ((done !=EOF) && (done != ' ' ) && (':' != (done = fgetc(InFile)))) {} //step over ':' or ' '
					if ((done == ' ') || (done == ':')) {
						done = fscanf ( InFile, " %f", &tempfloat ) ;
						y_orientation = (int) tempfloat ;
						while (y_orientation >= 360) y_orientation -= 360;
						while (y_orientation < 0) y_orientation += 360;
						setview(x_orientation, y_orientation);
						if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"O:%d:%d\n", x_orientation, y_orientation);
					}
				}
				break;
	
			case 'o' : // object visibility ... first 10  ..... legacy going to be dropped    .... -> v:s:[n]/[f]:number:label
				if ((echo) || (Search_count == ENVIRONMENT)) fputc('o', OutStream);
				for (i=0; i < 10; i++) {
					done = fscanf ( InFile, "%d:%d", &whichobject, &howvisible ) ;
					if ((echo) || (Search_count == ENVIRONMENT)) fprintf (OutStream, " %d:%d", whichobject, howvisible );
					
					if ((whichobject !=0) && (surfacePtr[whichobject-1])) surfacePtr[whichobject-1]->visible = howvisible;
				}
				for (i=0; i < 10; i++) {
					done = fscanf ( InFile, "%d:%d", &whichobject, &howvisible ) ;
					if ((echo) || (Search_count == ENVIRONMENT)) fprintf (OutStream, " %d:%d", whichobject, howvisible );
					if ((whichobject !=0) && (curvePtr[whichobject-1])) curvePtr[whichobject-1]->visible = howvisible;
				}
				if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
				break;
				
			case 'L' : // Orientation of lighting
				for (i=1; i <=3; i++) {
					done = read_till(InFile, " :");
					if (((echo) || (Search_count == ENVIRONMENT)) && (i==1) && ((done == ' ' ) || (done == ':'))) fputc('L', OutStream);
					if ((done == ' ' ) || (done == ':' )) {
						done = fscanf ( InFile, " %f", &tempfloat ) ;
						x_orientation = (int) tempfloat ;
						while (x_orientation >= 360) x_orientation -= 360;
						while (x_orientation < 0) x_orientation += 360;
						while ((done !=EOF) && (done != ' ' ) && (':' != (done = fgetc(InFile)))) {} //step over ':' or ' '
						if ((done == ' ') || (done == ':')) {
							done = fscanf ( InFile, " %f", &tempfloat ) ;
							y_orientation = (int) tempfloat ;
							while (y_orientation >= 360) x_orientation -= 360;
							while (y_orientation < 0) x_orientation += 360;
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,":%d:%d", x_orientation, y_orientation);
							switch (i) {
								case 1:
									setlight_obj(x_orientation, y_orientation);
									break;
								case 2:
									setlight_2(x_orientation, y_orientation);
									break;
								case 3:
									setlight_3(x_orientation, y_orientation);
									if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
									break;
							}
						}
					}
				}
				break;
	
			case 'C' : // colour gradient for surfaces ... nested input to ensure partial parsing possible with legacy data
				done = read_till(InFile, " :");
				if (done != EOF) { 								// valid separator
					done = fscanf ( InFile, " %f", &tempfloat ) ;
					if (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
					else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
					Glowr = (double) tempfloat ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &tempfloat ) ;
						if (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
						else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
						Glowg = (double) tempfloat ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {  									// valid separator
							done = fscanf ( InFile, " %f", &tempfloat ) ;
							if (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
							else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
							Glowb = (double) tempfloat ;
	
							done = read_till(InFile, " :");
							if (done != EOF) {  									// valid separator
								done = fscanf ( InFile, " %f", &tempfloat ) ;
								if (tempfloat > MAX_LUM) tempfloat  = MAX_LUM ;
								else if (tempfloat < MIN_LUM) tempfloat = MIN_LUM ;
								ScaleIntensity = (double) tempfloat ;
								if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"C:%.2f:%.2f:%.2f:%.2f", (float) Glowr, (float) Glowg, (float) Glowb, (float) ScaleIntensity); // legacy stop off point
	
								done = read_till(InFile, " :");
								if (done != EOF) {  									// valid separator
									done = fscanf ( InFile, " %f", &tempfloat ) ;
									if (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
									else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
									background_r = (double) tempfloat ;
	
									done = read_till(InFile, " :");
									if (done != EOF) {  									// valid separator
										done = fscanf ( InFile, " %f", &tempfloat ) ;
										if (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
										else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
										background_g = (double) tempfloat ;
	
										done = read_till(InFile, " :");
										if (done != EOF) {  									// valid separator
											done = fscanf ( InFile, " %f", &tempfloat ) ;
											if  (tempfloat > MAX_CHROM) tempfloat  = MAX_CHROM ;
											else if (tempfloat < MIN_CHROM) tempfloat = MIN_CHROM ;
											background_b = (double) tempfloat ;
											if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,":%.2f:%.2f:%.2f", (float) background_r, (float) background_g, (float) background_b ); // legacy stop off point
										}
									}
								}
							}
							if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
						}
					}
				}
				break;
	
			case 'I' : // scanning index [-1, 1] and window width
				done = read_till(InFile, " :");
				if (done != EOF) { 							// valid separator
					done = fscanf ( InFile, " %f", &start_scan ) ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  						// valid separator
						done = fscanf ( InFile, " %f", &scan_incr ) ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {  						// valid separator
							done = fscanf ( InFile, " %f", &width_scan ) ; // should really check this is in range
	
							done = read_till(InFile, " :");
							if (done != EOF) {  						// valid separator
								done = fscanf ( InFile, " %f", &trace ) ;
	
								done = read_till(InFile, " :");
								if (done != EOF) {  						// valid separator
									done = fscanf ( InFile, " %f", &tracex ) ;
									
									done = read_till(InFile, ":\n");
									if (done != EOF) {
									
										if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"I:%g:%g:%g:%g:%g\n", start_scan, scan_incr, width_scan, trace, tracex);
										
										Jumpcntr++;
										jumpPtr[Jumpcntr] = malloc( sizeof( JUMP )) ;  // loads starting from 0
							
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
							
							
										for (i=0; i < 10; i++) {
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
							
										maxJumpcntr = Jumpcntr;
									}
								}
							}
						}
					}
				}

				break;
	
			case 'G' : // Grid axis display brightness
				xaxis = yaxis = zaxis = DEF_LUM ; 			// defaults brightness
				xlimit = DEF_X ; ylimit = DEF_Y ; zlimit = DEF_Z ;	// defaults 
				xorig = yorig = zorig = ORIGIN ; 			// defaults 
				
			// grid intensity
			
				done = read_till(InFile, " :");
				if (done != EOF) { 								// valid separator
					done = fscanf ( InFile, " %f", &xaxis ) ;
					if (xaxis > MAX_LUM) xaxis  = MAX_LUM ;
					else if (xaxis < MIN_CHROM ) xaxis = 0.0 ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &yaxis ) ;
						if (yaxis > MAX_LUM) yaxis  = MAX_LUM ;
						else if (yaxis < MIN_CHROM ) yaxis = MIN_CHROM ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {  									// valid separator
							done = fscanf ( InFile, " %f", &zaxis ) ;
							if (zaxis > MAX_LUM) zaxis  = MAX_LUM ;
							else if (zaxis < MIN_CHROM ) zaxis = MIN_CHROM ;
			//  bounds
	
							done = read_till(InFile, " :");
							if (done != EOF) {  									// valid separator
								done = fscanf ( InFile, " %f", &xlimit ) ;
								if (xlimit > MAX_X ) xlimit  = MAX_X ;
								else if (xlimit < ORIGIN ) xlimit = NORM ;
	
								done = read_till(InFile, " :");
								if (done != EOF) {  									// valid separator
									done = fscanf ( InFile, " %f", &ylimit ) ;
									if (ylimit > MAX_Y ) ylimit  = MAX_Y ;
									else if (ylimit < ORIGIN ) ylimit = NORM ;
	
									done = read_till(InFile, " :");
									if (done != EOF) {  									// valid separator
										done = fscanf ( InFile, " %f", &zlimit ) ;
										if (zlimit > MAX_Z ) zlimit  = MAX_Z ;
										else if (zlimit < ORIGIN) zlimit = NORM ;
	
	
			//  axis offset
										done = read_till(InFile, " :");
										if (done != EOF) {  									// valid separator
											done = fscanf ( InFile, " %f", &xorig ) ;
											if (xorig > MAX_X ) xorig  = MAX_X ;
											else if (xorig < -MAX_X) xorig = -MAX_X ;
	
											done = read_till(InFile, " :");
											if (done != EOF) {  									// valid separator
												done = fscanf ( InFile, " %f", &yorig ) ;
												if (yorig > MAX_Y) yorig  = MAX_Y ;
												else if (yorig < -MAX_Y) yorig = -MAX_Y ;
				
												done = read_till(InFile, " :");
												if (done != EOF) {  									// valid separator
													done = fscanf ( InFile, " %f", &zorig ) ;
													if (zorig > MAX_Z) zorig  = MAX_Z ;
													else if (zorig < -MAX_Z) zorig = -MAX_Z ;
	
													if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"G:%.2f:%.2f:%.2f:%.3f:%.3f:%.3f:%.3f:%.3f:%.3f", xaxis, yaxis, zaxis, xlimit, ylimit, zlimit, xorig, yorig, zorig); // legacy stop off point
			//  grid colour
													done = read_till(InFile, " :\n\r");
													if ((done == ' ' ) || (done == ':' )) {  									// valid separator
														done = fscanf ( InFile, " %f", &grid_r ) ;
														if (grid_r > MAX_CHROM) grid_r  = MAX_CHROM ;
														else if (grid_r < MIN_CHROM) grid_r = MIN_CHROM ;
				
														done = read_till(InFile, " :");
														if (done != EOF) {  									// valid separator
															done = fscanf ( InFile, " %f", &grid_g ) ;
															if (grid_g > MAX_CHROM) grid_g  = MAX_CHROM ;
															else if (grid_g < MIN_CHROM) grid_g = MIN_CHROM;
							
															done = read_till(InFile, " :");
															if (done != EOF) {  									// valid separator
																done = fscanf ( InFile, " %f", &grid_b ) ;
																if (grid_b > MAX_CHROM) grid_b  = MAX_CHROM ;
																else if (grid_b < MIN_CHROM) grid_b = MIN_CHROM ;
																if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,":%.2f:%.2f:%.2f", grid_r, grid_g, grid_b); // legacy stop off point
															}
														}
													}
													if ((echo) || (Search_count == ENVIRONMENT)) fputc('\n', OutStream);
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;
	
			case 'B' : // legacy Grid axis display bounds (to be deleted)
				done = read_till(InFile, " :");
				if (done != EOF) {  									// valid separator
					done = fscanf ( InFile, " %f", &xlimit ) ;
					if (xlimit > MAX_X) xlimit  = MAX_X ;
					else if (xlimit < ORIGIN) xlimit = NORM ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &ylimit ) ;
						if (ylimit > MAX_Y) ylimit  = MAX_Y ;
						else if (ylimit < ORIGIN) ylimit = NORM ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {  									// valid separator
							done = fscanf ( InFile, " %f", &zlimit ) ;
							if (zlimit > MAX_Z) zlimit  = MAX_Z ;
							else if (zlimit < ORIGIN) zlimit = NORM ;
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"B:%.2f:%.2f:%.2f", xlimit, ylimit, zlimit); // legacy 
						}
					}
				}
				if ((xlimit < 0) || (xlimit < 0)) xlimit = MAX_X ;
				if ((ylimit < 0) || (ylimit < 0)) ylimit = MAX_Y ;
				if ((zlimit < 0) || (zlimit < 0)) zlimit = MAX_Z ;			
				break;
	
			case 'E' : // legacy Grid axis display origin (to be deleted)
				done = read_till(InFile, " :");
				if (done != EOF) {  									// valid separator
					done = fscanf ( InFile, " %f", &xorig ) ;
					if (xorig > MAX_X ) xorig  = MAX_X ;
					else if (xorig < -MAX_X) xorig = -MAX_X ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &yorig ) ;
						if (yorig > MAX_Y) yorig  = MAX_Y ;
						else if (yorig < -MAX_Y) yorig = -MAX_Y ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {  									// valid separator
							done = fscanf ( InFile, " %f", &zorig ) ;
							if (zorig > MAX_Z) zorig  = MAX_Z ;
							else if (zorig < -MAX_Z) zorig = -MAX_Z ;
	
							if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"E:%.2f:%.2f:%.2f\n", xorig, yorig, zorig); // legacy 
						}
					}
				}
				if (xorig < ORIGIN) xorig = ORIGIN; else if (xorig > MAX_X ) xorig = MAX_X ;
				if (yorig < -MAX_Y) yorig = -MAX_Y;  else if  (yorig > MAX_Y) yorig = MAX_Y;
				if (zorig < -MAX_Z ) zorig = -MAX_Z; else if (zorig > MAX_Z) zorig = MAX_Z;
				break;
	
	
			case 'A' : // legacy e.g.  A:x:intesity
			
				done = read_till(InFile, " :");
				if (done != EOF) {  														// valid separator
					done = read_till(InFile, "xXyYzZ\r\n");
					key = done;
					done = read_till(InFile, " :");
					if (done != EOF) {  														// valid separator
						fscanf ( InFile, " %f",  &tempfloat);
						if (tempfloat < MIN_LUM) tempfloat = MIN_LUM;
						else if (tempfloat > 2 * MAX_LUM) tempfloat = 2 * MAX_LUM;
					
						switch (key) {
							case 'x' :
							case 'X' :
								xaxis = tempfloat; // grid intensity
								break;
							case 'y' :
							case 'Y' :
								yaxis = tempfloat; // grid intensity
								break;
							case 'z' :
							case 'Z' :
								zaxis = tempfloat; // grid intensity
								break;
							default:
								break;
						}
						if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"A:%c:%g\n",key,tempfloat);
					}
				}
				break;




			case 'M' : // display mode
				done = read_till(InFile, " :");
				if (done != EOF) { 								// valid separator
					done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds tbd
					simultrace = (int) tempfloat ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {   									// valid separator
						done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds tbd
						flat = (int) tempfloat ;
	
						done = read_till(InFile, " :");
						if (done != EOF) {   									// valid separator
							done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds tbd
							Showlabels = (int) tempfloat ;
	
							done = read_till(InFile, " :");
							if (done != EOF) {   									// valid separator
								done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds tbd
								Tracelight = tempfloat ;
								
								if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"M:%d:%d:%d:%.2f\n", simultrace, flat, Showlabels,Tracelight); 
	
							}
						}
					}
				}
				break;
	
			case 'P' : // partition
				done = read_till(InFile, " :");
				if (done != EOF) { 								// valid separator
					done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
					awake = tempfloat ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
						sleepwake = tempfloat ;
	
						done = read_till(InFile, " :");
						if (done != EOF) { 									// valid separator
							done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
							sleep21 = tempfloat ;
	
							done = read_till(InFile, " :");
							if (done != EOF) {  									// valid separator
								done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
								sleepREM = tempfloat ;
								if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"P:%g:%g:%g:%g\n", awake, sleepwake, sleep21, sleepREM ); 
	
							}
						}
					}
				}
				break;
	
			case 'D' : // duration delay
				done = read_till(InFile, " :");
				if (done != EOF) { 								// valid separator
					done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
					delay = tempfloat ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
						duration = tempfloat ;
						if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"D:%g:%g\n", delay, duration ); 
	
					}
				}
				break;
	
			case 'V' : // View-screen size  sets to full screen
				done = read_till(InFile, " :");
				if (done != EOF) {
					done = read_till(InFile, "fFnN\r\n");
					if ((done == 'F' ) || (done == 'f' )) { fullscreen = 1;	if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"V:f"); }	// full: initialise to full screen
					else if ((done == 'N' ) || (done == 'n' )) { fullscreen = 0; if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"V:n"); }// normal: defaults to small screen TBI allow it to set size of  default small screen
					
					done = read_till(InFile, " :");
					if (done != EOF) {								// handle the label
						read_label(InFile,label);
						if ((echo) || (Search_count == ENVIRONMENT)) print_label(OutStream, ":",label ,"\n");
					} 
				}
				break;
	
			case 'W' : // Window size
				done = read_till(InFile, " :");
				if (done != EOF) {								// valid separator
					done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
					winPos_x = (int) tempfloat ;
	
					done = read_till(InFile, " :");
					if (done != EOF) {  									// valid separator
						done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
						winPos_y = (int) tempfloat ;
	
						done = read_till(InFile, " :");
						if (done != EOF) { 									// valid separator
							done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
							winSize_x = (int) tempfloat ;
	
							done = read_till(InFile, " :");
							if (done != EOF) {  									// valid separator
								done = fscanf ( InFile, " %f", &tempfloat ) ;
				//	check bounds
								winSize_y = (int) tempfloat ;
								if ((winSize_x <= 0) || (winSize_y <= 0)) { winSize_x = 800; winSize_y = 500; } // if either out of range, reset both
								if ((winPos_x <= 0) || (winPos_y <= 0)) { winPos_x = -1; winPos_y = -1; } // if either out of range, reset both 
								fullscreen = 0; // defaults to small screen TBI allow it to set size of  default small screen
								if ((echo) || (Search_count == ENVIRONMENT)) fprintf(OutStream,"W:%d:%d:%d:%d\n", winPos_x, winPos_y, winSize_x, winSize_y ); 
	
							}
						}
					}
				}
				break;
	
	
	
			case 't' : // text  gives a text line, colour and position
				done = read_till(InFile, " :");
				if (done != EOF ) {
					done = read_till(InFile, "rgbmcyw "); // read colour

					if ( done != EOF ) {
						textPtr[t]->colour = done;
						done = read_till(InFile, " :");  // get second ' ' or :,  if a space then expect a '"' to open label
						
						if ( done != EOF ) {
							done = read_label(InFile, textPtr[t]->label);
							
							if (done != EOF) {
								
								done = fscanf ( InFile, " %f", &offsetx ) ;
								if (done != EOF) {
									while (((done = fgetc(InFile)) !=EOF) && (done != ' ' ) && (':' != done)) { } // very properly check for delimiters
									if ((done ==  ' ' ) || (':' == done)) done = fscanf ( InFile, "%f", &offsety ) ;
									if (done != EOF) {
										while (((done = fgetc(InFile)) !=EOF) && (done != ' ' ) && (':' != done)) { } // very properly check for delimiters
										if ((done ==  ' ' ) || (':' == done)) done = fscanf ( InFile, "%f", &offsetz ) ;
										if (done != EOF) { 
											textPtr[t]->xoffset = offsetx ;  
											textPtr[t]->yoffset = offsety ;  
											textPtr[t]->zoffset = offsetz ;

											if ((echo) || (Search_count == ENVIRONMENT)) {  // make call to print label here
												fprintf(OutStream,"t:%c",textPtr[t]->colour); print_label(OutStream, ":", textPtr[t]->label, ":"); 
												fprintf(OutStream,"%g:%g:%g\n", textPtr[t]->xoffset, textPtr[t]->yoffset, textPtr[t]->zoffset);
											}
											textPtr[t]->visible = GLUT_VISIBLE ; // always asumed to be visible
											lastVisPtr = ( int *) &(textPtr[t]->visible);  // can't recall what this is for ... 10th Sept 2007
											lastWidthPtr = ( float *) &(textPtr[t]->width);
											lastlinetypePtr = ( char *) &(textPtr[t]->linetype);
											t++; Num_texts++;
											textPtr[t] = malloc( sizeof( TXTLINE )) ;
											textPtr[t]->label[0] = 0;
										}
									}
								}
							}
						} // end if
					} // end if
				}			
				break;
			
	
			case 'h' :
			case 'f' :
			case 'x' :
			case 'y' :
			case 'z' :
			case 'k' :
			case 'K' :
			case 'F' :
			case 'Y' :
			case 'Z' :
				pdiv++; // counts the number of curves which are not X: types. Convention is to load all curves before X: types.
			case 'X' :

				Graph_count++;
				Object_count++;
				
				
				done = read_graph (key, p, InFile, curvePtr, Object_count, textin, OutStream, echo, raw, Search_labelPtr, Search_count, BufferPtrPtr ); 


				if (done !=EOF) {
				
					lastVisPtr = ( int *) &(curvePtr[p]->visible);
					lastWidthPtr = ( float *) &(curvePtr[p]->width);
					lastlinetypePtr = ( char *) &(curvePtr[p]->linetype);
					lastcolour_redPtr = ( float *) &(curvePtr[p]->colour_red);
					lastcolour_greenPtr = ( float *) &(curvePtr[p]->colour_green);
					lastcolour_bluePtr = ( float *) &(curvePtr[p]->colour_blue);
					Num_curve++; p++;
					curvePtr[p] = malloc( sizeof( LINE )) ;
					curvePtr[p]->npolylinenodes = 0;
					curvePtr[p]->polyline = 0;
					curvePtr[p]->linetype = '-';
					curvePtr[p]->width = 1.4; /* default width */
					curvePtr[p]->visible = GLUT_NOT_VISIBLE;
				}	
				break;

			case 'g' : // hidden colour function
				Graph_count++;
				Object_count++;

				done = read_till(InFile, ":");


				if (done != EOF) { 		// got a valid separator
					done = read_label(InFile, curvePtr[p]->label); // read in label
					if (done != EOF) {  // now get the index part scale and offsets			
						if (EOF != getint(InFile, &index)) {
							if (EOF != getint(InFile, &part)) {
		
								curvePtr[p]->visible = COLOUR_FUNC;
								(curvePtr[p])->npolylinenodes = index ;	// assumed that part should always be 1 //
								(curvePtr[p])->polyline = ( float * ) malloc ( (curvePtr[p])->npolylinenodes*3 * sizeof ( float )) ; //creating space

										// reading
								if (textin==TRUE)  { // read ascii floats three rows red[], green[], blue[] 
					
									for ( j=ii=0; j<index; j++, ii+=3) { // get the x values
										done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii] ) ; 				// get the data in
										if ( curvePtr[p]->polyline[ii] < xmin ) xmin = curvePtr[p]->polyline[ii]; // get min
										if ( curvePtr[p]->polyline[ii] > xmax ) xmax = curvePtr[p]->polyline[ii]; // get max
									}
									for ( j=ii=0; j<index; j++, ii+=3) { // get the y values
										done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii+1] ) ;
										if ( curvePtr[p]->polyline[ii+1] < ymin ) ymin = curvePtr[p]->polyline[ii+1]; // get min
										if ( curvePtr[p]->polyline[ii+1] > ymax ) ymax = curvePtr[p]->polyline[ii+1]; // get max
									}
									for (j=ii=0; j<index; j++, ii+=3) { // get the z values
										done = fscanf ( InFile," %f",   &(curvePtr[p])->polyline[ii+2] ) ;
										if ( curvePtr[p]->polyline[ii+2] < zmin ) zmin = curvePtr[p]->polyline[ii+2]; // get min
										if ( curvePtr[p]->polyline[ii+2] > zmax ) zmax = curvePtr[p]->polyline[ii+2]; // get max
									}
								}
								else { // read raw  data
									done = read_till(InFile, "\r\n"); //look for new line
									if (done != EOF) fread( (curvePtr[p])->polyline , 3 * sizeof ( float ) , (curvePtr[p])->npolylinenodes ,InFile);
									
									for ( j=ii=0; j<index; j++, ii+=3) { // derive min max 
										if ( curvePtr[p]->polyline[ii] < xmin ) xmin = curvePtr[p]->polyline[ii]; // get min
										if ( curvePtr[p]->polyline[ii] > xmax ) xmax = curvePtr[p]->polyline[ii]; // get max
										if ( curvePtr[p]->polyline[ii+1] < ymin ) ymin = curvePtr[p]->polyline[ii+1]; // get min
										if ( curvePtr[p]->polyline[ii+1] > ymax ) ymax = curvePtr[p]->polyline[ii+1]; // get max
										if ( curvePtr[p]->polyline[ii+2] < zmin ) zmin = curvePtr[p]->polyline[ii+2]; // get min
										if ( curvePtr[p]->polyline[ii+2] > zmax ) zmax = curvePtr[p]->polyline[ii+2]; // get max
									}
								}
									
								curvePtr[p]->xmax = xmax; curvePtr[p]->xmin = xmin ;
								curvePtr[p]->ymax = ymax; curvePtr[p]->ymin = ymin;
								curvePtr[p]->zmax = zmax; curvePtr[p]->zmin = zmin ;
									
									
								if ((echo) && (Search_count >= 0)) {
									if (raw==FALSE) {
										for ( j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii] ) ; //  echo x values
										fputc('\n',OutStream);		// echo retn
										for ( j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii+1] ) ; // echo y
										fputc('\n',OutStream);		// echo retn
										for (j=ii=0; j<index; j++, ii+=3) fprintf(OutStream,"%g ", (curvePtr[p])->polyline[ii+2] ) ; // echo z
										fputc('\n',OutStream);		// echo retn
									}
									else {
										fwrite( (curvePtr[p])->polyline, 3 * sizeof ( float ) , (curvePtr[p])->npolylinenodes, OutStream);
										fputc('\n',OutStream);		// echo retn
									}

								}
								lastVisPtr = ( int *) &(curvePtr[p]->visible);
								lastWidthPtr = ( float *) &(curvePtr[p]->width);
								lastlinetypePtr = ( char *) &(curvePtr[p]->linetype);
								Num_curve++; p++;
								curvePtr[p] = malloc( sizeof( LINE )) ;
								curvePtr[p]->npolylinenodes = 0;
								curvePtr[p]->polyline = 0;
								curvePtr[p]->linetype = '-';
								curvePtr[p]->width = 1.4; /* default width */
								curvePtr[p]->visible = GLUT_NOT_VISIBLE;
							} // end of read part
						} // end of read index
					}
				}

				

				break;
							
				
			 case 's' :							// get partial surface mesh data
			 case 'S' :							// get fully specified surface mesh data


				Surface_count++;
				Object_count++;

		//		done = read_surface ( InFile, surfacePtr, curvePtr, ballPtr, textPtr, textin, echo, raw, Search_labelPtr, Search_count );
				
				if(Search_count > 0) {
					if (Search_count == Object_count) Set_raw_switch(OutStream, echo = TRUE, raw); //  output a raw data switch to reflect the settings
					else echo = FALSE;
				}
				else if (Search_count < 0 ) { //  collecting headers
					echo = TRUE;
				}


				if (key == 'S')  surfacePtr[q]->kind = FULLYSPECIFIED; 
				else if (key == 's') surfacePtr[q]->kind = STATIC;

				done = read_till(InFile, ":");
				if (done != EOF) { 								// got a valid separator
					if ( EOF != (done = fgetc(InFile))) {
						if ((done == 'r') || (done == 'g') || (done == 'b') || (done == 'm') || (done == 'c') || (done == 'y') || (done == 'B') || (done == 'W') || (done == 'w') || (done == 'p') ) { // got a valid colour 
			
							surfacePtr[q]->colour = done;
							
							// here is a todo tag:   make this in to a routine....
							switch (done) {
								case 'r': //red
									surfacePtr[q]->colour_red = 1.0; surfacePtr[q]->colour_green = 0.0; surfacePtr[q]->colour_blue = 0.0;
									break;
								case 'g': //green
									surfacePtr[q]->colour_red = 0.0; surfacePtr[q]->colour_green = 1.0; surfacePtr[q]->colour_blue = 0.0;
									break;
								case 'b': //blue
									surfacePtr[q]->colour_red = 0.0; surfacePtr[q]->colour_green = 0.0; surfacePtr[q]->colour_blue = 1.0;
									break;
								case 'm': //magenta
									surfacePtr[q]->colour_red = 1.0; surfacePtr[q]->colour_green = 0.0; surfacePtr[q]->colour_blue = 1.0;
									break;
								case 'c': //cyan
									surfacePtr[q]->colour_red = 0.0; surfacePtr[q]->colour_green = 1.0; surfacePtr[q]->colour_blue = 1.0;
									break;
								case 'y': //yellow
									surfacePtr[q]->colour_red = 1.0; surfacePtr[q]->colour_green = 1.0; surfacePtr[q]->colour_blue = 0.0;
									break;
								case 'B': //black
									surfacePtr[q]->colour_red = 0.0; surfacePtr[q]->colour_green = 0.0; surfacePtr[q]->colour_blue = 0.0;
									break;
								case 'W': //white
								case 'w': //white
									surfacePtr[q]->colour_red = 1.0; surfacePtr[q]->colour_green = 1.0; surfacePtr[q]->colour_blue = 1.0;
									break;
							}
		
							done = read_till(InFile, " :");  // get second :
							read_label(InFile, surfacePtr[q]->label);
		
		
							if (strlen(Search_labelPtr) ) { // search by label ?
								echo = FALSE; // preset
								if (strlen(Search_labelPtr) == strlen(surfacePtr[q]->label)) { // if lengths same, check content
									if ( strcmp(Search_labelPtr, surfacePtr[q]->label ) == 0) {
										echo=TRUE;
										Set_raw_switch(OutStream, echo, raw); //  output a raw data switch to reflect the settings
									}
								}
							}
					

							if (done != EOF) {  // now get the index part scale and offsets
		
								if (EOF != getint(InFile, &index)) {
									if (EOF != getint(InFile, &part)) {
										if (EOF != getfloat(InFile, &scalex )) { 
											if (EOF != getfloat(InFile,  &scaley )) {
												if (EOF != getfloat(InFile,  &scalez )) {
													if (EOF != getfloat(InFile,  &offsetx )) {
														if (EOF != getfloat(InFile,  &offsety )) {
															if (EOF != getfloat(InFile,  &offsetz )) {
		
																if (echo) print_header(OutStream, key, 0, surfacePtr[q]->label, index , part , scalex, scaley, scalez, offsetx, offsety, offsetz);
		
																surfacePtr[q]->xscale = scalex ;  
																surfacePtr[q]->yscale = scaley ;  
																surfacePtr[q]->zscale = scalez ;  
																surfacePtr[q]->xoffset = offsetx ;  
																surfacePtr[q]->yoffset = offsety ;  
																surfacePtr[q]->zoffset = offsetz ;  
		
					  
					  
																surfacePtr[q]->part = part ;  
																surfacePtr[q]->index = index ;  
																if ( q > 0 ) (surfacePtr[q])->visible = GLUT_NOT_VISIBLE ;  else (surfacePtr[q])->visible = GLUT_VISIBLE ;  
		
																			  /*
																			   * Allocate some memory.
																			   */
			
																surfacePtr[q]->npolylinenodes = part * index;	
																surfacePtr[q]->polyline = ( float * ) malloc ( (surfacePtr[q])->npolylinenodes * 3 * sizeof ( float )) ; 
			
			
																if (key=='s') {
																	// reading
																	if (textin==TRUE)  { // read into last third of the available space
																		for ( y = 2 * surfacePtr[q]->npolylinenodes, ii = 0 ; y < 3 * surfacePtr[q]->npolylinenodes; y++, ii+=3) 
																				done = fscanf ( InFile," %f",   &(surfacePtr[q])->polyline[y] ) ;
																	}
																	else { // read raw  data
																		done = read_till(InFile, "\r\n"); //look for new line
																		if (done != EOF) fread( &(surfacePtr[q])->polyline[2 * surfacePtr[q]->npolylinenodes] , sizeof ( float ) , (surfacePtr[q])->npolylinenodes ,InFile);
																	}
		
																	//echo
																	if ((echo) && (Search_count >= 0)) {
																		if (raw==FALSE) { //text out
																			for ( y=0, ii = 2 * surfacePtr[q]->npolylinenodes ; y<part; y++ ) {
																				for ( x=0; x<index; x++, ii++) fprintf(OutStream,"%g ", (surfacePtr[q])->polyline[ii] ) ; 		// echo 
																				fputc('\n',OutStream);										  																			// retn
																			}
																		}
																		else { // raw
																			fwrite( &(surfacePtr[q])->polyline[2 * surfacePtr[q]->npolylinenodes], sizeof ( float ) , (surfacePtr[q])->npolylinenodes, OutStream);
																			fputc('\n',OutStream);		// echo retn
																		}
																	}
		
		
																	//rearange
																	for ( y = 2 * surfacePtr[q]->npolylinenodes, ii = 0 ; y < 3 * surfacePtr[q]->npolylinenodes; y++, ii+=3) {
																		(surfacePtr[q])->polyline[ii+2] = (surfacePtr[q])->polyline[y];
																	}
																	
																	
																	for ( y=ii= 0; y<part; y++ ) {
																		for ( x=0; x<index; x++, ii+=3) {
																			(surfacePtr[q])->polyline[ii] = offsetx + scalex*(-1.0 + 2.0 * ((float) x)/ ((float) index)) ;
																			(surfacePtr[q])->polyline[ii+1] = offsety + scaley*(-1 + 2.0 * ((float) y)/ ((float) part)) ;
																			(surfacePtr[q])->polyline[ii+2]= offsetz + (surfacePtr[q])->polyline[ii+2]*scalez;
																		}
																	}
																}
																else if (key == 'S') {
																	// reading
																	if (textin==TRUE)  { // read into last third of the available space
																		for ( y=ii=0; y<part; y++ ) {
																			for ( x=0; x<index; x++, ii+=3) {
																				done = fscanf ( InFile,"%f",   &(surfacePtr[q])->polyline[ii] ) ;
																			}
																		}
																		for ( y=ii=0; y<part; y++ ) {
																			for ( x=0; x<index; x++, ii+=3) {
																				done = fscanf ( InFile,"%f",   &(surfacePtr[q])->polyline[ii+1] ) ;
																			}
																		}
																		for ( y=ii=0; y<part; y++ ) {
																			for ( x=0; x<index; x++, ii+=3) {
																				done = fscanf ( InFile,"%f",   &(surfacePtr[q])->polyline[ii+2] ) ;
																			}
																		}
																	}
																	else { // read raw  data
																		done = read_till(InFile, "\r\n"); //look for new line
																		if (done != EOF) fread( (surfacePtr[q])->polyline , 3 * sizeof ( float ) , (surfacePtr[q])->npolylinenodes ,InFile);
																	}
		
		
		
																	//echo
																	if ((echo) && (Search_count >= 0)) {
																		if (raw==FALSE) { //text out
		
																			for ( y=ii=0; y<part; y++ ) {
																				for ( x=0; x<index; x++, ii+=3) {
																						if (echo) fprintf(OutStream,"%g ", (surfacePtr[q])->polyline[ii] ) ;  
																				}
																				if (echo) fputc('\n',OutStream);
																			}
																			for ( y=ii=0; y<part; y++ ) {
																				for ( x=0; x<index; x++, ii+=3 ) {
																						if (echo) fprintf(OutStream,"%g ", (surfacePtr[q])->polyline[ii+1] ) ; 
																				}
																				if (echo) fputc('\n',OutStream);
																			}
																			for ( y=ii=0; y<part; y++ ) {
																				for ( x=0; x<index; x++, ii+=3) {
																						if (echo) fprintf(OutStream,"%g ", (surfacePtr[q])->polyline[ii+2] ) ;  
																				}
																				if (echo) fputc('\n',OutStream);
																			}
																		}
																		else { // raw
																			fwrite( (surfacePtr[q])->polyline, 3 * sizeof ( float ) , (surfacePtr[q])->npolylinenodes, OutStream);
																			fputc('\n',OutStream);		// echo retn
																		}
																	}
																	
																	for ( y=ii= 0; y<part; y++ ) {
																		for ( x=0; x<index; x++, ii+=3) {
																			(surfacePtr[q])->polyline[ii]= offsetx + (surfacePtr[q])->polyline[ii]*scalex;
																			(surfacePtr[q])->polyline[ii+1]= offsety + (surfacePtr[q])->polyline[ii+1]*scaley;
																			(surfacePtr[q])->polyline[ii+2]= offsetz + (surfacePtr[q])->polyline[ii+2]*scalez;
																		}
																	}
																															
																}
		
																lastVisPtr = ( int *) &(surfacePtr[q]->visible); // note the last surface entered in case there is a follow on like v:f:visible
																lastcolourPtr = ( char *) &(surfacePtr[q]->colour);
																lastcolour_redPtr = ( float *) &(surfacePtr[q]->colour_red);
																lastcolour_greenPtr = ( float *) &(surfacePtr[q]->colour_green);
																lastcolour_bluePtr = ( float *) &(surfacePtr[q]->colour_blue);
		
																Num_surface++; q++;
																surfacePtr[q] = malloc( sizeof( SURF )) ;
																surfacePtr[q]->npolygons		= 0 ;
																surfacePtr[q]->npolylinenodes	= 0 ;
																surfacePtr[q]->polygon = 0;
																surfacePtr[q]->polyline =  (float *) -1;
																surfacePtr[q]->index = (surfacePtr[q])->part = 0;
																surfacePtr[q]->visible = GLUT_NOT_VISIBLE;
		
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}

				if ((Search_count != 0) || ( strlen(Search_labelPtr) > 0 )) echo = FALSE; // reset echo on exit, when selected search in progress 
				


				break;
				
			case 'Q':
			case EOF:
			default:
				break;
		} //end of switch

	} while ((done != EOF) && (done != 'Q'));
	

  if (OutStream != NULL) fclose ( OutStream);
  	
  	
  	
}
