/*
** Postscript file drawable - PH 09/22/99
*/
#include <string.h>
#include "PResourceManager.h"
#include "PDrawPostscriptFile.h"


PDrawPostscriptFile::PDrawPostscriptFile(char *filename, int landscape)
{
	strncpy(mFilename, filename, kMaxPSNameLen);
	mFilename[kMaxPSNameLen-1] = '\0';
	mFile = 0;
	mIsLandscape = landscape;
		
	int	num_scale = PResourceManager::sResource.num_cols;
	int	num_vessel = PResourceManager::sResource.ves_cols;
	mColours = new XColor[NUM_COLOURS + num_scale + num_vessel];
									  
	if (mColours) {
		int	i;
		// initialize colour pixel values
		for (i=0; i<NUM_COLOURS; ++i) {
			mColours[i].pixel = PResourceManager::sResource.colour[i];
		}
		for (i=0; i<num_scale; ++i) {
			mColours[i+NUM_COLOURS].pixel = PResourceManager::sResource.scale_col[i];
		}
		for (i=0; i<num_vessel; ++i) {
			mColours[i+NUM_COLOURS+num_scale].pixel = PResourceManager::sResource.vessel_col[i];
		}
		// get current X color map
		Display	  *	dpy  = PResourceManager::sResource.display;
		int			scr  = DefaultScreen(dpy);
		Colormap	cmap = DefaultColormap(dpy, scr);
		XQueryColors(dpy,cmap,mColours,NUM_COLOURS+num_scale+num_vessel);
	}
}

PDrawPostscriptFile::~PDrawPostscriptFile()
{
	if (mFile) fclose(mFile);
	delete [] mColours;
}

int PDrawPostscriptFile::BeginDrawing(int width, int height)
{
	if (!mColours) return(0);
	
	if (mFile) fclose(mFile);
	
	mFile = fopen(mFilename,"w");
	
	if (!mFile) return(0);

	float pageWidth = 7.5 * 72;
	float pageHeight = 10 * 72;
	float pageAspect = pageHeight / pageWidth;
	float scale;
	
	if (strstr(mFilename,".eps")) {
		mIsEPS = 1;
		scale = 1.0 / GetScaling();
	} else {
		mIsEPS = 0;
		if (mIsLandscape) {
			// landscape mode
			if (height/(float)width > 1/pageAspect) {
				// limited by height
				scale = pageWidth / height;
			} else {
				// limited by width
				scale = pageHeight / width;
			}
		} else {
			// portrait mode
			if (height/(float)width > pageAspect) {
				// limited by height
				scale = pageHeight / height;
			} else {
				// limited by width
				scale = pageWidth / width;
			}
		}
	}
	if (mIsEPS) {
		fputs("%!PS-Adobe-3.0 EPSF-3.0\n", mFile);
	} else {
		fputs("%!PS-Adobe-3.0\n", mFile);
	}
	fputs(	"%%Creator: (Phil Harvey)\n"
			"%%Title: (XSNOED Image)\n"
			"%%DocumentFonts: Helvetica Symbol\n"
			"%%BoundingBox: ", mFile);
	if (mIsEPS) {
		sprintf(mBoundingBoxStr, "0 0 %d %d\n",
				(int)(width * scale),
				(int)(height * scale));
		fputs(mBoundingBoxStr, mFile);
		fputs("%%Pages: 0\n", mFile);
	} else {
		if (mIsLandscape) {
			sprintf(mBoundingBoxStr, "%d %d %d %d\n",
					(int)(0.5 * 72),
					(int)(5.5 * 72 - width * scale / 2),
					(int)(0.5 * 72 + height * scale),
					(int)(5.5 * 72 + width * scale / 2));
		} else {
			sprintf(mBoundingBoxStr, "%d %d %d %d\n",
					(int)(4.25 * 72 - width * scale / 2),
					(int)(10.5 * 72 - height * scale),
					(int)(4.25 * 72 + width * scale / 2),
					(int)(10.5 * 72));
		}
		fputs(mBoundingBoxStr, mFile);
		fputs("%%Orientation: Portrait\n"
			  "%%PageOrder: Ascend\n"
			  "%%Pages: 1\n", mFile);
	}
	fputs(	"%%EndComments\n", mFile );
	if (!mIsEPS) {
		fputs("%%Page: 1 1\n", mFile);
	}
	fputs(	"\n"
			"% Start function definitions\n"
			"\n"
			"/inch { 72 mul } def\n"
			"/helv { /Helvetica findfont exch scalefont setfont } def\n"
			"/symb { /Symbol findfont exch scalefont setfont } def\n"
//			"/tsize { gsave newpath 0 0 moveto true charpath pathbbox grestore 4 2 roll pop pop } def\n"
			"% (text) width height textoutxx\n"
			"/textouttl { neg exch pop 0 exch rmoveto show } def\n"
			"/textouttc { neg exch 2 div neg exch rmoveto show } def\n"
			"/textouttr { neg exch neg exch rmoveto show } def\n"
			"/textoutml { 2 div neg exch pop 0 exch rmoveto show } def\n"
			"/textoutmc { 2 div neg exch 2 div neg exch rmoveto show } def\n"
			"/textoutmr { 2 div neg exch neg exch rmoveto show } def\n"
			"/textoutbl { pop pop show } def\n"
			"/textoutbc { pop 2 div neg 0 rmoveto show } def\n"
			"/textoutbr { pop neg 0 rmoveto show } def\n"
			"\n"
			"% Start drawing\n"
			"\n"
			"gsave\n"
			, mFile);
			
	if (mIsEPS) {
		fprintf(mFile,"%f %f scale 0 %d translate\n",scale,-scale,-height);
	} else {
		if (mIsLandscape) {
			fputs(".5 inch 5.5 inch translate\n", mFile);
			fputs("90.0 rotate\n", mFile);
		} else {
			fputs("4.25 inch 10.5 inch translate\n",mFile);
		}
		fprintf(mFile,"%f %f scale %d 0 translate\n",scale,-scale,-width/2);
	}
	fprintf(mFile,"newpath 0 0 moveto %d 0 lineto\n",width);
	fprintf(mFile,"%d %d lineto 0 %d lineto closepath clip\n",width,height,height);
	fputs("90 helv\n"
		  "1 setlinejoin\n"
		  "1 setlinecap\n", mFile);
	SetLineWidth(0.5);
	
	return(1);	// success!
}

