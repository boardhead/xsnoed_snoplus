#include <math.h>
#include "PCrateImage.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"

PCrateImage::PCrateImage(PImageWindow *owner, Widget canvas)
		   : PProjImage(owner,canvas)
{
	mScaleProportional = 0;
	mMarginPix	 	   = 0;
	mProj.proj_type = IDM_PROJ_CRATE;
	mMinMagAtan		= atan(0.5);
	mDiffMagAtan	= atan(20.0) - mMinMagAtan;
	mCrateMask		= 0xfffffUL;
	
	if (!canvas) {
		CreateCanvas("crateCanvas",kScrollLeftMask);
	}
}

void PCrateImage::TransformHits()
{
	int			i, num;
	Node		*n0;
	HitInfo		*hi;
	ImageData	*data = mOwner->GetData();
	
#ifdef PRINT_DRAWS
	Printf(":transform crate\n");
#endif	
	if ((num=data->hits.num_nodes) != 0) {

		hi = data->hits.hit_info;
		n0 = data->hits.nodes;

		for (i=0; i<num; ++i,++n0,++hi) {
			n0->x = mMargin_x + mCrate_offset_x * (hi->crate % 10) + hi->card * mChan_w;
			n0->y = mMargin_y + mCrate_offset_y * (hi->crate / 10) + (31-hi->channel) * mChan_h;
		}
	}

	/* 2-D hit node coordinates are now calculated for the crate image */
	PProjImage::TransformHits();
}

/*
** Draw crate image
*/
void PCrateImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	int			i, j, k, len, x, y, num;
	float		water_z;
	Pmt			*pmt;
	char		buff[10];
	XSegment	segments[MAX_EDGES], *sp;
	Node		*n0;
	HitInfo		*hi;
	int			chan_w, chan_h;
	int			crate_w, crate_h;
	int			crate_offset_x, crate_offset_y;
	int			margin_x, margin_y;
#ifdef PRINT_DRAWS
	Printf("drawCrateImage\n");
