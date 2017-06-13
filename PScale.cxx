/*----------------------------------------------------------------------------
** File:	PScale.cxx
**
** Created:	10/07/98 - PH (adapted from SNOTT scale C++ object)
*/
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>
#include "PDrawable.h"
#include "PScale.h"

const int	FIRST_POW	= -15;			// first symbol exponent
static char	symbols[]	= "fpnum\0kMG";	// symbols for exponents



//----------------------------------------------------------------------------
// PScale constructor
//
PScale::PScale(PDrawable *drawable,XFontStruct *font,int xa,int ya,int xb,int yb,int flags)
{
	mDrawable = drawable;
	mFont = font;
	mXa = xa; mYa = ya;
	mXb = xb; mYb = yb;
	
	log_scale   = flags & LOG_SCALE;
	upside_down = flags & TICKS_UPSIDE_DOWN;
	integer		= flags & INTEGER_SCALE;

	if (ya==yb) {		// horizontal scale if two y coords are the same
		xaxis = 1;
		opos1 = pos1 = xa;
		opos2 = pos2 = xb;
		dpos  = xb-xa;
		dlbl = XTextWidth(font, "00000", 5) * 2 * mDrawable->GetScaling();
	} else {
		xaxis = 0;
		opos1 = pos1 = yb;
		opos2 = pos2 = ya;
		dpos  = yb-ya;
		double spacing;
		int font_height = (font->ascent + font->descent) * mDrawable->GetScaling();
		int num_rows = dpos / font_height;
		if (num_rows <= 8) {
		    spacing = 1.5;
		} else if (num_rows <= 10) {
		    spacing = 2.0;
		} else if (num_rows <= 12) {
		    spacing = 2.5;
		} else {
		    spacing = 3.0;
		}
		dlbl = (int)(font_height * spacing);
	}
	min_val = max_val = val_rng = 0;
}


/* calculate scaling factors */
void PScale::CalcScalingFactors()
{
	double	lmin, lmax;

	if (log_scale) {
		if (min_val == 0) {
			offset = 1;
			fscl = dpos / log(val_rng+1);
			fpos = pos1 + 0.5;
		} else {
			offset = 0;
			lmin = log(min_val);
			lmax = log(max_val);
			if (xaxis) fscl = dpos / (lmin-lmax);
			else	   fscl = dpos / (lmax-lmin);
			fpos = ((pos1+pos2+1)+fscl*(lmin+lmax)) / 2.0;
		}
	} else {
		fscl = dpos / val_rng;
		if (!xaxis) fscl = (-fscl);
		fpos = pos1 - fscl*min_val + 0.5;
	}
}

/* set range for scale */
void PScale::SetRng(double low,double high)
{
	if (high!=max_val || low!=min_val) {
	    if (log_scale && low < 0) {
	        min_val = 0;    // 0 is minimum for log scale
	    } else {
		    min_val = low;
		}
		max_val = high;
		val_rng = max_val-min_val;
		CalcScalingFactors();
	}
	Draw();
}

/*----------------------------------------------------------------------------
** get relative scale value given relative pixel displacement
*/
double PScale::GetRelVal(int pix)
{
	return(pix*val_rng/dpos);
}

/* get absolute scale value */
double PScale::GetVal(int pix)
{
	if (log_scale) {
		return(exp((pix - fpos)/fscl) - offset);
	} else {
		return((pix - fpos) / fscl);
	}
}

/*----------------------------------------------------------------------------
** get absolute pixel location given absolute scale value
*/
int PScale::GetPix(double val)
{
	if (log_scale) return(GetLogPix(val));
	else		   return(GetLinPix(val));
}

