#include <Xm/RowColumn.h>
#include <math.h>
#include "PMapImage.h"
#include "PImageWindow.h"
#include "PMenu.h"
#include "PUtils.h"
#include "XSnoedWindow.h"
#include "xsnoed.h"
#include "menu.h"
#include "colours.h"

#define PROJ_PMT_SIZE				0.004		// hit size (relative to image size)
#define CONE_SEGMENT_TOL2			(20 * 20)	// maximum cone segment length (pixels squared)
#define CONE_SEGMENT_SPLIT_MAX		8			// maximum number of times to split a segment
#define QHS_OFFSET					600			// average Qhs pedestal
#define	QHS_FACTOR					0.3			// factor for hit size wrt charge
#define MOLLWEIDE_TOLERANCE			1e-6		// tolerance for Mollweide conversion
#define MOLLWEIDE_MAX_ITER			10			// maximum number of iterations

// Menu definition
static MenuStruct tp_arg_menu[] = {
	{ "Rectangular", 			0, XK_R,IDM_PROJ_RECTANGULAR, 	 NULL, 0, MENU_RADIO },
	{ "Cylindrical - Lambert", 	0, XK_C,IDM_PROJ_LAMBERT,    	 NULL, 0, MENU_RADIO },
	{ "Sinusoidal", 			0, XK_S,IDM_PROJ_SINUSOID, 		 NULL, 0, MENU_RADIO },
	{ "Elliptical - Linear", 	0, XK_L,IDM_PROJ_ELLIPTICAL, 	 NULL, 0, MENU_RADIO },
	{ "Elliptical - Mollweide", 0, XK_M,IDM_PROJ_MOLLWEIDE,		 NULL, 0, MENU_RADIO },
	{ "Elliptical - Hammer", 	0, XK_H,IDM_PROJ_HAMMER, 	 	 NULL, 0, MENU_RADIO },
	{ "Extended Hammer", 		0, XK_x,IDM_PROJ_EXTENDED_HAMMER,NULL, 0, MENU_RADIO },
	{ "Polar - Linear", 		0, XK_P,IDM_PROJ_POLAR,  		 NULL, 0, MENU_RADIO },
//	{ "Polar - Cosine", 		0, 0,	IDM_PROJ_POLAR_COS, 	 NULL, 0, MENU_RADIO },
	{ "Polar - Equal Area", 	0, XK_A,IDM_PROJ_POLAR_EQUAL, 	 NULL, 0, MENU_RADIO },
	{ NULL, 					0, 0,	0,						 NULL, 0, 0 },
	{ "Dual Sinusoidal", 		0, XK_u,IDM_PROJ_DUAL_SINUSOID,	 NULL, 0, MENU_RADIO },
	{ "Dual Elliptical - Linear",0, XK_n,IDM_PROJ_DUAL_ELLIPTICAL,NULL, 0, MENU_RADIO },
	{ "Dual Elliptical - Mollweide",0,XK_w,IDM_PROJ_DUAL_MOLLWEIDE, NULL,0,MENU_RADIO },
	{ "Dual Elliptical - Hammer",0,XK_e,IDM_PROJ_DUAL_HAMMER,  	 NULL, 0, MENU_RADIO },
	{ "Dual Polar - Linear",	0, XK_o,IDM_PROJ_DUAL_POLAR,	 NULL, 0, MENU_RADIO },
//	{ "Dual Polar - Cosine",	0, 0,	IDM_PROJ_DUAL_POLAR_COS, NULL, 0, MENU_RADIO },
	{ "Dual Polar - Equal Area",0, XK_q,IDM_PROJ_DUAL_POLAR_EQUAL,NULL, 0, MENU_RADIO },
};
static MenuStruct tp_move_menu[] = {
	{ "To Home" , 				0, XK_H,IDM_PROJ_TO_HOME, 		 NULL, 0, 0},
};
static MenuStruct tp_vw_menu[] = {
	{ "Detector Coordinates",  	0, XK_D,IDM_DETECTOR_COORD, 	 NULL, 0, MENU_RADIO },
	{ "Relative to Fit", 		0, XK_R,IDM_FIT_RELATIVE, 		 NULL, 0, MENU_RADIO },
};
static MenuStruct tp_hit_menu[] = {
	{ "Fixed Size",  			0, XK_F,IDM_SIZE_FIXED, 		 NULL, 0, MENU_RADIO },
	{ "Charge (Qhs)", 			0, XK_Q,IDM_SIZE_QHS, 	 		 NULL, 0, MENU_RADIO },
	{ "Solid Angle", 			0, XK_A,IDM_SIZE_SOLID_ANGLE, 	 NULL, 0, MENU_RADIO },
	{ NULL, 					0, 0,	0,						 NULL, 0, 0 },
	{ "Squares", 				0, XK_S,IDM_HIT_SQUARE, 	 	 NULL, 0, MENU_RADIO },
	{ "Circles", 				0, XK_C,IDM_HIT_CIRCLE, 	 	 NULL, 0, MENU_RADIO }
};
static MenuStruct tp_main_menu[] = {
	{ "Projection", 			0, 0,	0, tp_arg_menu,  XtNumber(tp_arg_menu),  0 },
	{ "Move", 					0, 0,	0, tp_move_menu, XtNumber(tp_move_menu), 0 },
	{ "View", 					0, 0,	0, tp_vw_menu,   XtNumber(tp_vw_menu),   0 },
	{ "Hits", 					0, 0,	0, tp_hit_menu,  XtNumber(tp_hit_menu),  0 },
};


