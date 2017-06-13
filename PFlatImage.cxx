#include <Xm/RowColumn.h>
#include <math.h>
#include "PFlatImage.h"
#include "PImageWindow.h"
#include "PUtils.h"
#include "xsnoed.h"
#include "menu.h"
#include "colours.h"

#define FLAT_PMT_SIZE		0.0028
#define REMAP_TOLERANCE		0.25		/* 0.02 is too small ...           */
										/* tubes around +-y-axis will fail */
										/* first ...                       */

static MenuStruct flat_vw_menu[] = {
	{ "From Outside",	0, XK_O,IDM_FLAT_VW_OUT, NULL, 0, MENU_RADIO | MENU_TOGGLE_ON },
	{ "From Inside",	0, XK_I,IDM_FLAT_VW_IN,  NULL, 0, MENU_RADIO },
};
static MenuStruct flat_main_menu[] = {
	{ "View",			0, 0,	0, flat_vw_menu, XtNumber(flat_vw_menu), 0 },
};

// ------------------------------------------------------------

PFlatImage::PFlatImage(PImageWindow *owner, Widget canvas)
		  : PProjImage(owner,canvas)
{
	int			n;
	Arg			wargs[10];
	ImageData	*data = owner->GetData();

/*
** Set projection point for flat map
*/
	mImageSizeY = 0.472;	// image is shorter in Y direction
	mFlatView	= IDM_FLAT_VW_OUT;
	mProj.proj_type = IDM_PROJ_FLAT;
	mInvisibleHits	= HIT_FECD | HIT_BUTTS | HIT_NECK;
	
	if (!canvas) {
		Widget w = mOwner->GetMainPane();
		n = 0;
		XtSetArg(wargs[n],XmNmarginHeight,		1); ++n;
		XtSetArg(wargs[n],XmNleftAttachment,	XmATTACH_FORM); ++n;
		XtSetArg(wargs[n],XmNtopAttachment,		XmATTACH_FORM); ++n;
		Widget menu = XmCreateMenuBar( w, "xsnoedMenu" , wargs, n);
		XtManageChild(menu);
		owner->CreateMenu(menu,flat_main_menu, XtNumber(flat_main_menu),this);
		CreateCanvas("flatCanvas",kScrollLeftMask);
	}
	memset(&mFrame,0,sizeof(mFrame));	// must zero polyhedron before loading
	char *msg = loadGeometry(&mFrame,IDM_FLAT_GEO,data->argv,data->tube_radius);
	if (msg) quit(msg);
}

PFlatImage::~PFlatImage()
{
	freePoly(&mFrame);
}

void PFlatImage::Listen(int message, void *dataPt)
{
	switch (message) {
		case kMessageHitSizeChanged:
		case kMessageFitChanged:
			SetDirty();
			break;
		default:
			PProjImage::Listen(message,dataPt);
			break;
	}
}


void PFlatImage::DoMenuCommand(int anID)
{
	switch (anID) {
		case IDM_FLAT_VW_IN:
		case IDM_FLAT_VW_OUT:
		{
			if (PMenu::UpdateTogglePair(&mFlatView)) {
				if (anID == IDM_FLAT_VW_OUT) {
					matrixIdent(mProj.rot);
				} else {
					mProj.rot[0][0] = -1.0;
				}
				SetDirty();
			}
		} break;
	}
}

