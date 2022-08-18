/* ==========================================================================
                          MAKE_SURFACE_C
=============================================================================

  
============================================================================ */

#include <math.h>           /* C header for any math functions               */
#include <stdio.h>          /* C header for standard I/O                     */
#include <string.h>         /* For String compare                            */
#include <stdlib.h>
//#ifndef _WIN32
#include <sys/types.h>
#include <sys/file.h>
//#endif

#include "memory.h"         /* Local memory allocation macros                */
#include "head.h"	    /* local header for the surface		     */


void make_surface ( SURF **surfacePtr, int Num_surface );

/* ========================================================================= */  
/* surface_reset	                                                             */
/* ========================================================================= */  
/*
** Resets the geometry of the surface to neutral.
**
*/

void surface_reset ( SURF *surface )
{
  int i,j,k ;

  
	for ( i=0; i<surface->npolygons; i++ ) {
		for ( j=0; j<3; j++ ) {
			for ( k=0; k<3; k++ ) {
				surface->polygon[i]->vertex[j]->xyz[k] = surface->polygon[i]->vertex[j]->nxyz[k] ;
			}
		}
	}
}


/* ========================================================================= */  
/* make_surface                                                                 */
/* ========================================================================= */  
/*
** makes the surface etc adding a pair of polygons at a time...
**
*/

void
make_surface ( SURF **surfacePtr, int Num_surface) 
{
  POLYGON  *p ;
  long j, k, x, y,
      p1, p2, p3, p4 ;
  int i;
  float tilex, tiley;

	for (i=0; i < Num_surface; i++) {
		if ((surfacePtr[i])->polyline !=  (float *) -1) {
//			printf("%ld\n", surfacePtr[i]->npolylinenodes * sizeof ( POLYGON ) ); // debug
			if (surfacePtr[i]->polygon == 0)  surfacePtr[i]->polygon = _new_array(POLYGON *,surfacePtr[i]->npolylinenodes * sizeof ( POLYGON ) ) ;
			
			tilex = .2;
			for ( x=0; x < surfacePtr[i]->index - 1 ; x++) {
				tiley = tilex = 1.0 - tilex;
			for ( y=0; y < surfacePtr[i]->part - 1 ; y++) {
				tiley = 1.0 - tiley;
		  

				p1 = x + y * surfacePtr[i]->index ;
				p2 = p1 + 1  ;
				p3 = p2 + surfacePtr[i]->index  ;
				p4 = p1 + surfacePtr[i]->index ;


				p = _new ( POLYGON ) ;
				for (j=0; j<3; j++) {
					p->vertex[j] = _new ( VERTEX ) ;
					p->vertex[j]->np = 0 ;
					p->rgb[j] = tiley ;			//default set up neutral colour: highlight
				}

				for (j=0; j<3; j++) {
					p->vertex[0]->nxyz[j] = 
					p->vertex[0]->xyz[j]  = surfacePtr[i]->polyline[ p1*3 + j ] ;
					
					p->vertex[1]->nxyz[j] =
					p->vertex[1]->xyz[j]  = surfacePtr[i]->polyline[ p2*3 + j ] ;
					
					p->vertex[2]->nxyz[j] = 
					p->vertex[2]->xyz[j]  = surfacePtr[i]->polyline[ p3*3 + j ] ;
				}

		//      add_polygon_to_surface ( p, surface ) ;
			surfacePtr[i]->polygon[surfacePtr[i]->npolygons] = p ;
			surfacePtr[i]->npolygons++;

				p = _new ( POLYGON ) ;
				for (j=0; j<3; j++) {
					p->vertex[j] = _new ( VERTEX ) ;
					p->vertex[j]->np = 0 ;
					p->rgb[j] = tiley ;			//set up neutral colour: lowlight
				}

				for (j=0; j<3; j++) {
					p->vertex[0]->nxyz[j] = 
					p->vertex[0]->xyz[j]  = surfacePtr[i]->polyline[ p1*3 + j ] ;
					
					p->vertex[1]->nxyz[j] =
					p->vertex[1]->xyz[j]  = surfacePtr[i]->polyline[ p3*3 + j ] ;
					
					p->vertex[2]->nxyz[j] = 
					p->vertex[2]->xyz[j]  = surfacePtr[i]->polyline[ p4*3 + j ] ;
				}

		//      add_polygon_to_surface ( p, surface ) ;
			surfacePtr[i]->polygon[surfacePtr[i]->npolygons] = p ;
			surfacePtr[i]->npolygons++;
		//      }
		  }
		  }
		  surfacePtr[i]->polygon = _resize_array(surfacePtr[i]->polygon,POLYGON *,surfacePtr[i]->npolygons) ;
//		  printf("size of data %ld \n", (long) surfacePtr[i]->npolygons); //debug
		}
	} // end for i
}


/* ========================================================================= */  
/* averaged_vertex_normals                                                   */
/* ========================================================================= */  
/*
** Caculates the averaged polygon normal.
*/

