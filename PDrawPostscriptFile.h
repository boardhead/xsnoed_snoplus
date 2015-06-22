/*
** Postscript file drawable - PH 09/22/99
*/
#ifndef __PDrawPostscriptFile_h__
#define __PDrawPostscriptFile_h__

#include <stdio.h>
#include "PDrawable.h"

const short kMaxPSNameLen		= 256;

class PDrawPostscriptFile : public PDrawable
{
public:
	PDrawPostscriptFile(char *filename, int landscape=0);
	virtual ~PDrawPostscriptFile();
	
	virtual int		BeginDrawing(int width,int height);
	virtual void	EndDrawing();
	
	virtual void	SetForeground(int col_num);
	virtual int		EqualColours(int col1, int col2);
	virtual void	SetLineWidth(float width);
	virtual void	SetLineType(ELineType type);
	virtual void	SetFont(XFontStruct *font);
	virtual void	Comment(char *str);
	virtual void	DrawSegments(XSegment *segments, int num);
	virtual void	DrawLine(int x1,int y1,int x2,int y2);
	virtual void	FillRectangle(int x,int y,int w,int h);
	virtual void	FillPolygon(XPoint *point, int num);
	virtual void	DrawString(int x, int y, char *str, ETextAlign_q align);
	virtual void	DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2);
	virtual void	FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2);

	virtual EDevice	GetDeviceType()		{ return kDevicePrinter; }

private:
	char			mFilename[kMaxPSNameLen];
	char			mBoundingBoxStr[256];
	FILE		  *	mFile;
	XColor		  *	mColours;
	int				mIsEPS;
	int				mIsLandscape;
};


#endif // __PDrawPostscriptFile_h__
