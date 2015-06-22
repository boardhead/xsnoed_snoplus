#include <math.h>
#include <stdlib.h>
#include "XSnoedImage.h"
#include "XSnoedWindow.h"
#include "PResourceManager.h"
#include "PHitInfoWindow.h"
#include "PMonteCarloWindow.h"
#include "PNCDHistogram.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"

#define STRETCH				2
#define RCON_WID			(2 * GetScaling())	// half-width of reconstructed point
#define SOURCE_WID			(2 * GetScaling())	// half-width of source monte-carlo point
#define	CELL				0.02
#define	HEX_ASPECT			0.866025
#define CONE_MAG			0.85	// default cone mag. for set-to-vertex
#define NECK_FACES			24		// faces in neck (first in list)

#ifdef AV_ANCHOR
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// AV anchor rope drawing constants
#define kAV_NSeg            10      // segments in curved AV rope section
#define kAV_RopeCol         SCALE_COL2  // colour of AV anchor ropes

// Adjustable AV anchor rope parameters
const double kAV_Buoy = 150; // 150 tonnes total buoyancy
const double ke = 1200;      // coefficient of elasticity for rope (tonnes)
const double kDamp = .3;      // damping factor for iterative solution

// anchor rope basket geometries:
// 1 - scratch geometry
// 2 - Phil's (over-constrained) 2-layer basket
// 3 - Ken's 3-layer basket
// 4 - Ken's 4-layer basket
// 5 - Ken's 5-layer basket
// 6 - Ken's 6-layer basket
// 7 - NetC 2-layer
// 8 - NetC 3-layer
// 9 - Aksel's net
#define ROPE_GEOMETRY       8   // rope basket geometry type
#define PERTURBATION        1   // flag to use AV anchor node/rope perturbations (0 or 1)
#define PRINT_ROPES         3   // print node positions and rope tensions first N anchor ropes

/*
** perturbations on AV anchor rope lengths and initial node positions
*/
#if PERTURBATION
    struct RopePerturbation {
        int rope_index;         // (0 to kAV_NRopeSegs-1)
        double delta_length;    // change in rope length (cm)
    };
    struct NodePerturbation {
        int node_index;         // (0 to kAV_NNodes-1)
        double delta_theta;     // change in node theta/phi (degrees)
        double delta_phi;
    };
    // (to break a rope, lengthen it by an arbitrarily large amount)
    static RopePerturbation perturb_rope[] = {
        { 7, 0 },
   //     { 0,-5},
   //       { 4, -10},
    };
    static NodePerturbation perturb_node[] = {
        { 0, 0.0, 0.0 },
    };
    // sigma in cm to perturb rope segments with a Gaussian distribution
    const double kPerturbSegment = 2;
    const double kPerturbAnchor = 4;
#endif

/*
** AV anchor rope geometries
*/
#if ROPE_GEOMETRY == 1      // scratch geometry

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar

    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        180, 0,
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 2,
        0, 1,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        100,10,10,1,1,1,
        kAV_Buoy/kAV_NCells/2,
        kAV_Buoy/kAV_NCells/2,
    };

#elif ROPE_GEOMETRY == 9      // Aksel's

    #define kAV_NCells          10      // number of rope cells
    #define kAV_NCellAnchors    2       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar
    #define kAV_TH1             19.25   // theta of middle AV rope collar
    #define kAV_TH2             20      // theta of bottom AV rope collar

    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, -360.0 / (kAV_NCells * 2),
        kAV_TH2, -360.0 / (kAV_NCells * 4),
        kAV_TH2, 360.0 / (kAV_NCells * 4),
        180, -360.0 / (kAV_NCells * 4),
        180, 360.0 / (kAV_NCells * 4),
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 6,
        0, 1,
        0, 7,
        1, 2,
        2, 3,
        3, 7,
        2, 4,
        3, 5,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        100,10,10,1,1,1,
        kAV_Buoy/kAV_NCells/2,
        kAV_Buoy/kAV_NCells/2,
    };

#elif ROPE_GEOMETRY == 8    // NetC 3 layer

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar
    #define kAV_TH1             19.25   // theta of middle AV rope collar
    #define kAV_TH2             20      // theta of bottom AV rope collar
    #define kAV_TH3             29.25   // theta of middle AV rope collar
    #define kAV_TH4             31      // theta of bottom AV rope collar
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 0,
        kAV_TH2, 360.0 / (kAV_NCells * 2),
        kAV_TH4, 0,
        kAV_TH3, 360.0 / (kAV_NCells * 2),
        180, 0,
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0,6,
        0,1,
        1,2,
        2,7,
        2,4,
        3,4,
        4,9,
        3,5,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
       9,3,15,15,5,9,9,kAV_Buoy/(kAV_NCells*kAV_NCellAnchors),
    };

#elif ROPE_GEOMETRY == 7    // NetC 2 layer

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar
    #define kAV_TH1             19.25      // theta of middle AV rope collar
    #define kAV_TH2             20      // theta of bottom AV rope collar
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 0,
        kAV_TH2, 360.0 / (kAV_NCells * 2),
        180, 360.0 / (kAV_NCells * 2),
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0,4,
        0,1,
        1,2,
        2,5,
        2,3,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        10,7,23,23,kAV_Buoy/(kAV_NCells*kAV_NCellAnchors),
    };

#elif ROPE_GEOMETRY == 6    // Ken's 6-layer scheme

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             20      // theta of top AV rope collar
    #define kAV_TH1             22      // theta of layer 1
    #define kAV_TH2             24      // theta of layer 2
    #define kAV_TH3             26      // theta of layer 3
    #define kAV_TH4             28      // theta of layer 4
    #define kAV_TH5             30      // theta of bottom layer
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 360.0 / (kAV_NCells * 2),
        kAV_TH2, 0,
        kAV_TH3, 360.0 / (kAV_NCells * 2),
        kAV_TH4, 0,
        kAV_TH5, 360.0 / (kAV_NCells * 2),
        180, 360.0 / (kAV_NCells * 2),
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 7,
        0, 1,
        1, 7,
        1, 2,
        1, 9,
        2, 3,
        3, 9,
        3, 4,
        3, 11,
        4, 5,
        5, 11,
        5, 6,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        20,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        kAV_Buoy/kAV_NCells
    };

#elif ROPE_GEOMETRY == 5    // Ken's 5-layer scheme

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             30      // theta of top AV rope collar
    #define kAV_TH1             36      // theta of layer 1
    #define kAV_TH2             42      // theta of layer 2
    #define kAV_TH3             48      // theta of layer 3
    #define kAV_TH4             54      // theta of bottom layer
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 360.0 / (kAV_NCells * 2),
        kAV_TH2, 0,
        kAV_TH3, 360.0 / (kAV_NCells * 2),
        kAV_TH4, 0,
        180, 0,
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 6,
        0, 1,
        1, 2,
        1, 6,
        1, 8,
        2, 3,
        3, 4,
        3, 8,
        3, 10,
        4, 5,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        20,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        kAV_Buoy/kAV_NCells
    };

#elif ROPE_GEOMETRY == 4    // Ken's 4-layer scheme

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             25      // theta of top AV rope collar
    #define kAV_TH1             26      // theta of layer 1
    #define kAV_TH2             27.5      // theta of layer 2
    #define kAV_TH3             30      // theta of bottom layer
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 360.0 / (kAV_NCells * 2),
        kAV_TH2, 0,
        kAV_TH3, 360.0 / (kAV_NCells * 2),
        180, 360.0 / (kAV_NCells * 2),
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 5,
        0, 1,
        1, 5,
        1, 2,
        1, 7,
        2, 3,
        3, 7,
        3, 4,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        20,
        4,
        4,
        4,
        4,
        4,
        4,
        kAV_Buoy/kAV_NCells
    };

#elif ROPE_GEOMETRY == 3    // Ken's 3-layer scheme

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar
    #define kAV_TH1             19.25      // theta of middle AV rope collar
    #define kAV_TH2             20      // theta of bottom AV rope collar

    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 360.0 / (kAV_NCells * 2),
        kAV_TH2, 0,
        180, 0,
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 4,
        0, 1,
        1, 4,
        1, 2,
        1, 6,
        2, 3,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        10,
        2,
        2,
        10,
        10,
        kAV_Buoy/kAV_NCells
    };

#elif ROPE_GEOMETRY == 2    // Phil's original 2-layer basket

    #define kAV_NCells          20      // number of rope cells
    #define kAV_NCellAnchors    1       // number of anchor ropes per cell
    #define kAV_TH0             10      // theta of top AV rope collar
    #define kAV_TH1             20      // theta of bottom AV rope collar
    
    // net node positions in theta (z angle) and phi (x-y) angle [degrees]
    // (use theta=180 for anchor point)
    static double node_pos[] = {
        kAV_TH0, 0,
        kAV_TH1, 360.0 / (kAV_NCells * 2),
        180, 360.0 / (kAV_NCells * 2),
    };
    // AV anchor rope node connections (must all be positive numbers)
    static int net_ropes[] = {
        0, 3,
        1, 4,
        1, 3,
        0, 1,
        1, 2,
    };
    // pre-tensions for each of the above ropes
    static double pretens[] = {
        12,
        12,
        3.7,
        3.7,
        kAV_Buoy/kAV_NCells
    };

#endif

// AV anchor rope constants:
// number of nodes per anchor rope
const int kAV_NodeIncr = sizeof(node_pos) / (2 * sizeof(double));
// number of rope segments per cell
const int kAV_RopeIncr = sizeof(net_ropes) / (2 * sizeof(int));
// total number of nodes (including anchor points)
const int kAV_NNodes = kAV_NodeIncr * kAV_NCells;
// total number of rope segments
const int kAV_NRopeSegs = kAV_RopeIncr * kAV_NCells;
// number of anchor ropes
const int kAV_NAnchors = kAV_NCells * kAV_NCellAnchors;
// radius of AV in drawing coordinates
const double kAV_R = 600.0 / 851.153;

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#endif // AV_ANCHOR

#define UPDATE_FIT    		0x01	// update fit information window
#define	UPDATE_HIT_VALS		0x02	// update displays where hit values are used

#define	NN_AXES				16
#define NE_AXES				11

#define NCD_TUBE_RADIUS     2.54    // NCD tube radius in cm

static Point3 axes_nodes[NN_AXES] = {
							{	0   ,  0   ,  0		},
							{	1.1 ,  0   ,  0		},
							{	0   ,  1.1 ,  0		},
							{	0   ,  0   ,  1.1	},
							{	1.15,  0.05, -0.05	},
							{	1.25, -0.05,  0.05	},
							{	1.15, -0.05,  0.05	},
							{	1.25,  0.05, -0.05	},
							{	0.05,  1.25, -0.05	},
							{	0   ,  1.2 ,  0		},
							{  -0.05,  1.25,  0.05	},
							{	0   ,  1.15,  0		},
							{  -0.05,  0.05,  1.15	},
							{	0.05, -0.05,  1.15	},
							{  -0.05,  0.05,  1.25	},
							{	0.05, -0.05,  1.25	} };

static int	axes_node1[NE_AXES] = { 0,0,0,4,6,8, 9, 9,12,13,14 };
static int	axes_node2[NE_AXES] = { 1,2,3,5,7,9,10,11,13,14,15 };

