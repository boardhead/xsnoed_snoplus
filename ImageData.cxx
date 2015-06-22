#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ImageData.h"
#include "matrix.h"
#include "menu.h"
#include "openfile.h"
#include "PProjImage.h"

#define	BUFFLEN				512
#define	PLOT_MAX			8000.0			/* maximum value for x,y plot coords */

char *sFilePath = NULL;

/*------------------------------------------------------------------------------------
*/
void initNodes(WireFrame *fm, Point3 *pt, int num)
{
	int		i;
	Node	*node;

	fm->nodes = (Node *)malloc(num * sizeof(Node));

	if (fm->nodes) {

		for (i=0,node=fm->nodes; i<num; ++i,++node) {
			node->x3 = pt[i].x;
			node->y3 = pt[i].y;
			node->z3 = pt[i].z;
		}
		fm->num_nodes = num;
	}
}
void initEdges(WireFrame *fm, int *n1, int *n2, int num)
{
	int			i;
	Edge		*edge;

	fm->edges = (Edge *)malloc(num * sizeof(Edge));

	if (fm->edges) {

		for (i=0,edge=fm->edges; i<num; ++i,++edge) {
			edge->n1  = fm->nodes + n1[i];
			edge->n2  = fm->nodes + n2[i];
			edge->flags = 0;
		}
		fm->num_edges = num;
	}
}
void freePoly(Polyhedron *poly)
{
	if (poly->num_nodes) {
		poly->num_nodes = 0;
		free(poly->nodes);
	}
	if (poly->num_edges) {
		poly->num_edges = 0;
		free(poly->edges);
	}
	if (poly->num_faces) {
		poly->num_faces = 0;
		free(poly->faces);
	}
}
void freeWireFrame(WireFrame *frame)
{
	if (frame->num_nodes) {
		frame->num_nodes = 0;
		free(frame->nodes);
	}
	if (frame->num_edges) {
		frame->num_edges = 0;
		free(frame->edges);
	}
}
char *loadGeometry(Polyhedron *poly, int geo, char *argv, float tube_rad)
{
	Edge		*edge;
	Node		*node, *t1, *t2;
	Face		*face;
	int			nn,ne,nf;
	int			i,j,k,ct,found,n,n1,n2;
	char		*pt,*geo_name,buff[BUFFLEN];
	float		x,y,z,x2,y2,z2,len;
	float		r;
	FILE		*fp;
	char		*msg;
	static char	rtn_msg[BUFFLEN];

	msg = 0;
	switch (geo) {
		case IDM_GEODESIC:
			geo_name = "geodesic.geo";
			break;
		case IDM_POLAR:
			geo_name = "polar.geo";
			break;
		case IDM_VESSEL:
			geo_name = "vessel.geo";
			break;
		case IDM_FLAT_GEO:
			geo_name = "flat.geo";
			break;
		case IDM_NO_FRAME:
			freePoly(poly);
			return(msg);
		default:
			return("Unknown geometry");
	}
	fp = openFile(geo_name,"r",sFilePath);
	if (!fp) {
		sprintf(rtn_msg,"Can't open Geometry file %s",geo_name);
		return(rtn_msg);
	}
	
	sprintf(rtn_msg, "Bad format .geo file: %s",geo_name);

	for (i=0; i<4; ++i) {

		fgets(buff,BUFFLEN,fp);
		pt = strchr(buff,'=');
		if (!pt) {
			fclose(fp);
			return(rtn_msg);
		}
		++pt;
		switch (buff[0]) {
			case 'R':
				sscanf(pt,"%f",&r);
				break;
			case 'N':
				sscanf(pt,"%d",&nn);
				break;
			case 'F':
				sscanf(pt,"%d",&nf);
				break;
			case 'E':
				sscanf(pt,"%d",&ne);
				break;
			default:
				fclose(fp);
				return(rtn_msg);
		}
	}
	if (geo == IDM_FLAT_GEO) {
		r = 1.0;	// do not scale flat geometry - PH 02/03/99
	} else {
		r /= tube_rad;
	}
	poly->radius = r;
/*
** Reallocate memory for new arrays
*/
	freePoly(poly);

	poly->nodes = (Node *)malloc(nn * sizeof(Node));
	poly->edges = (Edge *)malloc(ne * sizeof(Edge));
	poly->faces = (Face *)malloc(nf * sizeof(Face));

	if (!poly->nodes || !poly->edges || !poly->faces) {
		fclose(fp);
		return(rtn_msg);
	}

	poly->num_nodes = nn;
	poly->num_edges = ne;
	poly->num_faces = nf;
/*
** Get nodes, edges and faces arrays from file
*/
	for (i=0,node=poly->nodes; i<nn; ++i,++node) {

		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'N') {
			fclose(fp);
			return(rtn_msg);
		}
		j=sscanf(buff+2,"%f, %f, %f",&x,&y,&z);

		if (j!=3) {
			fclose(fp);
			return(rtn_msg);
		}

		node->x3 = x * r;
		node->y3 = y * r;
		node->z3 = z * r;
	}
	for (i=0,face=poly->faces; i<nf; ++i,++face) {
		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'F') {
			fclose(fp);
			return(rtn_msg);
		}
		pt = buff + 2;
		n = atoi(pt);
		if (n<3 || n>MAX_FNODES) {
			fclose(fp);
			return(rtn_msg);
		}
		face->num_nodes = n;
		for (j=0; j<n; ++j) {
			pt = strchr(pt,',');
			if (!pt) {
				fclose(fp);
				return(rtn_msg);
			}
			face->nodes[j] = poly->nodes + (atoi(++pt)-1);
		}
		if (face-poly->faces > nf) {
			fclose(fp);
			return(rtn_msg);
		}
