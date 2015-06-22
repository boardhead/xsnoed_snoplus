$ ! This DCL file links xsnoed.exe
$ ! Note that to use xsnoed with command-line parameters,
$ ! one must define it as a foreign command
$ ! i.e.
$ !  xsnoed :== $u1:[tagg.xsnoed]xsnoed.exe
$ !
$ ! N. Tagg, Guelph
$
$! Define libraries:
$ libs := sys$library:decwindows.olb/lib
$
$! Define objects:
$ objs := -
xsnoed.obj, -
xsnoedstream.obj, -
imagedata.obj,-
cutils.obj,-
calibrate.obj,-
fitter.obj,-
matrix.obj,-
openfile.obj,-
oca.obj,-
PImageWindow.obj,-
PScrollingWindow.obj,-
PScrollBar.obj,-
PWindow.obj,-
PImageCanvas.obj,-
PScale.obj,-
PMapImage.obj,-
PResourceManager.obj,-
PSpeaker.obj,-
PListener.obj,-
PHitInfoWindow.obj,-
PFitterWindow.obj,-
PProjImage.obj,-
PFlatImage.obj,-
PCrateImage.obj,-
PHistImage.obj,-
PEventInfoWindow.obj,-
PEventControlWindow.obj,- 
POpticalWindow.obj,- 
PMonteCarloWindow.obj,-
PZdabFile.obj,-
PZdabWriter.obj,-
PSettingsWindow.obj,-
PEventHistogram.obj,- 
PEventTimes.obj,- 
PMenu.obj,- 
PUtils.obj,-
XSnoedImage.obj,- 
XSnoedWindow.obj
"
$
$ link_flags = -
/executable=xsnoed.exe,-
/decc/prefix_library_entries=all_entries
$
$ dir 'objs'
$ link  /executable=xsnoed.exe 'objs' + 'libs'
