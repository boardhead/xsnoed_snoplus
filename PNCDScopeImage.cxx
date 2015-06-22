//
// File:		PNCDScopeImage.cxx
//
// Description:	NCD scope image
//
// Revisions:	09/05/03 - PH Created
//
#include <string.h>
#include "PNCDScopeImage.h"
#ifdef SNOPLUS
#include "PCaenWindow.h"
#else
#include "PNCDScopeWindow.h"
#endif
#include "PResourceManager.h"
#include "colours.h"

#define HIST_LABEL_Y				(-10 * GetScaling())
#define LABEL_YSEP                  (15 * GetScaling())
#define HIST_MARGIN_TOP				(14 * GetScaling())
#define HIST_MARGIN_RIGHT			(24 * GetScaling())

PNCDScopeImage::PNCDScopeImage(PImageWindow *owner, Widget canvas, int createCanvas)
	          : PHistImage(owner,canvas,createCanvas)
{
    memset(mIsCalibrated, 0, sizeof(mIsCalibrated));
}

void PNCDScopeImage::SetCalibrated(int scope, int cal)
{
    if (scope>=0 && scope<2) {
        mIsCalibrated[scope] = cal;
    }
}

void PNCDScopeImage::DoneGrab()
{
#ifdef SNOPLUS
    ((PCaenWindow *)GetOwner())->DoneGrab(this);
#else
    ((PNCDScopeWindow *)GetOwner())->DoneGrab(this);
#endif
}

void PNCDScopeImage::DrawSelf()
{
    PHistImage::DrawSelf();     // let the base class draw it
    
#ifndef SNOPLUS
    // draw our legend
	int y1 = HIST_MARGIN_TOP;
	int x2 = mWidth - HIST_MARGIN_RIGHT;
    int n = 0;
    for (int scope=0; scope<2; ++scope) {
        if (scope) {
            if (!GetOverlayPt()) continue;
        } else {
            if (!GetDataPt()) continue;
        }
        SetForeground(SCOPE0_COL + scope * (SCOPE1_COL - SCOPE0_COL));
		SetFont(PResourceManager::sResource.hist_font);
		char buff[64];
		if (IsCalibrated(scope)) {
		    sprintf(buff,"AntiLog %d", scope);
		} else {
		    sprintf(buff,"Scope %d", scope);
		}
		DrawString(x2,y1+HIST_LABEL_Y+(++n)*LABEL_YSEP,buff,kTextAlignTopRight);
		SetForeground(TEXT_COL);
	}
#endif
}