/*
** n0,n1 = points on line (if n0 is a null pointer, origin is on line)
** n2,n3,n4 = points on plane
**
** Returned vector:	out[0] = constant for intersection with (n3-n2)
**					out[1] = constant for (n4-n2)
**					out[2] = constant for (n1-n0) (actual intersection point)
**
** i.e. The intersection point between the line n0,n1 and the plane n2,n3,n4 is
** n0 + out[2] * (n1 - n0)  and  n2 + out[0] * (n3 - n2) + out[1] * (n4 - n3).
*/
void PFlatImage::GetIntersect(Node *n0,Node *n1,Node *n2,Node *n3,Node *n4,Vector3 out)
{
	Vector3		v0;
	Matrix3		m0;
	Matrix3		m1;
	
	m0[0][0] = n3->x3 - n2->x3;
	m0[1][0] = n3->y3 - n2->y3;
	m0[2][0] = n3->z3 - n2->z3;
	m0[0][1] = n4->x3 - n2->x3;
	m0[1][1] = n4->y3 - n2->y3;
	m0[2][1] = n4->z3 - n2->z3;
	
	if (n0) {
		m0[0][2] = n0->x3 - n1->x3;
		m0[1][2] = n0->y3 - n1->y3;
		m0[2][2] = n0->z3 - n1->z3;
		v0[0]	 = n0->x3 - n2->x3;
		v0[1]	 = n0->y3 - n2->y3;
		v0[2]	 = n0->z3 - n2->z3;
	} else {
		m0[0][2] = - n1->x3;
		m0[1][2] = - n1->y3;
		m0[2][2] = - n1->z3;
		v0[0]	 = - n2->x3;
		v0[1]	 = - n2->y3;
		v0[2]	 = - n2->z3;
	}
/*	matrixSolve(m0,v0,out); */
	matrixInvert(m0,m1);
	vectorMult(m1,v0,out);
}

/*
** quick test to determine if a node has a chance of lying within
** the specified face of the geodesic
*/
int PFlatImage::ChanceIntersect(Node *n1,Node *n2,Node *n3,Node *n4)
{
	float	t;
	float	tol = REMAP_TOLERANCE;
	
	if ((t=n1->x3) < n2->x3) {
		t += tol;
		if (t<n2->x3 && t<n3->x3 && t<n4->x3) return(0);
	} else {
		t -= tol;
		if (t>n2->x3 && t>n3->x3 && t>n4->x3) return(0);
	}
	if ((t=n1->y3) < n2->y3) {
		t += tol;
		if (t<n2->y3 && t<n3->y3 && t<n4->y3) return(0);
	} else {
		t -= tol;
		if (t>n2->y3 && t>n3->y3 && t>n4->y3) return(0);
	}
	if ((t=n1->z3) < n2->z3) {
		t += tol;
		if (t<n2->z3 && t<n3->z3 && t<n4->z3) return(0);
	} else {
		t -= tol;
		if (t>n2->z3 && t>n3->z3 && t>n4->z3) return(0);
	}
	return(1);
}

/*
** Remap the intersection of the line defined by (n0,n1) with first
** polyhedron onto the second polyhedron.  The face arrays of the two
** polyhedra must be equivalent.
*/
Face *PFlatImage::ReMap(Node *n0,Node *n1,Polyhedron *poly1,Polyhedron *poly2,Node *out)
{
	Vector3		v1;
	Node		*n2,*n3,*n4;		/* nodes on the face */
	Face		*face,*f0, *last;

	face = poly1->faces;
	last = face + poly1->num_faces;

	for (;face<last; ++face) {
		
		if (!ChanceIntersect(n1,face->nodes[0],face->nodes[1],face->nodes[2])) {
			continue;
		}
/*
** Get intersection point in plane of face. And determine if the
** line actually intersects the face on the proper (n1) side of n0.
*/
		GetIntersect(n0,n1,face->nodes[0],face->nodes[1],face->nodes[2],v1);
		
		if (v1[2]>=-.001 && v1[0]>=-.001 && v1[1]>=-.001 && v1[0]+v1[1]<=1.001) {
			f0 = poly2->faces + (face-poly1->faces);
			n2 = f0->nodes[0];
			n3 = f0->nodes[1];
			n4 = f0->nodes[2];
			out->x3 = n2->x3 + v1[0]*(n3->x3-n2->x3) + v1[1]*(n4->x3-n2->x3);
			out->y3 = n2->y3 + v1[0]*(n3->y3-n2->y3) + v1[1]*(n4->y3-n2->y3);
			out->z3 = n2->z3 + v1[0]*(n3->z3-n2->z3) + v1[1]*(n4->z3-n2->z3);
			return(f0);
		}
	}
/*	return((Face *)0); */
	return(NULL);
}

