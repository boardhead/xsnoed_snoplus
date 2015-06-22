#include "TROOT.h"
#include "TRint.h"
#include "QXsnoedSystem.h"
#include "QDispatch.h"

int main(int argc, char *argv[])
{
	extern int g_argc;
	extern char **g_argv;
	
	g_argc = argc;
	g_argv = argv;
	
	extern void  InitGui();
	static VoidFuncPtr_t initfuncs[] = { InitGui, 0 };
	
	TROOT theROOT("theRoot","xsnoed_root",initfuncs);
	

#ifdef R__UNIX
  	        if ( gSystem ) delete gSystem;
		gSystem = new QXsnoedSystem;	// install XSnoed system
	        printf("QXsnoedSystem installed.\n");
#endif

    TRint *theApp = new TRint("XSnoed ROOT", &g_argc, g_argv);

    // Init Intrinsics, build all windows, and enter event loop
    theApp->Run(kTRUE);
	
	printf("\n");
	
#ifndef NO_DISPATCH
	delete gDispatch;	// delete global dispatcher to avoid pipe errors
#endif
	
    printf("Done.\n");
}