/*----------------------------------------------------------------------------
** drawLinScale - draw linear scale
*/
void PScale::DrawLinScale()
{
	int			i, x, y;			// general variables
	double		val;				// true value of scale units
	int			ival;				// integer mantissa of scale units
	int			sep;				// mantissa of label separation
	int			power;				// exponent of label separation
	int			ticks;				// number of ticks per label
	int			sign = 1;			// sign of scale range
	double		order;				// 10^power
	double		step;				// distance between labels (scale units)
	double		tstep;				// distance between ticks (scale units)
	double		tol;				// tolerance for equality (1/2 pixel)
	double		lim;				// limit for loops
	char		suffix;				// si suffix for number
	char		dec = 0;			// flag to print decimal point
	char		buff[128];
	int			scaling = mDrawable->GetScaling();

	if (!dpos) return;		// check this to be safe - PH 12/21/99
	
    mDrawable->SetLineWidth(0.5);

	tstep  = GetRelVal(dlbl);

	if (tstep < 0) {
		sign = -1;
		tstep *= sign;
	}
/*
** Old getSep() Routine:  Determine axis labelling strategies.
**
** Input:	tstep = number of units between optimally spaced labels
** Output:	tick  = number of units between ticks
**          sep   = number of units between labels
*/
	if (tstep <= 0) {
		power = 1;
	} else {
		power = (int)floor(log10(tstep));					// exponent part of label sep
	}
	i = (int)(10.0*tstep/(order = pow(10.0,(double)power)));	// get first two digits

	if      (i >= 65) { sep = 1; ticks = 5; ++power; order*=10; }
	else if (i >= 35) { sep = 5; ticks = 5; }
	else if (i >= 15) { sep = 2; ticks = 4; }
	else		 	  { sep = 1; ticks = 5; }

	if (!power && integer) ticks = sep;		// no sub-ticks for integer scales
/*
** End of old getSep() routine
*/
    int pow0 = power - FIRST_POW;
    if ((pow0 < 0) || (pow0/3 >= 9)) return;  // just to be safe

	sep	  *= sign;
	step   = sep  * order;
	ival   = (int)floor(min_val/step);
	val	   = ival * step;			// value for first label below scale
	tstep  = step/ticks;
	ival  *= sep;
	suffix = symbols[pow0/3];

	switch (pow0 % 3) {
		case 0:
			break;
		case 1:
			ival *= 10;
			sep  *= 10;
			break;
		case 2:
			if (suffix) {			// avoid extra trailing zeros if suffix
				suffix= symbols[pow0/3+1];		// next suffix
				dec   = 1;			// use decimal point (/10)
			} else {
				ival *= 100;
				sep  *= 100;
			}
			break;
	}
	tol  = GetRelVal(1)/2.00001;
	lim  = -tol;
	val -= min_val;					// subtract origin

	if (val*sign < lim*sign) {
		ival += sep;					// get ival for first label
		for (i=0; val*sign<lim*sign; ++i) {
			val += tstep;				// find first tick on scale
		}
	} else i = ticks;					// first tick is at label

	lim  = max_val + tol;				// upper limit for scale value
	val += min_val;

	if (xaxis) {

		y = mYa + 1;
		mDrawable->DrawLine(opos1,mYa,opos2,mYa);
		for (;;) {
			x = GetLinPix(val);						// get pixel position
			if (i < ticks) {
				mDrawable->DrawLine(x,y,x,y+scaling);
				++i;
			} else {
				mDrawable->DrawLine(x,y,x,y+3*scaling);
				if (dec) {
					if (ival<0) sprintf(buff,"-%.1d.%.1d%c",(-ival)/10,(-ival)%10,suffix);
					else		sprintf(buff,"%.1d.%.1d%c",ival/10,ival%10,suffix);
				} else			sprintf(buff,"%d%c",ival,suffix);
				mDrawable->DrawString(x,y+5*scaling,buff,kTextAlignTopCenter);
				ival += sep;
				i = 1;
			}
			val += tstep;
			if (sign==1) {
				if (val > lim) break;
			} else {
				if (val < lim) break;
			}
		}

	} else {

	  if (upside_down) {

		x = mXa + 1;
		mDrawable->DrawLine(mXa,opos2,mXa,opos1);
		for (;;) {
			y = GetLinPix(val);						// get pixel position
			if (i < ticks) {
				mDrawable->DrawLine(x,y,x+scaling,y);
				++i;
			} else {
				mDrawable->DrawLine(x,y,x+3*scaling,y);
				if (dec) {
					if (ival<0) sprintf(buff,"-%.1d.%.1d%c",(-ival)/10,(-ival)%10,suffix);
					else		sprintf(buff,"%.1d.%.1d%c",ival/10,ival%10,suffix);
				} else			sprintf(buff,"%d%c",ival,suffix);
				mDrawable->DrawString(x+4*scaling, y,buff,kTextAlignMiddleLeft);
				ival += sep;
				i = 1;
			}
			val += tstep;
			if (sign==1) {
				if (val > lim) break;
			} else {
				if (val < lim) break;
			}
		}

	  } else {

		x = mXb - 1;
		mDrawable->DrawLine(mXb,opos2,mXb,opos1);
		for (;;) {
			y = GetLinPix(val);						// get pixel position
			if (i < ticks) {
				mDrawable->DrawLine(x-scaling,y,x,y);
				++i;
			} else {
				mDrawable->DrawLine(x-3*scaling,y,x,y);
				if (dec) {
					if (ival<0) sprintf(buff,"-%.1d.%.1d%c",(-ival)/10,(-ival)%10,suffix);
					else		sprintf(buff,"%.1d.%.1d%c",ival/10,ival%10,suffix);
				} else			sprintf(buff,"%d%c",ival,suffix);
				mDrawable->DrawString(x-5*scaling,y,buff,kTextAlignMiddleRight);
				ival += sep;
				i = 1;
			}
			val += tstep;
			if (sign==1) {
				if (val > lim) break;
			} else {
				if (val < lim) break;
			}
		}
	  }
	}
    mDrawable->SetLineWidth(1);
}

