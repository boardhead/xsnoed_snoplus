#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitter.h"
#include "CUtils.h"

static float	cn;
static int		line_num;

/* default fitter control data */
static char *def_data[] = { 
	"4 1.61 100. 2.249005686",
	"4 100 0.01 2. 2.3 10.",
	"30. 15. 3.8 3.8"
};


static char *getLine( FILE *fp )
{
	static char buff[256];

	if (!fp) {
		/* fp is NULL, so use default fitter control data */
		if (line_num < 3) {
			strcpy(buff, def_data[line_num]);
			++line_num;
		} else {
			return(NULL);
		}
	} else if( !fgets( buff, sizeof(buff), fp ) ) {
		if( feof( fp ) ) {
			Printf("Unexpected EOF\n" );
		} else {
			Printf("I/O error\n");
		}
		return( NULL );
	}
	return( buff);
}

/* lightSpeed in cm/ns -- set to zero to read this value from file */
/* - uses default fitter control data if fp is NULL - PH 01/28/00 */
fitControl *initFitter( FILE *fp, float lightSpeed )
{
	char *p;
	char *delim = " \t\n";
	int i;
	fitControl *pfit;

	line_num = 0;
	if( !( pfit = (fitControl *)malloc( sizeof(fitControl) ) ) ) {
		Printf("Cannot allocate memory for fitter control block\n");
		return(NULL);
	}
	if( !( p = getLine( fp ) ) ) {
		free( pfit );
		return( NULL );
	}
	p = strtok( p, delim );
	pfit->lhit = atoi( p );
	if( pfit->lhit < 4 ) {
		Printf("Fitter needs at least 4 tubes to work!\n" );
		free( pfit );
		return( NULL );
	}
	p = strtok( NULL, delim );
	pfit->tspmt = atof( p );
	p = strtok( NULL, delim );
	pfit->twin = atof( p );
	p = strtok( NULL, delim );
	if (lightSpeed != 0) {
		pfit->c = 0.1 * lightSpeed;
	} else {
		pfit->c = atof( p );
	}
	cn = 0.1 / pfit->c;
	if( !( p = getLine( fp ) ) ) return( NULL );
	p = strtok( p, delim );
	pfit->ipass = atoi( p );
	if( pfit->ipass > MAX_PASSES ) {
		Printf("Too many fitter passes\n" );
		free( pfit );
		return( NULL );
	}
	p = strtok( NULL, delim );
	pfit->imax = atoi( p );
	p = strtok( NULL, delim );
	pfit->disc = atof( p );
	p = strtok( NULL, delim );
	pfit->tsmax = atof( p );
	p = strtok( NULL, delim );
	pfit->tskew = atof( p );
	p = strtok( NULL, delim );
	pfit->tfixw = atof( p );
	if( !( p = getLine( fp ) ) ) {
		free( pfit );
		return( NULL );
	}
	p = strtok( p, delim );
	for( i=0; i<pfit->ipass; ++i ) {
		pfit->tcut[i] = atof( p );
		p = strtok( NULL, delim );
	}
	return( pfit );
}


float deltaTi( Vertex *pi, Vertex *p )
{
  float dr, dti;
	
  dr = sqrt( ( pi->x - p->x ) * ( pi->x - p->x ) +
						 ( pi->y - p->y ) * ( pi->y - p->y ) +
				     ( pi->z - p->z ) * ( pi->z - p->z ) );
						 dti = cn * dr - ( pi->t - p->t );
  return( dti );
}

float deltaTiP( Vertex *pi, Vertex *p, float dtip[] )
{
  float dr, dti;
	
  dr = sqrt( ( pi->x - p->x ) * ( pi->x - p->x ) +
	     ( pi->y - p->y ) * ( pi->y - p->y ) +
	     ( pi->z - p->z ) * ( pi->z - p->z ) );
			 dti = cn * dr - ( pi->t - p->t );
	dtip[3] = 1.;
	if( dr ) {
		dtip[2] = cn * ( p->z - pi->z ) / dr;
		dtip[1] = cn * ( p->y - pi->y ) / dr;
		dtip[0] = cn * ( p->x - pi->x ) / dr;
	} else {
		dtip[2] = 0.;
		dtip[1] = 0.;
		dtip[0] = 0.;
	}
  return( dti );
}

