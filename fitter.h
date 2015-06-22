#ifndef __FITTER_H__
#define __FITTER_H__

#include <stdio.h>

#define FLAMBDA  0.001
#define MAX_PASSES 4
#define FITTER_CONTROL_FILE "fitter.dat"


typedef struct {
  float x;
  float y;
  float z;
  float t;
  unsigned int stat;
} Vertex;



typedef struct {
	float u;
	float v;
	float w;
} Dirvec;



typedef struct {
	int lhit;
	float tspmt;
	float twin;
	float c;
	int ipass;
	int imax;
	float disc;
	float tsmax;
	float tskew;
	float tfixw;
	float tcut[MAX_PASSES];
} fitControl;


typedef struct {
	short int jpmt;
	short int jevnt;
	short int jactive;
	short int jstart;
	short int istat;
	Vertex vertex;
	Dirvec dirvec;
	float r;
	float chi;
	short int nhit;
	short int nhitw;
} Fit;

#ifdef  __cplusplus
extern "C" {
#endif

fitControl *initFitter( FILE *fp, float lightSpeed );
Fit *fitter( fitControl *pfit, int jtubes, Vertex *p );
float deltaTi( Vertex *pi, Vertex *p );
float deltaTiP( Vertex *pi, Vertex *p, float dtip[] );
void invrtMatrix4( double c[4][4] );



#ifdef  __cplusplus
}
#endif


#endif