/*
** calculate vector normal to this face
*/
		t1 = face->nodes[2];
		t2 = face->nodes[1];
		x  = t1->x3 - t2->x3;
		y  = t1->y3 - t2->y3;
		z  = t1->z3 - t2->z3;
		t1 = face->nodes[0];
		x2 = t1->x3 - t2->x3;
		y2 = t1->y3 - t2->y3;
		z2 = t1->z3 - t2->z3;
		face->norm.x = y*z2 - z*y2;
		face->norm.y = z*x2 - x*z2;
		face->norm.z = x*y2 - y*x2;
		len = vectorLen(face->norm.x, face->norm.y, face->norm.z);
		face->norm.x /= len;
		face->norm.y /= len;
		face->norm.z /= len;
	}
	for (i=0,edge=poly->edges; i<ne; ++i,++edge) {
	
		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'E') {
			fclose(fp);
			return(rtn_msg);
		}
		j=sscanf(buff+2,"%d, %d",&n1,&n2);
		if (j!=2) {
			fclose(fp);
			return(rtn_msg);
		}
		edge->n1 = poly->nodes + n1 - 1;
		edge->n2 = poly->nodes + n2 - 1;
		edge->flags = 0;
/*
** Figure out which faces belong to this edge
*/
		found = 0;
		for (j=0, face=poly->faces; j<nf; ++j,++face) {
			n = face->num_nodes;
			ct = 0;
			for (k=0; k<n; ++k) {
				if (face->nodes[k]==edge->n1 ||
					face->nodes[k]==edge->n2) ++ct;
			}
			if (ct==2) {
				if (found++) {
					edge->f2 = face;
					break;
				} else {
					edge->f1 = face;
				}
			}
		}
		if (found != 2 && (found!=1 || geo!=IDM_FLAT_GEO)) {
			sprintf(rtn_msg,"Error matching edges and faces in %s",geo_name);
			msg = rtn_msg;
		}
	}
	fclose(fp);
	return(msg);
}

/*
 * tranform the coordinates of a node by the specified projection
 * Inputs: node->x3,y3,z3
 * Outputs: node->x,y,xr,yr,zr,flags
 * resets NODE_HID and sets NODE_OUT appropriately
 */