// ----------------------------------------------------------------------------------------------------------
// PMapImage constructor
//
PMapImage::PMapImage(PImageWindow *owner, Widget canvas)
		 : PProjImage(owner,canvas)
{
	ImageData	*data = owner->GetData();
	
	mRelativeToFit	= (data->wProjCoords == IDM_FIT_RELATIVE);
	mSizeOption 	= data->wSizeOption;
	mShapeOption	= data->wShapeOption;
	mProjType		= data->wProjType;
	mInvisibleHits	= HIT_FECD | HIT_BUTTS | HIT_NECK;
	
	SetProjection(mProjType);
	
	if (!canvas) {
		
		// create our menu
		owner->CreateMenu(NULL,tp_main_menu,XtNumber(tp_main_menu),this);
	
		/* select current menu items */
		owner->GetMenu()->SetToggle(mProjType, TRUE);
		owner->GetMenu()->SetToggle(data->wProjCoords, TRUE);
		owner->GetMenu()->SetToggle(mSizeOption, TRUE);
		owner->GetMenu()->SetToggle(mShapeOption, TRUE);
		
		// make projection name agree with projection type
		MenuList *ms = owner->GetMenu()->FindMenuItem(mProjType);
		if (ms) {
			char *str = PMenu::GetLabel(ms);
			if (str) {
				XtFree(data->projName);
				data->projName = str;	// set current projection name
			}
			// set window title if not main window
			if (owner != data->mMainWindow) {
				Arg wargs[1];
				XtSetArg(wargs[0], XmNtitle, data->projName);
				XtSetValues(owner->GetShell(), wargs, 1);		/* set new title */
			}
		}
		
		CreateCanvas("tpCanvas", kScrollAllMask);
	}
}

PMapImage::~PMapImage()
{
}

void PMapImage::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageHitSizeChanged:
		case kMessageFitChanged:
			SetDirty();
			break;
		case kMessageAngleFormatChanged:
			if (mProj.theta || mProj.phi) {
				SetDirty();
			}
			break;
		default:
			PProjImage::Listen(message, dataPt);
			break;
	}
}

// add projection menu item to owner's menu
void PMapImage::AddMenuItem()
{
	if (mOwner->GetMenu()) {
		// add Projection menu to main menu bar (but use our handler!)
		mOwner->GetMenu()->AddMenuItem(tp_main_menu,NULL,this);
		// select current projection type in menu
		mOwner->GetMenu()->SetToggle(mProjType, TRUE);
	}
}

void PMapImage::SetScrolls()
{
	int	v;

	v = kScrollMax - (int)(kScrollMax * (atan(mProj.mag) - mMinMagAtan) / mDiffMagAtan + 0.5);
	mOwner->SetScrollValue(kScrollLeft, v);

	v = (int)( kScrollMax * (mProj.theta / (2*PI) + 0.5) );
	mOwner->SetScrollValue(kScrollRight, v);

	v = (int)( kScrollMax * (mProj.phi / (2*PI) + 0.5) );
	mOwner->SetScrollValue(kScrollBottom,v);
}

void PMapImage::ScrollValueChanged(EScrollBar bar, int value)
{
	double	ang = (value * (2*PI)) / kScrollMax - PI;
	
	switch (bar) {
		case kScrollRight:
			mProj.theta = ang;
			SetDirty();
			break;
		case kScrollBottom:
			mProj.phi = ang;
			SetDirty();
			break;
		default:
			PProjImage::ScrollValueChanged(bar,value);
			break;
	}
}


