#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "calibrate.h"
#include "openfile.h"
#include "CUtils.h"
#include "PZdabFile.h"

/* See comments in calibrate.h ... */


static int cal_time_offset;

char *readLine( char *buff, int bsiz, FILE *fd, int *n )
{
	char *p;

	for( ; ; ) {
		if( !fgets( buff, bsiz, fd ) ) {
			if( feof( fd ) ) {
				Printf("Unexpected EOF at line %d\n", *n );
			} else {
				Printf("I/O error at line %d\n", *n );
			}
			return( NULL );
		}
		++(*n);
		p = buff;
		if( *p == '*' && *( p + 1 ) == '-' ) continue;
		while( *p ) {
			if( *p == '\n' ) {
				*p = 0x0;
				break;
			}
			++p;
		}
		if( (p = strstr( buff, "#." )) != NULL ) *p = 0x0; // Ignore trailing comments
		p = buff;
		while( *p ) {
			if( *p != ' ' ) break;
			++p;
		}
		if( *p == 0x0 ) continue;  //Ignore blank lines
		break;
	}
	return( p );
}

int readHdr( FILE *fd, int *bnum, int std_hdr[], int *dat_typ, int *linenum )
{
	char buff[256], hdr[256];
	char *p, *q, *s;
	int n, m = 0;
	int hdr_cnt, hdr_typ = 0;
	int bank;
	float f;

	bank = 0;
	*dat_typ = 0;
	for( ;  ; ) {
		if( !(s = readLine( hdr, sizeof(hdr), fd, linenum ) ) ) return( -1 );
		if( *s != '*' ) break;
		if( ( s = strstr( s, "*DO" ) ) ) break;
	}
	s += 3;
	while( *s ) {
		if( *s != ' ' ) break;
		++s;
	}
	if( !(*s) ) {
		Printf("Error decoding database header\n" );
		return( -1 );
	}
	memcpy( &bank, s, 4 );
	s += 4;
	while( *s ) {
		if( *s != ' ' ) break;
		++s;
	}
	*bnum = atoi( s );


	if( !( p = strchr( s, '(' ) ) || !( q = strchr( s, ')' ) ) ) {
	Printf("Error decoding database header\n" );
	return( -1 );
 }
	++p;
	*q = 0x0;
	if( !( q = strchr( p, '-' ) ) ) {
		Printf("Error decoding database header ( no datatype )\n" );
		return( -1 );
	}
	*q = 0x0;
	*dat_typ = *( q + 1 );
	while( *p ) {
		for( ; *p == ' ' ; ++p );
		if( !(*p) ) break; 
		for( q = p; isdigit( *q ) || ( *q == '+' ); ++q );
		hdr_typ = *q;
		if( !(*q) ) {
			Printf("Error decoding database header ( no datatype )\n" );
			return( -1 );
		}
		*q = 0x0;
		hdr_cnt = atoi( p );
		p = q;
		++p;
		while( hdr_cnt ) {
			if( !(s = readLine( buff, sizeof(buff), fd, linenum ) ) ) return( -1 );
			while( *s ) { 
				while( *s ) {
					if( *s != ' ' ) break;
					++s;
				}
				if( !(*s) ) break;
				switch( hdr_typ) {
				case 'I' :
					n = atoi( s );
					if( m < TITLES_HDR_SIZE ) std_hdr[m] = n;
					++m;
					break;
				case 'F':
					f = atof( s );
					if( m < TITLES_HDR_SIZE ) std_hdr[m] = (int)f;
					break;
				default:
					while( *s != ' ' ) ++s;  //Just in case ...
					--hdr_cnt;
					continue;
				}
				--hdr_cnt;
				while( isdigit( *s ) || ( *s == '+' ) || ( *s == '-' ) ) ++s;
				
			}
		}
	}
	return( bank );
}


