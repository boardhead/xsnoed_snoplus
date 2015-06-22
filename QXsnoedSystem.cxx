////////////////////////////////////////////////////////////
// An object derived from TUnixSystem which allows proper //
// handling of events for integration of xsnoed into ROOT //
////////////////////////////////////////////////////////////
//*-- Author :	Phil Harvey - 11/26/98

#include "QXsnoedSystem.h"
#include "TOrdCollection.h"
#include "TError.h"
#include "xsnoed.h"
#include "xsnoedstream.h"

ClassImp(QXsnoedSystem)

// ROOT changed the TSystem interface as of version 4.
// I use the "HOWMANY" macro to determine which version this is
// to allow QSNO to be compiled with older versions of ROOT.
// Of course, this is likely to break (again) with future ROOT
// releases, but we are stuck with this problem since Rene doesn't
// seem to want to put a hook into TSystem so I can properly
// patch the behaviour of DispatchOneEvent() - PH 02/24/04

//------------------- Unix TFdSet ----------------------------------------------
#ifndef HOWMANY
#define ROOT4   // turn on our ROOT version 4 code - PH
#   define HOWMANY(x, y)   (((x)+((y)-1))/(y))

const Int_t kNFDBITS = (sizeof(Long_t) * 8);  // 8 bits per byte
#ifdef FD_SETSIZE
const Int_t kFDSETSIZE = FD_SETSIZE;          // Linux = 1024 file descriptors
#else
const Int_t kFDSETSIZE = 256;                 // upto 256 file descriptors
#endif


class TFdSet {
private:
   ULong_t fds_bits[HOWMANY(kFDSETSIZE, kNFDBITS)];
public:
   TFdSet() { memset(fds_bits, 0, sizeof(fds_bits)); }
   TFdSet(const TFdSet &org) { memcpy(fds_bits, org.fds_bits, sizeof(org.fds_bits)); }
   TFdSet &operator=(const TFdSet &rhs) { if (this != &rhs) { memcpy(fds_bits, rhs.fds_bits, sizeof(rhs.fds_bits));} return *this; }
   void   Zero() { memset(fds_bits, 0, sizeof(fds_bits)); }
   void   Set(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         fds_bits[n/kNFDBITS] |= (1UL << (n % kNFDBITS));
      } else {
         ::Fatal("TFdSet::Set","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
      }
   }
   void   Clr(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         fds_bits[n/kNFDBITS] &= ~(1UL << (n % kNFDBITS));
      } else {
         ::Fatal("TFdSet::Clr","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
      }
   }
   Int_t  IsSet(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         return (fds_bits[n/kNFDBITS] & (1UL << (n % kNFDBITS))) != 0;
      } else {
         ::Fatal("TFdSet::IsSet","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
         return 0;
      }
   }
   ULong_t *GetBits() { return (ULong_t *)fds_bits; }
};

#endif // HOWMANY

//______________________________________________________________________________

QXsnoedSystem::QXsnoedSystem()
{
	// QXsnoedSystem Constructor
	// - upon return, gSystem is set to this
	
	mExitRootWithXsnoed = kFALSE;	// by default, don't exit root when xsnoed quits
	
	// replace the default system
	gSystem = this;
	
	gSystem->Init();
}

QXsnoedSystem::~QXsnoedSystem()
{
	// QXsnoedSystem Destructor
}

void QXsnoedSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   // Dispatch a single event.

   Bool_t pollOnce = pendingOnly;

   while (1) {
   
   	  // handle XSnoed events - PH 11/26/98
   	  xsnoed_service_all();

   	  // send work proc message to anyone with a work proc installed
	  Speak(kMessageWorkProc);

#if ROOT_VERSION_CODE >= ROOT_VERSION(5,20,00)

      // first handle any X11 events
      if (gXDisplay && gXDisplay->Notify()) {
         if (fReadready->IsSet(gXDisplay->GetFd())) {
            fReadready->Clr(gXDisplay->GetFd());
            fNfd--;
         }
         if (!pendingOnly) return;
      }

      // check for file descriptors ready for reading/writing
      if (fNfd > 0 && fFileHandler && fFileHandler->GetSize() > 0)
         if (CheckDescriptors())
            if (!pendingOnly) return;
      fNfd = 0;
      fReadready->Zero();
      fWriteready->Zero();

      if (pendingOnly && !pollOnce)
         return;

      // check synchronous signals
      if (fSigcnt > 0 && fSignalHandler->GetSize() > 0)
         if (CheckSignals(kTRUE))
            if (!pendingOnly) return;
      fSigcnt = 0;
      fSignals->Zero();

      // check synchronous timers
      Long_t nextto;
      if (fTimers && fTimers->GetSize() > 0)
         if (DispatchTimers(kTRUE)) {
            // prevent timers from blocking file descriptor monitoring
            nextto = NextTimeOut(kTRUE);
            if (nextto > kItimerResolution || nextto == -1)
               return;
         }

      // if in pendingOnly mode poll once file descriptor activity
      nextto = NextTimeOut(kTRUE);
      if (pendingOnly) {
         if (fFileHandler && fFileHandler->GetSize() == 0)
            return;
         nextto = 0;
         pollOnce = kFALSE;
      }

      // nothing ready, so setup select call
      *fReadready  = *fReadmask;
      *fWriteready = *fWritemask;

      int mxfd = TMath::Max(fMaxrfd, fMaxwfd);
      if (mxfd > -1) mxfd++;

      // if nothing to select (socket or timer) return
      if (mxfd == -1 && nextto == -1)
         return;

      fNfd = UnixSelect(mxfd, fReadready, fWriteready, nextto);
      if (fNfd < 0 && fNfd != -2) {
         int fd, rc;
         TFdSet t;
         for (fd = 0; fd < mxfd; fd++) {
            t.Set(fd);
            if (fReadmask->IsSet(fd)) {
               rc = UnixSelect(fd+1, &t, 0, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: read error on %d\n", fd);
                  fReadmask->Clr(fd);
               }
            }
            if (fWritemask->IsSet(fd)) {
               rc = UnixSelect(fd+1, 0, &t, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: write error on %d\n", fd);
                  fWritemask->Clr(fd);
               }
            }
            t.Clr(fd);
         }
      }

#elif defined(ROOT4)

      // first handle any X11 events
      if (gXDisplay && gXDisplay->Notify()) {
         if (fReadready->IsSet(gXDisplay->GetFd())) {
            fReadready->Clr(gXDisplay->GetFd());
            fNfd--;
         }
         if (!pendingOnly) return;
      }

      // check for file descriptors ready for reading/writing
      if (fNfd > 0 && fFileHandler && fFileHandler->GetSize() > 0)
         if (CheckDescriptors())
            if (!pendingOnly) return;
      fNfd = 0;
      fReadready->Zero();
      fWriteready->Zero();

      // check synchronous signals
      if (fSigcnt > 0 && fSignalHandler->GetSize() > 0)
         if (CheckSignals(kTRUE))
            if (!pendingOnly) return;
      fSigcnt = 0;
      fSignals->Zero();

      // check synchronous timers
      if (fTimers && fTimers->GetSize() > 0)
         if (DispatchTimers(kTRUE)) {
            // prevent timers from blocking file descriptor monitoring
            Long_t to = NextTimeOut(kTRUE);
            if (to > kItimerResolution || to == -1)
               return;
         }

      if (pendingOnly) return;

      // nothing ready, so setup select call
      *fReadready  = *fReadmask;
      *fWriteready = *fWritemask;

      int mxfd = TMath::Max(fMaxrfd, fMaxwfd) + 1;
      fNfd = UnixSelect(mxfd, fReadready, fWriteready, NextTimeOut(kTRUE));
      if (fNfd < 0 && fNfd != -2) {
         int fd, rc;
         TFdSet t;
         for (fd = 0; fd < mxfd; fd++) {
            t.Set(fd);
            if (fReadmask->IsSet(fd)) {
               rc = UnixSelect(fd+1, &t, 0, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: read error on %d\n", fd);
                  fReadmask->Clr(fd);
               }
            }
            if (fWritemask->IsSet(fd)) {
               rc = UnixSelect(fd+1, 0, &t, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: write error on %d\n", fd);
                  fWritemask->Clr(fd);
               }
            }
            t.Clr(fd);
         }
      }

#else // not ROOT4

      // then handle any X11 events
      if (gXDisplay && gXDisplay->Notify())
         return;

      // check for file descriptors ready for reading/writing
      if (fNfd > 0 && fFileHandler->GetSize() > 0) {
         TFileHandler *fh;
         TOrdCollectionIter it((TOrdCollection*)fFileHandler);

         while ((fh = (TFileHandler*) it.Next())) {
            int fd = fh->GetFd();
            if (fd <= fMaxrfd && fReadready.IsSet(fd)) {
               fReadready.Clr(fd);
               if (fh->ReadNotify())
                  return;
            }
            if (fd <= fMaxwfd && fWriteready.IsSet(fd)) {
               fWriteready.Clr(fd);
               if (fh->WriteNotify())
                  return;
            }
         }
      }
      fNfd = 0;
      fReadready.Zero();
      fWriteready.Zero();

      // check synchronous signals
      if (fSigcnt > 0 && fSignalHandler->GetSize() > 0)
         if (CheckSignals(kTRUE))
            return;
      fSigcnt = 0;
      fSignals.Zero();

      // check synchronous timers
      if (fTimers && fTimers->GetSize() > 0)
         if (DispatchTimers(kTRUE))
            return;

      if (pendingOnly) return;

      // nothing ready, so setup select call
      fReadready  = fReadmask;
      fWriteready = fWritemask;
      int mxfd = TMath::Max(fMaxrfd, fMaxwfd) + 1;
      fNfd = UnixSelect(mxfd, &fReadready, &fWriteready, NextTimeOut(kTRUE));
      if (fNfd < 0 && fNfd != -2) {
         int fd, rc;
         TFdSet t;
         for (fd = 0; fd < mxfd; fd++) {
            t.Set(fd);
            if (fReadmask.IsSet(fd)) {
               rc = UnixSelect(fd+1, &t, 0, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: read error on %d\n", fd);
                  fReadmask.Clr(fd);
               }
            }
            if (fWritemask.IsSet(fd)) {
               rc = UnixSelect(fd+1, &t, 0, 0);
               if (rc < 0 && rc != -2) {
                  SysError("DispatchOneEvent", "select: write error on %d\n", fd);
                  fWritemask.Clr(fd);
               }
            }
            t.Clr(fd);
         }
      }
#endif // ROOT4
   }
}

int QXsnoedSystem::UnixSelect(UInt_t nfds, TFdSet *readready, TFdSet *writeready,
                            Long_t timeout)
{
	// Override TUnixSystem member to allow shortening of
	// timeout for handling special xsnoed events
	if (GetNumListeners()) {
		// return immediately so we can handle our work procs
		timeout = 0;
	} else {
		// force reasonable timeout to handle 'hidden' xsnoed events - PH 11/26/98
		timeout = 100;
	}
	return(TUnixSystem::UnixSelect(nfds,readready,writeready,timeout));
}