static void ReMapProj(Node *n0, Vector3 v0, Matrix3 rot, Projection *pp, Node *n1)
{
	int			i;
	Vector3		vec, v1;
	double		a, b, f, t;
	int			xcen = (int)(pp->xcen - pp->xscl * pp->pt[0]);
	int			ycen = (int)(pp->ycen + pp->yscl * pp->pt[1]);

	vec[0] = n0->x3 - v0[0];
	vec[1] = n0->y3 - v0[1];
	vec[2] = n0->z3 - v0[2];
	unitVector(vec);
	vectorMult(rot, vec, v1);
		
	switch (pp->proj_type) {
	
		case IDM_PROJ_RECTANGULAR:
			b = 1. - 2. * acos(v1[2]) / PI;
			goto ProjCyl;
			
		case IDM_PROJ_LAMBERT:
			b = v1[2];
ProjCyl:
			a = atan2( v1[1], v1[0] ) / PI;
			n1->x = xcen + (int)(a * pp->xscl);
			n1->y = ycen - (int)(b * pp->yscl);
			break;
			
		case IDM_PROJ_POLAR:
			v1[2] = acos(v1[2]) / PI;
			goto ProjPolar;
//		case IDM_PROJ_POLAR_COS:
//			v1[2] = (1. - v1[2]) /2.;
//			goto ProjPolar;
		case IDM_PROJ_POLAR_EQUAL:
			v1[2] = sqrt((1. - v1[2]) / 2.);
ProjPolar:
			if( !(a = sqrt( v1[0]*v1[0] + v1[1]*v1[1])) ) {
				v1[0] = v1[2];
				v1[1] = 0;
			} else {
				f = v1[2] / a;
				v1[0] *= f;
				v1[1] *= f; 
			}
			n1->x = xcen + (int)(v1[1] * pp->yscl);
			n1->y = ycen + (int)(v1[0] * pp->xscl);
			break;
			
		case IDM_PROJ_DUAL_POLAR_EQUAL:
			if (v1[2] > 0) {
				v1[2] = sqrt((1. - v1[2]) / 4.);
				n1->x = -pp->xscl / 2;
			} else {
				v1[2] = sqrt((1. + v1[2]) / 4.);
				n1->x = pp->xscl / 2;
				v1[1] = -v1[1];
			}
			goto ProjDualPolar2;
		case IDM_PROJ_DUAL_POLAR:
			v1[2] = acos(v1[2]) / PI;
//			goto ProjDualPolar;
//		case IDM_PROJ_DUAL_POLAR_COS:
//			v1[2] = (1 - v1[2]) / 2.;
//ProjDualPolar:
			if (v1[2] > 0.5) {
				n1->x = pp->xscl / 2;
				v1[2] = 1 - v1[2];
				v1[1] = -v1[1];
			} else {
				n1->x = -pp->xscl / 2;
			}
ProjDualPolar2:
			if( !(a = sqrt( v1[0]*v1[0] + v1[1]*v1[1])) ) {
				v1[0] = v1[2];
				v1[1] = 0;
			} else {
				f = v1[2] / a;
				v1[0] *= f;
				v1[1] *= f; 
			}
			n1->x += xcen + (int)(v1[1] * pp->yscl);
			n1->y = ycen + (int)(v1[0] * pp->xscl);
			break;
			
		case IDM_PROJ_SINUSOID:
			a = atan2( v1[1], v1[0] ) / PI;
			b = 1. - 2. * acos(v1[2]) / PI;
			n1->x = xcen + (int)(a * pp->xscl * cos(b*(PI/2)));
			n1->y = ycen - (int)(b * pp->yscl);
			break;
			
		case IDM_PROJ_ELLIPTICAL:
//		case IDM_PROJ_ELLIPTICAL_COS:
			a = atan2( v1[1], v1[0] ) / PI;
//			if (pp->proj_type == IDM_PROJ_ELLIPTICAL_COS) {
//				b = v1[2];
//			} else {
				b = 1. - 2. * acos(v1[2]) / PI;
//			}
			n1->x = xcen + (int)(a * sqrt(1-b*b) * pp->xscl);
			n1->y = ycen - (int)(b * pp->yscl);
			break;
			
		case IDM_PROJ_DUAL_SINUSOID:
			a = atan2( v1[1], v1[0] ) / PI;
			b = 1. - 2. * acos(v1[2]) / PI;
			if (a < -0.5) {
				n1->x = xcen + pp->xscl/2 + (int)((a + 1) * pp->xscl * cos(b*(PI/2)));
			} else if (a >= 0.5) {
				n1->x = xcen + pp->xscl/2 + (int)((a - 1) * pp->xscl * cos(b*(PI/2)));
			} else {
				n1->x = xcen - pp->xscl/2 + (int)(a * pp->xscl * cos(b*(PI/2)));
			}
			n1->y = ycen - (int)(b * pp->yscl);
			break;
			
		case IDM_PROJ_DUAL_ELLIPTICAL:
//		case IDM_PROJ_DUAL_ELLIPTICAL_COS:
			a = atan2( v1[1], v1[0] ) / PI;
//			if (pp->proj_type == IDM_PROJ_DUAL_ELLIPTICAL_COS) {
//				b = v1[2];
//			} else {
				b = 1. - 2. * acos(v1[2]) / PI;
//			}
			if (a < -0.5) {
				n1->x = xcen + pp->xscl/2 + (int)((a + 1) * sqrt(1-b*b) * pp->xscl);
			} else if (a >= 0.5) {
				n1->x = xcen + pp->xscl/2 + (int)((a - 1) * sqrt(1-b*b) * pp->xscl);
			} else {
				n1->x = xcen - pp->xscl/2 + (int)(a * sqrt(1-b*b) * pp->xscl);
			}
			n1->y = ycen - (int)(b * pp->yscl);
			break;

		case IDM_PROJ_MOLLWEIDE:
			a = atan2(v1[1], v1[0]);	// horizontal angle from x axis
			b = asin(v1[2]);		// vertical angle from x-y plane (up is neg)
			f = PI * v1[2];
			for (i=0; ; ) {
				b -= (t = (b + sin(b) - f) / (1. + cos(b)));
				if (fabs(t)<MOLLWEIDE_TOLERANCE || ++i>=MOLLWEIDE_MAX_ITER) {
					b *= 0.5;
					break;
				}
			}
			n1->x = xcen + (int)(pp->xscl * a * cos(b) / PI);
			n1->y = ycen - (int)(pp->yscl * sin(b));
			break;
			
		case IDM_PROJ_DUAL_MOLLWEIDE:
			a = atan2(v1[1], v1[0]);
			b = asin(v1[2]);
			f = PI * v1[2];
			if (a < -PI/2) {
				a = PI + a;
				xcen = xcen + pp->xscl/2;
			} else if (a > PI/2) {
				a = a - PI;
				xcen = xcen + pp->xscl/2;
			} else {
				xcen = xcen - pp->xscl/2;
			}
			for (i=0; ; ) {
				b -= (t = (b + sin(b) - f) / (1. + cos(b)));
				if (fabs(t)<MOLLWEIDE_TOLERANCE || ++i>=MOLLWEIDE_MAX_ITER) {
					b *= 0.5;
					break;
				}
			}
			n1->x = xcen + (int)(pp->xscl * a * cos(b) / PI);
			n1->y = ycen - (int)(pp->yscl * sin(b));
			break;
			
		case IDM_PROJ_HAMMER:
			a = 0.5 * atan2( v1[1], v1[0] );
			b = v1[2];
			f = sqrt(1. / (1. + (t=sqrt(1.-b*b)) * cos(a)));
			n1->x = (int)(xcen + pp->xscl * f * t * sin(a));
			n1->y = (int)(ycen - pp->yscl * f * b);
			break;
			
		case IDM_PROJ_EXTENDED_HAMMER:
			v1[0] = sqrt((1. - v1[0]) / 2.);
			if( !(a = sqrt( v1[1]*v1[1] + v1[2]*v1[2])) ) {
				v1[1] = v1[0];
				v1[2] = 0;
			} else {
				f = v1[0] / a;
				v1[1] *= f;
				v1[2] *= f; 
			}
			n1->x = xcen + (int)(v1[1] * pp->xscl);
			n1->y = ycen - (int)(v1[2] * pp->yscl);
			break;
			
		case IDM_PROJ_DUAL_HAMMER:
			if (v1[0] > 0) {
				xcen = xcen - pp->xscl/2;
			} else {
				xcen = xcen + pp->xscl/2;
				v1[0] = -v1[0];
				v1[1] = -v1[1];
			}
			f = sqrt(1. / (1. + v1[0]));
			n1->x = (int)(xcen + pp->xscl * f * v1[1] * 0.5);
			n1->y = (int)(ycen - pp->yscl * f * v1[2]);
			break;
	}
} 

