$ ! This DCL command file will compile and link xsnoed.exe
$ ! 
$ ! N. Tagg, Guelph
$
$ ! Compile each source file.
$ @compile xsnoed.cxx
$ @compile xsnoedstream.cxx
$ @compile imagedata.cxx 
$ @compile cutils.c
$ @compile calibrate.cxx
$ @compile fitter.c
$ @compile matrix.c
$ @compile openfile.c
$ @compile oca.cxx
$ @compile PImageWindow.cxx
$ @compile PScrollingWindow.cxx
$ @compile PScrollBar.cxx
$ @compile PWindow.cxx
$ @compile PImageCanvas.cxx
$ @compile PScale.cxx
$ @compile PMapImage.cxx
$ @compile PResourceManager.cxx
$ @compile PSpeaker.cxx
$ @compile PListener.cxx
$ @compile PHitInfoWindow.cxx
$ @compile PFitterWindow.cxx
$ @compile PProjImage.cxx
$ @compile PFlatImage.cxx
$ @compile PCrateImage.cxx
$ @compile PHistImage.cxx
$ @compile PEventInfoWindow.cxx
$ @compile PEventControlWindow.cxx 
$ @compile POpticalWindow.cxx 
$ @compile PMonteCarloWindow.cxx
$ @compile PZdabFile.cxx
$ @compile PZdabWriter.cxx
$ @compile PSettingsWindow.cxx
$ @compile PEventHistogram.cxx 
$ @compile PEventTimes.cxx 
$ @compile PMenu.cxx 
$ @compile PUtils.cxx
$ @compile XSnoedImage.cxx 
$ @compile XSnoedWindow.cxx
$
$
$ @link
