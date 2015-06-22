#ifndef __PNCDImage_h__
#define __PNCDImage_h__

#include <Xm/Xm.h>
#include "ImageData.h"
#include "PProjImage.h"
#include "PMenu.h"
#include "matrix.h"

struct MenuStruct;

class PNCDImage : public PProjImage, public PMenuHandler {
public:
	PNCDImage(PImageWindow *owner, Widget canvas=0);
	virtual ~PNCDImage();
	
	virtual void	DrawSelf();
	virtual void	Listen(int message, void *dataPt);
	
	virtual void	TransformHits();

	// no PMT's in this image, so no nearest hit
	virtual int     FindNearestHit()    { return 0; }
	virtual int     FindNearestNcd();

private:
};

#endif // __PNCDImage_h__