void PMapImage::TransformHits()
{
	Vector3		vec;
	Matrix3		rot1;
	ImageData	*data = mOwner->GetData();
	
	if (mRelativeToFit && data->curcon >= 0) {
		vec[0] = data->rcon[data->curcon].dir[0];
		vec[1] = data->rcon[data->curcon].dir[1];
		vec[2] = data->rcon[data->curcon].dir[2];
		getRotAlignInv( vec, rot1);
		vec[0] = data->rcon[data->curcon].pos[0];
		vec[1] = data->rcon[data->curcon].pos[1];
		vec[2] = data->rcon[data->curcon].pos[2];
	} else {					
		matrixIdent(rot1);
		vec[0] = 0;
		vec[1] = 0;
		vec[2] = 0;
	}
	matrixMult(rot1, mProj.rot );
	TransformHits(vec,rot1);
}


void PMapImage::TransformHits(Vector3 vec, Matrix3 rot1)
{
	int		i,num;
	HitInfo	*hi;
	Node	*n0, nod;
	ImageData *data = mOwner->GetData();
	
#ifdef PRINT_DRAWS
	Printf(":transform map\n");
#endif	
	if ((num=data->hits.num_nodes) != 0) {

		hi = data->hits.hit_info;
		n0 = data->hits.nodes;

		for (i=0; i<num; ++i,++n0,++hi) {
			/* map 3D tube coordinates into coordinates for this projection */
			ReMapProj( n0, vec, rot1, &mProj, &nod);
			/* save 2-d coordinates */
			n0->x = nod.x;
			n0->y = nod.y;
		}
	}
	
	/* must do this to save mLastProjImage */
	PProjImage::TransformHits();
}

/*
** Add line in projection to segment list, splitting it if necessary
** Do not add line if it is discontinuous
**
** On entry: sp->x1, sp->y1, sp->x2, sp->y2 are set corresponding to n0 and n1
**
** On exit:	 the new sp is returned. sp->x1 and sp->y1 are set corresponding to n1
*/
XSegment *PMapImage::AddProjLine(XSegment *sp,XSegment *segments,Node *n0,Node *n1,
						   Vector3 vec,Matrix3 rot1,Projection *proj,int nsplit)
{
	int		tx,ty;
	Node	tn, nod;
	
	// draw segments now if we are about to overrun segments array
	if (sp-segments >= MAX_EDGES-1) {
		DrawSegments(segments,sp-segments);
		// copy down the last segment
		memcpy(segments, sp, sizeof(XSegment));
		sp = segments;
	}
	
	// do not draw lines that span the halves of a dual plot
	int xcen = (int)(proj->xcen - proj->xscl * proj->pt[0]);
	if (proj->proj_type<IDM_PROJ_dummy || (sp->x1-xcen)*(sp->x2-xcen)>=0) {
		// allow segment if it isn't too long
		tx = sp->x2 - sp->x1;
		ty = sp->y2 - sp->y1;
		if (tx*tx + ty*ty < CONE_SEGMENT_TOL2) {
			sp[1].x1 = sp->x2;
			sp[1].y1 = sp->y2;
			++sp;
			return(sp);
		}
	}

	// have we split this line too many times already?
	if (++nsplit > mSplitThreshold) {
		sp->x1 = sp->x2;
		sp->y1 = sp->y2;
		// do not add segment (still too long after max splits)
		return(sp);
	}
	// calculate a midpoint for the segment
	tn.x3 = 0.5 * (n0->x3 + n1->x3);
	tn.y3 = 0.5 * (n0->y3 + n1->y3);
	tn.z3 = 0.5 * (n0->z3 + n1->z3);
	ReMapProj( &tn, vec, rot1, proj, &nod);
	
	// save coordinates of original endpoint (for n1)
	tx = sp->x2;
	ty = sp->y2;
	
	// insert our new endpoint (for tn)
	sp->x2 = nod.x;
	sp->y2 = nod.y;
	
	// add first half of segment
	sp = AddProjLine(sp,segments,n0,&tn,vec,rot1,proj,nsplit);
	
	// restore original endpoint (for n1)
	sp->x2 = tx;
	sp->y2 = ty;
	
	// add second half of segment 
	sp = AddProjLine(sp,segments,&tn,n1,vec,rot1,proj,nsplit);
	
	return(sp);
}

/* 
** utility routines to reflect X segments through the X and Y axes
** The new segments are added to the end of the array,
** leaving the original entries untouched.
*/
static XSegment *reflectSegmentsX(XSegment *start, int num, int xcen)
{
	int			b = xcen * 2;
	XSegment	*sp = start + num;
	XSegment	*sp2;

	for (sp2=sp-1; sp2>=start; ++sp,--sp2) {
		sp->x1 = b - sp2->x2;
		sp->y1 = sp2->y2;
		sp->x2 = b - sp2->x1;
		sp->y2 = sp2->y1;
	}
	return(sp);
}

static XSegment *reflectSegmentsY(XSegment *start, int num, int ycen)
{
	int			a = ycen * 2;
	XSegment	*sp = start + num;
	XSegment	*sp2;
	
	for (sp2=sp-1; sp2>=start; ++sp,--sp2) {
		sp->x1 = sp2->x2;
		sp->y1 = a - sp2->y2;
		sp->x2 = sp2->x1;
		sp->y2 = a - sp2->y1;
	}
	return(sp);
}