void averaged_vertex_normals ( SURF *surface, long p, float *n1, float *n2, float *n3 ) 
{
  long i,j,np, pt ;
  float norm[3] ;


  for (i=0; i<3; i++)
    norm[i] = 0.0 ;

  np = surface->polygon[p]->vertex[0]->np ;

  for ( i=0; i<np; i++) {
    pt = surface->polygon[p]->vertex[0]->plist[i] ;

    for ( j=0; j<3; j++)  {
      norm[j] += surface->polygon[pt]->vertex[0]->norm[j] ;
    }
  }

  for (i=0; i<3; i++)
    norm[i] = norm[i] / (float) np ;
    
  for (i=0; i<3; i++)
    n1[i] = norm[i] ;

  for (i=0; i<3; i++)
    norm[i] = 0.0 ;

  np = surface->polygon[p]->vertex[1]->np ;

  for ( i=0; i<np; i++) {
    pt = surface->polygon[p]->vertex[1]->plist[i] ;

    for ( j=0; j<3; j++) {
      norm[j] += surface->polygon[pt]->vertex[1]->norm[j] ;
    }
  }

  for (i=0; i<3; i++)
    norm[i] = norm[i] / (float) np ;

  for (i=0; i<3; i++)
    n2[i] = norm[i] ;

  for (i=0; i<3; i++)
    norm[i] = 0.0 ;

  np = surface->polygon[p]->vertex[2]->np ;

  for ( i=0; i<np; i++) {
    pt = surface->polygon[p]->vertex[2]->plist[i] ;

    for ( j=0; j<3; j++) {
      norm[j] += surface->polygon[pt]->vertex[2]->norm[j] ;
    }
  }

  for (i=0; i<3; i++)
    norm[i] = norm[i]/ (float) np ;
    
  for (i=0; i<3; i++)
    n3[i] = norm[i] ;
   
}

/* ========================================================================= */  
/* data_struct	                                                             */
/* ========================================================================= */  
/*
** Create a new data structure for the polygons.
**
*/
#define DATA_STRUCT_DEBUG 0