dBase *loadType1Calibration(FILE *fd, int nentries)
{

	char 	buff[256];
	char 	*delim = " \t\n";
	char 	*p;
	int 	crate, card, chan;
	int 	ccc, i, j, index, n, ver;
	float	*destpt;
	dBase 	*dbase;
	dBasePMT *pmt;
	Cmos	*cmos;

	/* See comments in calibrate.h regarding the file format ... */

	if (!fgets(buff, sizeof(buff), fd)) {
		Printf("Empty calibration file\n");
		return(NULL);
	}
	p = strtok(buff, delim);
	if (p) {
		ver = atoi(p);
	} else {
		ver = -1;
	}
	if (ver!=1 && ver!=2) {
		Printf("Invalid version %d for calibration constants file\n",ver);
		return(NULL);
	}

	/* allocate memory for database */
	dbase = (dBase *)malloc(sizeof(dBase) + (nentries - 1) * sizeof(dBasePMT));
	pmt = dbase->pmt;
	
	if (!dbase) {
		Printf("Out of memory for database\n");
		return(NULL);
	}
	dbase->magic_number = MAGIC_NUMBER;
	dbase->version = ver;
	dbase->nentries = nentries;
	dbase->nvalid = 0;
	
	/* read validity range */
	for (i=0; i<4; ++i) {
		p = strtok(NULL,delim);
		dbase->validity[i] = atoi(p);
	}
	
	/* read time walk constants */
	switch (ver) {
		case 1:
			for (i=0; i<2; ++i) {
				if (!fgets(buff, sizeof(buff), fd)) {
					Printf("Error reading walk constants\n");
					free(dbase);
					return(NULL);
				}
				p = strtok(buff, delim);
				for (n=0; ; ) {
					if (!p) {
						Printf("Error reading walk constants\n");
						free(dbase);
						return(NULL);
					}
					dbase->walk[i][n] = atof(p);
					if (++n >= 10) break;
					p = strtok(NULL, delim);
				}
			}
			break;
			
		case 2:
			n = 10;
			destpt = dbase->walk2;
			for (i=0; i<41; ++i) {
				if (!fgets(buff, sizeof(buff), fd)) {
					Printf("Error reading walk constants\n");
					free(dbase);
					return(NULL);
				}
				p = strtok(buff, delim);
				if (i == 40) {
					n = 2;
					destpt = dbase->walk3;
				}
				for (j=0; ; ) {
					if (!p) {
						Printf("Error reading walk constants\n");
						free(dbase);
						return(NULL);
					}
					*(destpt++) = atof(p);
					if (++j >= n) break;
					p = strtok(NULL, delim);
				}
			}
			break;
	}
	
	/* read PMT data */
	for(n=0; n<nentries; ++n) {
		if (!fgets(buff, sizeof(buff), fd)) {
			if (feof(fd)) {
				Printf("Unexpected EOF at PMT entry %d\n", n);
			} else {
				Printf("I/O error at PMT entry %d\n", n);
			}
			free(dbase);
			return(NULL);
		}
		p = strtok(buff, delim);
		index = atoi(p);
		p = strtok(NULL, delim);
		ccc = atoi(p);
		card = ccc / 1024;
		crate = (ccc - card * 1024) / 32;
		chan = ccc - (card * 1024 + crate * 32);
		if ((unsigned)crate>19u || (unsigned)card>15u || (unsigned)chan>31u) {
			Printf("Crate/card/channel out of range!\n");
			free(dbase);
			return(NULL);
		}
		pmt->ccc = (crate * 16 + card) * 32 + chan; /*XSnoed definition ...*/
		if (pmt->ccc != n) {
			Printf("Wrong CCC for PMT number %d\n", n);
			free(dbase);
			return(NULL);
		}

		for(i=0; i<16; ++i) {
			if (!fgets(buff, sizeof(buff), fd)) {
				if (feof(fd)) {
					Printf("Unexpected EOF at PMT/Cell entry %d/%d\n", n, i);
				} else {
					Printf("I/O error at PMT/Cell entry %d/%d\n", n, i);
				}
				free(dbase);
				return(NULL);
			}
			p = strtok(buff, delim);
			if (atoi(p) != i) {
				Printf("Invalid entry at PMT/Cell %d/%d\n", n, i);
				free(dbase);
				return(NULL);
			}
			cmos = pmt->cmos + i;
			p = strtok(NULL, delim);
			cmos->tac_offset = atof(p);
			p = strtok(NULL, delim);
			cmos->tac_slope = atof(p);
			p = strtok(NULL, delim);
			cmos->qhl_offset = atof(p);
			p = strtok(NULL, delim);
			cmos->qhl_slope = atof(p);
			p = strtok(NULL, delim);
			cmos->qhs_offset = atof(p);
			p = strtok(NULL, delim);
			cmos->qhs_slope = atof(p);
			p = strtok(NULL, delim);
			cmos->qlx_offset = atof(p);
			p = strtok(NULL, delim);
			cmos->qlx_slope = atof(p);
			
			/* set high bit of ccc to indicate an invalid calibration */
			if (cmos->tac_offset <= 0 || cmos->tac_slope <= 0 ||
				cmos->qhl_offset <= 0 || cmos->qhl_slope <= 0 ||
				cmos->qhs_offset <= 0 || cmos->qhl_slope <= 0 ||
				cmos->qlx_offset <= 0 || cmos->qlx_slope <= 0)
			{
				pmt->ccc |= 0x8000;
			}
		}
		if (!(pmt->ccc & 0x8000)) {
			++dbase->nvalid;
		}
		++pmt;
	}
	cal_time_offset = CAL_TIME_OFFSET;
	return(dbase);
}