void PMapImage::SetProjection(int proj_type)
{
	mProj.proj_type = proj_type;
	
	switch (proj_type) {
		case IDM_PROJ_RECTANGULAR:
		case IDM_PROJ_LAMBERT:
			mScaleProportional = 0;
			mMarginPix = 16;
			mImageSizeX = 1.0;
			mImageSizeY = 1.0;
			break;
		case IDM_PROJ_SINUSOID:
		case IDM_PROJ_DUAL_SINUSOID:
		case IDM_PROJ_ELLIPTICAL:
		case IDM_PROJ_DUAL_ELLIPTICAL:
		case IDM_PROJ_MOLLWEIDE:
		case IDM_PROJ_DUAL_MOLLWEIDE:
		case IDM_PROJ_HAMMER:
		case IDM_PROJ_EXTENDED_HAMMER:
		case IDM_PROJ_DUAL_HAMMER:
			mScaleProportional = 0;
			mMarginPix = 8;
			mImageSizeX = 1.0;
			mImageSizeY = 1.0;
			break;
		case IDM_PROJ_POLAR:
//		case IDM_PROJ_POLAR_COS:
		case IDM_PROJ_POLAR_EQUAL:
			mScaleProportional = 1;
			mMarginPix = 8;
			mImageSizeX = 1.0;
			mImageSizeY = 1.0;
			break;
		case IDM_PROJ_DUAL_POLAR:
//		case IDM_PROJ_DUAL_POLAR_COS:
		case IDM_PROJ_DUAL_POLAR_EQUAL:
			mScaleProportional = 1;
			mMarginPix = 8;
			mImageSizeX = 1.0;
			mImageSizeY = 0.5;
			break;
	}
	if (mCanvas && XtIsRealized(mCanvas)) {
		Resize();
	}
}


/*
** Draw image in Projection window
*/
void PMapImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	XSegment	segments[MAX_EDGES], *sp;
	HitInfo		*hi;
	Node		*n0, *n1, nod;
	int			i, j, k, loops, segs, num, num1, m, i1, i2;
	int			x, y=0, xcen, ycen, xscl, yscl;
	double		theta=0, thinc, fac;
	Matrix3		rot1;
	Vector3		vec;
	double		a, b, f, t;
	int			missed;
#ifdef PRINT_DRAWS
	Printf("drawProjImage\n");
#endif	
	// don't draw weird hits
	long bit_mask = HiddenHitMask();
	
	PImageCanvas::DrawSelf();	// let the base class clear the drawing area

	SetLineWidth(THIN_LINE_WIDTH);
	if (mDrawable->GetDeviceType() == kDevicePrinter) {
		SetForeground(TEXT_COL);
	} else {
		SetForeground(GRID_COL);
	}
	
	// pre-calculate line split threshold based on image resolution
	mSplitThreshold = CONE_SEGMENT_SPLIT_MAX;
	for (i=1; i<32; ++i) {
		if (GetScaling() < (1 << i)) break;
		++mSplitThreshold;
	}
	
	xscl = mProj.xscl;
	yscl = mProj.yscl;
	xcen = (int)(mProj.xcen - xscl * mProj.pt[0]);
	ycen = (int)(mProj.ycen + yscl * mProj.pt[1]);
