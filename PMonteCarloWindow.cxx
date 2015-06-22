#include <math.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include "PMonteCarloWindow.h"
#include "PImageWindow.h"
#include "PProjImage.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "SnoStr.h"
#include "xsnoed.h"
#include "menu.h"

enum QlxFlag {
	LABEL_QLX,
	LABEL_QLS,
	LABEL_QLL
};

// menu command ID's
enum {
	IDM_HIGHLIGHT = 40,	// leave room for IM_ codes
	IDM_PHOTON,
	IDM_GAMMA,
	IDM_ELECTRON,
	IDM_NEUTRINO,
	IDM_NEUTRON,
	IDM_UNKNOWN,
	IDM_SHOW_ALL,
	IDM_SHOW_NONE
};

static MenuStruct mc_display_menu[] = {
	{ "Hide Interaction Labels",0,XK_H,IDM_HIDE_NAMES,		NULL, 0, MENU_RADIO },
	{ "Show Interaction Labels",0,XK_S,IDM_INTERACTION_NAMES,NULL,0, MENU_RADIO },
	{ "Label Highlighted Track",0,XK_L,IDM_HIGHLIGHTED_NAMES,NULL,0, MENU_RADIO },
	{ NULL, 				0, 0,	0,						NULL, 0, 0 },
	{ "Highlight Current Track",0,XK_C,IDM_HIGHLIGHT,		NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
};
static MenuStruct mc_track_menu[] = {
	{ "All Tracks",			0, XK_A,IDM_MC_ALL_TRACKS,		NULL, 0, MENU_RADIO },
	{ "Track to One PMT",	0, XK_O,IDM_MC_SINGLE_TRACK,	NULL, 0, MENU_RADIO },
	{ "Selected Tracks:",	0, XK_S,IDM_MC_SELECTED,		NULL, 0, MENU_RADIO },
	{ NULL, 				0, 0,	0,						NULL, 0, 0 },
	{ "Spectral Reflection",0, XK_p,IM_SPECTRAL_REFL,		NULL, 0, MENU_TOGGLE },
	{ "Diffuse Reflection",	0, XK_R,IM_DIFFUSE_REFL,		NULL, 0, MENU_TOGGLE },
	{ "Diffuse Transmission",0,XK_T,IM_DIFFUSE_TRANS,		NULL, 0, MENU_TOGGLE },
	{ "Rayleigh Scattering",0, XK_y,IM_RAYLEIGH_SCAT,		NULL, 0, MENU_TOGGLE },
	{ "PMT Bounce",			0, XK_B,IM_PMT_BOUNCE,			NULL, 0, MENU_TOGGLE },
	{ "Reached PMT",		0, XK_e,IM_REACHED_PMT,			NULL, 0, MENU_TOGGLE },
};
static MenuStruct mc_particle_menu[] = {
	{ "Photon",				0, XK_P,IDM_PHOTON,  			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Gamma",				0, XK_G,IDM_GAMMA,  			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Electron",			0, XK_E,IDM_ELECTRON,			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Neutrino",			0, XK_N,IDM_NEUTRINO,  			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "Neutron",			0, XK_u,IDM_NEUTRON,  			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ "<unknown>",			0, XK_k,IDM_UNKNOWN,  			NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON },
	{ NULL, 				0, 0,	0,						NULL, 0, 0 },
	{ "All",				0, XK_A,IDM_SHOW_ALL,  			NULL, 0, 0 },
	{ "None",				0, XK_o,IDM_SHOW_NONE,  		NULL, 0, 0 },
};
static MenuStruct mc_main_menu[] = {
	{ "Display",			0, 0,	0, mc_display_menu, 	XtNumber(mc_display_menu), 0 },
	{ "Tracks",				0, 0,	0, mc_track_menu, 		XtNumber(mc_track_menu), 0 },
	{ "Particles",			0, 0,	0, mc_particle_menu, 	XtNumber(mc_particle_menu), 0 },
};


//---------------------------------------------------------------------------------
// PMonteCarloWindow constructor
//
PMonteCarloWindow::PMonteCarloWindow(ImageData *data)
			  	 : PWindow(data)
{
	Widget	w;
	int		n;
	Arg		wargs[16];
		
	mLastVertex = NULL;
	mLastFlagged = NULL;
	mNeedUpdate = 0;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Monte Carlo"); ++n;
	XtSetArg(wargs[n], XmNx, 250); ++n;
	XtSetArg(wargs[n], XmNy, 250); ++n;
//	XtSetArg(wargs[n], XmNminWidth, 100); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	XtSetArg(wargs[n], XmNwidth, 370); ++n;
	XtSetArg(wargs[n], XmNheight, 200); ++n;
	SetShell(CreateShell("mcPop",data->toplevel,wargs,n));
	
	n = 0;
	w = XtCreateManagedWidget("xsnoedForm",xmFormWidgetClass,GetShell(),wargs,n);
	SetMainPane(w);
	
	// Create Menubar
	CreateMenu(NULL,mc_main_menu, XtNumber(mc_main_menu),this);
	
	n = 0;
	XtSetArg(wargs[n], XmNleftAttachment,	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment,	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNtopWidget,		mMenu->GetWidget()); ++n;
	XtSetArg(wargs[n], XmNmarginLeft,		8); ++n;
	XtSetArg(wargs[n], XmNmarginRight,		20); ++n;
	XtSetArg(wargs[n], XmNmarginTop,		8); ++n;
	XtSetArg(wargs[n], XmNalignment,		XmALIGNMENT_BEGINNING); ++n;
	mTitleLabel = XtCreateManagedWidget("mcTitle",xmLabelWidgetClass,w,wargs,n);
	setLabelString(mTitleLabel,"Time (ns)   Energy    R    Z   Particle   Interaction");
//								123456789 12345678  1234 1234  1234567890 xxx

	n = 0;
	XtSetArg(wargs[n], XmNleftAttachment,	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment,	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNtopWidget,		mTitleLabel); ++n;
//	XtSetArg(wargs[n], XmNrightAttachment,	XmATTACH_FORM); ++n;
//	XtSetArg(wargs[n], XmNbottomAttachment,	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNmarginLeft,		8); ++n;
	XtSetArg(wargs[n], XmNmarginRight,		8); ++n;
	XtSetArg(wargs[n], XmNmarginTop,		0); ++n;
	XtSetArg(wargs[n], XmNmarginBottom,		8); ++n;
	XtSetArg(wargs[n], XmNalignment,		XmALIGNMENT_BEGINNING); ++n;
//	XtSetArg(wargs[n], XmNalignment,		XmALIGNMENT_BASELINE_TOP); ++n;
	mTrackLabel = XtCreateManagedWidget("mcTrack",xmLabelWidgetClass,w,wargs,n);

	data->mSpeaker->AddListener(this);	// listen for cursor motion
	
	/* initialize the menu toggles to the current settings */
	GetMenu()->SetToggle(data->wMCNames, TRUE);
	GetMenu()->SetToggle(IDM_HIGHLIGHT, data->mcHighlight);
	GetMenu()->SetToggle(data->wMCTrack, TRUE);
	for (n=1; n<7; ++n) {
		GetMenu()->SetToggle(n, data->mcFlags & (1 << n));
	}
	SetParticleItems();
	
	ClearEntries();
	
	Show();
}

PMonteCarloWindow::~PMonteCarloWindow()
{
}


/* check or uncheck particle names in menu according to current settings */
void PMonteCarloWindow::SetParticleItems()
{
	int			hidden;
	ImageData	*data = GetData();
	
	hidden = (data->mcParticle & (1UL << SnoStr::GetParticleIndex(1)));
	GetMenu()->SetToggle(IDM_PHOTON, !hidden);
	hidden = (data->mcParticle & (1UL << SnoStr::GetParticleIndex(2)));
	GetMenu()->SetToggle(IDM_GAMMA, !hidden);
	hidden = (data->mcParticle & (1UL << SnoStr::GetParticleIndex(20)));
	GetMenu()->SetToggle(IDM_ELECTRON, !hidden);
	hidden = (data->mcParticle & (1UL << SnoStr::GetParticleIndex(30)));
	GetMenu()->SetToggle(IDM_NEUTRINO, !hidden);
	hidden = (data->mcParticle & (1UL << SnoStr::GetParticleIndex(81)));
	GetMenu()->SetToggle(IDM_NEUTRON, !hidden);
	hidden = (data->mcParticle & 1UL);
	GetMenu()->SetToggle(IDM_UNKNOWN, !hidden);
}


/* clear all vertex entries from displayed track list */
void PMonteCarloWindow::ClearEntries()
{
	setLabelString(mTrackLabel,"");
}


/* get particle name string */
char * PMonteCarloWindow::ParticleName(MonteCarloVertex *vertex)
{
	return SnoStr::LookupParticle(vertex->particle);
}

/* get interaction type string */
char * PMonteCarloWindow::InteractionName(MonteCarloVertex *vertex)
{
	return SnoStr::LookupInteraction((vertex->int_code / 1000) % 1000);
}

/* get particle bitmask for specified MC vertex */
/* bit zero = <unknown>, all other bits offset by one from particle index in lookup */
unsigned long PMonteCarloWindow::ParticleMask(MonteCarloVertex *vertex)
{
	return(1UL << SnoStr::GetParticleIndex(vertex->particle));
}

/* show track information */
int PMonteCarloWindow::ShowTrackInfo(MonteCarloVertex *vertex, MonteCarloVertex *flagged)
{
	int			len;
	int			count, baseCount=0;
	const int	kMaxListLen = 50;
	const int	kBuffLen = 4196;
	const int	kMaxTrack = 2000;
	char		time_str[32];
	char		energy_str[32];
	char 		buff[kBuffLen];
	int			track[kMaxTrack];
	int			rtnVal = 0;
	
	if (mLastVertex!=vertex || mLastFlagged!=flagged || mNeedUpdate) {
		mLastVertex = vertex;
		mLastFlagged = flagged;
		mNeedUpdate = 0;
		if (vertex) {
			buff[0] = 0;
			if (vertex && GetData()->monteCarlo) {
				MonteCarloVertex *first = (MonteCarloVertex *)(GetData()->monteCarlo + 1);
				int numVertices = GetData()->monteCarlo->nVertices;
				int flaggedNum = -1;
				// generate list of vertices in reverse order
				for (count=0; count<kMaxTrack;) {
					track[count++] = vertex - first;
					if (vertex == flagged) flaggedNum = count;
					if (vertex->parent<0 || vertex->parent>=numVertices) break;
					vertex = first + vertex->parent;
				}

				len = 0;
				// if track is too long, decide what parts we will display
				if (count > kMaxListLen) {
					if (flaggedNum >= 0) {
						// center list on flagged vertex
						baseCount = flaggedNum - kMaxListLen / 2;
						// check for running off end of list
						if (baseCount + kMaxListLen > count) {
							baseCount = count - kMaxListLen;
						} else {
							if (baseCount < 0) baseCount = 0;
							count = baseCount + kMaxListLen;
							// print elipsis at start of list because we are skipping entries
							len = sprintf(buff, "  [...]\n");
						}
					}
				}
				// print out information in forward order
				float lx,ly,lz;
				for (;;) {
					--count;
					vertex = first + track[count];
					// calculate step from last vertex
/*					float step;
					if (len) {
						step = sqrt((lx-vertex->x)*(lx-vertex->x) +
									(ly-vertex->y)*(ly-vertex->y) +
									(lz-vertex->z)*(lz-vertex->z));
					} else {
						step = 0;
					}
*/					// remember this vertex position
					lx = vertex->x; ly = vertex->y; lz = vertex->z;
					// print vertex information
					double theTime = vertex->time;
					if (theTime<999999.9 && theTime>-99999.9) {
						sprintf(time_str, "%8.1f", theTime);
					} else {
						sprintf(time_str, "%8.2g", theTime);
					}
					if (vertex->energy<99999.99 && vertex->energy>-9999.99) {
						sprintf(energy_str, "%8.2f", vertex->energy);
					} else {
						sprintf(energy_str, "%8.2g", vertex->energy);
					}
					char *flag_str;
					if (vertex == flagged) {
						flag_str = "*";
					} else {
						flag_str = "";
					}
					char *particle_name = ParticleName(vertex);
					char *interaction_name = InteractionName(vertex);
					len += sprintf(buff+len, "%s%s %s  %4.0f %4.0f  %-10s %s",
							flag_str,
							time_str,
							energy_str,
							sqrt(lx*lx + ly*ly + lz*lz),
							vertex->z,
// for some reason this causes the hpcvl compiler to barf - PH 10/16/01
//							ParticleName(vertex),
//							InteractionName(vertex));
							particle_name,
							interaction_name);
					if (count<=baseCount || len>kBuffLen-120) {
						if (count) len += sprintf(buff+len,"\n  [...]");
						break;
					}
					len += sprintf(buff+len,"\n");	// add new line
				}
			}
			setLabelString(mTrackLabel, buff);
		} else {
			ClearEntries();
		}
		rtnVal = 1;
	}
	return(rtnVal);
}

/* must only call this routine if data->monteCarlo is not NULL */
int PMonteCarloWindow::ShowTrackInfo(int cur_x,int cur_y, int forced, PProjImage *proj)
{
	int dx, dy, t;
	ImageData *data = GetData();

	/* cursor moved - return non-zero if data changed */
	Node node[2];
	int numVertices = data->monteCarlo->nVertices;
	
	MonteCarloVertex *first = (MonteCarloVertex *)(data->monteCarlo + 1);
	MonteCarloVertex *last = first + numVertices;
	MonteCarloVertex *vertex;
	MonteCarloVertex *closest = NULL;
	float scale = 1.0 / data->tube_radius;
	int dist = 100000;
	for (vertex=first; vertex<last; ++vertex) {
		int n = vertex->parent;
		MonteCarloVertex *parent;
		if (vertex->flags & VERTEX_FLAG_HIDDEN) {
			if (n<0 || n>=numVertices) continue;
			parent = first + n;
			if (parent->flags & VERTEX_FLAG_HIDDEN) continue;
		} else if (n>=0 && n<numVertices) {
			parent = first + n;
			if (parent->flags & VERTEX_FLAG_HIDDEN) {
				parent = NULL;	// ignore parent if hidden
			}
		} else {
			parent = NULL;
		}
		node[0].x3 = vertex->x * scale; 
		node[0].y3 = vertex->y * scale; 
		node[0].z3 = vertex->z * scale;
		proj->Transform(node,1);

		// find distance to node
		dx = cur_x - node[0].x;
		dy = cur_y - node[0].y;
		t = dx * dx + dy * dy;
		if (t < dist) {
			closest = vertex;
			dist = t;
		}

		if (parent) {
			// find distance to track
			node[1].x3 = parent->x * scale; 
			node[1].y3 = parent->y * scale; 
			node[1].z3 = parent->z * scale;
			proj->Transform(node+1,1);
			int x0 = node[1].x - node[0].x;
			int y0 = node[1].y - node[0].y;
			t = x0 * x0 + y0 * y0;
			if (!t) continue;
			double u = (dx * x0 + dy * y0) / (double)t;
			if (u<0 || u>1) continue;	// point is outside endpoints of line
			double v;
			if (y0) {
				v = (u * x0 - dx) / y0;
			} else {
				v = (dy - u * y0) / x0;
			}
			t = (int)(t * v * v);	// find square of distance from line
			if (t < dist) {
				if (u < 0.5) {
					closest = vertex;	// closest to original vertex
				} else {
					closest = parent;		// closest to parent
				}
				dist = t;
			}
		}
	}
	if (dist > 256) closest = NULL;	// too far away
	MonteCarloVertex *daughter = closest;
	if (daughter) {
		do {
			int num = daughter - first;
			for (vertex=first; vertex<last; ++vertex) {
				if (vertex->parent == num) {
					daughter = vertex;	// work down to the last descendant
					break;
				}
			}
		} while (vertex < last);
	}
	int	rtnVal = ShowTrackInfo(daughter, closest);
	if (data->mcHighlight && rtnVal) {
		HighlightTrack(mLastVertex);
	}
	return(rtnVal);
}


/* highlight track in 3D display that corresponds to printed vertex information */
void PMonteCarloWindow::HighlightTrack(MonteCarloVertex *theVertex)
{
	ImageData *data = GetData();
	
	if (!data->monteCarlo) return;
	
	// reset SPECIAL and MARK flags
	monteCarloSetFlags(data, VERTEX_FLAG_SPECIAL | VERTEX_FLAG_MARK, 0);
	
	// set all vertices in printed track to SPECIAL
	if (theVertex) {
		int numVertices = data->monteCarlo->nVertices;
		MonteCarloVertex *first = (MonteCarloVertex *)(data->monteCarlo + 1);
		for (MonteCarloVertex *vertex=theVertex;;) {
			vertex->flags |= VERTEX_FLAG_SPECIAL;
			if (vertex == mLastFlagged) {
				// test this inside the loop so we can be guaranteed that we
				// don't use a stale mLastFlagged pointer.
				mLastFlagged->flags |= VERTEX_FLAG_MARK;
			}
			if (vertex->parent<0 || vertex->parent>=numVertices) break;
			vertex = first + vertex->parent;
		}
	}
	// force redraw of monte carlo data
	sendMessage(data, kMessageMonteCarloChanged);
}


void PMonteCarloWindow::Listen(int message, void *message_data)
{
	ImageData *data;
	
	switch (message) {
		case kMessageMCVertexChanged:
			ShowTrackInfo((MonteCarloVertex *)message_data);
			break;
		case kMessageEventCleared:
			if (mLastVertex) {
				// avoid accessing stale data
				mLastVertex = NULL;
				// force next update
				mNeedUpdate = 1;
			}
			break;
		case kMessage3dCursorMotion:
			data = GetData();
			if (data->monteCarlo && data->wMCTrack!=IDM_MC_SINGLE_TRACK) {
				ShowTrackInfo(data->last_cur_x, data->last_cur_y, 0, (PProjImage *)message_data);
			}
			break;
	}
}

void PMonteCarloWindow::DoMenuCommand(int anID)
{
	ImageData *	data = GetData();
	u_int32		particle;
	
	switch (anID) {
		case IDM_HIDE_NAMES:
		case IDM_INTERACTION_NAMES:
		case IDM_HIGHLIGHTED_NAMES:
			PMenu::UpdateTogglePair(&data->wMCNames);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_HIGHLIGHT:
			data->mcHighlight ^= 1;
			HighlightTrack(data->mcHighlight ? mLastVertex : NULL);
			break;
		case IDM_MC_ALL_TRACKS:
		case IDM_MC_SINGLE_TRACK:
		case IDM_MC_SELECTED:
			PMenu::UpdateTogglePair(&data->wMCTrack);
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IM_SPECTRAL_REFL:
		case IM_DIFFUSE_REFL:
		case IM_DIFFUSE_TRANS:
		case IM_RAYLEIGH_SCAT:
		case IM_PMT_BOUNCE:
		case IM_REACHED_PMT:
			if (data->wMCTrack != IDM_MC_SELECTED) {
				GetMenu()->SetToggle(data->wMCTrack, FALSE);
				data->wMCTrack = IDM_MC_SELECTED;
				GetMenu()->SetToggle(data->wMCTrack, TRUE);
			}
			data->mcFlags ^= (1<<anID);
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_PHOTON:
			// 1 = photon
			particle = (1UL << SnoStr::GetParticleIndex(1));
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_GAMMA:
			// 2 = gamma
			particle = (1UL << SnoStr::GetParticleIndex(2));
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_ELECTRON:
			// 20 = electron
			particle = (1UL << SnoStr::GetParticleIndex(20));
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_NEUTRINO:
			// 30 = Nu-e
			particle = (1UL << SnoStr::GetParticleIndex(30));
			particle |= (particle << 1);	// add bit mask for Nu-e-bar
			particle |= (particle << 1);	// add bit mask for Nu-mu
			particle |= (particle << 1);	// add bit mask for Nu-mu-bar
			particle |= (particle << 1);	// add bit mask for Nu-tau
			particle |= (particle << 1);	// add bit mask for Nu-tau-bar
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_NEUTRON:
			// 81 = neutron
			particle = (1UL << SnoStr::GetParticleIndex(81));
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_UNKNOWN:
			// 0x01 = <unknown> bitmask
			particle = 1UL;
			data->mcParticle ^= particle;
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_SHOW_ALL:
			data->mcParticle = 0;
			SetParticleItems();
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
		case IDM_SHOW_NONE:
			data->mcParticle = 0xffffffffUL;
			SetParticleItems();
			monteCarloProcess(data);
			sendMessage(data, kMessageMonteCarloChanged);
			break;
	}
}

