#ifndef __matrix_h__
#define __matrix_h__

#define		MNUM	3

typedef float	Vector3[MNUM];
typedef float	Matrix3[MNUM][MNUM];

#ifdef  __cplusplus
extern "C" {
#endif

void	combineVector4(float m[3][4], int row, int col, int src);
void 	getRotAlign(Vector3 p, Matrix3 out);
void 	getRotAlignInv(Vector3 p, Matrix3 out);
void 	getRotAlignScaled(Vector3 p, Matrix3 out);
void	get3DMatrix(Matrix3 rot, float alpha, float beta, float gamma);
void	getRotMatrix(Matrix3 rot, float theta, float phi, float alpha);
void	getRotMatrixYXZ(Matrix3 rot, double alpha, double beta, double gamma);
void 	matrixIdent(Matrix3 m);
void	matrixInvert(Matrix3 in,Matrix3 out );
void	matrixSolve(Matrix3 m, Vector3 in, Vector3 out);
void 	matrixTranspose(Matrix3 in, Matrix3 out);
void 	matrixMult(Matrix3 d, Matrix3 s);
void 	vectorMult(Matrix3 m, Vector3 in, Vector3 out);
void	lubksb(float a[MNUM][MNUM], int indx[MNUM], float b[MNUM]);
void	ludcmp(float a[MNUM][MNUM], int indx[MNUM], float *d);
float	vectorLen(float x, float y, float z );
int	unitVector(Vector3 v);

#ifdef  __cplusplus
}
#endif

#endif