/*
** Draw grid
*/
	switch (mProj.proj_type) {

		case IDM_PROJ_RECTANGULAR:
		case IDM_PROJ_LAMBERT:
			sp = segments;
			num = 6;
			for (i=0; i<=num; ++i) {
				sp->x1 = xcen - xscl;
				sp->x2 = xcen + xscl;
				if (mProj.proj_type == IDM_PROJ_RECTANGULAR) {
				    sp->y1 = (int)(ycen - yscl * (2*i/(float)num - 1));
				} else {
				    sp->y1 = (int)(ycen - yscl * sin((i/(float)num-0.5)*PI));
				}
				sp->y2 = sp->y1;
				++sp;
			}
			num = 12;
			for (i=0; i<=num; ++i) {
				sp->x1 = (int)(xcen - xscl * (2*i/(float)num - 1));
				sp->x2 = sp->x1;
				sp->y1 = ycen - yscl;
				sp->y2 = ycen + yscl;
				++sp;
			}
			DrawSegments(segments,sp-segments);
			break;

		case IDM_PROJ_HAMMER:
			fac = 1.;
			loops = 1;
			segs = 24;
			goto Do_Proj_Hammer;
			
		case IDM_PROJ_EXTENDED_HAMMER:
			fac = 0.5 * sqrt(2.);
			loops = 1;
			segs = 48;
			goto Do_Proj_Hammer;
		
		case IDM_PROJ_DUAL_HAMMER:
			xscl /= 2;
			xcen -= xscl;
			fac = 1.;
			loops = 2;
			segs = 24;
Do_Proj_Hammer:
			// increase number of segments for higher resolution images
			if (GetScaling() > 2) segs *= 2;
			for (k=0; k<loops; ++k) {
				/* draw containing ellipse */
				DrawArc(xcen,ycen,xscl,yscl);
				/* draw lines of latitude */
				sp = segments;
				sp->x1 = xcen - xscl;
				sp->x2 = xcen + xscl;
				sp->y1 = sp->y2 = ycen;
				DrawSegments(segments,1);	/* draw equator */
				num = 3;
				num1 = segs / loops;
				for (i=1; i<num; ++i) {
					b = PI * (i / (float)(num * 2) - 0.5);
					m = num1 / 2;
					sp = segments + m;
					for (j=m; ; --j) {
						a = PI * (j / (float)num1 - 0.5);
						if (mProj.proj_type == IDM_PROJ_EXTENDED_HAMMER) a *= 2.;
						f = sqrt(1. / (1. + (t=cos(b)) * cos(a)));
						x = (int)(fac * xscl * f * t * sin(a));
						y = (int)(fac * yscl * f * sin(b));
						if (j <= 0) {
							sp->x1 = xcen + x;
							sp->y1 = ycen - y;
							break;
						} else {
							--sp;
							sp[0].x2 = sp[1].x1 = xcen + x;
							sp[0].y2 = sp[1].y1 = ycen - y;
						}
					}
					sp = reflectSegmentsX(segments, m, xcen);
					sp = reflectSegmentsY(segments, sp-segments, ycen);
					DrawSegments(segments,sp-segments);
				}
				
				/* draw lines of longitude */
				sp = segments;
				sp->x1 = sp->x2 = xcen;
				sp->y1 = ycen - yscl;
				sp->y2 = ycen + yscl;
				++sp;
				DrawSegments(segments,sp-segments);
				
				num = 6 / loops;
				for (i=1; i<num; ++i) {
					a = PI * (i / (float)(num * 2) - 0.5);
					if (mProj.proj_type == IDM_PROJ_EXTENDED_HAMMER) a *= 2.;
					if ((i&0x01) && mProj.proj_type==IDM_PROJ_HAMMER) {
						i1 = num1 / 6;
					} else {
						i1 = 0;
					}
					i2 = num1 / 2;
					m = i2 - i1;
					sp = segments + m;
					for (j=i2; ; --j) {
						b = PI * (j / (float)num1 - 0.5);
						f = sqrt(1. / (1. + (t=cos(b)) * cos(a)));
						x = (int)(fac * xscl * f * t * sin(a));
						y = (int)(fac * yscl * f * sin(b));
						if (j <= i1) {
							sp[0].x1 = xcen + x;
							sp[0].y1 = ycen - y;
							break;
						} else {
							--sp;
							sp[0].x2 = sp[1].x1 = xcen + x;
							sp[0].y2 = sp[1].y1 = ycen - y;
						}
					}
					sp = reflectSegmentsX(segments, m, xcen);
					sp = reflectSegmentsY(segments, sp-segments, ycen);
					DrawSegments(segments,sp-segments);
				}
				xcen += 2 * xscl;
			}
			break;
			
		case IDM_PROJ_POLAR:
//		case IDM_PROJ_POLAR_COS:
		case IDM_PROJ_POLAR_EQUAL:
			num = 6;
			loops = 1;
			goto Do_Proj_Polar;

		case IDM_PROJ_DUAL_POLAR:
//		case IDM_PROJ_DUAL_POLAR_COS:
		case IDM_PROJ_DUAL_POLAR_EQUAL:
			num = 3;	/* number of circles */
			loops = 2;
			xscl /= 2;
			yscl /= 2;
			xcen -= xscl;
Do_Proj_Polar:
			sp = segments;
			num1 = 3;	/* lines of longitude in a quarter circle */
			for (k=0; k<loops; ++k) {
				DrawArc(xcen,ycen,xscl,yscl);
				for (j=1; j<num; ++j) {
					a = j / (float)num;
					switch (mProj.proj_type) {
//						case IDM_PROJ_POLAR_COS:
//							a = 0.5 - 0.5 * cos(a*PI);
//							break;
//						case IDM_PROJ_DUAL_POLAR_COS:
//							a = 1. - cos(a*(0.5*PI));
//							break;
						case IDM_PROJ_POLAR_EQUAL:
							a = sqrt(0.5 - 0.5 * cos(a*PI));
							break;
						case IDM_PROJ_DUAL_POLAR_EQUAL:
							a = sqrt(1. - cos(a*(0.5*PI)));
							break;
					}
					DrawArc(xcen,ycen,(int)(xscl*a),(int)(yscl*a));
				}
				sp = segments;
				sp->x1 = xcen;	sp->x2 = xcen;
				sp->y1 = ycen - yscl; sp->y2 = ycen + yscl;
				++sp;
				sp->x1 = xcen - xscl; sp->x2 = xcen + xscl;
				sp->y1 = ycen; sp->y2 = ycen;
				++sp;
				for (i=1; i<num1; ++i) {
					a = (PI * i / (float)(num1 * 2));
					x = (int)(xscl * cos(a));
					y = (int)(yscl * sin(a));
					sp->x1 = xcen - x;	sp->x2 = xcen + x;
					sp->y1 = ycen - y;	sp->y2 = ycen + y;
					++sp;
					sp->x1 = xcen + x;	sp->x2 = xcen - x;
					sp->y1 = ycen - y;	sp->y2 = ycen + y;
					++sp;
				}
				DrawSegments(segments,sp - segments);
				xcen += 2 * xscl;
			}
			break;

		case IDM_PROJ_SINUSOID:
			num = 12;
			loops = 1;
			goto Do_Proj_Sinusoid;
			
		case IDM_PROJ_DUAL_SINUSOID:
			num = 6;
			loops = 2;
			xscl /= 2;
			xcen -= xscl;
Do_Proj_Sinusoid:
			segs = 24;	/* number of segments */
			// increase number of segments for higher resolution images
			if (GetScaling() > 2) segs *= 2;
			thinc = PI / segs;
			for (k=0; k<loops; ++k) {
				/* draw lines of longitude */
				for (j=0; j<num/2; ++j) {
					a = 2 * j/(float)num - 1;
					if (j&0x01 && mProj.proj_type!=IDM_PROJ_DUAL_SINUSOID) {
						i1 = segs / 6;
						theta = thinc * i1;
					} else {
						i1 = 0;
						theta = 0;
					}
					i2 = segs / 2;
					sp = segments;
					for (i=i1; ;theta+=thinc) {
						y = ycen + (int)(yscl * (2 * i/(float)segs - 1));
						x = xcen + (int)(xscl * sin(theta) * a);
						if (i != i1) {
							sp->x2 = x;
							sp->y2 = y;
							sp++;
						}
						if (++i > i2) break;
						sp->x1 = x;
						sp->y1 = y;
					}
					/* reflect this line 4 times */
					sp = reflectSegmentsX(segments, sp-segments, xcen);
					sp = reflectSegmentsY(segments, sp-segments, ycen);
					/* add central meridian */
					sp->x1 = sp->x2 = xcen;
					sp->y1 = ycen - yscl;
					sp->y2 = ycen + yscl;
					++sp;
					DrawSegments(segments,sp-segments);
				}
				/* draw lines of latitude */
				num = 6;
				sp = segments;
				for (i=1; i<num; ++i) {
					theta = PI * i / num;
					y = ycen + (int)(yscl * (2 * i/(float)num - 1));
					x = (int)(xscl * sin(theta));
					sp->x1 = xcen - x;
					sp->y1 = y;
					sp->x2 = xcen + x;
					sp->y2 = y;
					++sp;
				}
				DrawSegments(segments,sp-segments);
				xcen += 2 * xscl;
			}
			break;

		case IDM_PROJ_ELLIPTICAL:
		case IDM_PROJ_MOLLWEIDE:
			num = 6;		/* number of longitude lines / 2 */
			loops = 1;
			goto Do_Proj_Elliptical;
			
		case IDM_PROJ_DUAL_ELLIPTICAL:
		case IDM_PROJ_DUAL_MOLLWEIDE:
			num = 3;		/* number of longitude lines / 2 */
			loops = 2;
			xscl /= 2;
			xcen -= xscl;
Do_Proj_Elliptical:
			for (k=0; k<loops; ++k) {
				/* draw lines of longitude */
				for (j=0; j<num; ++j) {
					a = 1. - j / (float)num;
					DrawArc(xcen,ycen,(int)(xscl*a),yscl);
				}
				sp = segments;
				/* draw central meridian */
				sp->x1 = sp->x2 = xcen;
				sp->y1 = ycen - yscl;
				sp->y2 = ycen + yscl;
				++sp;
				/* draw lines of latitude */
				num1 = 6;
				for (i=1; i<num1; ++i) {
					switch (mProj.proj_type) {
						case IDM_PROJ_ELLIPTICAL:
						case IDM_PROJ_DUAL_ELLIPTICAL:
							theta = acos( 2 * i/(float)num1 - 1 );
							y = ycen + (int)(yscl * (2 * i/(float)num1 - 1));
							break;
						case IDM_PROJ_MOLLWEIDE:
						case IDM_PROJ_DUAL_MOLLWEIDE:
							b = PI * (i / (float)num1 - 0.5);
							f = PI * sin(b);
							for (j=0; ; ) {
								b -= (t = (b + sin(b) - f) / (1. + cos(b)));
								if (fabs(t)<MOLLWEIDE_TOLERANCE || ++j>=MOLLWEIDE_MAX_ITER) {
									b *= 0.5;
									break;
								}
							}
							y = ycen - (int)(yscl * sin(b));
							theta = PI/2 - b;
							break;
/*						case IDM_PROJ_ELLIPTICAL_COS:
						case IDM_PROJ_DUAL_ELLIPTICAL_COS:
							theta = PI * i / (float)num1;
							y = ycen + (int)(yscl * cos(theta));
							break;
*/					}
					x = (int)(xscl * sin(theta));
					sp->x1 = xcen - x;
					sp->y1 = y;
					sp->x2 = xcen + x;
					sp->y2 = y;
					++sp;
				}
				DrawSegments(segments,sp-segments);
				xcen += 2 * xscl;
			}
			break;
	}
