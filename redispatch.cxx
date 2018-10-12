/*
** File:        redispatcher.c
**
** Created:     08/14/98 by Phil Harvey (based on bdcollector.c)
**
** Revisions:   11/03/98 (2.0) PH - rewrite to allow multiple outputs and input from file
**              11/29/99 (2.1) PH - redispatch RECHDR records
**              05/16/00 (2.2) PH - allow different rates for each target dispatcher
**              10/27/00 (2.3) PH - redispatch all recognized banks from zdab file
**                                - truncate extended PMT event records
**              07/03/03 (2.4) PH - specify subscription string on command line
**              02/04/04 (2.5) PH - modify for redispatching NCD data
**              10/11/18 (2.6) T.Latorre - add some command line options
**                             PH - change default dispatch time to 0 sec
**                                - don't abort on startup if source dispatcher not available
**
** Description: program to redistribute SNO data to outside listeners
**              Sends max one event per second (the largest NHIT event that
**              occurred in the time interval). Also redistributes cmos rates data.
**
** Syntax:      redispatch [-d <target>...] [-t <time>] [-r] [-h] <source>
**
**              -d  target dispatcher (may be many -d options)
**              -t  time period to wait between sending events in steady rate mode (sec)
**              -R  dispatch events at rate determined by 50 MHz timestamps
**              -r  repeat source file indefinitely (only valid for file source)
**              -s  subscription string (default "w RAWDATA w CMOSDATA w RECHDR w REDCMD")
**              -h  show this help information
**              <source>  ZDAB file name or dispatcher address
**              <target>  dispatcher address
**              <time>  floating point number of seconds
*/
#define VERSION                 "2.6"       /* redispatcher version number */

#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "displowl.h"
#include "PZdabFile.h"
#include "PZdabWriter.h"
#include "CUtils.h"
#include "include/Record_Info.h"

#define DISP_BUFFSIZE           0x400000    /* 4 MB dispatcher buffer */
#define EVENT_BUFFSIZE          (256L*1024) /* event buffer size (must be <= DISP_BUFFSIZE) */
#define MAX_DATALESS1           10          /* maximum loops without data before we get nice */
#define MAX_DATALESS2           30          /* number of loops before getting real nice */
#define DATALESS_SLEEP_TIME1    25000       /* time to sleep after MAX_DATALESS1 loops (microsec) */
#define DATALESS_SLEEP_TIME2    500000      /* time to sleep after MAX_DATALESS2 loops (microsec) */
#define DEFAULT_TIME            0           /* default seconds between transmitted events */
#define CONNECT_DST_RETRY       60          /* time between reconnect attempts for target dispatchers */
#define CONNECT_SRC_RETRY       10          /* time between reconnect attempts for source dispatcher */
#define DEFAULT_DISP            "localhost" /* default dispatcher hostname */
#define LOOPBACK                0x7f000001  /* loopback IP address (127.0.0.1) */

typedef struct SDispEntry {
    struct SDispEntry *next;    /* pointer to next dispatcher in list */
    char *  hostname;           /* host name */
    int     iaddr;              /* IP address */
    int     socket;             /* dispatcher socket */
    double  retry_time;         /* timer used for retrying connections */
    // the following variables used by target dispatchers only
    double  dispatch_time;      /* time interval between sending events */
    double  next_time;          /* the next time to send an event */
    int     event_size;         /* size of largest event in this time period */
    char *  buffer;             /* buffer for target dispatcher */
} DispEntry;

extern int optind;

extern char *progname;

static char buffer[DISP_BUFFSIZE+1];
static DispEntry *disp_queue = NULL;
static DispEntry disp_source;
static char *dstid = "REDISPATCH_TO";
static char *srcid = "REDISPATCH_FROM";
static char *subs = "w RAWDATA w CMOSDATA w RECHDR w REDCMD"; //" w STATUS";

/*----------------------------------------------------------------------------------*/

/* put string to log file */
static void logPuts(char *str)
{
    time_t  the_time;
    struct tm   *tms;
    FILE        *fp;
    
    fp = fopen("redispatch.log","a");
    if (fp) {
        the_time = time(NULL);
        tms = localtime(&the_time);
        while (*str == '\n') {
            fputs("\n",fp);
            ++str;
        }
        fprintf(fp,"%.2d/%.2d/%.2d %.2d:%.2d:%.2d %s",
                    tms->tm_mon+1, tms->tm_mday, tms->tm_year % 100,
                    tms->tm_hour, tms->tm_min, tms->tm_sec,
                    str);
        fclose(fp);
    }
}