void transform(Node *node, Projection *pp, int num)
{
	int		i;
	float	f;
	float	x,y,z;
	float	xt,yt,zt;
	float	axt,ayt;
	float	xsc   = pp->xscl;
	float	ysc   = pp->yscl;
	int		xcn   = pp->xcen;
	int		ycn   = pp->ycen;
	float	(*rot)[3] = pp->rot;
	float  *vec  = pp->pt;
	int		pers  = vec[2] < pp->proj_max;

	for (i=0; i<num; ++i,++node) {
		x = node->x3;
		y = node->y3;
		z = node->z3;
		xt = (node->xr = x*rot[0][0] + y*rot[0][1] + z*rot[0][2]) - vec[0];
		yt = (node->yr = x*rot[1][0] + y*rot[1][1] + z*rot[1][2]) - vec[1];
		zt = (node->zr = x*rot[2][0] + y*rot[2][1] + z*rot[2][2]) - vec[2];

		// reset NODE_OUT and NODE_HID flags
		node->flags &= ~(NODE_OUT | NODE_HID);
		
		if (pers) {
			if (zt >= 0) {
				node->flags |= NODE_OUT;
				axt = fabs(xt);
				ayt = fabs(yt);
				if (axt > ayt) f = PLOT_MAX/axt;
				else  if (ayt) f = PLOT_MAX/ayt;
				else {
					f = PLOT_MAX;
					xt = yt = 1;
				}
				node->x =   (int)(f * xt);
				node->y = - (int)(f * yt);
			} else {
/*
** Distort image according to projection point while maintaining
** a constant magnification for the projection screen.
*/
				x = xcn + xt * (pp->proj_screen-vec[2]) * xsc / zt;
				y = ycn - yt * (pp->proj_screen-vec[2]) * ysc / zt;
				axt = fabs(x);
				ayt = fabs(y);
				if (axt>PLOT_MAX || ayt>PLOT_MAX) {
					node->flags |= NODE_OUT;
					if (axt > ayt) f = PLOT_MAX/axt;
					else  if (ayt) f = PLOT_MAX/ayt;
					else {
						f = PLOT_MAX;
						x = y = 1;
					}
					node->x = (int)(f * x);
					node->y = (int)(f * y);
				} else {
					node->x = (int)(x);
					node->y = (int)(y);
				}
			}
		} else {
			node->x = (int)(xcn + xsc * xt);
			node->y = (int)(ycn - ysc * yt);
		}
	}
}

void transformPoly(Polyhedron *poly, Projection *pp)
{
	int		i;
	Vector3	rp, t;
	float	dot;
	Node	*node;
	Face	*face = poly->faces;
	int		num   = poly->num_faces;
	float	*pt   = pp->pt;

	transform(poly->nodes,pp,poly->num_nodes);
/*
** Calculate dot product of normals to face with vector to proj point.
** If product is negative, face is hidden.
** First, rotate projection point into detector frame.
*/
	if (pt[2] < pp->proj_max) {

		vectorMult(pp->inv, pt, rp);

		node = poly->nodes;

		for (i=0; i<num; ++i,++face) {
			node = face->nodes[0];			/* get first node of face */
			dot = (rp[0] - node->x3) * face->norm.x +
				  (rp[1] - node->y3) * face->norm.y +
				  (rp[2] - node->z3) * face->norm.z;
			if (dot < 0) face->flags |= FACE_HID;
			else		 face->flags &= ~FACE_HID;
		}

	} else {

		t[0] = 0;							/* no  perspective */
		t[1] = 0;
		t[2] = 1;
		vectorMult(pp->inv, t, rp);

		for (i=0; i<num; ++i,++face) {
			dot = rp[0] * face->norm.x +
				  rp[1] * face->norm.y +
				  rp[2] * face->norm.z;

			if (dot < 0) face->flags |= FACE_HID;
			else		 face->flags &= ~FACE_HID;
		}
	}
}

/* returns non-zero if current fit is real (and not water) */
int isRealFit(ImageData *data)
{
	return(data && data->nrcon && data->curcon!=data->watercon[0] && data->curcon!=data->watercon[1]);
}

int isIntegerDataType(ImageData *data)
{
	switch (data->wDataType) {
		case IDM_TAC:
		case IDM_QHS:
		case IDM_QHL:
		case IDM_QLX:
		case IDM_QHL_QHS:
			if (data->wCalibrated == IDM_UNCALIBRATED) {
				return(1);
			}
			break;
		case IDM_NHIT:
#ifdef OPTICAL_CAL
			if (data->wCalibrated != IDM_UNCALIBRATED) break;
#endif
			// fall through!
		case IDM_CMOS_RATES:	/* cmos rates use NHIT entry of hit info */
		case IDM_DISP_CRATE:
		case IDM_DISP_CARD:
		case IDM_DISP_CHANNEL:
		case IDM_DISP_CELL:
			return(1);
	}
	return(0);
}