/*----------------------------------------------------------------------------
** Draw log scale
**
** Note: TICKS_UPSIDE_DOWN doesn't work for log scales
*/
void PScale::DrawLogScale()
{
	int		n,x,y,nmax;
	int		label_sep,tick_sep;
	double	val, mval, kval, base, inc;
	char	buff[128];
	int		scaling = mDrawable->GetScaling();

	if (!dpos) return;	// check this to be safe - PH 12/21/99
	
    mDrawable->SetLineWidth(0.5);

	val  = fabs(log(11.0)*fscl);
	inc  = 1.0;
	nmax = 10;
	while (val > 300 * scaling) {
		val  /= 10;
		inc  /= 10;
		nmax *= 10;
	}

	if      (val < dlbl) 	{ tick_sep=5;  label_sep=10; }
	else if (val < dlbl*1.8){ tick_sep=2;  label_sep=10; }
	else if (val < dlbl*5.5){ tick_sep=1;  label_sep=5;  }
	else if (val < dlbl*10) { tick_sep=1;  label_sep=2;  }
	else					{ tick_sep=1;  label_sep=1;  }

	kval = 1e3 / inc;
	mval = 1e6 / inc;

	if (xaxis) {
		y = mYa + 1;
		mDrawable->DrawLine(opos1,mYa,opos2,mYa);
		if (min_val == 0) {
			mDrawable->DrawLine(pos1,y,pos1,y+3*scaling);
			strcpy(buff,"0");
			mDrawable->DrawString(pos1,y+5*scaling,buff,kTextAlignTopCenter);
			base = 1;
			val = inc;
			if (label_sep == 2) n = 0;		// special case to draw "1" label
			else n = (int)inc;
		} else {
			val = base = pow(10,floor(log10(min_val)));
			n = 0;
			while (val < min_val) {
				++n;
				val += base * inc;
			}
		}

		for (;;) {

			if (!(n%tick_sep)) {
				x = GetLogPix(val);						// get pixel position
				if ((!(n%label_sep)) || (n==2 && label_sep==5)) {
					if (!n) n = (int)inc;
					mDrawable->DrawLine(x,y,x,y+3*scaling);
					if (val<kval) 	   sprintf(buff,"%.4g",val);
					else if (val<mval) sprintf(buff,"%.4gk",val*1e-3);
					else		   	   sprintf(buff,"%.4gM",val*1e-6);
					mDrawable->DrawString(x,y+5*scaling,buff,kTextAlignTopCenter);
				} else {
					mDrawable->DrawLine(x,y,x,y+scaling);
				}
			}
			if (++n > nmax) {
				n = nmax/10;
				base *= 10;
/*
** draw tick at 15, 150, etc
*/
				if (label_sep<=5 && (val=base*1.5)<=max_val) {
					x = GetLogPix(val);					// get pixel position
					mDrawable->DrawLine(x,y,x,y+scaling);
				}
				val  = base;
			} else {
				val += base * inc;
			}
			if (val > max_val) break;
		}
	} else {
		x = mXb - 1;
		mDrawable->DrawLine(mXb,opos2,mXb,opos1);
		if (min_val == 0) {
			mDrawable->DrawLine(x-3*scaling,pos1,x,pos1);
			strcpy(buff,"0");
			mDrawable->DrawString(x-5*scaling,pos1,buff,kTextAlignMiddleRight);
			val = inc;
			base = 1;
			if (label_sep == 2) n = 0;		// special case to draw "1" label
			else n = 1;
		} else {
			val = base = pow(10,floor(log10(min_val)));
			n = 0;
			while (val < min_val) {
				++n;
				val += base * inc;
			}
		}

		for (;;) {

			if (!(n%tick_sep)) {
				y = GetLogPix(val);						// get pixel position
				if ((!(n%label_sep)) || (n==2 && label_sep==5)) {
					if (!n) n = 1;
					mDrawable->DrawLine(x-3*scaling,y,x,y);
					if (val<kval) 	   sprintf(buff,"%.4g",val);
					else if (val<mval) sprintf(buff,"%.4gk",val*1e-3);
					else		   	   sprintf(buff,"%.4gM",val*1e-6);
					mDrawable->DrawString(x-5*scaling,y,buff,kTextAlignMiddleRight);
				} else {
					mDrawable->DrawLine(x-scaling,y,x,y);
				}
			}
			if (++n > nmax) {
				n = nmax/10;
				base *= 10;
/*
** draw tick at 15, 150, etc
*/
				if (label_sep<=5 && (val=base*1.5)<=max_val) {
					y = GetLogPix(val);					// get pixel position
					mDrawable->DrawLine(x-scaling,y,x,y);
				}
				val  = base;
			} else {
				val += base * inc;
			}
			if (val > max_val) break;
		}
	}
    mDrawable->SetLineWidth(1);
}