#ifdef AV_ANCHOR
// lookup to convert from node index in rnodes array to index in matrix
// (there are no corresponding elements for anchor points in the matrix)
static int  matrixNodeNum[kAV_NNodes];

struct DNode {
    double x3,y3,z3;
};
#endif // AV_ANCHOR

//--------------------------------------------------------------------------------------------

XSnoedImage::XSnoedImage(PImageWindow *owner, Widget canvas)
		   : PProjImage(owner,canvas)
{
	char		*msg;
	ImageData	*data = owner->GetData();
	
	mHitSize = 0;
	mMaxMagAtan = atan(100.0);	// 100.0 = maximum magnification
	mMCVertex = NULL;
	mMarginPix = 2;
	mMarginFactor = 1.25;
	mProj = data->proj;
	mProj.proj_type = IDM_PROJ_3D;
	mInvisibleHits = HIT_FECD;  // don't draw FECD's
	SetDirty(kDirtyAll);
	
	// initialize constant elements of mTubeNode
	mTubeNode[0].y3 = 0;
	mTubeNode[3].y3 = 0;
	for (int i=0; i<6; ++i) {
		mTubeNode[i].z3 = 1;
	}
	data->mSpeaker->AddListener(this);

	/* transform the sun direction */
	Transform(&data->sun_dir, 1);
	
	memset(&mAxes, 0, sizeof(mAxes));
	memset(&mVessel, 0, sizeof(mVessel));
	memset(&mFrame, 0, sizeof(mFrame));
	
	initNodes(&mAxes, axes_nodes, NN_AXES);
	initEdges(&mAxes, axes_node1, axes_node2, NE_AXES);
	msg = loadGeometry(&mVessel,IDM_VESSEL,data->argv,data->tube_radius);
	if (msg) quit(msg);
	msg = loadGeometry(&mFrame,data->wGeo,data->argv,data->tube_radius);
	if (msg) quit(msg);
	
	SetToHome();

	if (data->show_vessel) {
		CalcVesselShading();
	}

	if (data->wMCTrack == IDM_MC_SINGLE_TRACK) {
		FindMonteCarloVertex(1);
	}
}

XSnoedImage::~XSnoedImage()
{
	freeWireFrame(&mAxes);
	freePoly(&mVessel);
	freePoly(&mFrame);
}


void XSnoedImage::Listen(int message, void *dataPt)
{
	ImageData	*data = mOwner->GetData();
	
	switch (message) {
		case kMessageSetToVertex:
			if (data->nrcon) {
				SetToVertex();
				SetScrolls();
				SetDirty();
			}
			break;
		case kMessageSetToSun:
			SetToSun();
			SetScrolls();
			SetDirty();
			break;
		case kMessageVesselChanged:
			if (data->show_vessel) {
				CalcVesselShading();
				SetDirty(kDirtyVessel);
			} else {
				SetDirty();
			}
			break;
		case kMessageSunMoved:
			if (data->show_vessel) {
				CalcVesselShading();
			}
			break;
		case kMessageGeometryChanged:
			loadGeometry(&mFrame, data->wGeo, data->argv,data->tube_radius);
			SetDirty(kDirtyFrame);
			break;
		case kMessageMonteCarloChanged:
			if (data->monteCarlo) {
				SetDirty();
			}
			break;
		case kMessageCursorHit:
			if (data->wMCTrack == IDM_MC_SINGLE_TRACK) {
				if (FindMonteCarloVertex(0)) {
					SetDirty();
				}
			}
			break;
		case kMessageEventCleared:
			mMCVertex = NULL;	// reset vertex pointer to avoid accessing stale data
			break;
		case kMessageHitLinesChanged:
		case kMessageFitLinesChanged:
		case kMessageHitSizeChanged:
		case kMessageNCDSizeChanged:
			SetDirty();
			break;
		case kMessageNewEvent:
			mMCVertex = NULL;	// reset vertex pointer to avoid accessing stale data
			SetDirty(kDirtyAll);
			break;
		case kMessageFitChanged:
			SetDirty(kDirtyFit);
			break;
		case kMessageAngleFormatChanged:
			if (mProj.theta || mProj.phi || mProj.gamma) {
				SetDirty();
			}
			break;
		default:
			PProjImage::Listen(message,dataPt);
			break;
	}
}


// initialize variable elements of mTubeNode
void XSnoedImage::SetHitSize()
{
	if (mHitSize != mOwner->GetData()->hit_size) {
		mHitSize = mOwner->GetData()->hit_size;
		
		mTubeNode[0].x3 = mHitSize * ( CELL);
		mTubeNode[1].x3 = mHitSize * ( CELL/2);
		mTubeNode[2].x3 = mHitSize * (-CELL/2);
		mTubeNode[3].x3 = mHitSize * (-CELL);
		mTubeNode[4].x3 = mHitSize * (-CELL/2);
		mTubeNode[5].x3 = mHitSize * ( CELL/2);
		mTubeNode[1].y3 = mHitSize * ( CELL) * HEX_ASPECT;
		mTubeNode[2].y3 = mHitSize * ( CELL) * HEX_ASPECT;
		mTubeNode[4].y3 = mHitSize * (-CELL) * HEX_ASPECT;
		mTubeNode[5].y3 = mHitSize * (-CELL) * HEX_ASPECT;
		
		SetDirty();
	}
}

void XSnoedImage::CalcGrab3(int x,int y)
{
	float		al,fr;

	CalcGrab2(x,y);
	al = sqrt(mGrabX*mGrabX + mGrabY*mGrabY);
	mGrabZ = cos(al);
	if (mProj.inside) mGrabZ = (-mGrabZ);		
	if (!al) fr = 0;
	else     fr = sin(al)/al;
	mGrabX *= fr;				
	mGrabY *= fr;	
}


void XSnoedImage::CalcGrab2(int x,int y)
{
/*
** Calculate distance from center (xc,yc) expressed as a fraction of r
*/
	mGrabX = mProj.pt[0] + (x - mProj.xcen)/(float)mProj.xscl;
	mGrabY = mProj.pt[1] - (y - mProj.ycen)/(float)mProj.yscl;
}