/* print message to stderr and log file */
static int rPrintf( char *format, ... )
{
    va_list ap;
    char    buff[512];
    int     result;

    va_start( ap, format );
    result = vsprintf( buff, format, ap );
    va_end( ap );
    
    /* print to stderr with program name */
    fprintf(stderr, "%s: %s", progname, buff);
    
    /* write to log file with date/time */
    logPuts(buff);

    return( result );
}


/* drop all dispatcher connections */
/* (do it this way because dispalldrop() causes core dumps) */
static void dropAll()
{
    DispEntry   *x;
    
    for (x=disp_queue; x!=NULL; x=x->next) {
        if (x->socket >= 0) dispdrop(x->socket);
    }
    if (disp_source.socket >= 0) {
        dispdrop(disp_source.socket);
    }
}

/* drop connections and exit on error */
static void quit()
{
    rPrintf("Terminating on error\n");
    dropAll();
    exit(1);
}

/* remove entry but do not free memory so the item's next pointer is still valid */
static void removeEntry(DispEntry *anEntry)
{
    DispEntry *x, **last_pt = &disp_queue;
    
    for (x=disp_queue; x!=NULL; x=x->next) {
        // remove this entry from the list
        if (x==anEntry) {
            *last_pt = x->next;
            break;
        }
        last_pt = &x->next;
    }
    if (disp_queue == NULL) {
        rPrintf("No target dispatchers\n");
        quit();
    }
}

/* print error and disconnect from dispatcher */
static void broken(DispEntry *disp)
{
    char    *type;
    
    if (disp->socket >= 0) {
        dispdrop(disp->socket);
        disp->socket = -1;
        if (disp == &disp_source) {
            disp->retry_time = double_time() + CONNECT_SRC_RETRY;
            type = "source";
        } else {
            disp->retry_time = double_time() + CONNECT_DST_RETRY;
            type = "target";
        }
        rPrintf("Connection to %s dispatcher %s broken\n",
                type, disp->hostname);
    }
}

/* send packet to one target dispatcher */
/* (handle disconnecting and reconnecting automatically) */
static void sendToOne(DispEntry *x, const char *tag, const char *buff, int size)
{
    /* is this dispatcher still connected? */
    if (x->socket < 0) {
        /* is it time to retry our connection? */
        if (double_time() < x->retry_time) return;  /* still waiting */
        x->socket = dispconnect(x->hostname);
        if (x->socket >= 0) {
            if (dispsend(x->socket,DISPTAG_MyId,dstid,strlen(dstid)) <= 0) {
                dispdrop(x->socket);
                x->socket = -1;
            } else {
                rPrintf("Connection to target dispatcher %s restored\n",x->hostname);
            }
        }
        if (x->socket < 0) {
            /* try again later */
            x->retry_time = double_time() + CONNECT_DST_RETRY;
            return;
        }
    }
    if (put_tagged_bwait(x->socket,tag,buff,size) <= 0) {
        broken(x);
    }
}

/* send packet to all target dispatchers */
static void sendToAll(const char *tag, const char *buff, int size)
{
    DispEntry *x;
    
    for (x=disp_queue; x!=NULL; x=x->next) {
        sendToOne(x, tag, buff, size);  // send to one dispatcher
    }
}

/* handle SIGPIPE errors */
static void sigPipeHandler(int sig)
{
    rPrintf("Broken pipe\n");
    return;
}

#ifdef USE_NANOSLEEP
static void usleep_ph(unsigned long usec)
{
    if (usec) {
        struct timespec ts;
        ts.tv_sec = usec / 1000000UL;
        ts.tv_nsec = (usec - ts.tv_sec * 1000000UL) * 1000;
        nanosleep(&ts,NULL);
    }
}
#else
static void usleep_ph(unsigned long usec)
{
    // sleep for whole seconds first
    // (the usleep argument must be less than 1000000)
    if (usec) {
        if (usec >= 1000000UL) {
            int secs = (int)(usec / 1000000UL);
            sleep(secs);
            usec -= secs * 1000000UL;
        }
        usleep(usec);
    }
}
#endif

