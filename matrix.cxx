#include <math.h>
#include <string.h>
#include "matrix.h"

#define		NEAR_ZERO	1.0e-20

#define		MNUM	3

/*---------------------------------------------------------------------------
** Get the length of a vector
*/
float vectorLen(float x, float y, float z)
{
	return(sqrt(x*x+y*y+z*z));
}
int unitVector(Vector3 v)
{
	float	r = vectorLen(v[0],v[1],v[2]);

	if (r) { 
		v[0]/=r; 
		v[1]/=r; 
		v[2]/=r;
		return(0);
	} else {
		return(1);
	}
}
/*
** Get the rotation matrix for the given angles:
**		theta - angle of rotation axis from the x axis measured ccw in the x-y plane
**		phi   - angle of rotation axis from the z axis
**		alpha - angle of cw rotation about the axis of rotation
*/
void getRotMatrix(Matrix3 rot, float theta, float phi, float alpha)
{
	float	st,sp,sa;
	float	ct,cp,ca;

	st = sin(theta); sp = sin(phi); sa = sin(alpha);
	ct = cos(theta); cp = cos(phi); ca = cos(alpha);

	rot[0][0] = ((ct*cp*ca+st*sa)*cp + ct*sp*sp)*ct - (ct*cp*sa-st*ca)*st;
	rot[0][1] = ((ct*cp*ca+st*sa)*cp + ct*sp*sp)*st + (ct*cp*sa-st*ca)*ct;
	rot[0][2] = -(ct*cp*ca+st*sa)*sp + ct*sp*cp;
	rot[1][0] = ((st*cp*ca-ct*sa)*cp + st*sp*sp)*ct - (st*cp*sa+ct*ca)*st;
	rot[1][1] = ((st*cp*ca-ct*sa)*cp + st*sp*sp)*st + (st*cp*sa+ct*ca)*ct;
	rot[1][2] = -(st*cp*ca-ct*sa)*sp + st*sp*cp;
	rot[2][0] = (-sp*ca*cp+cp*sp)*ct + sp*sa*st;
	rot[2][1] = (-sp*ca*cp+cp*sp)*st - sp*sa*ct;
	rot[2][2] =   sp*ca*sp+cp*cp;
}
/*
**	Construct 3-D rotation matrix for Euler angles (alpha,beta,gamma)
**	using standard y-axis convention.
*/
void get3DMatrix(Matrix3 rot, float alpha, float beta, float gamma)
{
	float	ca = cos(alpha);
	float	sa = sin(alpha);
	float	cb = cos(beta);
	float	sb = sin(beta);
	float	cg = cos(gamma);
	float	sg = sin(gamma);
/*
** alpha) about z axis, beta) about new x axis, gamma) about new z axis
	rot[0][0] =  cg*ca - cb*sa*sg;
	rot[0][1] =  cg*sa + cb*ca*sg;
	rot[0][2] =  sg*sb;
	rot[1][0] = -sg*ca - cb*sa*cg;
	rot[1][1] = -sg*sa + cb*ca*cg;
	rot[1][2] =  cg*sb;
	rot[2][0] =  sb*sa;
	rot[2][1] = -sb*ca;
	rot[2][2] =  cb;
*/
/*
** alpha) about z axis, beta) about new y axis, gamma) about new z axis
*/
	rot[0][0] =  cg*cb*ca - sg*sa;
	rot[0][1] =  cg*cb*sa + sg*ca;
	rot[0][2] = -cg*sb;
	rot[1][0] = -sg*cb*ca - cg*sa;
	rot[1][1] = -sg*cb*sa + cg*ca;
	rot[1][2] =  sg*sb;
	rot[2][0] =  sb*ca;
	rot[2][1] =  sb*sa;
	rot[2][2] =  cb;
}
/*
** Construct a 3-D rotation matrix for rotations about the
** Y, X, then Z axes
*/
void getRotMatrixYXZ(Matrix3 rot, double alpha, double beta, double gamma)
{
	double ca = cos(alpha);
	double sa = sin(alpha);
	double cb = cos(beta);
	double sb = sin(beta);
	double cg = cos(gamma);
	double sg = sin(gamma);
/*
** alpha) about y axis, beta) about new x axis, gamma) about new z axis
*/
	rot[0][0] =  ca*cg - sa*sb*sg;
	rot[0][1] = -cb*sg;
	rot[0][2] =  sa*cg + ca*sb*sg;
	rot[1][0] =  ca*sg + sa*sb*cg;
	rot[1][1] =  cb*cg;
	rot[1][2] =  sa*sg - ca*sb*cg;
	rot[2][0] = -sa*cb;
	rot[2][1] =  sb;
	rot[2][2] =  ca*cb;
}