void PFlatImage::TransformHits()
{
	int			i,num;
	HitInfo		*hi;
	Node		*n0, nod;
	int			failed_remaps = 0;
	ImageData * data = mOwner->GetData();
	
#ifdef PRINT_DRAWS
	Printf(":transform flat\n");
#endif	
	if ((num=data->hits.num_nodes) != 0) {

		hi = data->hits.hit_info;
		n0 = data->hits.nodes;

		for (i=0; i<num; ++i,++n0,++hi) {
			if (hi->flags & mInvisibleHits) continue;
			if (data->remap_done[hi->index]) {
			  memcpy(&nod.x3, data->remap_data + hi->index, sizeof(Point3));
			} else {
			  if( !ReMap(0,n0,&data->geod,&mFrame,&nod) ) {
			    // print failed message unless this tube is missing
			    if (data->tube_coordinates[hi->index].tube != -1) {
			  	    ++failed_remaps;
				    Printf("Remap failed for channel %d/%d/%d\n", hi->crate, hi->card, hi->channel);
				    nod.x3 = nod.y3 = 0;
				    nod.z3 = 1;
				}
			  }
			  memcpy(data->remap_data + hi->index, &nod.x3, sizeof(Point3));
			  data->remap_done[hi->index] = 1;
			}
			transform(&nod,&mProj,1);
			/* save 2D node coordinates for hit on flat map */
			n0->x = nod.x;
			n0->y = nod.y;
		}
	}
	/* 2-D hit node coordinates are now calculated for the flat image */
	PProjImage::TransformHits();
	
	if (failed_remaps) Printf("Flat Map: Remap failed for %d tubes!\n",failed_remaps);
}