void XSnoedImage::HandleEvents(XEvent *event)
{
	float		xl,yl,zl;
	Vector3		v1,v2;
	float		theta, phi, alpha;
	Matrix3		tmp;
	ImageData	*data = mOwner->GetData();
	static int	wMove;
	static int	rotate_flag;
	static int	update_flags;
	static int	button_down  = 0;

	switch (event->type) {
	
		case ButtonPress:
			if (HandleButton3(event)) return;
			if (!button_down) {
				if (IsInLabel(event->xbutton.x, event->xbutton.y)) {
					ShowLabel(!IsLabelOn());
					SetCursorForPos(event->xbutton.x, event->xbutton.y);
					break;
				}
				XGrabPointer(data->display, XtWindow(mCanvas),0,
							 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
							 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
				button_down = event->xbutton.button;
				
				wMove = data->wMove;
				if (wMove==IDM_MOVE_EVENT && (data->curcon<0 ||
					((data->curcon==data->watercon[0] || data->curcon==data->watercon[1])
						 && !data->waterLevel)))
				{
					/* move the sphere because no event is available */
					wMove = IDM_MOVE_SPHERE;
				}
			
				switch (button_down) {
					case Button1:
						rotate_flag = (wMove == IDM_MOVE_SPHERE);
						break;
					case Button2:
						rotate_flag = (wMove != IDM_MOVE_SPHERE);
						break;
					default:
						button_down = 0;
						break;
				}
				if (button_down) {
					if (rotate_flag) {
						CalcGrab3(event->xbutton.x, event->xbutton.y);
					} else {
						CalcGrab2(event->xbutton.x, event->xbutton.y);
					}
					SetCursor(CURSOR_MOVE_4);
				}
				update_flags = 0;
			}
			break;			
		case ButtonRelease:
			if (button_down == (int)event->xbutton.button) {
				SetCursor(CURSOR_XHAIR);
				XUngrabPointer(data->display, CurrentTime);
				button_down = 0;
/*
** Update all necessary windows after grab is released
*/
				if (update_flags & UPDATE_HIT_VALS)  {
					sendMessage(data, kMessageHitsChanged);
				}
				if (update_flags & UPDATE_FIT) {
					sendMessage(data, kMessageFitChanged);
				}
			}
			break;

		case MotionNotify:
			if (!button_down) {
				// let the base class handle pointer motion
				PProjImage::HandleEvents(event);
				data->mSpeaker->Speak(kMessage3dCursorMotion,(void *)this);
				break;
			}

			if (event->xmotion.is_hint) break;

			xl = mGrabX;
			yl = mGrabY;
			zl = mGrabZ;
/*
** Shift event or sphere
*/
			if (!rotate_flag) {
				CalcGrab2(event->xbutton.x, event->xbutton.y);
				switch (wMove) {
					case IDM_MOVE_SPHERE:
						mProj.pt[0] += xl - mGrabX;
						mProj.pt[1] += yl - mGrabY;
						mGrabX = xl;
						mGrabY = yl;
						SetDirty(kDirtyAll);
						mOwner->SetScrolls();
						break;
					case IDM_MOVE_EVENT:
						v1[0] = mGrabX - xl;
						v1[1] = mGrabY - yl;
						v1[2] = 0;
						vectorMult(mProj.inv, v1, v2);
						data->rcon[data->curcon].pos[0] += v2[0];
						data->rcon[data->curcon].pos[1] += v2[1];
						data->rcon[data->curcon].pos[2] += v2[2];
						/* update water level if we change the water */
						if (data->curcon == data->watercon[0]) {
							data->water_level[0] = data->rcon[data->curcon].pos[2] * data->tube_radius;
						} else if (data->curcon == data->watercon[1]) {
							data->water_level[1] = data->rcon[data->curcon].pos[2] * data->tube_radius;
						}
						setRconNodes(data->rcon+data->curcon);
						Transform(data->rcon[data->curcon].nodes, data->rcon[data->curcon].num_nodes);
						calcCalibratedVals(data);
						update_flags |= UPDATE_FIT | UPDATE_HIT_VALS;
						break;
				}
				Draw();
				break;
			}
			CalcGrab3(event->xmotion.x, event->xmotion.y);

			v1[0] = yl*mGrabZ - zl*mGrabY;		/* calculate axis of rotation */
			v1[1] = zl*mGrabX - xl*mGrabZ;
			v1[2] = xl*mGrabY - yl*mGrabX;

			unitVector(v1);				/* (necessary for phi calculation) */

			if (v1[0] || v1[1]) theta = atan2(v1[1],v1[0]);
			else theta = 0;
			phi   = acos(v1[2]);
			alpha = -vectorLen(mGrabX-xl, mGrabY-yl, mGrabZ-zl);

			getRotMatrix(tmp, theta, phi, alpha);

			switch (wMove) {
				case IDM_MOVE_SPHERE:
					matrixMult(mProj.rot, tmp);
					RotationChanged();
					SetDirty(kDirtyAll);
					break;
				case IDM_MOVE_EVENT:
					if (data->rcon[data->curcon].num_nodes > 1) {
						memcpy(v1, data->rcon[data->curcon].dir, sizeof(Vector3));
						vectorMult(mProj.rot, v1, data->rcon[data->curcon].dir);
						vectorMult(tmp, data->rcon[data->curcon].dir, v1);
						vectorMult(mProj.inv, v1, data->rcon[data->curcon].dir);
						setRconNodes(data->rcon+data->curcon);

						Transform(data->rcon[data->curcon].nodes, data->rcon[data->curcon].num_nodes);
						update_flags |= UPDATE_FIT;
					}
					break;
			}
			SetDirty();
			break;
			
		default:
			PProjImage::HandleEvents(event);
			break;
	}
}

/*
** Set scrollbars to proper location
*/
void XSnoedImage::SetScrolls()
{
	int			pos;

	pos = kScrollMax - (int)(kScrollMax * atan(mProj.mag) / mMaxMagAtan + 0.5);
	mOwner->SetScrollValue(kScrollLeft, pos);
	pos = (kScrollMax/2) - (int)(kScrollMax*mSpinAngle/(2*PI));
	mOwner->SetScrollValue(kScrollBottom, pos);
/*
	pos = (kScrollMax/2) + (int)(kScrollMax/2*mProj.pt[0]);
	mOwner->SetScrollValue(kScrollBottom, pos);
*/	if (mProj.pt[2] >= mProj.proj_max) {
		pos = kScrollMax;
	} else if (mProj.pt[2] <= mProj.proj_min) {
		pos = 0;
	} else {
		pos = (int)(kScrollMax*(1.0-atan(STRETCH/(mProj.pt[2]-mProj.proj_min))/(PI/2))+0.5);
	}
	mOwner->SetScrollValue(kScrollRight, pos);
}


void XSnoedImage::ScrollValueChanged(EScrollBar bar, int value)
{
	int			val;
	float		t;
	
	switch (bar) {
		case kScrollRight:
			val = kScrollMax - value;
			if (val == 0) {
				mProj.pt[2] = mProj.proj_max;
			} else if (val == kScrollMax) {
				mProj.pt[2] = mProj.proj_min;
			} else {
				t = val * (PI/(2*kScrollMax));
				mProj.pt[2] = STRETCH / tan(t) + mProj.proj_min;
			}
			SetDirty(kDirtyAll);
			break;
		case kScrollLeft:
			mProj.mag = tan(mMaxMagAtan * (kScrollMax - value) / kScrollMax);
			Resize();
			break;
		case kScrollBottom: {
			Matrix3	rot;
			float newSpin = (kScrollMax/2 - value) * (2*PI) /kScrollMax;
			get3DMatrix(rot,0.0,0.0,mSpinAngle-newSpin);
			mSpinAngle = newSpin;
			matrixMult(rot,mProj.rot);
			memcpy(mProj.rot,rot,sizeof(rot));
			RotationChanged();
			SetDirty(kDirtyAll);
		} 	break;
		
		default:
			break;
	}
}


void XSnoedImage::Resize()
{
	mProj.xsiz = mWidth;
	mProj.ysiz = mHeight;

	mProj.xcen = mWidth/2;
	mProj.ycen = mHeight/2;

	mProj.xscl = (int)(0.4 * mProj.mag * (mProj.xsiz<mProj.ysiz ? mProj.xsiz : mProj.ysiz));
	if (mProj.xscl < 1) mProj.xscl = 1;
	mProj.yscl = mProj.xscl;

	SetDirty(kDirtyAll);
}

#ifdef AV_ANCHOR
/*
** matrix stuff
*/
#define     MSIZ        ((kAV_NNodes - kAV_NAnchors) * 2 + 1)
#define		NEAR_ZERO	1.0e-20
void lubksb(double a[MSIZ][MSIZ],int indx[MSIZ],double b[MSIZ])
{
	int		i,ii=-1,ip,j;
	double	sum;

	for (i=0; i<MSIZ; ++i) {
		ip = indx[i];
		sum = b[ip];
		b[ip] = b[i];
		if (ii>=0) for (j=ii; j<i; ++j) sum -= a[i][j]*b[j];
		else if (sum) ii = i;
		b[i] = sum;
	}
	for (i=MSIZ-1; i>=0; --i) {
		sum = b[i];
		for (j=i+1; j<MSIZ; ++j) sum -= a[i][j]*b[j];
		b[i] = sum/a[i][i];
	}
}
int ludcmp(double a[MSIZ][MSIZ],int indx[MSIZ], double *d)
{
	int		i,imax=0,j,k;
	double	big,dum,sum,temp;
	double	vv[MSIZ];

	*d = 1.0;
	for (i=0; i<MSIZ; ++i) {
		big = 0.0;
		for (j=0; j<MSIZ; ++j) if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) {
		    printf("Singular at %d!\n",i);
		    return 1;		/* ERROR: singular matrix!!! */
		}
		vv[i] = 1.0/big;
	}
	for (j=0; j<MSIZ; ++j) {
		for (i=0; i<j; ++i) {
			sum = a[i][j];
			for (k=0; k<i; ++k) sum -= a[i][k]*a[k][j];
			a[i][j] = sum;
		}
		big = 0.0;
		for (i=j; i<MSIZ; ++i) {
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
			for (k=0; k<MSIZ; ++k) {
				dum = a[imax][k];
				a[imax][k] = a[j][k];
				a[j][k] = dum;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (a[j][j] == 0.0) a[j][j] = NEAR_ZERO;
		if (j != MSIZ-1) {
			dum = 1.0/a[j][j];
			for (i=j+1; i<MSIZ; ++i) a[i][j] *= dum;
		}
	}
	return 0;
}

int matrixInvert(double in[MSIZ][MSIZ], double out[MSIZ][MSIZ])
{
	int		i,j,indx[MSIZ];
	double	d;
	double  tmp[MSIZ][MSIZ];
	double	col[MSIZ];

	memcpy(tmp,in,sizeof(tmp));

	if (ludcmp(tmp,indx,&d)) return 1;
	for (j=0; j<MSIZ; ++j) {
		for (i=0; i<MSIZ; ++i) col[i] = 0.0;
		col[j] = 1.0;
		lubksb(tmp,indx,col);
		for (i=0; i<MSIZ; ++i) out[i][j] = col[i];
	}
	return 0;
}
void vectorMult(double m[MSIZ][MSIZ], double in[MSIZ], double out[MSIZ])
{
    for (int i=0; i<MSIZ; ++i) {
        out[i] = 0;
        for (int j=0; j<MSIZ; ++j) {
            out[i] += m[i][j] * in[j];
        }
    }
}
static double matrix[MSIZ][MSIZ];
static double minv[MSIZ][MSIZ];

void AddRope(XSegment **avr, Node *nodes, int n0, int n1, int **irope, int rope_index)
{
    int an = (nodes[n0].zr <= 0) ? 0 : 1;
    avr[an]->x1 = nodes[n0].x;  avr[an]->y1 = nodes[n0].y;
    avr[an]->x2 = nodes[n1].x;  avr[an]->y2 = nodes[n1].y;
    ++avr[an];
    *(irope[an]) = rope_index;
    ++irope[an];
}

// calculate great circle distance between 2 points on AV
double StraightLineDist(DNode *nodes, int n0, int n1)
{
    double dx = nodes[n0].x3 - nodes[n1].x3;
    double dy = nodes[n0].y3 - nodes[n1].y3;
    double dz = nodes[n0].z3 - nodes[n1].z3;
    return sqrt(dx*dx+dy*dy+dz*dz);
}
double GreatCircleDist(DNode *nodes, int n0, int n1)
{
    return 2 * kAV_R * asin(StraightLineDist(nodes,n0,n1)/(2 * kAV_R));
}
double VectorLen(DNode *n)
{
    return sqrt(n->x3*n->x3 + n->y3*n->y3 + n->z3*n->z3);
}
void UnitVector(DNode *in, DNode *out)
{
    double d = VectorLen(in);
    out->x3 = in->x3 / d;
    out->y3 = in->y3 / d;
    out->z3 = in->z3 / d;
}
void CrossProduct(DNode *in0, DNode *in1, DNode *out)
{
    out->x3 = in0->y3 * in1->z3 - in0->z3 * in1->y3;
    out->y3 = in0->z3 * in1->x3 - in0->x3 * in1->z3;
    out->z3 = in0->x3 * in1->y3 - in0->y3 * in1->x3;
}
double DotProduct(DNode *in0, DNode *in1)
{
    return(in0->x3*in1->x3 + in0->y3*in1->y3 + in0->z3*in1->z3);
}
void VectorAdd(DNode *in0, DNode *in1, DNode *out)
{
    out->x3 = in0->x3 + in1->x3;
    out->y3 = in0->y3 + in1->y3;
    out->z3 = in0->z3 + in1->z3;
}
void VectorSubtr(DNode *in0, DNode *in1, DNode *out)
{
    out->x3 = in0->x3 - in1->x3;
    out->y3 = in0->y3 - in1->y3;
    out->z3 = in0->z3 - in1->z3;
}
void ScalarMult(DNode *in, double s, DNode *out)
{
    out->x3 = in->x3 * s;
    out->y3 = in->y3 * s;
    out->z3 = in->z3 * s;
}
void PrintTP(DNode *node)
{
    double d = sqrt(node->x3 * node->x3 + node->y3 * node->y3);
    double t = atan2(d, node->z3);
    double p = atan2(node->y3, node->x3);
    printf("(%7.4f %7.4f)",t*180/PI,p*180/PI);
}
void PrintVector(DNode *n)
{
    printf("(%.4g,%.4g,%.4g)",n->x3,n->y3,n->z3);
}
void DumpMatrix(double m[MSIZ][MSIZ])
{
    printf("-------------------\n");
    for (int i=0; i<MSIZ; ++i) {
        int zeros = 0;
        for (int j=0; j<MSIZ; ++j) {
            double v = m[i][j];
            double fv = fabs(v);
            if (fv > 1e-12) {
                if (j > zeros) printf(" ");
                if (zeros) {
                    if (zeros > 2) {
                        printf("%dx0 ",zeros);
                    } else if (zeros == 2) {
                        printf("0 0 ");
                    } else {
                        printf("0 ");
                    }
                }
                zeros = 0;
                if (fv > 100 && fv < 99999) {
                    printf("%.0f",v);
                } else {
                    printf("%.3g",v);
                }
            } else {
                ++zeros;
            }
        }
        if (zeros) {
            if (zeros < MSIZ) printf(" ");
            if (zeros > 2) {
                printf("%dx0",zeros);
            } else if (zeros == 2) {
                printf("0 0");
            } else {
                printf("0");
            }
        }
        printf("\n");
    }
    printf("-------------------\n");
}
// fill in tension matrix for shift in n0 position on tension in rope segment rseg
void FillMatrix(DNode *node, int nn0, int nn1, double *restlen, double *ropelen, int rseg, double ts[MSIZ])
{
    DNode tn0, tn1, t01, ph0, th0, t10, th1, ph1, z0, p0, p1, t0, t1;
    DNode dTdt0, dTdp0, dTdt1, dTdp1;
    DNode *n0 = node + nn0;  // shifting this node
    DNode *n1 = node + nn1;  // stationary node
    z0.x3 = z0.y3 = 0;
    z0.z3 = 1;

    double tens = ke * (ropelen[rseg] - restlen[rseg]) / restlen[rseg];
//printf("%d %d %g ",nn0,nn1,tens);PrintVector(n0);PrintVector(n1);printf("\n");
    CrossProduct(n0, n1, &tn0);
    CrossProduct(&tn0, n0, &tn1);
    UnitVector(&tn1, &t01);         // t01 = direction of tension on n0 from n1
    CrossProduct(&z0, n0, &tn1);
    UnitVector(&tn1, &ph0);         // ph0 = unit vector in phi direction at n0
    CrossProduct(&tn1, n0, &tn0);
    UnitVector(&tn0, &th0);         // th0 = unit vector in theta direction at n0
    CrossProduct(n0, &t01, &tn0);
    UnitVector(&tn0, &p0);          // p0 = unit vector perpendicular to tension at n0
    
    // calculate rate of change of tension vectors at n0
    // wrt change in n0 theta and phi positions
    double dist = StraightLineDist(node, nn0, nn1);
    // (note: can't calculate rope length from GreatCirclDist() because
    //  it doesn't account for bottom sections of anchor ropes)
    // double rlen = GreatCircleDist(node, nn0, nn1);
    double rsint = kAV_R * sqrt(n0->x3*n0->x3+n0->y3*n0->y3) / VectorLen(n0);
    // apply factor to make it difficult to push on a rope
    // (factor can't be zero or we get singular matrix with too many slack ropes)
    double f = (tens > 0) ? 1 : 1e-4;
    // dTdp_u0 is change in tension component in direction of T at n0 due to phi motion of n0
    // dTdp_v0 is change in tension component perpendicular to T at n0 due to phi motion of n0
    double dTdp_u0 = - rsint * DotProduct(&ph0,&t01) * ke   * f / restlen[rseg];
    double dTdp_v0 = - rsint * DotProduct(&ph0,&p0)  * tens * f / dist;
    double dTdt_u0 = - kAV_R * DotProduct(&th0,&t01) * ke   * f / restlen[rseg];
    double dTdt_v0 = - kAV_R * DotProduct(&th0,&p0)  * tens * f / dist;
    ScalarMult(&t01, dTdp_u0, &t0);
    ScalarMult(&p0,  dTdp_v0, &t1);
    // dTdp0 = change in tension vector at n0 due to phi motion of n0
    VectorAdd(&t0, &t1, &dTdp0);
    ScalarMult(&t01, dTdt_u0, &t0);
    ScalarMult(&p0,  dTdt_v0, &t1);
    // dTdt0 = change in tension vector at n0 due to theta motion of n0
    VectorAdd(&t0, &t1, &dTdt0);

    CrossProduct(n1, n0, &tn0);
    CrossProduct(&tn0, n1, &tn1);
    UnitVector(&tn1, &t10);         // t10 is direction of tension on n1 from n0
    CrossProduct(&z0, n1, &tn1);
    UnitVector(&tn1, &ph1);         // unit vector in phi direction at n1
    CrossProduct(&tn1, n1, &tn0);
    UnitVector(&tn0, &th1);         // unit vector in theta direction at n1
    CrossProduct(n1, &t10, &tn0);
    UnitVector(&tn0, &p1);          // p1 = unit vector perpendicular to tension at n1

    // calculate rate of change of tension vectors at n1
    // wrt change in n0 theta and phi positions
    ScalarMult(&t10, dTdp_u0, &t0);
    ScalarMult(&p1,  dTdp_v0, &t1);
    // dTdp1 = change in tension vector at n1 due to phi motion of n0
    VectorAdd(&t0, &t1, &dTdp1);
    ScalarMult(&t10, dTdt_u0, &t0);
    ScalarMult(&p1,  dTdt_v0, &t1);
    // dTdt1 = change in tension vector at n1 due to theta motion of n0
    VectorAdd(&t0, &t1, &dTdt1);

    if (nn0 >= kAV_NNodes) {   // handle anchor point separately
        // effect of moving the anchor point (in Z only) on tension sum at n1
        int i1 = 2 * matrixNodeNum[nn1];
        matrix[i1]      [MSIZ - 1] += ke * f / restlen[rseg]; // theta component of movement in z
        matrix[MSIZ - 1][MSIZ - 1] -= ke * f / restlen[rseg]; // z component of movement in z
        double diff = ropelen[rseg] - restlen[rseg];
        ts[MSIZ - 1]        += diff * ke * f / restlen[rseg]; // total z tension of all anchors
        return;
    }
    // determine our matrix index
    int i0 = 2 * matrixNodeNum[nn0];
    // the effect of moving n0 on tension sum at n0
    matrix[i0]    [i0]     += DotProduct(&dTdt0, &th0); // theta component of movement in theta
    matrix[i0]    [i0 + 1] += DotProduct(&dTdp0, &th0); // theta component of movement in phi
    matrix[i0 + 1][i0]     += DotProduct(&dTdt0, &ph0); // phi component of movement in theta
    matrix[i0 + 1][i0 + 1] += DotProduct(&dTdp0, &ph0); // phi component of movement in phi
//printf("%d %d %g %g %g %g\n",nn0, nn1, restlen[rseg], ropelen[rseg], dt_t, dt_p);
    // the effect of moving n0 on tension sum at n1
    if (nn1 >= kAV_NNodes) {  // handle anchor point separately
        // effect on total tension sum
        matrix[MSIZ - 1][i0]     += DotProduct(&dTdt1, &z0); // z component of movement in theta
        matrix[MSIZ - 1][i0 + 1] += DotProduct(&dTdp1, &z0); // z component of movement in phi
    } else {
        int i1 = 2 * matrixNodeNum[nn1];
        matrix[i1]    [i0]     += DotProduct(&dTdt1, &th1); // theta component of movement in theta
        matrix[i1]    [i0 + 1] += DotProduct(&dTdp1, &th1); // theta component of movement in phi
        matrix[i1 + 1][i0]     += DotProduct(&dTdt1, &ph1); // phi component of movement in theta
        matrix[i1 + 1][i0 + 1] += DotProduct(&dTdp1, &ph1); // phi component of movement in phi
    }
    double fac = ke * f * (ropelen[rseg] - restlen[rseg]) / restlen[rseg];
    ts[i0]     += fac * DotProduct(&t01, &th0); // add to theta component of tension sum at node 0
    ts[i0 + 1] += fac * DotProduct(&t01, &ph0); // add to phi component of tension sum at node 0
}
void ShiftNode(DNode *node, double dt, double dp)
{
    // rotate node in theta (downward angle from z axis) and phi (horizontal angle from x axis)
    double d = sqrt(node->x3 * node->x3 + node->y3 * node->y3);
    double t = atan2(d, node->z3) + dt;
    double p = atan2(node->y3, node->x3) + dp;
    double ct = cos(t);
    d = sqrt(1 - ct * ct);
    node->x3 = kAV_R * d * cos(p);
    node->y3 = kAV_R * d * sin(p);
    node->z3 = kAV_R * ct;
}
#endif // AV_ANCHOR

/*
** Main Xsnoed 3-D drawing routine
*/
void XSnoedImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	int			rx, ry;
	XSegment	segments[MAX_EDGES], *sp;
	XPoint		point[6];
	int			i,j,n,track_col,vertex_col,num,in;
	int			x,y;
	Node		*n0,*n1,*n2;
	Edge		*edge, *last;
	Face		*face, *lface;
	Projection	tube_proj;
	HitInfo		*hi;
	long		bit_mask = HiddenHitMask();	// don't draw hidden tubes
	static Node	tnode = { 0, 0, 1 };

/*
** recalculate necessary values for display
*/
	if (IsDirty() & kDirtyAll) {

		if (mProj.pt[2] < mProj.proj_max) {
			mProj.inside = (vectorLen(mProj.pt[0],mProj.pt[1],mProj.pt[2]) < mFrame.radius);
		} else {
			mProj.inside = 0;
		}
		if (IsDirty() & kDirtyHits) {
			TransformHits();
		}
		if (IsDirty() & kDirtyFrame) {
			transformPoly(&mFrame, &mProj);
		}
		if (IsDirty() & kDirtyAxes) {
			Transform(mAxes.nodes, mAxes.num_nodes);
		}
		if (IsDirty() & kDirtySun) {
			Transform(&data->sun_dir, 1);
		}
		if (IsDirty() & kDirtyFit) {
			for (i=0; i<data->nrcon; ++i) {
				Transform(data->rcon[i].nodes, data->rcon[i].num_nodes);
			}
		}
		if ((IsDirty() & kDirtyVessel) && data->show_vessel) {
			Vector3		ip;
	
			transformPoly(&mVessel,&mProj);
			vectorMult(mProj.inv,mProj.pt,ip);
			mProj.neck_first = (ip[2] < NECK_BASE/data->tube_radius);
			
			if (mProj.pt[2] < mProj.proj_max) {
				mProj.in_ves = (vectorLen(mProj.pt[0],mProj.pt[1],mProj.pt[2]) < mVessel.radius);
				if (!mProj.in_ves && mProj.inside) {
					// rotate projection point into vessel frame of reference
					if (ip[2]>0 && ip[2]<1 && sqrt(ip[0]*ip[0]+ip[1]*ip[1])<NECK_RADIUS/data->tube_radius) {
						mProj.in_ves = 1;
					}
				}
			} else {
				mProj.in_ves = 0;
			}
		}
	}
/*
** Start drawing
*/
#ifdef PRINT_DRAWS
	Printf("draw 3-D image\n");
#endif

	PImageCanvas::DrawSelf();	// let the base class clear the drawing area

	// transform hits for this image if necessary
	if (data->mLastImage != this) {
	    TransformHits();
	}
	SetHitSize();
	SetFont(data->hist_font);

	/* draw projection angles */
	if (mProj.theta || mProj.phi || mProj.gamma) DrawAngles(0,kAngleAll);
/*
** Draw hidden edges of figure (should really avoid drawing nodes behind screen!!)
*/
	mDrawable->Comment("Back of PSUP");
	edge = mFrame.edges;
	last = edge + mFrame.num_edges;
	for (sp=segments; edge<last; ++edge) {
		n1 = edge->n1;
		n2 = edge->n2;
		if (n1->flags & n2->flags & (NODE_HID | NODE_OUT)) continue;
		if (edge->f1->flags & edge->f2->flags & FACE_HID) {
			sp->x1 = n1->x;
			sp->y1 = n1->y;
			sp->x2 = n2->x;
			sp->y2 = n2->y;
			++sp;
		} else {
			edge->flags |= EDGE_HID;
		}
	}
	SetLineWidth(THIN_LINE_WIDTH);
	if (mDrawable->GetDeviceType() == kDevicePrinter) {
		// must be postscript output
		if (mDrawable->EqualColours(HID_COL, BKG_COL)) {
			SetForeground(HID_COL);	// hidden lines are invisible
		} else {
			SetForeground(FRAME_COL);
		}
		SetLineType(kLineTypeDot);
	} else {
		if (mProj.inside) {
			SetForeground(FRAME_COL);
		} else {
			SetForeground(HID_COL);
		}
	}
	DrawSegments(segments,sp-segments);
	SetLineWidth(THICK_LINE_WIDTH);
	SetLineType(kLineTypeSolid);

#ifdef AV_ANCHOR
/*
** calculate AV anchor rope positions and draw back AV ropes
*/
    static int once = 0;
    static int rope_colour[kAV_NRopeSegs];
    static Node nodes[kAV_NNodes+kAV_NAnchors];
    double ropelen_av, ropelen_tot;
    if (data->anchor_ropes && !once) {
        double ropetens[kAV_NRopeSegs];
        // solve differential tension matrix to find
        // final node positions and rope tensions
        for (int iter = 0; iter < 500; ++iter) {
            double restlen[kAV_NRopeSegs];
            double ropelen[kAV_NRopeSegs];
            DNode rnode[kAV_NNodes+kAV_NAnchors];
            double ts0[MSIZ];
            memset(ts0, 0, sizeof(ts0));
            ts0[MSIZ-1] = kAV_Buoy; 
            memset(ropetens, 0, sizeof(ropetens));  // start with zero tensions
            if (!iter) {
                int bn = 0;
                // calculate initial node positions
                for (n=0; n<kAV_NCells; ++n) {
                    double pincr = 2 * PI * n / kAV_NCells;
                    for (int np=0; np<kAV_NodeIncr; ++np) {
                        int n0 = n * kAV_NodeIncr + np;
                        double t, cost;
                        double p = node_pos[np*2+1] * PI / 180 + pincr;
                        if (node_pos[np*2] < 179) {
                            matrixNodeNum[n0] = bn++;   // calculate matrix node numbers
                            t = (90 - node_pos[np*2]) * PI / 180;
                            cost = cos(t);
                            rnode[n0].z3 = kAV_R * sin(t);
                        } else {
                            cost = 1;
                            rnode[n0].z3 = -2 * kAV_R;  // anchor point Z position
                        }
                        rnode[n0].x3 = kAV_R * cost * cos(p);
                        rnode[n0].y3 = kAV_R * cost * sin(p);
                    }
                }
            }
            // use current node locations for drawing
            // (note: there is an extra tangent point for each anchor)
            for (i=0; i<kAV_NNodes+kAV_NAnchors; ++i) {
                nodes[i].x3 = rnode[i].x3;
                nodes[i].y3 = rnode[i].y3;
                nodes[i].z3 = rnode[i].z3;
            }
            // calculate rope tensions and matrix elements at current node positions
            memset(matrix, 0, sizeof(matrix));
            ropelen_av = ropelen_tot = 0;
            for (i=0, n=0; i<kAV_NCells; ++i, n += kAV_NodeIncr) {
                int anchor = 0;
                for (int rope=0; rope < kAV_RopeIncr; ++rope) {
                    int n0 = n + net_ropes[rope * 2];
                    int n1 = n + net_ropes[rope * 2 + 1];
                    if (n0 >= kAV_NNodes) n0 -= kAV_NNodes;
                    if (n1 >= kAV_NNodes) n1 -= kAV_NNodes;
                    int ropenum = i*kAV_RopeIncr + rope;
                    if (rnode[n0].z3 > -kAV_R && rnode[n1].z3 > -kAV_R) {
                        // calculate rope length on AV
                        ropelen[ropenum] = GreatCircleDist(rnode, n0, n1);
                        ropelen_av += ropelen[ropenum];
                        ropelen_tot += ropelen[ropenum];
                    } else {
                        // calculate rope length to bottom anchor point
                        if (rnode[n0].z3 <= -kAV_R) {
                            // make n1 the bottom attachment point
                            int t = n0; n0 = n1; n1 = t;
                        }
                        // calculate point where rope leaves AV
                        // solve for rx (point where rope leaves AV):
                        // (n0 x n1) x rx = 0
                        // rx . rx = kAV_R * kAV_R
                        // rx . (n1 - rx) = 0  (same as rx . n1 = rx . rx)
                        DNode u;
                        CrossProduct(rnode+n0, rnode+n1, &u);
                        double ux = u.x3;
                        double uy = u.y3;
                        double uz = u.z3;
                        double ax = rnode[n1].x3;
                        double ay = rnode[n1].y3;
                        double az = rnode[n1].z3;
                        int use_ux;
                        double a, b, c, c0, c1, c2, c3, xa, ya, za, xb, yb, zb, x, y, z;
                        double r2 = kAV_R * kAV_R;
                        if (fabs(ux) > fabs(uy)) {
                            use_ux = 1;
                            c0 = ax * uz / ux - az;
                            c1 = ay - ax * uy / ux;
                            c2 = -uy * r2 / (ux * c1);
                            c3 = uy * c0 / (ux * c1) + uz / ux;
                        } else {
                            use_ux = 0;
                            c0 = ay * uz / uy - az;
                            c1 = ax - ay * ux / uy;
                            c2 = -ux * r2 / (uy * c1);
                            c3 = ux * c0 / (uy * c1) + uz / uy;
                        }
                        a = c3 * c3 + c0 * c0 / (c1 * c1) + 1;
                        b = -2 * c2 * c3 + 2 * r2 * c0 / (c1 * c1);
                        c = c2 * c2 + r2 * r2 / (c1 * c1) - r2;
                        za = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
                        zb = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
                        if (use_ux) {
                            ya = (r2 + c0 * za) / c1;
                            yb = (r2 + c0 * zb) / c1;
                            xa = (-uy * ya - uz * za) / ux;
                            xb = (-uy * yb - uz * zb) / ux;
                        } else {
                            xa = (r2 + c0 * za) / c1;
                            xb = (r2 + c0 * zb) / c1;
                            ya = (-ux * xa - uz * za) / uy;
                            yb = (-ux * xb - uz * zb) / uy;
                        }
                        double da2 = (xa - ax) * (xa - ax) + (ya - ay) * (ya - ay);
                        double db2 = (xb - ax) * (xb - ax) + (yb - ay) * (yb - ay);
                        if (da2 < db2) {
                            x = xa;  y = ya;  z = za;
                        } else {
                            x = xb;  y = yb;  z = zb;
                        }
                        // save position of tangent point where rope leaves vessel
                        int nt = kAV_NNodes+i*kAV_NCellAnchors+anchor;
                        rnode[nt].x3 = x;
                        rnode[nt].y3 = y;
                        rnode[nt].z3 = z;
                        ++anchor;
                        // calculate length of anchor rope on the AV
                        ropelen[ropenum] = GreatCircleDist(rnode, n0, nt);
                        ropelen_av += ropelen[ropenum];
                        // add length of straight segment below equator to anchor point
                        DNode tnode;
                        tnode.x3 = x - ax;
                        tnode.y3 = y - ay;
                        tnode.z3 = z - az;
                        ropelen[ropenum] += VectorLen(&tnode);
                        ropelen_tot += ropelen[ropenum];
                    }
                    if (iter) {
                        // printf("%d %g %g\n",ropenum,ropelen[ropenum],restlen[ropenum]);
                        ropetens[ropenum] = ke * (ropelen[ropenum] - restlen[ropenum]) /
                                                  restlen[ropenum];
                        // you can't push on a rope
                        if (ropetens[ropenum] < 0) ropetens[ropenum] = 0;
                    }
                }
            }
            if (!iter) {
                const int N = kAV_NRopeSegs / kAV_NCells;
                for (int i=0; i<kAV_NCells; ++i) {
                    for (int j=0; j<kAV_RopeIncr; ++j) {
                        // pre-tension all ropes
                        restlen[i*N+j] = ropelen[i*N+j] / (1 + pretens[j] / ke);
                        ropetens[i*N+j] = pretens[j];
                    }
                }
            }
            // fill in the matrix elements
            // (tension sums at all nodes in order except anchor point)
            double ts[MSIZ];
            memset(ts, 0, sizeof(ts));
            for (i=0, n=0; i<kAV_NCells; ++i, n += kAV_NodeIncr) {
                int anchor = 0;
                // get base matrix node number (don't count anchor points)
                for (int rope=0; rope < kAV_RopeIncr; ++rope) {
                    int n0 = n + net_ropes[rope * 2];
                    int n1 = n + net_ropes[rope * 2 + 1];
                    if (n0 >= kAV_NNodes) n0 -= kAV_NNodes;
                    if (n1 >= kAV_NNodes) n1 -= kAV_NNodes;
                    int ropenum = i*kAV_RopeIncr + rope;
                    if (rnode[n0].z3 > -kAV_R && rnode[n1].z3 > -kAV_R) {
                        // find change in tensions due to movement of node 0
                        FillMatrix(rnode, n0, n1, restlen, ropelen, ropenum, ts);
                        // find change in tensions due to movement of node 1
                        FillMatrix(rnode, n1, n0, restlen, ropelen, ropenum, ts);
                    } else {
                        if (rnode[n0].z3 <= -kAV_R) {
                            // make n1 the bottom attachment point
                            int t = n0; n0 = n1; n1 = t;
                        }
                        // tangent point
                        int nt = kAV_NNodes + i*kAV_NCellAnchors+anchor;
                        ++anchor;
                        // NOTE: sphere is always at 0, but bottom ropes move down by av_z
                        // find change in tensions due to movement of node 0
                        FillMatrix(rnode, n0, nt, restlen, ropelen, ropenum, ts);
                        // find change in tensions due to AV Z movement
                        FillMatrix(rnode, nt, n0, restlen, ropelen, ropenum, ts);
                    }
                }
            }
            //   printf("--------- inverted -----------\n");
            // invert the matrix and stop if it goes singular
            if (matrixInvert(matrix,minv)) break;
        
            // calculate all tension sum differences
            double dts[MSIZ];
            for (i=0; i<MSIZ; ++i) {
                dts[i] = ts0[i] - ts[i];
            }
            double dx[MSIZ];
            vectorMult(minv, dts, dx);
        
            if (PRINT_ROPES) {
                printf("%d) ===========================================\n",iter);
            }
            // debugging code
            if (0) {
                printf("Matrix: (%d x %d)\n",MSIZ,MSIZ);
                DumpMatrix(matrix);
                printf("0: ");
                for (int j=0; j<3; ++j) {
                    for (i=0; i<kAV_NodeIncr; ++i) {
                        PrintTP(rnode+i+j*kAV_NodeIncr);
                    }
                    for (i=0; i<kAV_RopeIncr; ++i) {
                        printf(" %.2g",ropetens[i+j*kAV_RopeIncr]);
                    }
                    printf("\n");
                    if (j<kAV_RopeIncr-1) printf("   ");
                }
                //printf("Inverted:\n");
                //DumpMatrix(minv);
                printf("dT(t,p): ");
                for (i=0; i<MSIZ; ++i) {
                    if (dts[i]) printf("%.1f ",dts[i]);
                    else printf("0 ");
                }
                printf("\ndr(t,p): ");
                for (i=0; i<MSIZ; ++i) {
                    if (dx[i]) printf("%.1f ",dx[i]);
                    else printf("0 ");
                }
                printf("\n");
            }
            // print node positions and tensions for first PRINT_ROPES anchor ropes
            for (int j=0; j<PRINT_ROPES; ++j) {
                printf("%d: ", j * kAV_NodeIncr);
                for (i=0; i<kAV_NodeIncr; ++i) {
                    PrintTP(rnode+i+j*kAV_NodeIncr);
                }
                for (i=0; i<kAV_RopeIncr; ++i) {
                    printf(" %.1f",ropetens[i+j*kAV_RopeIncr]);
                }
                printf("\n");
            }
// PERTURBATION TEST!!!!
#if PERTURBATION
            if (iter==0) {
                printf("PERTURBATION!!!!\n");
                for (i=0; i<(int)(sizeof(perturb_rope)/sizeof(RopePerturbation)); ++i) {
                    // perturb a rope rest length by a fixed amount
                    restlen[perturb_rope[i].rope_index] +=
                            perturb_rope[i].delta_length * kAV_R / 600.0;
                }
                // Gaussian perturbations
                for (i=0; i<kAV_NRopeSegs; ++i) {
                    // perturb anchor ropes and segments differently
                    double sigma = (restlen[i] > kAV_R) ? kPerturbAnchor : kPerturbSegment;
                    double dl = sigma * sqrt(-2.0 * log(rand()/(double)RAND_MAX)) *
                                cos(2 * 3.1415926536 *  rand()/(double)RAND_MAX);
                    restlen[i] += dl * kAV_R / 600.0;
                    if (restlen[i] < 0) {
                        restlen[i] = 0;
                        printf("Rope %d rest length is NEGATIVE!!!!\n",i);
                    }
                }
                for (i=0; i<(int)(sizeof(perturb_node)/sizeof(NodePerturbation)); ++i) {
                    // perturb the initial position of a node by a fixed amount
                    ShiftNode(rnode + perturb_node[i].node_index,
                                      perturb_node[i].delta_theta * PI / 180,
                                      perturb_node[i].delta_phi   * PI / 180);
                }
            }
#endif
            // adjust all node positions based on calculated shifts (* damping factor)
            double max = 0;
            for (i=0; i<kAV_NCells; ++i) {
                for (int j=0; j<kAV_NodeIncr; ++j) {
                    int rn = i * kAV_NodeIncr + j; // node index in rnode array
                    if (rnode[rn].z3 > -kAV_R) {
                        int mn = 2 * matrixNodeNum[rn]; // node index in matrix
                        double dt, dp;
                        dt = kDamp * dx[mn];
                        dp = kDamp * dx[mn + 1];
                        if (fabs(dt) > max) max = fabs(dt);
                        if (fabs(dp) > max) max = fabs(dp);
                        ShiftNode(rnode + rn, dt, dp);
                    } else {
                        // shift anchor point
                        rnode[rn].z3 += kDamp * dx[MSIZ-1];
                    }
                }
            }
            double maxdeg = max * 180 / PI;
            if (maxdeg < 1e-5) {
                printf("Converged after %d iterations\n", iter);
                printf("Total rope length:%6.1f m\n", ropelen_tot * 6.0 / kAV_R);
                printf("Total rope on AV: %6.1f m\n", ropelen_av * 6.0 / kAV_R);
                break;
            }
            if (maxdeg > 90) {
                printf(">>> KAAAAAPPPOOOWWW after %d iterations!!!!\n",iter);
                break;
            }
        }
        // determine rope colours based on tension
        double tmax = 2 * kAV_Buoy / kAV_NAnchors;
	    int ncols = data->num_cols - 2;
	    double max_tension = 0;
        for (i=0; i<kAV_NRopeSegs; ++i) {
            double tens = ropetens[i];
            if (tens > max_tension) max_tension = tens;
            int j;
            if (tens < tmax * 1e-4) {   // anything this close to zero is slack
                j = 0;
            } else if (tens >= tmax) {
                j = ncols;  // use top scale colour instead of overscale colour
            } else {
                j = 1 + int(tens * ncols / tmax);
            }
            rope_colour[i] = j + FIRST_SCALE_COL;
       }
       printf("Max rope tension: %6.1f tonnes\n", max_tension);
       once = 1;   // only calculate tensions and node positions once
    }
/*
** generate AV anchor rope segments and draw back AV ropes
*/
    const int kNDrawSegs = kAV_NCells * (kAV_RopeIncr + kAV_NSeg * kAV_NCellAnchors);
    XSegment avrope0[kNDrawSegs];
    XSegment avrope1[kNDrawSegs];
    XSegment *avr[2] = { avrope0, avrope1 };
    int irope0[kNDrawSegs];
    int irope1[kNDrawSegs];
    int *irope[2] = { irope0, irope1 };
    if (data->anchor_ropes) {
        Transform(nodes, kAV_NNodes);
        int ir=0;
        for (n=0; n<kAV_NCells; ++n) {
            int anchor = 0;
            for (int nr=0; nr<kAV_RopeIncr; ++nr, ++ir) {
                int n0 = n * kAV_NodeIncr + net_ropes[nr * 2];
                int n1 = n * kAV_NodeIncr + net_ropes[nr * 2 + 1];
                if (n0 >= kAV_NNodes) n0 -= kAV_NNodes;
                if (n1 >= kAV_NNodes) n1 -= kAV_NNodes;
                if (nodes[n0].z3 > -kAV_R && nodes[n1].z3 > -kAV_R) {
                    // add simple segment
                    AddRope(avr, nodes, n0, n1, irope, ir);
                    continue;
                }
                if (nodes[n0].z3 < -kAV_R) {
                    // make n0 the top node
                    int t = n0; n0 = n1; n1 = t;
                }
                // index of tangent node
                int nt = kAV_NNodes + n*kAV_NCellAnchors + anchor;
                ++anchor;
                double dx = (nodes[nt].x3 - nodes[n0].x3) / kAV_NSeg;
                double dy = (nodes[nt].y3 - nodes[n0].y3) / kAV_NSeg;
                double dz = (nodes[nt].z3 - nodes[n0].z3) / kAV_NSeg;
                // add segments for curve of anchor rope
                Node segnodes[2];
                memcpy(segnodes + 0, nodes + n0, sizeof(Node));
                for (int m=1; m<=kAV_NSeg; ++m) {
                    double xs = nodes[n0].x3 + dx * m;
                    double ys = nodes[n0].y3 + dy * m;
                    double zs = nodes[n0].z3 + dz * m;
                    double rs = sqrt(xs*xs+ys*ys+zs*zs);
                    segnodes[1].x3 = kAV_R * xs / rs;
                    segnodes[1].y3 = kAV_R * ys / rs;
                    segnodes[1].z3 = kAV_R * zs / rs;
                    Transform(segnodes + 1, 1);
                    AddRope(avr, segnodes, 0, 1, irope, ir);  // (anchor segment)
                    memcpy(segnodes, segnodes+1, sizeof(Node));
                }
                memcpy(segnodes + 1, nodes + n1, sizeof(Node));
                AddRope(avr, segnodes, 0, 1, irope, ir);      // (anchor bottom)
            }
        }
        SetLineWidth(3);
        n = avr[0] - avrope0;
        for (i=0; i<n; ++i) {
#ifdef INVISIBLE_ROPES
            if (rope_colour[irope1[i]] == NUM_COLOURS) continue;
#endif
            SetForeground(rope_colour[irope0[i]]);
		    DrawSegments(avrope0 + i, 1);
		}
        SetLineWidth(THICK_LINE_WIDTH);
    }
#endif // AV_ANCHOR
/*
** Draw vessel
*/
	if (data->show_vessel) {
		mDrawable->Comment("Acrylic vessel");
		face = mVessel.faces;
		lface= face + mVessel.num_faces;
		n2 = &data->sun_dir;
		in = mProj.in_ves;
		if (!mProj.neck_first) face += NECK_FACES;
		for (; face<lface; ++face) {
			if (in) {
				if (!(face->flags & FACE_HID)) continue;
			} else {
				if (face->flags & FACE_HID) continue;
			}
			SetForeground(FIRST_VESSEL_COL + (face->flags>>FACE_COL_SHFT));
			num = face->num_nodes;
			for (i=0,n=0; i<num; ++i) {
				n1 = face->nodes[i];
				if (n1->flags & NODE_OUT) ++n;	/* count # of nodes behind proj screen */
				point[i].x = n1->x;
				point[i].y = n1->y;
			}
			if (n<num) {
				FillPolygon(point,num);
			}
		}
	}
#ifdef AV_ANCHOR
/*
** draw front AV ropes
*/
    if (data->anchor_ropes) {
        SetLineWidth(3);
        int n = avr[1] - avrope1;
        for (i=0; i<n; ++i) {
#ifdef INVISIBLE_ROPES
            if (rope_colour[irope1[i]] == NUM_COLOURS) continue;
#endif
            SetForeground(rope_colour[irope1[i]]);
		    DrawSegments(avrope1 + i, 1);
		}
        SetLineWidth(THICK_LINE_WIDTH);
    }
#endif // AV_ANCHOR
/*
** Draw vessel neck
*/
    if (data->show_vessel) {
		if (!mProj.neck_first) {
			face = mVessel.faces;
			lface= face + NECK_FACES;
			for (; face<lface; ++face) {
				if (in) {
					if (!(face->flags & FACE_HID)) continue;
				} else {
					if (face->flags & FACE_HID) continue;
				}
				SetForeground(FIRST_VESSEL_COL + (face->flags>>FACE_COL_SHFT));
				num = face->num_nodes;
				for (i=0,n=0; i<num; ++i) {
					n1 = face->nodes[i];
					if (n1->flags & NODE_OUT) ++n;
					point[i].x = n1->x;
					point[i].y = n1->y;
				}
				if (n<num) {
					FillPolygon(point,num);
				}
			}
		}
    }
/*
** Draw hits on back of sphere
*/
	if (data->curcon>=0 && data->curcon!=data->watercon[0] && data->curcon!=data->watercon[1]) {
		/* set rx,ry to current reconstructed vertex */
		rx = data->rcon[data->curcon].nodes->x;
		ry = data->rcon[data->curcon].nodes->y;
	} else {
		/* set rx,ry to origin since we have no reconstructed vertex for hit lines */
		rx = mAxes.nodes->x;
		ry = mAxes.nodes->y;
	}
	if ((num=data->hits.num_nodes) != 0) {

		mDrawable->Comment("Hits on back of sphere");
		hi = data->hits.hit_info;
		n0 = data->hits.nodes;
		
		memcpy(&tube_proj,&mProj,sizeof(Projection));
		
		for (i=0; i<num; ++i,++n0,++hi) {
		
			if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
			
			/* don't draw nodes which are out of the display or hidden */
			if (n0->flags & (NODE_OUT | NODE_HID)) continue;

			SetForeground(FIRST_SCALE_COL + hi->hit_val);

			/* rotate the tube node hexagon to the proper place for drawing */
			if (hi->flags & HIT_NORMAL) {
				getRotAlign(&n0->xr, tube_proj.rot);
			} else {
				// handle the case where the node radius may not be 1.0
				getRotAlignScaled(&n0->xr, tube_proj.rot);
			}
			transform(mTubeNode,&tube_proj,6);

			/* draw unhidden tube node */
			if (mProj.inside) {
				for (j=0; j<6; ++j) {
					point[j].x = mTubeNode[j].x;
					point[j].y = mTubeNode[j].y;
				}
				FillPolygon(point, 6);
			} else {
				n1 = mTubeNode + 5;
				n2 = mTubeNode;
				sp = segments;
				for (j=0; j<6; ++j) {
					sp->x1 = n1->x;
					sp->y1 = n1->y;
					sp->x2 = n2->x;
					sp->y2 = n2->y;
					++sp;
					n1 = n2++;
				}
				DrawSegments(segments,sp-segments);
			}	

			if (data->hit_lines) {
				transform(&tnode,&tube_proj,1);
				DrawLine(rx,ry,tnode.x,tnode.y);
			}
		}
	}
/*
** Draw axes
*/
	mDrawable->Comment("Axes and Sun vector");
	edge = mAxes.edges;
	last = edge + mAxes.num_edges;
	for (sp=segments; edge<last; ++edge) {
		n1  = edge->n1;
		n2  = edge->n2;
		if (n1->flags & n2->flags & (NODE_HID | NODE_OUT)) continue;
		sp->x1 = n1->x;
		sp->y1 = n1->y;
		sp->x2 = n2->x;
		sp->y2 = n2->y;
		++sp;
	}
	SetForeground(AXES_COL);
	DrawSegments(segments,sp-segments);
/*
** Draw sun
*/
	n1 = mAxes.nodes;
	n2 = &data->sun_dir;
	SetForeground(SUN_COL);
#ifdef THICK_SUN
	if (mDrawable->GetDeviceType() == kDeviceVideo) SetLineWidth(2);	// double width for sun on screen
#endif
	DrawLine(n1->x,n1->y,n2->x,n2->y);
#ifdef THICK_SUN
	if (mDrawable->GetDeviceType() == kDeviceVideo) SetLineWidth(THICK_LINE_WIDTH);
#endif
/*
** Draw reconstructed points and cones
*/
	if (data->nrcon) {
		mDrawable->Comment("Fits");
	}
	for (i=0; i<data->nrcon; ++i) {
		n1 = data->rcon[i].nodes;
		x = n1->x;
		y = n1->y;
		if (i==data->watercon[0] || i==data->watercon[1]) {
			if (!data->waterLevel) continue;
			SetForeground(WATER_COL);
		} else if (i == data->curcon) {
			SetForeground(CURCON_COL);
		} else {
			SetForeground(RCON_COL);
		}
		
		++n1;
		point[0].x = point[3].x = x - RCON_WID;
		point[1].x = point[2].x = x + RCON_WID;
		point[0].y = point[1].y = y - RCON_WID;
		point[2].y = point[3].y = y + RCON_WID;
		num = data->rcon[i].num_nodes;
		
		if (i!=data->watercon[0] && i!=data->watercon[1]) {
			FillPolygon(point, 4);
			
			if (num <= 1) continue;
			
			/* draw event line */
			DrawLine(x,y,n1->x,n1->y);
		}
			
		num -= 2;						/* get # of cone nodes */
		n2  = n1 + 1;					/* set n2 to first cone node */
		n1  = n2 + num - 1;				/* set n1 initially to last node */
		sp  = segments;
		for (j=0; j<num; ++j) {
			if (!(n1->flags & n2->flags & NODE_OUT)) {
				sp->x1 = n1->x;
				sp->y1 = n1->y;
				sp->x2 = n2->x;
				sp->y2 = n2->y;
				++sp;
				// add lines to cherenkov cone for current fit if option selected
				if (data->rcon_lines && data->curcon==i) {
					sp->x1 = n2->x;
					sp->y1 = n2->y;
					sp->x2 = x;
					sp->y2 = y;
					++sp;
				}
			}
			n1 = n2++;
		}
		DrawSegments(segments,sp-segments);
	}
/*
** draw monte carlo tracks
*/
	if (data->monteCarlo) {
		mDrawable->Comment("Monte Carlo tracks");
		SetLineWidth(THIN_LINE_WIDTH);
		MonteCarloVertex *first = (MonteCarloVertex *)(data->monteCarlo + 1);
		MonteCarloVertex *vertex, *parent;
		int incr;
		if (data->wMCTrack == IDM_MC_SINGLE_TRACK) {
			FindMonteCarloVertex(1);
			parent = mMCVertex;			// start drawing at this vertex
			incr = 0;	// don't use the increment, so set it to zero
		} else {
			incr = 1;
		}
		num = data->monteCarlo->nVertices;
		Node nodes[2];
		float scale = 1.0 / data->tube_radius;
		long text_x = -1000;
		long text_y = -1000;
		int num_passes = 1;
		int count = 0;
		// do two passes if necessary to draw highlighted tracks on top of other tracks
		for (j=0; j<num_passes; ++j) {
			for (i=0,vertex=first; i<num; i+=incr,++vertex) {
	
				// draw only this track chain from daughter through the parents
				// if a single track is displayed
				if (data->wMCTrack == IDM_MC_SINGLE_TRACK) {
					if (++count > num) {
						Printf("Cyclical redundancy in Monte Carlo data\n");
						break;
					}
					if (!parent) break;	// all done if no parent
					vertex = parent;	// set vertex to current parent
					i = vertex - first;	// calculate index for this particle
				}
				
				n = vertex->parent;		// get index of parent vertex
	
				if (n < -1) {
					parent = NULL;
					continue;	// < -1 means don't draw vertex
				}
				int draw_track, draw_vertex;
				
				// does the vertex have a valid parent?
				if (n>=0 && n<num && n!=i) {
					// get pointer to parent vertex
					parent = first + n;
					if (parent->flags & VERTEX_FLAG_HIDDEN) {
						// nothing to draw if both vertex and parent are hidden
						if (vertex->flags & VERTEX_FLAG_HIDDEN) continue;
						// draw the vertex
						draw_vertex = 1;
						// don't draw track since parent is hidden
						draw_track = 0;
					} else {
						if ((vertex->flags & VERTEX_FLAG_HIDDEN) && !(vertex->flags & VERTEX_FLAG_MARK)) {
							draw_vertex = 0;
						} else {
							draw_vertex = 1;
						}
						draw_track = 1;
					}
				} else {
					parent = NULL;	// must set this to NULL before continuing for single track display
					// do nothing if this vertex is hidden
					if (vertex->flags & VERTEX_FLAG_HIDDEN) continue;
					draw_vertex = 1;
					draw_track = 0;
				}
				if (vertex->flags & VERTEX_FLAG_SPECIAL) {
					if (j==0) {
						num_passes = 2;	// force 2 passes
						continue;		// draw this later
					}
					// use special colour for highlighted tracks
					track_col = vertex_col = MC_SPECIAL_COL;
				} else {
					if (j==1) continue;	// don't draw regular vertices on 2nd pass
					
					for (int index=i;;) {	// start using the index for this particle
						switch (first[index].particle) {
							// set colour according to parent particle type
							case 1:
								track_col = MC_PHOTON_COL;
								break;
							case 2:
								track_col = MC_GAMMA_COL;
								break;
							case 20:
								track_col = MC_ELECTRON_COL;
								break;
							case 30: // nu_e
							case 31: // nu_e_bar
							case 32: // nu_mu
							case 33: // nu_mu_bar
							case 34: // nu_tau
							case 35: // nu_tau_bar
								track_col = MC_NEUTRINO_COL;
								break;
							case 81:
								track_col = MC_NEUTRON_COL;
								break;
							default:
								track_col = MC_OTHER_COL;
								break;
						}
						// we have just obtained the colour for the track
						// (the parent particle's colour), so break
						if (index == n) break;
						// we have just obtained the colour for our particle
						vertex_col = track_col;
						// don't need track colour if source track
						if (!parent) break;
						// now get the colour of the parent particle
						index = n;
					}
				}
				nodes[0].x3 = vertex->x * scale; 
				nodes[0].y3 = vertex->y * scale; 
				nodes[0].z3 = vertex->z * scale;
	
				Transform(nodes, 1);
				
				// draw track
				if (draw_track) {
					// get coordinates of parent vertex
					nodes[1].x3 = parent->x * scale; 
					nodes[1].y3 = parent->y * scale; 
					nodes[1].z3 = parent->z * scale; 
					Transform(nodes+1, 1);
					SetForeground(track_col);
					DrawLine(nodes[1].x,nodes[1].y,nodes[0].x,nodes[0].y);
				}
				if (draw_vertex) {
					if (!parent || (vertex->flags & VERTEX_FLAG_MARK)) {
						point[0].x = point[3].x = nodes[0].x - SOURCE_WID;
						point[1].x = point[2].x = nodes[0].x + SOURCE_WID;
						point[0].y = point[1].y = nodes[0].y - SOURCE_WID;
						point[2].y = point[3].y = nodes[0].y + SOURCE_WID;
						SetForeground(vertex_col);
						FillPolygon(point, 4);
					}
				}
				if (data->wMCNames==IDM_INTERACTION_NAMES ||
				   (data->wMCNames==IDM_HIGHLIGHTED_NAMES && vertex_col==MC_SPECIAL_COL))
				{
					x = nodes[0].x - text_x;
					y = nodes[0].y - text_y;
					if (x < 0) x = -x;
					if (y < 0) y = -y;
					if (y>8 || x>50) {
						text_x = nodes[0].x;
						text_y = nodes[0].y;
						char *str = PMonteCarloWindow::InteractionName(vertex);
						SetForeground(vertex_col);
						DrawString(nodes[0].x + 4*GetScaling(), nodes[0].y, str, kTextAlignMiddleLeft);
					}
				}
			}
		}
		SetLineWidth(THICK_LINE_WIDTH);
	}
/*
** draw NCD's
*/
    if (data->numNcdHit) {
        float first, last;
        PNCDHistogram::GetBins(data, &first, &last);
        float range = last - first;
        long ncols = data->num_cols - 2;
        for (n=0; n<data->numNcds; ++n) {
            // don't draw NCD's which only have one segment because they aren't physical
            // (just as we don't draw FECD's in the 3D image)
            if (data->ncdMap[n].num_segments == 1) continue;
            // generate hit mask for this ncd hit
            NCDHit *nh = data->ncdHit + n;
            // draw any NCD that has an unmasked bit
			if (!(nh->flags & ~bit_mask)) continue;
            float val = ncols * (getNcdHitVal(data, nh) - first) / range;
            if (val < 0) {
                val = -1;
            } else if (val >= ncols) {
                val = ncols;
            }
            Node *nodes = data->ncdMap[n].nodes;
            SetForeground(FIRST_SCALE_COL + (int)val + 1);
            float r = mProj.xscl * data->ncd_size * NCD_TUBE_RADIUS / data->tube_radius;
            SetLineWidth(2*r);
            DrawLine(nodes[1].x,nodes[1].y,nodes[0].x,nodes[0].y);
            SetLineWidth(THICK_LINE_WIDTH);
            // draw endcaps
            int ir = (int)(r + 0.5);
            if (ir < 1) ir = 1;
            FillArc(nodes[0].x, nodes[0].y, ir, ir);
            FillArc(nodes[1].x, nodes[1].y, ir, ir);
        }
    }
/*
** draw hits on front of sphere
*/
	n  = 0;				/* initialize counter for # of displayed hits */
	if ((num=data->hits.num_nodes) != 0) {

		mDrawable->Comment("Hits on front of sphere");
		hi = data->hits.hit_info;
		n0 = data->hits.nodes;
		
		for (i=0; i<num; ++i,++n0,++hi) {
		
			if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
			++n;
			if (n0->flags & NODE_OUT) continue;
			if (!(n0->flags & NODE_HID)) continue;
			
			SetForeground(FIRST_SCALE_COL + hi->hit_val);

			/* rotate the tube node hexagon to the proper place for drawing */
			if (hi->flags & HIT_NORMAL) {
				getRotAlign(&n0->xr, tube_proj.rot);
			} else {
				// handle the case where the node radius may not be 1.0
				getRotAlignScaled(&n0->xr, tube_proj.rot);
			}
			transform(mTubeNode,&tube_proj,6);

			/* draw tube on front of sphere */
			if (mProj.inside) {
				n1 = mTubeNode + 5;
				n2 = mTubeNode;
				sp = segments;
				for (j=0; j<6; ++j) {
					sp->x1 = n1->x;
					sp->y1 = n1->y;
					sp->x2 = n2->x;
					sp->y2 = n2->y;
					++sp;
					n1 = n2++;
				}
				DrawSegments(segments,sp-segments);
			} else {
				for (j=0; j<6; ++j) {
					point[j].x = mTubeNode[j].x;
					point[j].y = mTubeNode[j].y;
				}
				FillPolygon(point, 6);
			}	

			if (data->hit_lines) {
				transform(&tnode,&tube_proj,1);
				DrawLine(rx,ry,tnode.x,tnode.y);
			}
		}
	}
/*
** Draw edges in front
*/
	mDrawable->Comment("Front of PSUP");
	edge = mFrame.edges;
	last = edge + mFrame.num_edges;
	for (sp=segments; edge<last; ++edge) {
		if (edge->flags & EDGE_HID) {
			edge->flags &= ~EDGE_HID;
			n1 = edge->n1;
			n2 = edge->n2;
			sp->x1 = n1->x;
			sp->y1 = n1->y;
			sp->x2 = n2->x;
			sp->y2 = n2->y;
			++sp;
		}
	}
	SetForeground(FRAME_COL);
	SetLineWidth(THIN_LINE_WIDTH);
	DrawSegments(segments,sp-segments);
	SetLineWidth(THICK_LINE_WIDTH);
}


void XSnoedImage::TransformHits()
{
	int			i;
	ImageData	*data = mOwner->GetData();
	int			num = data->hits.num_nodes;
#ifdef PRINT_DRAWS
	Printf(":transform 3-D\n");
#endif
	if (num) {

		Node *node = data->hits.nodes;
		
		Transform(node,num);

		float *pt = mProj.pt;
		
		if (pt[2] < mProj.proj_max) {

			for (i=0; i<num; ++i,++node) {
				float dot = (pt[0] - node->xr) * node->xr +
						    (pt[1] - node->yr) * node->yr +
						    (pt[2] - node->zr) * node->zr;
				if (dot > 0) node->flags |= NODE_HID;
			}

		} else {

			for (i=0; i<num; ++i,++node) {
				if (node->zr > 0) node->flags |= NODE_HID;
			}
		}
	}

	/* transform NCD hits */
    for (int n=0; n<data->numNcds; ++n) {
        Node *nodes = data->ncdMap[n].nodes;
        Transform(nodes,2);
    }

	/* must do this to save mLastImage */
	PProjImage::TransformHits();
}


// find ncd nearest current cursor position
int XSnoedImage::FindNearestNcd()
{
	ImageData *data = mOwner->GetData();

	if (data->mLastImage != this) {
		TransformHits();	// must transform hits to this projection
	}
	
	int num = data->numNcds;
	int cur_num = -1;
	double x3 = data->last_cur_x;
	double y3 = data->last_cur_y;

    double d = 1000000;

	long bit_mask = HiddenHitMask();	// don't consider hidden NCD's
	
	for (int i=0; i<num; ++i) {
	    NCDHit *nh = data->ncdHit + i;
		if (!(nh->flags & ~bit_mask)) continue;	/* only consider unmasked NCDs */
	    NCDMap *map = data->ncdMap + i;
	    double x1 = map->nodes[0].x;
	    double y1 = map->nodes[0].y;
	    double x2 = map->nodes[1].x;
	    double y2 = map->nodes[1].y;
	    double dx, dy;
	    if (x1==x2 && y1==y2) {
	        dx = x3 - x1;
	        dy = y3 - y1;
	    } else {
	        // find point (x4,y4) on NCD (x1,y1)-(x2,y2)
	        // that is closest to the cursor (x3,y3)
    	    double u = ((x3-x1)*(x2-x1) + (y3-y1)*(y2-y1)) / 
    	               ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
    	    // limit to a line segment
    	    if (u < 0) u = 0;
    	    if (u > 1) u = 1;
    	    // (x4,y4) is the point on the line segment closest to (x3,y3)
    	    double x4 = x1 + u * (x2-x1);
    	    double y4 = y1 + u * (y2-y1);
    	    dx = x3 - x4;
    	    dy = y3 - y4;
	    }
		double t = dx*dx + dy*dy;
		if (t <= d) {			/* find closest node	*/
			cur_num = i;
			d = t;
		}
	}
    /* don't show hit info if too far away from hit (16 pixels) */
    if (d > 256) {
        cur_num = -1;
    }
    if (data->cursor_ncd != cur_num) {
        data->cursor_ncd = cur_num;
        return(1);
    }
	return(0);
}

/* the rotation matrix has changed */
void XSnoedImage::RotationChanged()
{
	matrixTranspose(mProj.rot,mProj.inv);
	
	/* calculate current viewing angles */
	// theta is the angle from the z axis to the viewing direction
	if (mProj.rot[2][2]) {
		mProj.theta = acos(mProj.rot[2][2]) - PI/2;
	} else {
		mProj.theta = 0;
	}
	// phi is the angle from the x axis to the viewing direction
	// measured CCW in the x-y plane
	if (mProj.rot[2][0]!=1.0 || mProj.rot[2][1]) {
		double len = sqrt(mProj.rot[2][0]*mProj.rot[2][0] + mProj.rot[2][1]*mProj.rot[2][1]);
		if (len) {
			double dx = mProj.rot[2][0] / len;
			double dy = mProj.rot[2][1] / len;
			mProj.phi = atan2(dy,dx);
		} else {
			mProj.phi = 0;
		}
	} else {
		mProj.phi = 0;
	}
	// gamma is the CW angle between the projected z axis and 'up' on the screen
	if (mProj.rot[0][2] || mProj.rot[1][2]) {
		mProj.gamma = atan2(mProj.rot[0][2], mProj.rot[1][2]);
	} else {
		mProj.gamma = 0;
	}
}

void XSnoedImage::SetToHome(int n)
{
	mProj.pt[0] = mProj.pt[1] = 0;
	mProj.pt[2] = mProj.proj_max;
	mProj.mag   = 1.0;
	mProj.theta	= 0.;
	mProj.phi	= 0.;
	mProj.gamma = 0.;
	mSpinAngle	= 0.;

	if (n == 0) {
        /* set home position to z up, y right */
        mProj.rot[0][0] = 0;  mProj.rot[0][1] = 1;  mProj.rot[0][2] = 0;
        mProj.rot[1][0] = 0;  mProj.rot[1][1] = 0;  mProj.rot[1][2] = 1;
        mProj.rot[2][0] = 1;  mProj.rot[2][1] = 0;  mProj.rot[2][2] = 0;
    } else {
        // set to top position - x right, y up
        mProj.rot[0][0] = 1;  mProj.rot[0][1] = 0;  mProj.rot[0][2] = 0;
        mProj.rot[1][0] = 0;  mProj.rot[1][1] = 1;  mProj.rot[1][2] = 0;
        mProj.rot[2][0] = 0;  mProj.rot[2][1] = 0;  mProj.rot[2][2] = 1;
    }
	RotationChanged();

	Resize();
}

void XSnoedImage::SetToSun()
{
	Node	*sun_dir = &mOwner->GetData()->sun_dir;
	
	mProj.pt[0] = mProj.pt[1] = 0;
	mProj.pt[2] = mProj.proj_max;
	mProj.mag   = 1.0;
	mProj.theta	= 0.;
	mProj.phi	= 0.;
	mProj.gamma	= 0.;
	mSpinAngle	= 0.;

	getRotAlignInv(&sun_dir->x3, mProj.rot);
	RotationChanged();
	Resize();
}

void XSnoedImage::SetToVertex()
{
	Vector3		vec;
	ImageData *	data = mOwner->GetData();
	RconEvent *	rcon = data->rcon + data->curcon;

	if (rcon->num_nodes > 1) {
		vec[0] = -rcon->dir[0];				/* want to align with (0,0,-1) */
		vec[1] = -rcon->dir[1];
		vec[2] = -rcon->dir[2];
	} else {
		/* no direction information -- use (0,0,1) arbitrarily */
		vec[0] = 0;
		vec[1] = 0;
		vec[2] = 1;
	}
	getRotAlignInv(vec,mProj.rot);
	vectorMult(mProj.rot,rcon->pos,vec);
	mProj.pt[0] = vec[0];
	mProj.pt[1] = vec[1];
	mProj.pt[2] = vec[2] + 1e-6;			/* add a bit to avoid round-off errors */
/*
** Calculate magnification to make cone a constant size
*/
	mProj.mag	= CONE_MAG / (mProj.pt[2] - mProj.proj_screen);
	mProj.theta = 0.;
	mProj.phi	= 0.;
	mProj.gamma	= 0.;
	mSpinAngle	= 0.;
	
	RotationChanged();
	Resize();
}

/*
** Calculate shading for vessel faces
*/
void XSnoedImage::CalcVesselShading()
{	
	Face		*face, *lface;
	Node		*n2;
	float		dot;
	ImageData	*data = mOwner->GetData();

	face = mVessel.faces;
	lface= face + mVessel.num_faces;
	n2   = &data->sun_dir;
	for (; face<lface; ++face) {
		dot = face->norm.x*n2->x3 + face->norm.y*n2->y3 + face->norm.z*n2->z3;
		face->flags = ((int)((dot + 1) * data->ves_cols / 2) << FACE_COL_SHFT) 
						| (face->flags & FACE_HID);	/* preserve hidden flags */
	}
}


/* find monte carlo vertex nearest the cursor hit */
int XSnoedImage::FindMonteCarloVertex(int forced)
{
	ImageData *data = mOwner->GetData();
	
	/* cursor moved - return non-zero if data changed */
	if (data->monteCarlo) {
		MonteCarloVertex *first = (MonteCarloVertex *)(data->monteCarlo + 1);
		MonteCarloVertex *last = first + data->monteCarlo->nVertices;
		MonteCarloVertex *vertex;
		MonteCarloVertex *closest = NULL;
		int	num = data->cursor_hit;
		float radius = data->tube_radius;
		if (num >= 0) {
			Node	*nod = data->hits.nodes + num;
			float	dist2 = 1e6;
			float	dx, dy, dz;
			for (vertex=first; vertex<last; ++vertex) {
				int int_code = (vertex->int_code / 1000) % 1000;
				if (int_code == 402) {	// if reached PMT
					dx = nod->x3 * radius - vertex->x;
					dy = nod->y3 * radius - vertex->y;
					dz = nod->z3 * radius - vertex->z;
					float t = dx*dx + dy*dy + dz*dz;
					if (t < dist2) {
						closest = vertex;
						dist2 = t;
					}
				}
			}
			if (dist2 > 900.0) closest = NULL;	// too far away if > 30cm
//else printf("%.2f %d\n",sqrt(dist2),num);
		}
		if (closest!=mMCVertex || forced) {
/*			// hide all vertices
			for (vertex=first; vertex<last; ++vertex) {
				vertex->flags |= VERTEX_FLAG_HIDDEN;
			}
			if (closest) {
				vertex = closest;
				do {
					vertex->flags &= ~VERTEX_FLAG_HIDDEN;
					MonteCarloVertex *prev = vertex;
					vertex = first + vertex->parent;
					if (vertex == prev) break;
				} while (vertex>=first && vertex<last);
			}*/
			mMCVertex = closest;
			data->mSpeaker->Speak(kMessageMCVertexChanged, (void *)mMCVertex);
			return(1);
		}
	} else {
		mMCVertex = NULL;
	}
	return(0);
}
	