dBase *newCalibrationDatabase(char *name, char *defaultDirectory, int mode )
{
	int		num;
	FILE	*fp;
	long	size;
	dBase	*dbase = NULL;
	int		create_binary = 0;
	
	/* try to open binary version first */
	fp = openAltFile(name,"rb",defaultDirectory,".bin");
	size = sizeof(dBase) + (NENTRIES - 1) * (long)sizeof(dBasePMT);
	
	if (fp) {
		/* use malloc() to allocate memory because that's how the calibration routines do it */
		dbase = (dBase *)malloc(size);
		if (!dbase) return(NULL);
		
		// Grrrrr Peter!  You broke this for the regular xsnoed calibration files --
		// Set time offset to the default when loading a binary file - PH 01/13/99
		cal_time_offset = CAL_TIME_OFFSET;
		
		Printf("Reading calibration database %s\n",getOpenFileNameNoPath());
		/* get the binary database */
		num = fread(dbase,size,1,fp);
		fclose(fp);
		if (num!=1 || dbase->magic_number!=MAGIC_NUMBER) {
			/* error -- try loading ascii file instead */
			free(dbase);
			dbase = NULL;
			Printf("Invalid binary calibration file %s\n",getOpenFileName());
			Printf("--> Will load ASCII version of calibration file\n");
		}
	} else {
		create_binary = 1;
	}
	if( mode ) return( dbase );
	/* if binary database wasn't loaded... */
	if (!dbase) {
		/* try to open ascii database file */
		fp = openFile(name,"r",defaultDirectory);
		if (fp) {
			Printf("Reading calibration database %s\n",getOpenFileNameNoPath());
			dbase = loadType1Calibration(fp,NENTRIES);
			fclose(fp);
			if (dbase && create_binary) {
				/* success! -- write binary file for next time */
				fp = createAltFile(getOpenFileName(),"wb",".bin");
				if (fp) {
					fwrite(dbase,size,1,fp);
					fclose(fp);
					Printf("Created calibration database %s\n",getOpenFileNameNoPath());
				}
			}
		}
	}
	return(dbase);
}


// checkDatabase - returns zero if database entry is valid
static int checkDatabase(dBasePMT *pmt,int index)
{
	/* check a specific database entry for validity */
	static int	num_msgs = 0;
	const int MAX_MSGS = 20;
	
	/* check the database for consistency */
	if (index < NENTRIES) {
		if( ( pmt->ccc & 0x7fff ) != index ) {
			if (num_msgs < MAX_MSGS) {
				Printf("Inconsistent databases - expected ccc = 0x%x - got 0x%x\n",
						 index, pmt->ccc );
				if (++num_msgs == MAX_MSGS) {
					Printf("Maximum number of database messages exceeded\n");
				}
			}
		} else if( !( pmt->ccc & 0x8000 ) ) {		/* is calibration valid? */
			return(0);
		}
	}
	return(1);
}