void
data_struct ( SURF **surfacePtr, int Num_surface )
{
  long i,j, offset, n ;
  long flag, cptr ;
  float x1,y1,z1, x2, y2, z2, x3, y3, z3 ;
  float tx1, ty1, tz1, tx2, ty2, tz2, tx3, ty3, tz3 ;
  int p ;

for (p = 0; p < Num_surface; p++) {

	 offset = 2*surfacePtr[p]->part-1;
	 
	  for (i=0; i<surfacePtr[p]->npolygons; i++ ){

		  x1 = surfacePtr[p]->polygon[i]->vertex[0]->xyz[0] ;
		  y1 = surfacePtr[p]->polygon[i]->vertex[0]->xyz[1] ;
		  z1 = surfacePtr[p]->polygon[i]->vertex[0]->xyz[2] ;

		  x2 = surfacePtr[p]->polygon[i]->vertex[1]->xyz[0] ;
		  y2 = surfacePtr[p]->polygon[i]->vertex[1]->xyz[1] ;
		  z2 = surfacePtr[p]->polygon[i]->vertex[1]->xyz[2] ;

		  x3 = surfacePtr[p]->polygon[i]->vertex[2]->xyz[0] ;
		  y3 = surfacePtr[p]->polygon[i]->vertex[2]->xyz[1] ;
		  z3 = surfacePtr[p]->polygon[i]->vertex[2]->xyz[2] ;


		  flag = 0 ;	  
		  if (0 > (j = i - 3)) j = 0;
				  while ( !flag  &&
					  j<surfacePtr[p]->npolygons ) {

					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( (x1 == tx1 && y1 == ty1 && z1 == tz1) ||
						 (x1 == tx2 && y1 == ty2 && z1 == tz2) ||
						 (x1 == tx3 && y1 == ty3 && z1 == tz3)) {
					  cptr = j ;
	// printf("got here 0 %ld\n", j -i );
					  n = surfacePtr[p]->polygon[i]->vertex[0]->np ;
					  surfacePtr[p]->polygon[i]->vertex[0]->plist[n] = cptr ;
					  surfacePtr[p]->polygon[i]->vertex[0]->np++ ;
					}
					
					j++;
					if (j > i + 1) flag = 1;
				  } /* end while */

				flag = 0 ;
				if (0 > (j = i - offset - 3)) j = 0;
				  while ( !flag  &&
					  j<surfacePtr[p]->npolygons ) {
			
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( (x1 == tx1 && y1 == ty1 && z1 == tz1) ||
						 (x1 == tx2 && y1 == ty2 && z1 == tz2) ||
						 (x1 == tx3 && y1 == ty3 && z1 == tz3)) {
					  cptr = j ;
	//	printf("got here 0 %ld\n", j -i );
					  n = surfacePtr[p]->polygon[i]->vertex[0]->np ;
					  surfacePtr[p]->polygon[i]->vertex[0]->plist[n] = cptr ;
					  surfacePtr[p]->polygon[i]->vertex[0]->np++ ;
					}
					j++ ;
					if (j > i - offset + 3 ) flag = 1;

			} /* end while */
			

			  flag = 0 ; j = 0;
			if (0 > (j = i - 3)) j = 0;
			  while ( !flag  &&
				  j<surfacePtr[p]->npolygons ) {
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( (x2 == tx1 && y2 == ty1 && z2 == tz1) ||
						 (x2 == tx2 && y2 == ty2 && z2 == tz2) ||
						 (x2 == tx3 && y2 == ty3 && z2 == tz3)) {
					  cptr = j ;
					  
	//	printf("got here 1 %ld\n", j -i );
		
					  n = surfacePtr[p]->polygon[i]->vertex[1]->np ;
					  surfacePtr[p]->polygon[i]->vertex[1]->plist[n] = j ;
					  surfacePtr[p]->polygon[i]->vertex[1]->np++ ;

					}
					j++ ;
					if (j > i + 2) flag = 1;
				} /* end while */

			  flag = 0 ;
			if (0 > (j = i +(offset+1) -3 )) j = 0;
			  while ( !flag  &&
				  j<surfacePtr[p]->npolygons ) {
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( (x2 == tx1 && y2 == ty1 && z2 == tz1) ||
						 (x2 == tx2 && y2 == ty2 && z2 == tz2) ||
						 (x2 == tx3 && y2 == ty3 && z2 == tz3)) {
					  cptr = j ;
	//	printf("got here 1 %ld\n", j -i );

					  n = surfacePtr[p]->polygon[i]->vertex[1]->np ;
					  surfacePtr[p]->polygon[i]->vertex[1]->plist[n] = j ;
					  surfacePtr[p]->polygon[i]->vertex[1]->np++ ;

					}
					j++ ;
					if (j > i + (offset+1) + 1) flag = 1;
				} /* end while */

			  flag = 0 ;
			  if (0 > (j = i) ) j = 0;
			  while ( !flag  &&
				  j<surfacePtr[p]->npolygons ) {
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( x3 == tx1 &&  y3 == ty1 &&  z3 == tz1 ||
						 x3 == tx2 &&  y3 == ty2 &&  z3 == tz2 ||
						 x3 == tx3 &&  y3 == ty3 &&  z3 == tz3) {
					  cptr = j ;

	// printf("got here 2 %ld\n", j-i );

					  n = surfacePtr[p]->polygon[i]->vertex[2]->np ;
					  surfacePtr[p]->polygon[i]->vertex[2]->plist[n] = cptr ;
					  surfacePtr[p]->polygon[i]->vertex[2]->np++ ;

					}
					j++ ;
					if (j > i + 2) flag = 1;

				  } /* end while */
			  flag = 0 ;
			if (0 > (j = i - offset )) j = 0;
			  while ( !flag  &&
				  j<surfacePtr[p]->npolygons ) {
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( x3 == tx1 &&  y3 == ty1 &&  z3 == tz1 ||
						 x3 == tx2 &&  y3 == ty2 &&  z3 == tz2 ||
						 x3 == tx3 &&  y3 == ty3 &&  z3 == tz3) {
					  cptr = j ;
	// printf("got here 2 %ld\n", j-i );

					  n = surfacePtr[p]->polygon[i]->vertex[2]->np ;
					  surfacePtr[p]->polygon[i]->vertex[2]->plist[n] = cptr ;
					  surfacePtr[p]->polygon[i]->vertex[2]->np++ ;

					}
					j++ ;
					if (j > i - offset + 3) flag = 1;

				  } /* end while */
			  flag = 0 ;
			if (0 > (j = i + offset )) j = 0;
			  while ( !flag  &&
				  j<surfacePtr[p]->npolygons ) {
					tx1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[0] ;
					ty1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[1] ;
					tz1 = surfacePtr[p]->polygon[j]->vertex[0]->xyz[2] ;

					tx2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[0] ;
					ty2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[1] ;
					tz2 = surfacePtr[p]->polygon[j]->vertex[1]->xyz[2] ;

					tx3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[0] ;
					ty3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[1] ;
					tz3 = surfacePtr[p]->polygon[j]->vertex[2]->xyz[2] ;

					if ( x3 == tx1 &&  y3 == ty1 &&  z3 == tz1 ||
						 x3 == tx2 &&  y3 == ty2 &&  z3 == tz2 ||
						 x3 == tx3 &&  y3 == ty3 &&  z3 == tz3) {
					  cptr = j ;
	//	printf("got here 2 %ld\n", j-i );

					  n = surfacePtr[p]->polygon[i]->vertex[2]->np ;
					  surfacePtr[p]->polygon[i]->vertex[2]->plist[n] = cptr ;
					  surfacePtr[p]->polygon[i]->vertex[2]->np++ ;

					}
					j++ ;
					if (j > i + offset + 2 ) flag = 1;

				  } /* end while */

		} /* end for i */
	} /* end for p */

}