/*
** Multiply a vector by a matrix
*/
void vectorMult(Matrix3 m, Vector3 in, Vector3 out)
{
	out[0] = m[0][0]*in[0] + m[0][1]*in[1] + m[0][2]*in[2];
	out[1] = m[1][0]*in[0] + m[1][1]*in[1] + m[1][2]*in[2];
	out[2] = m[2][0]*in[0] + m[2][1]*in[1] + m[2][2]*in[2];
}
/*
** Multiply a destination matrix (d) by a source matrix (s)
*/
void matrixMult(Matrix3 d, Matrix3 s)
{
	int		i,j;
	Matrix3		t;

	memcpy(t,d,sizeof(Matrix3));
	for (i=0; i<3; ++i) {
		for (j=0; j<3; ++j) {
			d[j][i] = s[j][0]*t[0][i] + s[j][1]*t[1][i] + s[j][2]*t[2][i];
		}
	}
}
/*
** Initialize a matrix to the identity matrix
*/
void matrixIdent(Matrix3 m)
{
	memset(m,0,sizeof(Matrix3));
	m[0][0] = m[1][1] = m[2][2] = 1;
}
/*.........................................................................
** Get rotation matrix to align (0,0,1) with given vector
** (rotation about y (t) then about x (p), where
**  ct=p[2]/r, st=p[1]/r, cp=r, sp=p[0])
**
** Note: input vector must have unit length!
*/
void getRotAlign(Vector3 p, Matrix3 out)
{
	float	r = sqrt(1 - p[0]*p[0]);

	if (!r) {
		out[0][0] =  0;
		out[0][1] =  0;
		out[0][2] =  1;
		out[1][0] =  0;
		out[1][1] =  1;
		out[1][2] =  0;
		out[2][0] = -1;
		out[2][1] =  0;
		out[2][2] =  0;
	} else {
		out[0][0] =  r;
		out[0][1] =  0;
		out[0][2] =  p[0];
		out[1][0] = -p[0] * p[1] / r;
		out[1][1] =  p[2] / r;
		out[1][2] =  p[1];
		out[2][0] = -p[0] * p[2] / r;
		out[2][1] = -p[1] / r;
		out[2][2] =  p[2];
	}
}
/* input vector doesn't need to be unit length here */
void getRotAlignScaled(Vector3 p, Matrix3 out)
{
	float	len = vectorLen(p[0], p[1], p[2]);
	float	fac = 1.0 / len;
	float	r = sqrt(1 - p[0]*p[0]*fac*fac);

	if (!r) {
		out[0][0] =  0;
		out[0][1] =  0;
		out[0][2] =  len;
		out[1][0] =  0;
		out[1][1] =  len;
		out[1][2] =  0;
		out[2][0] = -len;
		out[2][1] =  0;
		out[2][2] =  0;
	} else {
		out[0][0] =  len * r;
		out[0][1] =  0;
		out[0][2] =  p[0];
		out[1][0] = -p[0] * p[1] * fac / r;
		out[1][1] =  p[2] / r;
		out[1][2] =  p[1];
		out[2][0] = -p[0] * p[2] * fac / r;
		out[2][1] = -p[1] / r;
		out[2][2] =  p[2];
	}
}
/*
** Get rotation matrix to align given vector with (0,0,1)
*/
void getRotAlignInv(Vector3 p, Matrix3 out)
{
	float	r = sqrt(1 - p[0]*p[0]);

	if (!r) {
		out[0][0] =  0;
		out[0][1] =  0;
		out[0][2] = -1;
		out[1][0] =  0;
		out[1][1] =  1;
		out[1][2] =  0;
		out[2][0] =  1;
		out[2][1] =  0;
		out[2][2] =  0;
	} else {
		out[0][0] =  r;
		out[0][1] = -p[0] * p[1] / r;
		out[0][2] = -p[0] * p[2] / r;
		out[1][0] =  0;
		out[1][1] =  p[2] / r;
		out[1][2] = -p[1] / r;
		out[2][0] =  p[0];
		out[2][1] =  p[1];
		out[2][2] =  p[2];
	}
}
/*
** Find the transpose of a matrix (this inverts a rotation matrix)
*/
void matrixTranspose(Matrix3 in, Matrix3 out)
{
	int		i,j;

	for (j=0; j<3; ++j) {
		for (i=0; i<3; ++i) {
			out[j][i] = in[i][j];
		}
	}
}
/*
** Set element (row,col) of matrix m to zero by combining this row
** with an appropriately scaled multiple of the source row
*/
void combineVector4(float m[3][4], int row, int col, int src)
{
	int	i;
	float	f, d = m[src][col];

	if (!d) return;

	f = m[row][col] / d;

	for (i=0; i<4; ++i) m[row][i] -= m[src][i] * f;
}
/*
** Solve a matrix equation by combining the rows
** to eliminate off-diagonal elements.
*/
void matrixSolve(Matrix3 m, Vector3 in, Vector3 out)
{
	int		i,j,k;
	float	tmp[3][4], t;

	for (i=0; i<3; ++i) {
		memcpy(tmp[i],m[i],3*sizeof(float));
		tmp[i][3] = in[i];
	}
	for (i=0; i<6; ++i) {
		j = i%3;
		k = 2-i/2;
		combineVector4(tmp, j, k, k);
		if (i>=3) {
			t = tmp[j][j];
			if (t) out[j] = tmp[j][3] / t;
			else   out[j] = tmp[j][3];
		}
	}
}
void lubksb(float a[MNUM][MNUM],int indx[MNUM],float b[MNUM])
{
	int		i,ii=-1,ip,j;
	float	sum;

	for (i=0; i<MNUM; ++i) {
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii>=0) for (j=ii; j<i; ++j) sum -= a[i][j]*b[j];
		else if (sum) ii = i;
		b[i] = sum;
	}
	for (i=MNUM-1; i>=0; --i) {
		sum = b[i];
		for (j=i+1; j<MNUM; ++j) sum -= a[i][j]*b[j];
		b[i] = sum/a[i][i];
	}
}
void ludcmp(float a[MNUM][MNUM],int indx[MNUM], float *d)
{
	int		i,imax=0,j,k;
	float	big,dum,sum,temp;
	float	vv[MNUM];

	*d = 1.0;
	for (i=0; i<MNUM; ++i) {
		big = 0.0;
		for (j=0; j<MNUM; ++j) if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) return;		/* ERROR: singular matrix!!! */
		vv[i] = 1.0/big;
	}
	for (j=0; j<MNUM; ++j) {
		for (i=0; i<j; ++i) {
			sum = a[i][j];
			for (k=0; k<i; ++k) sum -= a[i][k]*a[k][j];
			a[i][j] = sum;
		}
		big = 0.0;
		for (i=j; i<MNUM; ++i) {
			sum = a[i][j];
			for (k=0; k<j; ++k)
				sum -= a[i][k]*a[k][j];
			a[i][j] = sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (k=0; k<MNUM; ++k) {
				dum = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = dum;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (a[j][j] == 0.0) a[j][j] = NEAR_ZERO;
		if (j != MNUM-1) {
			dum = 1.0/a[j][j];
			for (i=j+1; i<MNUM; ++i) a[i][j] *= dum;
		}
	}
}

void matrixInvert(Matrix3 in, Matrix3 out)
{
	int		i,j,indx[MNUM];
	float	d;
	Matrix3	tmp;
	Vector3	col;

	memcpy(tmp,in,sizeof(Matrix3));

	ludcmp(tmp,indx,&d);
	for (j=0; j<MNUM; ++j) {
		for (i=0; i<MNUM; ++i) col[i] = 0.0;
		col[j] = 1.0;
		lubksb(tmp,indx,col);
		for (i=0; i<MNUM; ++i) out[i][j] = col[i];
	}
}
