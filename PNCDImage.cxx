#include <Xm/RowColumn.h>
#include <math.h>
#include "PNCDImage.h"
#include "PImageWindow.h"
#include "PNCDHistogram.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "colours.h"

const float kNcdRadius   = 400;
const float kNcdTubeSize = 20;
const int   kCursorNear  = 8;   // pixels that cursor is near NCD tube

#define NCD_MARGIN_BOTTOM		(25 * GetScaling())
#define NCD_MARGIN_LEFT			(48 * GetScaling())
#define NCD_MARGIN_TOP			(14 * GetScaling())
#define NCD_MARGIN_RIGHT		(24 * GetScaling())
#define NCD_LABEL_Y				(-10 * GetScaling())

// ------------------------------------------------------------

PNCDImage::PNCDImage(PImageWindow *owner, Widget canvas)
		  : PProjImage(owner,canvas)
{
	int			n;
	Arg			wargs[10];
	
	if (!canvas) {
		n = 0;
		XtSetArg(wargs[n],XmNmarginHeight,		1); ++n;
		XtSetArg(wargs[n],XmNleftAttachment,	XmATTACH_FORM); ++n;
		XtSetArg(wargs[n],XmNtopAttachment,		XmATTACH_FORM); ++n;
		CreateCanvas("ncdCanvas", kScrollLeftMask);
	}
}

PNCDImage::~PNCDImage()
{
}

void PNCDImage::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageNCDSizeChanged:
			SetDirty();
			break;
		default:
			PProjImage::Listen(message,dataPt);
			break;
	}
}

// find ncd nearest current cursor position
int PNCDImage::FindNearestNcd()
{
	ImageData *data = mOwner->GetData();

	if (data->mLastImage != this) {
		TransformHits();	// must transform hits to this projection
	}
	
	int num = data->numNcds;
	int cur_num = -1;
	int	curx = data->last_cur_x;
	int	cury = data->last_cur_y;

    int d = 1000000;

	float fr = mProj.xscl * kNcdTubeSize / kNcdRadius;
    int r = (int)(fr + 0.5) + kCursorNear;
    int r2 = r * r;

	for (int i=0; i<num; ++i) {
	    NCDMap *map = data->ncdMap + i;
	    int x = map->nodes[0].x;
	    int y = map->nodes[0].y;
		int dx = x - curx;
		int dy = y - cury;
		int t = dx*dx + dy*dy;
		if (t <= d) {			/* find closest node	*/
			cur_num = i;
			d = t;
		}
	}
    /* don't show hit info if too far away from hit (r + kCursorNear pixels) */
    if (d > r2) {
        cur_num = -1;
    }
    if (data->cursor_ncd != cur_num) {
        data->cursor_ncd = cur_num;
        return(1);
    }
	return(0);
}

/*
** Draw NCD image
*/
void PNCDImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	int			i,num;
#ifdef PRINT_DRAWS
	Printf("drawNCDImage\n");
#endif	

	PImageCanvas::DrawSelf();	// let the base class clear the drawing area
	
	if (IsDirty()) {
	    TransformHits();
	}

	long bit_mask = HiddenHitMask();	// don't draw hidden tubes

	num = data->numNcds;
	
	float fr = mProj.xscl * kNcdTubeSize / kNcdRadius;
    int r = (int)(fr + 0.5);
    if (r < 1) r = 1;

    float first, last;
    PNCDHistogram::GetBins(data, &first, &last);
    float range = last - first;
    long ncols = data->num_cols - 2;
	
	SetFont(PResourceManager::sResource.hist_font);
	
	for (i=0; i<num; ++i) {
	    NCDMap *map = data->ncdMap + i;
	    int x = map->nodes[0].x;
	    int y = map->nodes[0].y;
        NCDHit *nh = data->ncdHit + i;
        if (!(nh->flags & ~bit_mask)) {
            if (nh->flags) {
                // use hidden colour for cut NCD hits
                SetForeground(HID_COL);
            } else {
                SetForeground(FRAME_COL);
            }
            DrawArc(x,y,r,r);
        } else {
            float val = ncols * (getNcdHitVal(data, nh) - first) / range;
            if (val < 0) {
                val = -1;
            } else if (val >= ncols) {
                val = ncols;
            }
            SetForeground(FIRST_SCALE_COL + (int)val + 1);
            FillArc(x,y,r,r);
        }
        SetForeground(TEXT_COL);
        DrawString(x,y+r*4/3,map->label,kTextAlignTopCenter);
	}
	int y1 = NCD_MARGIN_TOP;
	int x2 = mWidth - NCD_MARGIN_RIGHT;
    char buff[256];
    PNCDHistogram::GetHistogramLabel(data, buff);
    SetForeground(TEXT_COL);
	DrawString(x2+1,y1+NCD_LABEL_Y,buff,kTextAlignTopRight);
}

void PNCDImage::TransformHits()
{
	ImageData	*data = mOwner->GetData();
#ifdef PRINT_DRAWS
	Printf(":transform NCD\n");
#endif
	
	int num = data->numNcds;
	int xscl = mProj.xscl;
	int yscl = mProj.yscl;
	int xcen = (int)(mProj.xcen - xscl * mProj.pt[0]);
	int ycen = (int)(mProj.ycen + yscl * mProj.pt[1]);
	
	for (int i=0; i<num; ++i) {
	    NCDMap *map = data->ncdMap + i;
	    int x = (int)(xcen + (map->x / kNcdRadius) * xscl + 0.5);
	    int y = (int)(ycen - (map->y / kNcdRadius) * yscl + 0.5);
        map->nodes[0].x = map->nodes[1].x = x;
        map->nodes[0].y = map->nodes[1].y = y;
	}
	
	PProjImage::TransformHits();
}


