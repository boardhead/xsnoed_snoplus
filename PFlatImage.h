#ifndef __PFlatImage_h__
#define __PFlatImage_h__

#include <Xm/Xm.h>
#include "ImageData.h"
#include "PProjImage.h"
#include "PMenu.h"
#include "matrix.h"

struct MenuStruct;

class PFlatImage : public PProjImage, public PMenuHandler {
public:
	PFlatImage(PImageWindow *owner, Widget canvas=0);
	virtual ~PFlatImage();
	
	virtual void	DrawSelf();
	virtual void	TransformHits();
	virtual void	DoMenuCommand(int anID);
	virtual void	Listen(int message, void *dataPt);

private:
	int				SplitLine(XSegment **spp,Node *lnode,Node *node,Face *lface,Face *face);
	static void		GetIntersect(Node *n0,Node *n1,Node *n2,Node *n3,Node *n4,Vector3 out);
	static int		ChanceIntersect(Node *n1,Node *n2,Node *n3,Node *n4);
	static Face	  *	ReMap(Node *n0,Node *n1,Polyhedron *poly1,Polyhedron *poly2,Node *out);

	int				mFlatView;	// view from inside or outside the vessel
	Polyhedron		mFrame;		// flat detector map
};

#endif // __PFlatImage_h__