void invrtMatrix4( double c[4][4] )
{
	int k, l, m;
	double a;

	for( k=0; k<4; ++k ) {
		a = 1. / c[k][k];
		c[k][k] = 1.;
		for( l=0; l<4; ++l ) c[k][l] *= a;
		for( m=0; m<4; ++m ) {
			if( m == k ) continue;
			a = c[m][k];
			c[m][k] = 0.;
			for( l=0; l<4; ++l ) c[m][l] -= a * c[k][l];
		}
	}
	return;
}


Fit *fitter( fitControl *f_ctrl, int jtubes, Vertex *p_i )
{
	int i, j, j1, j2, jtmp, l, m, nj;
	int ifit=0, ip;
	float chi0, chi1=0, dti, dtip[4], t1;
	double fl, b[4], b1[4], c[4][4], c1[4][4];
	Vertex *p, *p1;
	Dirvec *e;
	Fit *fit;

	if( !( p1 = (Vertex *)malloc( sizeof(Vertex) ) ) ) {
		return( NULL );
	}
	if( !( fit = (Fit *)malloc( sizeof(Fit) ) ) ) {
		free( p1 );
		return( NULL );
	}

	fit->jpmt = jtubes;
	fit->jevnt = -1;
	fit->jactive = -1;
	fit->istat = -2;
	fit->nhit = -1;
	fit->nhitw = -1;
	if( jtubes < f_ctrl->lhit ) {
		free( p1 );
		return( fit );    /*istat = -2*/
	}
	++fit->istat;
	p = &fit->vertex;
	e = &fit->dirvec;

	while( 1 ) {
		i = 1;
		for( j=1; j<jtubes; ++j ) {
			t1 = ( p_i + j - 1 )->t;
			if( t1 > ( p_i + j )->t ) {
				i = 0;
				memcpy( p, ( p_i + j - 1 ), sizeof(Vertex) );
				memcpy( ( p_i + j - 1 ), ( p_i + j ), sizeof(Vertex) );
				memcpy( ( p_i + j ), p, sizeof(Vertex) );
			}
		}
		if( i ) break;
	}

	j = 0;
	for( j1=0; j1<jtubes; ++j1 ) {
		if( ( p_i + j1 )->stat & 0x8000 ) continue;
		t1 = ( p_i + j1 )->t + f_ctrl->twin;
		for( j2=jtubes-1; j1<=j2; --j2 ) {
			if( ( p_i + j2 )->stat & 0x8000 ) continue;
			if( ( p_i + j2 )->t <= t1 ) break;
  	}
	  jtmp = j2 - j1 + 1;
		if( jtmp >= fit->jevnt ) {
			fit->jevnt = jtmp;
		  j = j1;
		}
	}
	j1 = j;
	j2 = fit->jevnt + j1 - 1;
	fit->jstart = j1;
	if( fit->jevnt < f_ctrl->lhit ) {
		free( p1 );
		return( fit );    /*istat = -1 */
	}

	memset( p, 0, sizeof(Vertex) );
	fit->jactive = 0;
	for( j=0; j<jtubes; ++j ) {
		if( ( p_i + j )->stat & 0x8000 ) continue;
		if( j < j1 || j2 < j ) {
			(	p_i + j )->stat |= 0x8000;
			continue;
		}
		p->x += ( p_i + j )->x;
		p->y += ( p_i + j )->y;
		p->z += ( p_i + j )->z;
		++fit->jactive;
	}
	p->x /= fit->jactive;
	p->y /= fit->jactive;
	p->z /= fit->jactive;
	t1 = 0.;
	for( j=j1; j<=j2; ++j ) {
		if( ( p_i + j )->stat & 0x8000 ) continue;
		t1 += deltaTi( p_i + j, p );
	}
	p->t = - t1 / fit->jactive;

	for( ip=0; ip<f_ctrl->ipass; ++ip ) {
		++fit->istat;
		fl = FLAMBDA;
		ifit = 0;
		for( i=0; i<f_ctrl->imax; ++i ) {
			memset( b, 0, sizeof(b) );
			memset( c, 0, sizeof(c) );
			chi0 =  0.;
			nj = 0;
			for( j=j1; j<=j2; ++j ) {
				if( ( p_i + j )->stat & 0x8000 ) continue;
				dti = deltaTiP( p_i + j, p, dtip );
				if( f_ctrl->tcut[ip] < fabs( dti ) ) continue;
				++nj;
				if( dti < -f_ctrl->tskew ) {
					for( l=0; l<4; ++l ) b[l] += f_ctrl->tskew * dtip[l];
					chi0 -= f_ctrl->tskew * ( 2. * dti + f_ctrl->tskew );
				} else {
					for( l=0; l<4; ++l ) {
						b[l] -= dtip[l] * dti;
						for( m=0;m<4;++m ) c[l][m] += dtip[l] * dtip[m];
					}
					chi0 += dti * dti;
				}
			}
			if( nj <= f_ctrl->lhit ) {
				fit->nhit = nj;
				free( p1 );
				return( fit );
			}
			for( l=0; l<4; ++l ) {
				if( c[l][l] == 0. ) {
					free( p1 );
					return( fit );
				}
			}

			while( 1 ) {
				for( l=0; l<4; ++l ) b1[l] = sqrt( c[l][l] );
				for( l=0; l<4; ++l ) {
					for( m=0; m<4; ++m ) c1[l][m] = c[l][m] / ( b1[l] * b1[m] );
				}
				for( l=0; l<4; ++l ) c1[l][l] += fl;
				invrtMatrix4( c1 );
				memcpy( p1, p, sizeof(Vertex) );
				for( l=0; l<4; ++l ) {
					for( m=0; m<4; ++m ) ((float *)p1)[l] += 
										 c1[l][m] * b[m] / ( b1[l] * b1[m] );
				}
				if( fabs( p->t - p1->t ) < f_ctrl->tsmax ) {
					nj = 0;
					chi1 = 0.;
					for( j=j1; j<=j2; ++j ) {
						if( ( p_i + j )->stat & 0x8000 ) continue;
						dti = deltaTi( p_i + j, p1 );
						if( fabs( dti ) > f_ctrl->tcut[ip] ) continue;
						++nj;
						if( dti < -f_ctrl->tskew ) {
							chi1 -= f_ctrl->tskew * ( 2. * dti + f_ctrl->tskew );
						} else {
							chi1 += dti * dti;
						}
					}
					if( chi0 >= chi1 ) break;
				}
				fl *= 10.;
			}
			
			memcpy( p, p1, sizeof(Vertex) );
			fl /= 10.;
			if( fabs( chi0 - chi1 ) <= f_ctrl->disc * chi1 ) break;
 		}	
		if( i == f_ctrl->imax ) continue;

		memset( e, 0, sizeof(Dirvec) );
		for( j=j1; j<=j2; ++j ) {
			if( ( p_i + j )->stat & 0x8000 ) continue;
			dti = deltaTiP( p_i + j, p, dtip );
			if( fabs( dti ) > f_ctrl->tcut[ip] ) {
				( p_i + j )->stat |= 0x8000;
			} else {
				for( l=0; l<3; ++l ) ((float *)e)[l] -= dtip[l];
				t1 = sqrt( e->u * e->u + e->v * e->v + e->w * e->w );
				for( l=0; l<3; ++l ) ((float *)e)[l] /= t1;
				ifit = 1;
			}
		}
	}
	if( ifit ) {
		fit->nhitw = 0;
		fit->nhit = 0;
		for( j=j1; j<=j2; ++j ) {
			dti = deltaTi( p_i + j, p );
			if( fabs( dti ) <= f_ctrl->tfixw ) ++fit->nhitw;
			if( !( ( p_i + j )->stat & 0x8000 ) ) ++fit->nhit;
		}

		if( fit->nhit <= f_ctrl->lhit ) {
			fit->istat = MAX_PASSES;    /*This should never happen ... */
		}
	}

	fit->chi = chi1 / ( f_ctrl->tspmt * f_ctrl->tspmt * ( fit->nhit - 4 ) );
	fit->r = sqrt( p->x * p->x + p->y * p->y + p->z * p->z );
	free( p1 );
	return( fit );
}
