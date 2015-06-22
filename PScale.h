#ifndef __PScale_h__
#define __PScale_h__

#include <Xm/Xm.h>
#include <math.h>

enum {
	LOG_SCALE			= 0x01,
	TICKS_UPSIDE_DOWN	= 0x02,
	INTEGER_SCALE		= 0x04
};

class PDrawable;

class PScale {
public:
	PScale(PDrawable *drawable,XFontStruct *font,int xa,int ya,int xb,int yb,int flags);
    virtual ~PScale() { }

	virtual void	Draw() { if (log_scale) DrawLogScale(); else DrawLinScale(); }

	virtual void	SetRng(double low,double high);
	
	int 		GetPix(double val);
	double		GetRelVal(int pix);
	double		GetVal(int pix);
	double		GetMinVal()				{ return min_val;	}
	double		GetMaxVal()				{ return max_val;	}
	int			IsInteger()				{ return(integer);	}
	void		SetInteger(int is_int)	{ integer = is_int;	}

private:
	void		DrawLinScale();
	void		DrawLogScale();
	void		CalcScalingFactors();
	int			GetLinPix(double val) { return((int)(fpos + fscl * val)); }
	int			GetLogPix(double val) { return(val<0 ? (int)fpos : (int)(fpos - fscl*log(val+offset))); }
	
	PDrawable	*mDrawable;
	XFontStruct *mFont;				// X font for labels
	
	int			mXa, mYa;			// pixel coordinates for left or top of scale
	int			mXb, mYb;			// pixel coordinates for right or bottom of scale
	double		min_val;			// minimum value for scale
	double		max_val;			// maximum value for scale
	double		val_rng;			// max_val - min_val
	double		fscl;				// scaling factor
	double		fpos;				// pixel offset to start of scale
	double		offset;				// log scale offset
	int			pos1;				// pixel position of scale start
	int			pos2;				// pixel position of scale end
	int			dpos;				// pos2 - pos1
	int			opos1,opos2;		// original pos1 and pos2
	int			dlbl;				// optimal label separation (pixels)
	char		xaxis;				// flag for x scale
	char		log_scale;			// flag for log scale
	char		upside_down;		// flag for ticks on other side
	char		integer;			// flag for integer scale
};

#endif // __PScale_h__


