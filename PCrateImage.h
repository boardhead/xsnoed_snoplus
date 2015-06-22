#ifndef __PCrateImage_h__
#define __PCrateImage_h__

#include "PProjImage.h"


class PCrateImage : public PProjImage {
public:
	PCrateImage(PImageWindow *owner, Widget canvas=0);
	
	virtual void	DrawSelf();
	
	virtual void	TransformHits();
	
private:
	unsigned long	mCrateMask;
	int				mMargin_x;
	int				mMargin_y;
	int				mCrate_offset_x;
	int				mCrate_offset_y;
	int				mChan_w;
	int				mChan_h;
};

#endif // __PCrateImage_h__