/*
** Routine to split a line from 'lnode' to 'node' as drawn on the flat map,
** beginning in 'lface' and ending in 'face'.  It is assumed that 'lface' and
** 'face' have at least one common point on the 3D detector.  If not, an error
** flag is returned.  If 'lface' and 'face' have two common points on the flat
** map, the subroutine returns without splitting the line.
**
** Inputs:		 lface,face	- old and new faces in flat map
**				 lnode	- current position of pen for drawing
**				 node	- node to draw to
**
** Returned:	 0 - faces are adjacent (line not split)
**				 1 - line split
**				-1 - line doesn't intersect given edge
**				 2 - error (should never happen unless data corrupted)
*/
int PFlatImage::SplitLine(XSegment **spp,Node *lnode,Node *node,Face *lface,Face *face)
{
	ImageData *data = mOwner->GetData();
	int		i,j,k,s,found,found0,c;
	int		n0i=0,n0j=0,n1i=0,n1j=0,nn[6];
	Node	*n0=0,*n1=0,*n,*t,nod;
	Face	*f0,*f1,*f2,*f3,*fa[6],*fb[6];
	Vector3	vec;	
	
/*
** If faces are adjacent on flat map, return without doing anything
*/
	found = 0;
	for (i=0; i<3; ++i) {
		t = lface->nodes[i];
		for (j=0; j<3; ++j) {
			if (t == face->nodes[j]) {
				if (found++) return(0);
				break;
			}
		}
	}
/*
** Find nodes on detector (n0,n1) which define the crossed edge
*/
	f0 = data->geod.faces + (lface - mFrame.faces);
	f1 = data->geod.faces + ( face - mFrame.faces);
	
	found = 0;
	for (i=0; i<3; ++i) {
		t = f0->nodes[i];
		for (j=0; j<3; ++j) {
			if (t == f1->nodes[j]) {
				if (found++) {
					n1 = t;
					n1i = i;
					n1j = j;
				} else {
					n0 = t;
					n0i = i;
					n0j = j;
				}
				break;
			}
		}
	}
/*
** If two common nodes not found, we must call SplitLine recursively
** for each of the edges which are crossed.  The tricky part will
** be determining which edges are crossed.
*/
	if (found != 2) {
		if (!found) return(2);
		c = 0;
		found = found0 = -1;
		f3 = data->geod.faces + data->geod.num_faces;
		for (f2=data->geod.faces; f2<f3; ++f2) {
			for (i=0; i<3; ++i) {
				if (f2->nodes[i] == n0) {
					if (f2 == f1) found  = c;
					if (f2 == f0) found0 = c;
					fa[c] = f2;
					nn[c] = i;
					++c;
				}
			} 
		}
		if (found>=0 && found0>=0) {
			j = found0;
			found0 = -1;
			fb[0] = mFrame.faces + (fa[j]-data->geod.faces);
			for (i=1; i<c; ++i) {
				k = nn[j] + 1;
				if (k>=3) k=0;			/* j points to next ccw node from common node */
				n = fa[j]->nodes[k];	/* n is node to look for */
				for (j=0; j<c; ++j) {
					k = nn[j] - 1;
					if (k<0) k=2;
					if (n == fa[j]->nodes[k]) {
						fb[i] = mFrame.faces + (fa[j]-data->geod.faces);
						if (j == found) found0 = i;
						break;
					}
				}
			}
			if (found0 < 0) return(2);
/*
** Now we have the ordered faces around the common vertex, pass them
** back to SplitLine in order (counterclockwise) unless we get an indication
** that the edge doesn't intersect with the line (a negative return value).
** In this case, we start again, passing the faces in clockwise order instead.
*/
			if (found0 <= 3) {
				for (i=0,s=0; i<found0; ++i) {
					k = SplitLine(spp,lnode,node,fb[i],fb[i+1]);
					if (k < 0) break;
					s += k;
				}
				if (i==found0 && s) return(0);
			}
			if (found0 >= c-3) {
				j = 0;
				for (i=c-1; i>=found0; --i) {
					if (SplitLine(spp,lnode,node,fb[j],fb[i]) < 0) break;
					j = i;
				}
			}
		}
		return(0);
	}
/*
** Find the intersection point and separate the drawn line
*/
	nod.x3 = nod.y3 = nod.z3 = 0;
	GetIntersect(n0,n1,lnode,node,&nod,vec);
/*
** Return -1 if no intersection with edge (vec[2]), or if intersection
** point is not between 'lnode' and 'node' (vec[0]).  Note that vec[0]
** must be less than 1.05 here -- this is to compensate for round-off
** errors.  In theory, 1.000001 would be sufficient, but the errors have
** been as large is 1.015 in practice (I don't know why).
*/
	if (vec[2]<-.001 || vec[2]>1.001 || vec[0]<-.001 || vec[0]>1.05) return(-1);
	n0 = lface->nodes[n0i];
	n1 = lface->nodes[n1i];
	nod.x3 = n0->x3 + vec[2] * (n1->x3 - n0->x3);
	nod.y3 = n0->y3 + vec[2] * (n1->y3 - n0->y3);
	nod.z3 = n0->z3 + vec[2] * (n1->z3 - n0->z3);
	transform(&nod,&mProj,1);
	(*spp)->x2 = nod.x;
	(*spp)->y2 = nod.y;
	++(*spp);
	n0 = face->nodes[n0j];
	n1 = face->nodes[n1j];
	nod.x3 = n0->x3 + vec[2] * (n1->x3 - n0->x3);
	nod.y3 = n0->y3 + vec[2] * (n1->y3 - n0->y3);
	nod.z3 = n0->z3 + vec[2] * (n1->z3 - n0->z3);
	transform(&nod,&mProj,1);
	(*spp)->x1 = nod.x;
	(*spp)->y1 = nod.y;
	return(1);
}

/*
** Draw flat image
*/
void PFlatImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	XSegment	segments[MAX_EDGES], *sp;
	int			i,j,num;
	Node		*n0,*n1,*n2,nod,tmp;
	Edge		*edge, *last;
	HitInfo		*hi;
	Face		*face,*lface;
	float		t;
	int			d1, d2;
#ifdef PRINT_DRAWS
	Printf("drawFlatImage\n");
#endif	

	PImageCanvas::DrawSelf();	// let the base class clear the drawing area

	// don't draw missing or weird hits
	long bit_mask = HiddenHitMask();

	transform(mFrame.nodes,&mProj,mFrame.num_nodes);
/*
** Draw flat frame
*/
	edge = mFrame.edges;
	last = edge + mFrame.num_edges;
	for (sp=segments; edge<last; ++edge) {
		n1 = edge->n1;
		n2 = edge->n2;
		if (n1->flags & n2->flags & (NODE_HID | NODE_OUT)) continue;
		sp->x1 = n1->x;
		sp->y1 = n1->y;
		sp->x2 = n2->x;
		sp->y2 = n2->y;
		++sp;
	}
	SetForeground(GRID_COL);
	SetLineWidth(THIN_LINE_WIDTH);
	DrawSegments(segments,sp-segments);
	SetLineWidth(1);