/*
** get calibrated time
** - 'num' is the pmt index (crate * 512 + card * 32 + channel)
** - will do walk correction if qhs >= 0
** - returns calibrated value, or INVALID_CAL on error
*/
double getCalibratedTac(dBase *dbase, int tac, int cell, int num, int qhs)
{
	int			icharge;
	double		charge, calibrated;
	float		*walk;
	dBasePMT 	*pmt = dbase->pmt + num;
	Cmos		*cmos;
	
	/* check the database for consistency */
	if (!checkDatabase(pmt,num)) {
	
		cmos = pmt->cmos + cell;

		calibrated = cal_time_offset + ( cmos->tac_offset - tac ) /
			cmos->tac_slope;

		if (qhs >= 0) {
#ifdef OLD_WALK
			/* Richard Ford's old walk correction, based on Qhl */
			if ((cmos->qhl_offset > 0)
				charge = qhl - cmos->qhl_offset;
				if (charge < -50) charge = 4096 - cmos->qhl_offset;
				if (charge < 0) charge = 0;
				if (charge < 400) {
					calibrated += 18.84 - exp(2.463 +0.0001130 * charge)
								   		- exp(2.259   -0.04550 * charge)
								   		- exp(1.580  -0.001660 * charge);
				} else {
					calibrated += 3.54 + 0.00130 * charge;
				}
			}
#else
			charge = qhs - cmos->qhs_offset;
			if (charge < -50. ) charge = 4096.0 - cmos->qhs_offset; 
			if (charge < 0. ) charge = 0.0;

			switch (dbase->version) {
				case 1:
					/* new walk correction (from Mark Boulay) based on Qhs */
					if (charge <= 25.) {
						walk = dbase->walk[0];	/* lo walk */
						calibrated -= walk[0] + exp(walk[1] + walk[2]*charge)
								+ exp( walk[3] + walk[4]*charge);
					}
					else if ( charge > 25. ){
						walk = dbase->walk[1];	/* hi walk */
				  		calibrated -= walk[0] + exp(walk[1] + walk[2]*charge)
								+ exp(walk[3] + walk[4]*charge)
				   				+ walk[5]/charge;
					}
					break;
			
				case 2:
					icharge = (int)(charge + 0.5);
				  	if (icharge < 400) {
				  		calibrated -= dbase->walk2[icharge];
				  	} else {
				  		calibrated -= dbase->walk3[0] + dbase->walk3[1] * (charge - 400.0);
				  	}
				  	break;
			}
#endif /* OLD_WALK */
		}
	} else {
		calibrated = INVALID_CAL;
	}
	return(calibrated);
}

/*
** get calibrated Qhs
** - returns calibrated value, or INVALID_CAL on error
*/
double getCalibratedQhs(dBase *dbase, int qhs, int cell, int num)
{
	Cmos	 *cmos;
	dBasePMT *pmt = dbase->pmt + num;
	
	/* check the database for consistency */
	if (!checkDatabase(pmt,num)) {
		cmos = pmt->cmos + cell;
		return((qhs - cmos->qhs_offset) / cmos->qhs_slope);
	}
	return(INVALID_CAL);
}

/*
** get calibrated Qhl
** - returns calibrated value, or INVALID_CAL on error
*/
double getCalibratedQhl(dBase *dbase, int qhl, int cell, int num)
{
	Cmos	 *cmos;
	dBasePMT *pmt = dbase->pmt + num;
	
	/* check the database for consistency */
	if (!checkDatabase(pmt,num)) {
		cmos = pmt->cmos + cell;
		return((qhl - cmos->qhl_offset) / cmos->qhl_slope);
	}
	return(INVALID_CAL);
}

/*
** get calibrated Qlx
** - returns calibrated value, or INVALID_CAL on error
*/
double getCalibratedQlx(dBase *dbase, int qlx, int cell, int num)
{
	Cmos	 *cmos;
	dBasePMT *pmt = dbase->pmt + num;
	
	/* check the database for consistency */
	if (!checkDatabase(pmt,num)) {
		cmos = pmt->cmos + cell;
		return((qlx - cmos->qlx_offset) / cmos->qlx_slope);
	}
	return(INVALID_CAL);
}