void PDrawPostscriptFile::EndDrawing()
{
	fputs("grestore\n",mFile);
	if (!mIsEPS) {
		fputs("showpage\n", mFile);
	}
	fputs("%%PageTrailer\n"
		  "%%Trailer\n"
		  "%%BoundingBox: ", mFile);
	fputs(mBoundingBoxStr, mFile);
	fputs("%%EOF\n", mFile);
	fclose(mFile);
	mFile = 0;
}


void PDrawPostscriptFile::SetLineWidth(float width)
{
	fprintf(mFile,"%.2f setlinewidth\n",GetScaling() * width);
}

void PDrawPostscriptFile::SetLineType(ELineType type)
{
	switch (type) {
		case kLineTypeSolid:
			fputs("[] 0 setdash\n",mFile);
			break;
		case kLineTypeDot:
			fprintf(mFile,"[%.2f %.2f] %.2f setdash\n",
					0.5 * GetScaling(), 2.0 * GetScaling(), 1.5 * GetScaling());
			break;
		case kLineTypeDash:
			fprintf(mFile,"[%.2f %.2f] %.2f setdash\n",
					3.0 * GetScaling(), 2.0 * GetScaling(), 4.0 * GetScaling());
			break;
	}
}

void PDrawPostscriptFile::SetForeground(int col_num)
{
	fprintf(mFile,"%.3f %.3f %.3f setrgbcolor\n",
			mColours[col_num].red / 65535.0,
			mColours[col_num].green / 65535.0,
			mColours[col_num].blue / 65535.0);
}

int PDrawPostscriptFile::EqualColours(int col1, int col2)
{
	return((mColours[col1].red   == mColours[col2].red  ) &&
		   (mColours[col1].green == mColours[col2].green) &&
		   (mColours[col1].blue  == mColours[col2].blue ));
}

void PDrawPostscriptFile::SetFont(XFontStruct *font)
{
	PDrawable::SetFont(font);
	
	fprintf(mFile,"%d helv\n",GetScaling() * GetFont()->ascent);
}

// Comment - enter a comment into the postscript file
void PDrawPostscriptFile::Comment(char *str)
{
	fprintf(mFile,"\n%% %s\n\n",str);
}