#endif	

	PImageCanvas::DrawSelf();	// let the base class clear the drawing area
	
	SetFont(PResourceManager::sResource.hist_font);

	int text_height = GetScaling() * (GetFont()->ascent + GetFont()->descent);
	
	float fchan_w = 16 * (mWidth - 8*GetScaling() * 11) / (float)(17 *160);	/* minimum x margin is 8 pixels */
	float fchan_h = (mHeight - 5 * text_height) / (float)64;	/* minimum y margin is 2 * text_height */
	// make the channels square
	if (fchan_h > fchan_w) fchan_h = fchan_w;
	chan_h = (int)(fchan_h * mProj.mag);
	if (chan_h < 1) chan_h = 1;
	chan_w = chan_h;
	
	crate_offset_x = chan_w * 17 + 8 * GetScaling();
	crate_offset_y = chan_h * 33 + 8 * GetScaling() + 3 * text_height / 2;
		
	/* adjust margins to take up the slack from round-off errors */
	margin_x = (mWidth - crate_offset_x * 10) / 2 + 6 * GetScaling();
	margin_y = (mHeight - crate_offset_y * 2) / 2 + 3 * text_height / 2;
	
	/* adjust offset according to projection point */
	float effMag = chan_w / fchan_w;
	margin_x -= (int)(mProj.pt[0] * mWidth * 0.5 * effMag);
	margin_y += (int)(mProj.pt[1] * mHeight * 0.5 * effMag);
	
	/* draw crates */
	crate_w = chan_w * NUM_CRATE_CARDS;
	crate_h = chan_h * NUM_CARD_CHANNELS;
	SetLineWidth(THIN_LINE_WIDTH);
	int ticklen = 2 * GetScaling();
	for (i=0; i<NUM_SNO_CRATES; ++i) {
		x = margin_x + crate_offset_x * (i % 10);
		y = margin_y + crate_offset_y * (i / 10);
		sp = segments;
		sp[0].x1 = sp[2].x2 = sp[3].x1 = sp[3].x2 = x - 1;
		sp[0].y1 = sp[0].y2 = sp[1].y1 = sp[3].y2 = y - 1;
		sp[0].x2 = sp[1].x1 = sp[1].x2 = sp[2].x1 = x + crate_w;
		sp[1].y2 = sp[2].y1 = sp[2].y2 = sp[3].y1 = y + crate_h;
		sp += 4;
		// draw x ticks
		for (j=0; j<=4; ++j) {
			sp->x1 = sp->x2 = x - 1 + 4 * j * chan_w;
			sp->y1 = y + crate_h;
			sp->y2 = sp->y1 + ticklen;
			++sp;
		}
		// draw y ticks
		for (j=0; j<=4; ++j) {
			sp->x1 = x - 1;
			sp->x2 = x - 1 - ticklen;
			sp->y1 = sp->y2 = y + crate_h - 8 * j * chan_h;
			++sp;
		}
		SetForeground(GRID_COL);
		DrawSegments(segments,sp-segments);
		len = sprintf(buff,"%d",i);
		SetForeground(TEXT_COL);
		DrawString(x+(crate_w)/2,y-text_height/2, buff, kTextAlignBottomCenter);
	}
	SetLineWidth(1);
	
	/* draw water level */
	if (data->waterLevel) {
		SetForeground(WATER_COL);
		pmt = data->tube_coordinates;
		water_z = data->water_level[0] / data->tube_radius;

		for (i=0; i<NUM_SNO_CRATES; ++i) {
			x = margin_x + crate_offset_x * (i % 10);
			y = margin_y + crate_offset_y * (i / 10) + NUM_CARD_CHANNELS * chan_h - 1;
			sp = segments;
			for (j=0; j<NUM_CRATE_CARDS; ++j) {
				for (k=0; k<NUM_CARD_CHANNELS; ++k) {
//printf("i=%d j=%d k=%d\n",i,j,k);
					/* nothing to do if this channel isn't in the water */
					if (!(pmt->status & PMT_OK) || pmt->z > water_z) {
						++pmt;
						continue;
					}
					if (sp - segments >= MAX_EDGES-5) break;
					/* draw water along left edge if necessary */
					if (j==0 || !(pmt[-NUM_CARD_CHANNELS].status & PMT_OK) || (pmt[-NUM_CARD_CHANNELS].z > water_z)) {
						sp->x1 = sp->x2 = x + j * chan_w;
						sp->y1 = y - k * chan_h;
						sp->y2 = sp->y1 - chan_h + 1;
						++sp;
					}
					/* draw water along right edge if necessary */
					if (j==NUM_CRATE_CARDS-1 || !(pmt[NUM_CARD_CHANNELS].status & PMT_OK) || (pmt[NUM_CARD_CHANNELS].z > water_z)) {
						sp->x1 = sp->x2 = x + j * chan_w + chan_w - 1;
						sp->y1 = y - k * chan_h;
						sp->y2 = sp->y1 - chan_h + 1;
						++sp;
					}
					/* draw water along top edge if necessary */
					if (k==NUM_CARD_CHANNELS-1 || !(pmt[1].status & PMT_OK) || (pmt[1].z > water_z)) {
						sp->x1 = x + j * chan_w;
						sp->x2 = sp->x1 + chan_w - 1;
						sp->y1 = sp->y2 = y - k * chan_h - chan_h + 1;
						++sp;
					}
					/* draw water along bottom edge if necessary */
					if (k==0 || !(pmt[-1].status & PMT_OK) || (pmt[-1].z > water_z)) {
						sp->x1 = x + j * chan_w;
						sp->x2 = sp->x1 + chan_w - 1;
						sp->y1 = sp->y2 = y - k * chan_h;
						++sp;
					}
					++pmt;
				}
			}
			DrawSegments(segments,sp-segments);
		}
	}
	
	/* draw hits */
	if ((num=data->hits.num_nodes) != 0) {

		hi = data->hits.hit_info;
		n0 = data->hits.nodes;
		x = chan_w / 2;
		y = chan_h / 2;

		for (i=0; i<num; ++i,++n0,++hi) {
			if (hi->flags & data->bit_mask) continue;	/* only consider unmasked hits */
			n0->x = margin_x + crate_offset_x * (hi->crate % 10) + hi->card * chan_w;
			n0->y = margin_y + crate_offset_y * (hi->crate / 10) + (31-hi->channel) * chan_h;
			SetForeground(NUM_COLOURS + hi->hit_val);
			FillRectangle(n0->x,n0->y,chan_w,chan_h);
			n0->x += x;
			n0->y += y;
		}
	}
	// save these for calculations in TransformHits
	mMargin_x = margin_x + chan_w / 2;
	mMargin_y = margin_y + chan_h / 2;
	mCrate_offset_x = crate_offset_x;
	mCrate_offset_y = crate_offset_y;
	mChan_w = chan_w;
	mChan_h = chan_h;

	/* 2-D hit node coordinates are now calculated for the crate image */
	PProjImage::TransformHits();
}