/*
** Draw axes ...
*/
	tmp.z3 = 0.0;
	tmp.y3 = 0.0;
	tmp.x3 = 1.0;
	SetForeground(AXES_COL);
	for(i=0; i<4; ++i ) {
		ReMap(0,&tmp,&data->geod,&mFrame,&nod);
		transform(&nod,&mProj,1);
		sp = segments;
		int scale = GetScaling();
		switch (i) {
			case 2:
				sp->x1 = nod.x - 6*scale;  sp->y1 = nod.y;
				sp->x2 = nod.x - 3*scale;  sp->y2 = nod.y;
				++sp;
			case 0:
				sp->x1 = nod.x - 3*scale;  sp->y1 = nod.y - 3*scale;
				sp->x2 = nod.x + 3*scale;  sp->y2 = nod.y + 3*scale;
				++sp;
				sp->x1 = nod.x + 3*scale;  sp->y1 = nod.y - 3*scale;
				sp->x2 = nod.x - 3*scale;  sp->y2 = nod.y + 3*scale;
				++sp;
				break;
			case 3:
				sp->x1 = nod.x - 6*scale;  sp->y1 = nod.y;
				sp->x2 = nod.x - 3*scale;  sp->y2 = nod.y;
				++sp;
			case 1:
				sp->x1 = nod.x - 3*scale;  sp->y1 = nod.y - 3*scale;
				sp->x2 = nod.x; 		   sp->y2 = nod.y;
				++sp;
				sp->x1 = nod.x + 3*scale;  sp->y1 = nod.y - 3*scale;
				sp->x2 = nod.x; 	 	   sp->y2 = nod.y;
				++sp;
				sp->x1 = nod.x;  		   sp->y1 = nod.y;
				sp->x2 = nod.x;  		   sp->y2 = nod.y + 3*scale;
				++sp;
				break;
		}
		DrawSegments(segments,sp-segments);
/*		XFillRectangle(dpy, pix, gc, nod.x-1,nod.y-1,3,3);*/
		t = tmp.x3;
		tmp.x3 = - tmp.y3;
		tmp.y3 = t;
	}
/*
** Draw reconstructed points and cones
*/
	for (i=0; i<data->nrcon; ++i) {

		num = data->rcon[i].num_nodes - 2;
		if (num <= 0) continue;
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
		n0  = data->rcon[i].nodes;
		n2  = n0 + 2;
		n1  = n2 + num - 1;
		lface = ReMap(0,n1,&data->geod,&mFrame,&nod);
		transform(&nod,&mProj,1);
		sp = segments;
		sp->x1 = nod.x;
		sp->y1 = nod.y;
		
		for (j=0; j<num; ++j) {
			face = ReMap(0,n2,&data->geod,&mFrame,&nod);
			if (face) {
				transform(&nod,&mProj,1);
				if (lface) {
					if (face != lface) SplitLine(&sp,n1,n2,lface,face);
					sp->x2 = nod.x;
					sp->y2 = nod.y;
					++sp;
					sp->x1 = nod.x;
					sp->y1 = nod.y;
				} else {
					sp->x1 = nod.x;
					sp->y1 = nod.y;
				}
			}
			lface = face;
			n1 = n2++;
		}
		DrawSegments(segments,sp-segments);
	}
/*
** draw hits
*/
	d1 = (int)(mProj.xscl * FLAT_PMT_SIZE * data->hit_size);
	if (d1 < 1) d1 = 1;
	d2 = d1 * 2 + 1;
	
	if ((num=data->hits.num_nodes) != 0) {

		TransformHits();
		
		hi = data->hits.hit_info;
		n0 = data->hits.nodes;

		for (i=0; i<num; ++i,++n0,++hi) {
			if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
			SetForeground(NUM_COLOURS + hi->hit_val);
			FillRectangle(n0->x-d1,n0->y-d1,d2,d2);
		}
	}
}