// getCurrentEventIndex - get index of event in history 'all' buffer
//
// - negative indicates future event
// - 9999 indicates event not found
int getCurrentEventIndex(ImageData *data)
{
	int	cur_event_num;
	HistoryEntry *entry;
	
	if (data->was_history && data->history_all) {
		cur_event_num = data->history_evt;
	} else {
		/* find currently viewed event in 'all' history */
		cur_event_num = 9999;	// assume that event isn't in history unless we find it below
		if (data->was_history) {
			entry = data->history_buff[HISTORY_VIEWED][data->history_evt];
		} else {
			/* look for the currently viewed event in the 'all' history */
			entry = data->history_buff[HISTORY_VIEWED][0];
		}
		if (entry) {
			int num = data->history_size[HISTORY_ALL];
			for (int i=0; i<num; ++i) {
				if (entry == data->history_buff[HISTORY_ALL][i]) {
					cur_event_num = i;	// found it!
					break;
				}
			}
		}
	}
	return(cur_event_num);
}

// getCurrentHistoryEntry - return HistoryEntry pointer to currently viewed event
// - returns NULL if not looking at an event
HistoryEntry *getCurrentHistoryEntry(ImageData *data)
{
	int		buffNum, entryNum;
	
	if (data->was_history) {
		if (data->history_evt >= 0) {
			// event is from 'all' or 'viewed' history buffer
			buffNum = data->history_all;
			entryNum = data->history_evt;
		} else {
			// event is from 'future' history buffer (indicated by negative history number)
			buffNum = HISTORY_FUTURE;
			entryNum = -1 - data->history_evt;
		}
	} else {
		// not looking at history -- return last viewed event
		buffNum = HISTORY_VIEWED;
		entryNum = 0;
	}
	// range check the entry number to be safe
	if (data->history_size[buffNum] > entryNum) {
		return(data->history_buff[buffNum][entryNum]);
	} else {
		return(NULL);	// entry does not exist
	}
}

PmtEventRecord *getHistoryEvent(ImageData *data, int index)
{
	HistoryEntry *entry;
	
	if (index < 0) {	// future event
		index = -1 - index;
		if (index < data->history_size[HISTORY_FUTURE]) {
			entry = data->history_buff[HISTORY_FUTURE][index];
			if (entry) return((PmtEventRecord *)(entry + 1));
		}
	} else {
		if (index < data->history_size[HISTORY_ALL]) {
			entry = data->history_buff[HISTORY_ALL][index];
			if (entry) return((PmtEventRecord *)(entry + 1));
		}
	}
	return((PmtEventRecord *)NULL);
}

// getPmtCounts - get counts for number of unique Pmt's in event
// - if 'data' is NULL, returns total counts (must call with non-zero data
//   before it is valid to do this) - PH 05/23/00
int *getPmtCounts(ImageData *data)
{
	int		i, j;
	const int kFirstBit = NHIT_PMT_NORMAL;
	const int kLastBit = NHIT_PMT_BUTTS;
	static int pmt_counts[32];
	static int pmt_extras[32];	// extra hits on this pmt
	
	if (!data) return(pmt_extras);
	
	int		n = data->hits.num_nodes;
	HitInfo	*hi = data->hits.hit_info;
	
	memset(pmt_counts, 0, sizeof(pmt_counts));
	memset(pmt_extras, 0, sizeof(pmt_extras));
	
	for (i=0; i<n; ++i,++hi) {
		// count number of each tube type
		for (j=kFirstBit; j<=kLastBit; ++j) {
			if (hi->flags & (1 << j)) {
				++pmt_counts[j];
				pmt_extras[j] += (int)(hi->nhit - 1);
			}
		}
	}
#ifndef SNOPLUS
	// count NCD hits too
	if (data->numNcdHit) {
        for (i=0; i<=data->numNcds; ++i) {
            if (data->ncdHit[i].mux_count) {
                if (i==data->numNcds) {
                    pmt_extras[NHIT_NCD_MUX] += data->ncdHit[i].mux_count;
                } else {
                    ++pmt_counts[NHIT_NCD_MUX];
                    pmt_extras[NHIT_NCD_MUX] += data->ncdHit[i].mux_count - 1;
                }
            }
            if (data->ncdHit[i].shaper_count) {
                if (i==data->numNcds) {
                    pmt_extras[NHIT_NCD_SHAPER] += data->ncdHit[i].shaper_count;
                } else {
                    ++pmt_counts[NHIT_NCD_SHAPER];
                    pmt_extras[NHIT_NCD_SHAPER] += data->ncdHit[i].shaper_count - 1;
                }
            }
            if (data->ncdHit[i].scope_count) {
                if (i==data->numNcds) {
                    pmt_extras[NHIT_NCD_SCOPE] += data->ncdHit[i].scope_count;
                } else {
                    ++pmt_counts[NHIT_NCD_SCOPE];
                    pmt_extras[NHIT_NCD_SCOPE] += data->ncdHit[i].scope_count - 1;
                }
            }
        }
    }
    pmt_counts[NHIT_NCD_GENERAL] = data->general_count;
#endif
	return(pmt_counts);
}