/********************************************************************/
int main (int argc, char *argv[])
{
    int iaddr;                      /* temporary variable for IP address */
    DispEntry *x;                   /* temporary variable for dispatcher entry */
    int repeat;
    int realtime;                   /* flag indicating dispatch rate governed by 50MHz timestamp */
    int was_dest;                   /* flag indicating a target has been specified */
    char *host;                     /* temporary variable for host name */
    int i, n;                       /* temporary variables */
    u_int32 sleep_time;             /* microseconds to sleep between sending events */
    u_int32 rsleep_time;            /* seconds to sleep when dispatching 'realtime'*/
    double last50MHzTime=0;         /* 50MHZ timestamp of previously dispatched event */
    FILE *fp;                       /* zdab file */
    aPmtEventRecord *aPmt, *per;    /* pointers to SNO PMT records */
    aGenericRecordHeader    *grh;   /* pointers to generic records */
    u_int32 *dataPt;                /* pointer to bank data */
    double dispatch_time = DEFAULT_TIME;
    struct sigaction act;
    int dataless = 0;
    PZdabFile   zdab_file;
    int min_nhit = 0;               /* skip events with an nhit less than min_nhit */
    u_int32 max_sleep_time = 0;     /* maximum sleep time in microseconds. */
    int skip_pedestals = 0;         /* skip pedestal events. */
 
    int my_iaddr = my_inet_addr();  /* IP address of machine running this program */
    
#ifdef SWAP_BYTES
    // for some reason, the bytes of my_inet_addr() need swapping relative to the
    // return from his_inet_addr() for Intel machines - PH 05/17/00
    char *ch_pt = (char *)&my_iaddr;
    char tmp = ch_pt[0];
    ch_pt[0] = ch_pt[1];
    ch_pt[1] = tmp;
    tmp = ch_pt[2];
    ch_pt[2] = ch_pt[3];
    ch_pt[3] = tmp;
#endif

    printf("------- redispatch version " VERSION " by Phil Harvey -------\n");
    logPuts("\nRedispatch " VERSION " started\n");
    
    /* get pointer to program name */
    progname = strrchr(argv[0],'/');
    if (progname) ++progname;
    else progname = argv[0];
    
    was_dest = 0;
    repeat = 0;
    realtime = 0;
    disp_source.socket = -1;
    disp_source.hostname = NULL;
    
    // Set up SIGPIPE handler
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sigPipeHandler;
    if (sigaction(SIGPIPE, &act, (struct sigaction *)NULL) < 0) {
        rPrintf("Error installing SIGPIPE handler\n");
        quit();
    }
    
    for (i=1; i<=argc; ++i) {
        if (i == argc || !strcmp(argv[i],"-d")) {
            if (i == argc) {
                if (was_dest != 0) break;
                host = DEFAULT_DISP;
                rPrintf("No target dispatcher specified.  Using default target %s\n",host);
            } else if (i < argc-1) {
                was_dest = 1;
                host = argv[++i];
            } else {
                continue;
            }

            iaddr = his_inet_addr(host);
            if (iaddr == 0) {
                rPrintf("Unknown host %s ignored in argument list.\n",host);
                continue;
            }

            for (x=disp_queue; x!=NULL; x=x->next) {
                if (x->iaddr == iaddr) {
                    rPrintf("Second reference to host %s ignored in argument list.\n",host);
                    break;
                }
            }

            x = (DispEntry *)malloc(sizeof(DispEntry));
            if (x == NULL) {
                rPrintf("Out of memory!\n");
                quit();
            }

            x->hostname = host;
            x->iaddr = iaddr;
            x->socket = dispconnect(host);
            x->next = disp_queue;
            x->dispatch_time = dispatch_time;
            x->next_time = 0;
            x->event_size = 0;
            x->buffer = (char *)malloc(EVENT_BUFFSIZE);
            if (x->buffer == NULL) {
                rPrintf("Not enough memory for event buffers\n");
                quit();
            }
            disp_queue = x;

            if (x->socket < 0) {
                rPrintf("Could not connect to target dispatcher %s -- will keep trying\n",host);
                x->retry_time = double_time() + CONNECT_DST_RETRY;
            } else if (dispsend(x->socket,DISPTAG_MyId,dstid,strlen(dstid)) <= 0) {
                broken(x);
            }
            continue;
        }

        if (!strncmp(argv[i], "--", 2)) {
            /* Long option. */
            if ((i+1 < argc) && !strcmp(argv[i]+2, "min-nhit")) {
                min_nhit = atoi(argv[++i]);
            } else if ((i+1 < argc) && !strcmp(argv[i]+2, "max-sleep-time")) {
                max_sleep_time = atof(argv[++i])*1e6;
            } else if (!strcmp(argv[i]+2, "skip-pedestals")) {
                skip_pedestals = 1;
            } else {
                printf("unknown command line argument '%s'\n", argv[i]);
                exit(1);
            }
            continue;
        }

        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'h':
                printf("Syntax: %s [-d <target>...] [-t <time>] [-r] [-h] <source>\n",progname);
                printf("    -d  target dispatcher (may be many -d options)\n");
                printf("    -t  time period to wait between sending events in steady rate mode (sec)\n");
                printf("    -R  dispatch events at rate determined by 50 MHz timestamps\n");
                printf("    -r  repeat source file indefinitely (only valid for file source)\n");
                printf("    -s  subscription string (default \"w RAWDATA w CMOSDATA w RECHDR w REDCMD\")\n");
                printf("    --min-nhit        minimum nhit to redispatch\n");
                printf("    --max-sleep-time  maximum number of seconds to sleep\n");
                printf("    --skip-pedestals  don't redispatch pedestal events\n");
                printf("    -h  show this help information\n");
                printf("    <source>  ZDAB file name or dispatcher address\n");
                printf("    <target>  dispatcher address\n");
                printf("    <time>  floating point number of seconds\n");
                printf("If no source or target is specified, '%s' is assumed\n",DEFAULT_DISP);
                dropAll();
                exit(0);
            case 't':
                if (i < argc-1) {
                    dispatch_time = atof(argv[++i]);
                }
                break;
            case 'r':
                repeat = 1;
                break;
            case 'R':
                realtime = 1;
                break;
            case 's':
                if (i < argc-1) {
                    subs = argv[++i];
                    printf("Subscription string set to \"%s\"\n", subs);
                }
                break;
            default:
                printf("unknown command line argument '%s'\n", argv[i]);
                exit(1);
            }
        } else {
            disp_source.hostname = argv[i];
        }
    }

    if (disp_source.hostname == NULL) {
        disp_source.hostname = DEFAULT_DISP;
        rPrintf("No source specified.  Using default source %s\n",disp_source.hostname);
    }

    if (!disp_queue) {
        rPrintf("No target dispatcher(s) connected\n");
        quit();
    }
    
    /* first try to open source as a file */
    fp = fopen(disp_source.hostname,"rb");
  
    if (fp) {
/*
** Redispatch from source zdab file
*/
        if (zdab_file.Init(fp) < 0) {
            rPrintf("Zdab file format error\n");
            quit();
        }
      
        if ( realtime ==1 ) {
            rPrintf("Reading from ZDAB file %s in 'realtime' (based on 50MHz timestamps)\n",
                disp_source.hostname);
        }
        else {
            rPrintf("Reading from ZDAB file %s at a minimum period of %.2g sec\n",
                disp_source.hostname, dispatch_time);
        }
        for (x=disp_queue; x; x=x->next) {
            rPrintf("Redispatching to: %s\n",x->hostname);
        }

        grh = (aGenericRecordHeader *)buffer;
        grh->RecordID = PMT_RECORD;
        grh->RecordLength = sizeof( aPmtEventRecord );
        grh->RecordVersion = PTK_RECORD_VERSIONS;
        per = (aPmtEventRecord *) ( grh + 1 );
        
        sleep_time = (u_int32)(dispatch_time * 1e6);
    
        SWAP_INT32(buffer, 3);  // swap the generic record header
    
        /* infinite loop to dispatch SNO data */
        for (i=0;;) {
        
            nZDAB *nzdabPtr = zdab_file.NextRecord();
            if (nzdabPtr) {
                aPmt = zdab_file.GetPmtRecord(nzdabPtr);
                if (aPmt) {
                    ++i;
                    n = PZdabFile::GetSize(aPmt);
                    memcpy(per,aPmt,n);
                    // swap event to network byte-ordering
                    SWAP_INT32(per, (n + sizeof(u_int32) - 1) / sizeof(u_int32));
                                        if (min_nhit > 0 && aPmt->NPmtHit < min_nhit) continue;
                                        if (skip_pedestals > 0 && aPmt->TriggerCardData.Pedestal) continue;
                    sendToAll("RAWDATA",buffer,n+sizeof(aGenericRecordHeader));

                    if (realtime == 1) {
                        rsleep_time = (u_int32)((get50MHzTime(aPmt)-last50MHzTime)*1e6);
                        if (max_sleep_time > 0 && rsleep_time > max_sleep_time) rsleep_time = max_sleep_time;
                        if (rsleep_time > 0 && (last50MHzTime != 0) && (get50MHzTime(aPmt) != 0)) {
                            usleep_ph(rsleep_time);
                        }
                        last50MHzTime = get50MHzTime(aPmt);
                    }
                    else {
                        usleep(sleep_time);
                    }

                } else {
                    // write all other known banks (will write after a MAST bank)
                    int index = PZdabWriter::GetIndex(nzdabPtr->bank_name);
                    if (index>=0 && index!=kMASTindex) {
                        // redispatch all recognized banks (but not MAST banks)
                        n = PZdabWriter::GetBankNWords(index) * sizeof(u_int32);
                        // copy the data into our buffer
                        dataPt = (u_int32 *)(nzdabPtr + 1);
                        memcpy(per, dataPt, n);
                        // NOTE: no need to swap the bank here, because the data was never
                        // swapped by the PZdabFile object, since we generated a pointer
                        // to the data manually above
                        if (nzdabPtr->bank_name == RHDR_RECORD) {
                            // must translate run record ID for dispatching
                            grh->RecordID = RUN_RECORD;
                        } else {
                            // all others are the same when dispatched
                            grh->RecordID = nzdabPtr->bank_name;
                        }
                        grh->RecordLength = n;
                        SWAP_INT32(buffer, 2);  // swap the record ID and length
                        sendToAll("RECHDR", buffer, n + sizeof(aGenericRecordHeader));
                        // return generic record header to defaults
                        grh->RecordID = PMT_RECORD;
                        grh->RecordLength = sizeof( aPmtEventRecord );
                        SWAP_INT32(buffer, 2);  // save generic record header in swapped state
                    }
                }
            } else {
                if (repeat && i) {
                    i = 0;
                    zdab_file.Init(fp);
                    continue;
                }
                break;
            }
        }
    
        rPrintf("Done dispatching file %s\n",disp_source.hostname);
        fclose(fp);
    
    } else {
/*
** Redispatch from source dispatcher
*/  
        disp_source.iaddr = his_inet_addr(disp_source.hostname);
        for (x=disp_queue; x!=NULL; x=x->next) {
            if  (x->iaddr==disp_source.iaddr ||
                // loopback address is the same as 'my_addr'
                (x->iaddr==LOOPBACK && disp_source.iaddr==my_iaddr) ||
                (x->iaddr==my_iaddr && disp_source.iaddr==LOOPBACK))
            {
                rPrintf("Ignoring target %s (same as source)\n",
                        disp_source.hostname);
                broken(x);
                removeEntry(x);
            }
        }
        disp_source.socket = dispconnect(disp_source.hostname);

        if (disp_source.socket < 0 ||
            dispsubscribe(disp_source.socket,subs) <= 0 ||
            dispalways(disp_source.socket) <= 0 ||
            dispsend(disp_source.socket,DISPTAG_MyId,srcid,strlen(srcid)) <= 0)
        {
            rPrintf("Could not connect to source dispatcher %s -- will keep trying\n", disp_source.hostname);
        } else {
            rPrintf("Source dispatcher %s\n", disp_source.hostname);
        }
/*      dispprio(disp_source.socket,1);*/
        
        for (x=disp_queue; x; x=x->next) {
            if (realtime == 1 ) {
                rPrintf("Redispatching to %s in 'realtime' (from 50MHz timestamps)\n",
                    x->hostname);
            }
            else {
                rPrintf("Redispatching to %s at a minimum period of %.2g sec\n",
                    x->hostname, x->dispatch_time);
            }
        }

        /* infinite loop to redispatch SNO data */
        for (;;) {
            char htag[TAGSIZE+1];
            int hsize;
            int rc;
            double cur_time;
    
/*          if (dispchannels() <= 1) {
                rPrintf("All connections broken\n");
                quit();
            }
*/          
            cur_time = double_time();
            
            /* send events now if it is time */
            for (x=disp_queue; x; x=x->next) {
                if (x->event_size!=0 && cur_time>=x->next_time) {
                    sendToOne(x,"RAWDATA", x->buffer, x->event_size);
                    // reset event size after dispatching the event
                    x->event_size = 0;
                }
            }
            
            /* have we disconnected from our source? */
            if (disp_source.socket < 0) {
                if (cur_time < disp_source.retry_time) {
                    sleep(1);       /* avoid using up too much CPU time while waiting */
                    continue;       /* keep looping until it is time to retry */
                }
                /* try to reconnect */
                disp_source.socket = dispconnect(disp_source.hostname);
                if (disp_source.socket >= 0) {
                    if (dispsubscribe(disp_source.socket,subs) > 0 &&
                        dispalways(disp_source.socket) > 0 &&
                        dispsend(disp_source.socket,DISPTAG_MyId,srcid,strlen(srcid)) > 0)
                    {
                        rPrintf("Connection to source dispatcher %s restored\n",
                                disp_source.hostname);
                    } else {
                        dispdrop(disp_source.socket);
                        disp_source.socket = -1;
                    }
                }
                if (disp_source.socket < 0) {
                    /* try again later */
                    disp_source.retry_time = double_time() + CONNECT_SRC_RETRY;
                    continue;
                }
            }
            
            rc = dispcheck(disp_source.socket,htag,&hsize,0);
            if (rc == 0) {
                if (dataless > MAX_DATALESS2) {
                    usleep_ph(DATALESS_SLEEP_TIME2);    // sleep for a long time
                } else if (dataless > MAX_DATALESS1) {
                    usleep_ph(DATALESS_SLEEP_TIME1);    // nap for a bit
                    ++dataless;
                } else {
                    ++dataless;
                }
                continue;
            }
            dataless = 0;
            
            if (rc<0 || hsize<0) {
                broken(&disp_source);
                continue;
            }
    
            // is the packet too large?
            if (hsize >= (int)sizeof(buffer)) {
                rPrintf("Packet too large (%d) -- ignored\n", hsize);
                // skip this packet
                if (skipbwait(disp_source.socket, hsize) <= 0) {
                    broken(&disp_source);   // dispatcher problem
                }
                continue;
            }

            // get the packet from the source dispatcher
            if (getbwait(disp_source.socket,buffer,hsize) <= 0) {
                broken(&disp_source);   // dispatcher problem
                continue;
            }
            
            if (!strcmp(htag,"RAWDATA") && hsize<=EVENT_BUFFSIZE) {
                // give the PmtEventRecord to the target dispatchers
                for (x=disp_queue; x; x=x->next) {
                    if (hsize > x->event_size) {
                        // are we sending at full speed?
                        if (!x->dispatch_time) {
                            // Yes: send event immediately
                            sendToOne(x, htag, buffer, hsize);
                        } else {
                            // No: copy into the target dispatcher buffer
                            memcpy(x->buffer, buffer, hsize);
                            // Is this the first event into our buffer?
                            if (!x->event_size) {
                                // Yes: set time to dispatch the event
                                x->next_time = cur_time + x->dispatch_time;
                            }
                            // save the size of the event
                            x->event_size = hsize;
                        }
                    }
                }
            } else if (!strcmp(htag,"REDCMD")) {
                /* interpret dispatcher commands */
                buffer[hsize] = '\0';   // null terminate string
                if (!memcmp(buffer,"time=",5)) {
                    host = strstr(buffer,"host=");
                    if (host) {
                        host += 5;
                    } else {
                        host = DEFAULT_DISP;
                    }
                    // get internet address of specified host
                    iaddr = his_inet_addr(host);
                    if (!iaddr) {
                        rPrintf("TimeCmd: Can't resolve hostname '%s'\n", host);
                    } else {
                        // look for specified target hostname
                        for (x=disp_queue; ; x=x->next) {
                            if (!x) {
                                rPrintf("TimeCmd: Host '%s' is not a target\n",
                                        host,(iaddr>>24)&0xff,(iaddr>>16)&0xff,(iaddr>>8)&0xff,iaddr&0xff);
                                break;
                            }
                            if  (x->iaddr == iaddr ||
                                // loopback address is the same as 'my_addr'
                                (x->iaddr==LOOPBACK && iaddr==my_iaddr) ||
                                (x->iaddr==my_iaddr && iaddr==LOOPBACK))
                            {
                                x->dispatch_time = atof(buffer+5);
                                rPrintf("TimeCmd: Target %s period set to %.2g sec\n", x->hostname, x->dispatch_time);
                                break;
                            }
                        }
                    }
                } else {
                    rPrintf("Unknown redispatcher command '%s'\n", buffer);
                }
            } else if (!strcmp(htag,"STATUS")) {
                /* do not retransmit STATUS data */
                continue;
            } else {
                /* retransmit all other packet types */
                sendToAll(htag,buffer,hsize);
            }
        }
    }
    dropAll();
    return 0;
}
