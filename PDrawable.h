/*
** Base class for drawing objects - PH 09/22/99
**
** Notes: 1) BeginDrawing() must be called (and return non-zero) before
**			 calling any other function (except SetScaling()/GetScaling()).
**
**		  2) SetFont() must be called before DrawString().
**
**		  3) SetScaling() causes the font size and line width to be
**			 scaled by the specified factor (if supported).
*/
#ifndef __PDrawable_h__
#define __PDrawable_h__

#include <Xm/Xm.h>

enum ETextAlign_q {
	kTextAlignTopLeft,
	kTextAlignTopCenter,
	kTextAlignTopRight,
	kTextAlignMiddleLeft,
	kTextAlignMiddleCenter,
	kTextAlignMiddleRight,
	kTextAlignBottomLeft,
	kTextAlignBottomCenter,
	kTextAlignBottomRight
};

enum ELineType {
	kLineTypeSolid,
	kLineTypeDot,
	kLineTypeDash
};

enum EDevice {
	kDeviceUnknown,
	kDevicePrinter,
	kDeviceVideo
};

class PDrawable
{
public:
	PDrawable() : mColours(NULL), mFont(0), mScaling(1) { }
	virtual ~PDrawable()   { }
	
	XFontStruct	  *	GetFont()	{ return mFont; }
	
	void			SetColourMap(Pixel *cols)	{ mColours = cols;  }
	void			SetScaling(int scale)		{ mScaling = scale; }
	int				GetScaling()				{ return mScaling;  }
	
	virtual int		BeginDrawing(int width, int height) 		{ return 0; }
	virtual void	EndDrawing()								{ }
	
	virtual void	SetForeground(int col_num) 					{ }
	virtual void	SetForegroundPixel(Pixel pixel)				{ }
	virtual int		EqualColours(int col1, int col2)			{ return 0; }
	virtual void	SetLineWidth(float width)					{ }
	virtual void	SetLineType(ELineType type)					{ }
	virtual void	SetFont(XFontStruct *font)					{ mFont = font; }
	virtual void	DrawSegments(XSegment *segments, int num) 	{ }
	virtual void	DrawPoint(int x, int y)						{ }
	virtual void	DrawLine(int x1,int y1,int x2,int y2) 		{ }
	virtual void	DrawRectangle(int x,int y,int w,int h)		{ }
	virtual void	FillRectangle(int x,int y,int w,int h)		{ }
	virtual void	FillPolygon(XPoint *point, int num) 		{ }
	virtual void	Comment(char *str)							{ }
	virtual void	DrawString(int x, int y, char *str, ETextAlign_q align) { }
	virtual void	DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2) { }
	virtual void	FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2) { }
	
	virtual void	PutImage(XImage *image, int dest_x, int dest_y) { }
	virtual XImage*	GetImage(int x, int y, int width, int height) { return NULL; }
	virtual int		CopyArea(int x,int y,int w,int h,Window dest) { return 0; }
	virtual int		HasPixmap()			{ return 0; }

	virtual EDevice	GetDeviceType()		{ return kDeviceUnknown; }		
		
protected:	
	Pixel		  *	mColours;
	
private:
	XFontStruct	  *	mFont;
	int				mScaling;
};


#endif // __PDrawable_h__