/*
** get transformation matrix
*/
	if (mRelativeToFit && data->curcon >= 0) {
		vec[0] = data->rcon[data->curcon].dir[0];
		vec[1] = data->rcon[data->curcon].dir[1];
		vec[2] = data->rcon[data->curcon].dir[2];
		getRotAlignInv( vec, rot1);
		vec[0] = data->rcon[data->curcon].pos[0];
		vec[1] = data->rcon[data->curcon].pos[1];
		vec[2] = data->rcon[data->curcon].pos[2];
	} else {
		matrixIdent(rot1);
		vec[0] = 0;
		vec[1] = 0;
		vec[2] = 0;
	}
	get3DMatrix(mProj.rot, mProj.phi, mProj.theta, 0.);
	matrixMult(rot1, mProj.rot );
	SetLineWidth(1);
/*
** Draw viewing angle if not set to home
*/
	if (mProj.theta || mProj.phi) {
	    int horiz = (mProj.proj_type == IDM_PROJ_RECTANGULAR ||
	                 mProj.proj_type == IDM_PROJ_LAMBERT);
		DrawAngles(horiz);
	}
/*
** Draw reconstructed points and cones
*/
	for (i=0; i<data->nrcon; ++i) {

		num1 = data->rcon[i].num_nodes - 2;
		
		if (num1 <= 0) continue;
		if (i == data->watercon[0]) {
			if (!data->waterLevel) continue;
			SetForeground(WATER_COL);
		} else if (i == data->watercon[1]) {
			continue;
		} else if (i == data->curcon) {
			SetForeground(CURCON_COL);
		} else {
			SetForeground(RCON_COL);
		}
		n1  = data->rcon[i].nodes + 2;		/*First node of circle*/
		n0  = n1 + num1 - 1;			/*Last node */
		sp = segments;
		if (n0->flags & NODE_MISSED) {
			missed = 0x02;
		} else {
			ReMapProj( n0, vec, rot1, &mProj, &nod);
			sp->x1 = nod.x;
			sp->y1 = nod.y;
			missed = 0;
		}

		for (j=0; j<num1; ++j) {
			missed >>= 1;	// shift last node missed flag to bit 0
			if (n1->flags & NODE_MISSED) {
				missed |= 0x02;
			} else {
				ReMapProj( n1, vec, rot1, &mProj, &nod);
				if (!missed) {
					sp->x2 = nod.x;
					sp->y2 = nod.y;
					sp = AddProjLine(sp,segments,n0,n1,vec,rot1,&mProj,0);
				} else {
					sp->x1 = nod.x;
					sp->y1 = nod.y;
				}
			}
			n0 = n1;
			n1++;
		}
		if (sp-segments) {
			DrawSegments(segments,sp-segments);
		}
	}