static int is_sudbury_dst(struct tm *tms)
{
	int		mday;
	int 	is_dst = 0;		// initialize is_dst flag

    // check to see if we should have been in daylight savings time
    if (tms->tm_year < 107) {    // before 2007
        if (tms->tm_mon>3 && tms->tm_mon<9) {   // after April and before October
            is_dst = 1;             // we are in daylight savings time
        } else if (tms->tm_mon == 3) {          // the month is April
            // calculate the date of the first Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 1;
            if (tms->tm_mday > mday ||          // after the first Sunday or
                (tms->tm_mday == mday &&        // (on the first Sunday and
                tms->tm_hour >= 2))             //  after 2am)
            {
                is_dst = 1;         // we are in daylight savings time
            }
        } else if (tms->tm_mon == 9) {          // the month is October
            // calculate the date of the last Sunday in the month
            // (remember, October has 31 days)
            mday = ((tms->tm_mday - tms->tm_wday + 10) % 7) + 25;
            if (tms->tm_mday < mday ||          // before the last Sunday or
                (tms->tm_mday == mday &&        // (on the last Sunday and
                tms->tm_hour < 1))              //  before 1am (2am DST))
            {
                is_dst = 1;         // we are in daylight savings time
            }
        }
    } else {                    // 2007 and later
        if (tms->tm_mon>2 && tms->tm_mon<10) {  // after March and before November
            is_dst = 1;             // we are in daylight savings time
        } else if (tms->tm_mon == 2) {          // the month is March
            // calculate the date of the 2nd Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 8;
            if (tms->tm_mday > mday ||          // after the 2nd Sunday or
                (tms->tm_mday == mday &&        // (on the 2nd Sunday and
                tms->tm_hour >= 2))             //  after 2am)
            {
                is_dst = 1;         // we are in daylight savings time
            }
        } else if (tms->tm_mon == 10) {         // the month is November
            // calculate the date of the first Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 1;
            if (tms->tm_mday < mday ||          // before the first Sunday or
                (tms->tm_mday == mday &&        // (on the first Sunday and
                tms->tm_hour < 1))              //  before 1am (2am DST))
            {
                is_dst = 1;         // we are in daylight savings time
            }
        }
    }
	return(is_dst);
}

// return a time structure in the specified time zone (0=sudbury, 1=local, 2=UTC)
// - input time in UTC seconds since unix time zero
struct tm *getTms(double aTime, int time_zone)
{
	int			is_dst;
	struct tm	*tms;
	time_t 		the_time = (time_t)aTime;
	
	switch (time_zone) {
		default: //case kTimeZoneSudbury:
			// adjust to EST (+5:00) -- initially without daylight savings time
			the_time -= 5 * 3600L;
			tms = gmtime(&the_time);
			is_dst = is_sudbury_dst(tms);
			if (is_dst) {
				the_time += 3600L;		// spring forward into DST
				tms = gmtime(&the_time);
				tms->tm_isdst = 1;		// set isdst flag
			}
			break;
		case kTimeZoneLocal:
			tms = localtime(&the_time);
			break;
		case kTimeZoneUTC:
			tms = gmtime(&the_time);
			break;
	}
	return(tms);
}

// return time_t from given tms and time zone
time_t getTime(struct tm *tms, int time_zone)
{
	time_t theTime;
	
	tms->tm_isdst = 0;	// reset DST flag initially
	theTime = mktime(tms);
	
	switch (time_zone) {
		default: //case kTimeZoneSudbury:
#ifdef __MACHTEN__
			theTime -= 5 * 3600L;	// convert to GMT
#else
			theTime	-= timezone;	// convert to GMT
#endif
			// adjust to EST (intially with no DST)
			theTime += 5 * 3600L;
			// SHOULD FIX THIS FOR OTHER TIME ZONES!
			if (is_sudbury_dst(tms)) {
				theTime -= 3600L;	// sprint forward into DST
			}
			break;
		case kTimeZoneLocal:
			// nothing to do, mktime assumes local time
			break;
		case kTimeZoneUTC:
#ifdef __MACHTEN__
			theTime -= 5 * 3600L;	// convert to GMT
#else
			theTime	-= timezone;	// convert to GMT
#endif
			break;
	}
	return(theTime);
}