void PDrawPostscriptFile::DrawSegments(XSegment *segments, int num)
{
	fprintf(mFile,"newpath\n");
	XSegment *curpos = NULL;
	while (num) {
	    // move to start of segment if necessary
	    if (!curpos || segments->x1 != curpos->x2 || segments->y1 != curpos->y2) {
            fprintf(mFile,"%d %d moveto ",segments->x1,segments->y1);
        }
        fprintf(mFile,"%d %d lineto\n",segments->x2,segments->y2);
        curpos = segments;
		++segments;
		--num;
	}
	fprintf(mFile,"stroke\n");
}

void PDrawPostscriptFile::DrawLine(int x1,int y1,int x2,int y2)
{
	fprintf(mFile,"newpath %d %d moveto %d %d lineto stroke\n",
			x1, y1, x2, y2);
}

void PDrawPostscriptFile::FillRectangle(int x,int y,int w,int h)
{
	fprintf(mFile,"newpath %d %d moveto %d %d lineto\n",x,y,x+w,y);
	fprintf(mFile,"%d %d lineto %d %d lineto closepath fill\n",
			x+w,y+h,x,y+h);
}

void PDrawPostscriptFile::FillPolygon(XPoint *point, int num)
{
	fprintf(mFile,"newpath\n");
	fprintf(mFile,"%d %d moveto\n",point->x,point->y);
	while (--num > 0) {
		++point;
		fprintf(mFile,"%d %d lineto\n",point->x,point->y);
	}
	fprintf(mFile,"closepath fill\n");
}

void PDrawPostscriptFile::DrawString(int x, int y, char *str, ETextAlign_q align)
{
	if (GetFont()) {
		static char *draw_cmds[] = { "textouttl", "textouttc", "textouttr",
									 "textoutml", "textoutmc", "textoutmr",
									 "textoutbl", "textoutbc", "textoutbr" };
		char ch, *pt, *start;
		int is_symbol;
		
		fprintf(mFile,"gsave %d %d translate 0 0 moveto 1 -1 scale\n", x, y);
		fputs("gsave newpath 0 0 moveto\n", mFile);

		// loop=0 draws to path so we can get text size,
		// loop=1 actually draws text
		for (int loop=0; ; ++loop) {
			is_symbol = 0;
			start = str;
			for (pt=str; ; ++pt) {
				ch = *pt;
				if (ch != '\0') {
					if (ch & 0x80) {
						if (is_symbol) continue;
					} else {
						if (!is_symbol) continue;
					}
				}
				if (pt != start) {
					if (is_symbol) {
						fprintf(mFile,"%d symb\n",GetScaling() * GetFont()->ascent);
					} else if (start != str) {
						fprintf(mFile,"%d helv\n",GetScaling() * GetFont()->ascent);
					}
					fprintf(mFile,"(%.*s) ", (int)(pt-start), start);
					if (loop) {
						// show this section of the string
						if (start == str) {
							fprintf(mFile,"3 1 roll %s\n", draw_cmds[align]);
						} else {
							fputs("show\n", mFile);
						}
					} else {
						// draw string into current path
						fputs("true charpath\n", mFile);
					}
				}
				if (ch == '\0') break;
				start = pt;
				is_symbol = !is_symbol;
			}
			if (loop) break;	// stop now if we have drawn the text
			
			// finish getting the text size
			fputs("pathbbox grestore 4 2 roll pop pop\n", mFile);
			
			// print whole string now if all in normal text
			if (start==str && !is_symbol) {
				// quick, easy print everthing
				fprintf(mFile,"(%s) 3 1 roll %s\n", str, draw_cmds[align]);
				break;	// all done
			}
		}
		// all done -- restore the graphics state before we leave
		fputs("grestore\n", mFile);
	}
}

void PDrawPostscriptFile::DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
	fprintf(mFile,"gsave %d %d translate 1 %f scale newpath 0 0 %d %f %f arc stroke grestore\n",
			cx, cy, (float)ry/rx, rx, ang1, ang2);
}

void PDrawPostscriptFile::FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
	fprintf(mFile,"gsave %d %d translate 1 %f scale newpath 0 0 %d %f %f arc fill grestore\n",
			cx, cy, (float)ry/rx, rx, ang1, ang2);
}