/*
** Transform the hits into the current projection coordinates
*/
	TransformHits(vec, rot1);
/*
** Draw hits
*/
	if ((num=data->hits.num_nodes) != 0) {
		int d1, d2;
		hi = data->hits.hit_info;
		n0 = data->hits.nodes;
		float scale = mProj.xscl * PROJ_PMT_SIZE * data->hit_size;

		if (mSizeOption==IDM_SIZE_SOLID_ANGLE && mRelativeToFit && data->curcon >= 0) {
			RconEvent *rcon = data->rcon + data->curcon;
			for (i=0; i<num; ++i,++n0,++hi) {
				if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
				float xt = n0->x3 - rcon->pos[0];
				float yt = n0->y3 - rcon->pos[1];
				float zt = n0->z3 - rcon->pos[2];
				float r = sqrt(xt*xt + yt*yt + zt*zt);
				if (r <= .01) {
					d1 = (int)(scale * 100.0);
				} else {
					d1 = (int)(scale / r);
				}
				if (d1 < 1) d1 = 1;
				d2 = d1 * 2 + 1;
				SetForeground(NUM_COLOURS + hi->hit_val);
				if (mShapeOption == IDM_HIT_SQUARE) {
					FillRectangle(n0->x-d1, n0->y-d1,d2,d2);
				} else {
					FillArc(n0->x, n0->y, d1, d1);
				}
			}
		} else if (mSizeOption == IDM_SIZE_QHS) {
			for (i=0; i<num; ++i,++n0,++hi) {
				if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
				float q = fabs((float)hi->qhs -QHS_OFFSET);
				d1 = (int)(sqrt(q) * scale * QHS_FACTOR);
				if (d1 < 1) d1 = 1;
				d2 = d1 * 2 + 1;
				SetForeground(NUM_COLOURS + hi->hit_val);
				if (mShapeOption == IDM_HIT_SQUARE) {
					FillRectangle(n0->x-d1, n0->y-d1,d2,d2);
				} else {
					FillArc(n0->x, n0->y, d1, d1);
				}
			}
		} else {
			d1 = (int)scale;
			if (d1 < 1) d1 = 1;
			d2 = d1 * 2 + 1;
			for (i=0; i<num; ++i,++n0,++hi) {
				if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
				SetForeground(NUM_COLOURS + hi->hit_val);
				if (mShapeOption == IDM_HIT_SQUARE) {
					FillRectangle(n0->x-d1, n0->y-d1,d2,d2);
				} else {
					FillArc(n0->x, n0->y, d1, d1);
				}
			}
		}
	}
}

void PMapImage::SetFitRelative(int rel)
{
	mRelativeToFit = rel;
	SetDirty();
}

//----------------------------------------------------------------------------------------------
// Menu callbacks
//
void PMapImage::DoMenuCommand(int anID)
{
	switch (anID) {
		case IDM_PROJ_RECTANGULAR:
		case IDM_PROJ_LAMBERT:
		case IDM_PROJ_SINUSOID:
		case IDM_PROJ_ELLIPTICAL:
		case IDM_PROJ_MOLLWEIDE:
		case IDM_PROJ_HAMMER:
		case IDM_PROJ_EXTENDED_HAMMER:
		case IDM_PROJ_POLAR:
		case IDM_PROJ_POLAR_EQUAL:
		case IDM_PROJ_DUAL_SINUSOID:
		case IDM_PROJ_DUAL_ELLIPTICAL:
		case IDM_PROJ_DUAL_MOLLWEIDE:
		case IDM_PROJ_DUAL_HAMMER:
		case IDM_PROJ_DUAL_POLAR:
		case IDM_PROJ_DUAL_POLAR_EQUAL:
		{
			ImageData	*data = mOwner->GetData();
			
			if (PMenu::UpdateTogglePair(&mProjType)) {
				data->wProjType = mProjType;	// update latest projection type selected
				MenuList *ms = PMenu::GetCurMenuItem();
				char *str = PMenu::GetLabel(ms);
				if (str) {
					XtFree(data->projName);
					data->projName = str;
				}
				SetProjection(mProjType);
				// set window title if not main window
				if (mOwner != data->mMainWindow) {
					Arg	wargs[1];
					XtSetArg(wargs[0], XmNtitle, data->projName);
					XtSetValues(mOwner->GetShell(), wargs, 1);		/* set new title */
				}
			}
		}	break;
		
		case IDM_PROJ_TO_HOME:
			mOwner->SetToHome();
			break;
			
		case IDM_DETECTOR_COORD:
		case IDM_FIT_RELATIVE:
		{
			int val = IsFitRelative() ? IDM_FIT_RELATIVE : IDM_DETECTOR_COORD;
			if (PMenu::UpdateTogglePair(&val)) {
				mOwner->GetData()->wProjCoords = val;
				SetFitRelative(val == IDM_FIT_RELATIVE);
			}
		}	break;
		
		case IDM_SIZE_FIXED:
		case IDM_SIZE_SOLID_ANGLE:
		case IDM_SIZE_QHS:
			if (PMenu::UpdateTogglePair(&mSizeOption)) {
				mOwner->GetData()->wSizeOption = mSizeOption;
				SetDirty();
			}
			break;
		
		case IDM_HIT_SQUARE:
		case IDM_HIT_CIRCLE:
			if (PMenu::UpdateTogglePair(&mShapeOption)) {
				mOwner->GetData()->wShapeOption = mShapeOption;
				SetDirty();
			}
			break;
	}
}
