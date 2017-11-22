/*
** File:            xsnoed_version.h
**
** Description:     xsnoed version number definition
**
** Versions:
**
** 3.2.0 (12/06/98) - PH Added QSnoed ROOT object.
** 3.2.5 (12/10/98) - PH Write and read binary versions of database and calibration file.
** 3.2.6 (12/17/98) - PH Added snoman calibration and dynamic water levels in ROOT version.
** 3.2.7 (01/14/99) - PH Added calibrated PmtEventRecord support & NO_DISPATCH option.
** 3.2.8 (01/19/99) - PH Added additional NHIT logic to Event Control.
** 3.5.0 (01/29/99) - PH Upgraded back to C++.
** 3.5.1 (02/26/99) - PH Added monte carlo track display and extended PmtEventRecord format.
** 3.5.2 (04/13/99) - PH Switched back to 'w' dispatcher subscription to avoid memory leak.
** 3.5.3 (04/16/99) - PH Changed history buffer size to 50 events.
** 3.5.4 (05/05/99) - PH Added Peak/Int/Diff display.
** 3.5.5 (05/13/99) - PH Changed zdab reader to allow events with NHIT up to 10,000.
**                       Added File/Settings menu with hex-GTID and UTC-time options.
** 3.5.6 (05/17/99) - PH Mouse button 3 now selects hits to be discarded from a fit.
**                       Can now type in fit position, direction and time.
** 3.5.7 (05/27/99) - PH Added optional XYZ hit display.
**                       Added rotation angle display.
** 3.6.0 (06/03/99) - PH Added ability to hide under/overscale hits.
**                       Added display of number of visible hits in Event Info window.
**                       Added "Apply" button to histogram scales window.
**                       Added ability to save events to ZDAB file.
**                       Changed Goto logic.  Changed About box.
** 3.6.1 (06/08/99) - PH Fixed 'Josh' bar problem introduced in 3.6.0 and changed so
**                       the rotation is in the same sense as horizontal scrollbar
**                       in projection window.  Fixed zdab writer bugs.
**                       Added ability to write to 4 separate zdab files.
**                       Added output zdab filenames to XSnoed resources.
** 3.6.2 (06/14/99) - PH Changed calibration code to use new database file.
** 3.6.3 (06/16/99) - PH Centralized duplicated calibration code into calibrate.c.
**                       Added Peak/Int/Diff support to QEvent conversions.
** 3.7.0 (06/22/99) - PH Added Event Time window.
**                       Added new "future" history buffer and expanded buffers to 100 events.
**                       Added Settings window, and new hit size setting.
** 3.7.1 (06/30/99) - PH Added Peter Skensved's code to read titles file calibration constants.
** 3.7.2 (07/14/99) - PH Added Nathaniel Tagg's xsnoman code and VAX support.
**                       Added DEBUG_MEMORY code to check for memory leaks.
** 3.7.3 (08/13/99) - PH Added Next/Prev event time display.
** 3.7.4 (08/30/99) - PH Changed XmString handling to fix memory leaks.
** 3.7.5 (09/02/99) - PH Added prev/next times to event history buffer.
** 3.7.6 (09/10/99) - PH Added cone angle to Fitter window.
** 3.8.0 (09/14/99) - PH Added ability to save xsnoed settings.
** 3.8.1 (09/17/99) - PH Fixed some problems associated with opening extra main windows.
** 3.8.2 (09/20/99) - PH Added print capability.
** 3.8.3 (09/22/99) - PH Added print image (vector postscript) capability.
** 3.8.4 (10/01/99) - PH Save actual tube radii for readout of tube positions (affects
**                       format of binary database file -- old files are now invalid).
**                       No longer restores trigger settings and other potentially
**                       confusing settings from resources.
** 3.8.5 (10/08/99) - PH Fixed Hit Info XYZ display for Missing, Owl and Low Gain tubes.
**                       Added display of BUTTS and Neck tubes. No longer displays Missing
**                       tubes in 3-D image, or Missing, BUTTS or neck tubes in projection
**                       and flat images.  Changed name of Missing tubes to FECD.  Added
**                       support for new format calibration files.  Expanded NHIT cut to
**                       accept PMT type keywords.  Updated database.dat and changed format
**                       of binary database and calibration files.  Fixed potential crash
**                       problem introduced by trying to hide the cursor in text edit fields.
** 3.8.6 (10/13/99) - PH Added owl positions to database.dat.  Save dispatcher name setting.
**                       Moved Save Settings button from Settings window to File menu.
**                       Made "Save Settings on Quit" off by default.
** 3.8.7 (10/15/99) - PH Create PLabel object to avoid unnecessary label updates.
**                       Added Rch file support to ROOT version.
** 3.8.8 (10/19/99) - PH Major changes to histogram scales window allowing it to be used
**                       for all histograms.  Can now change scales of Event Times histogram.
** 3.9.0 (10/22/99) - PH Structural change -- moved xsnoed resources into ImageData structure.
**                       Cleaned up ImageData structure.  Some resource name changes.
**                       Fixed fitter problem -- was using the speed of light for air fill.
** 3.9.1 (10/25/99) - PH Added Rch overlay feature.  Fixed problem with selecting image for
**                       "Print Image" if window manager doesn't raise window on mouse click.
** 3.9.2 (10/27/99) - PH Changed Motif object names to simplify XSnoed resources.  Now
**                       generates new home resource file if existing copy is out of date.
**                       Removed Elliptical Cosine projection and added Elliptical Mollweide.
**                       Removed Polar Cosine projection and added Polar Equal Area.
** 3.9.3 (11/03/99) - PH Darkened detector frame lines in postscript output.
**                       Added -n command line option.
** 3.9.4 (11/11/99) - PH Changed Event Info window to update text more quickly.
**                       Changed name of settings file from "XSnoed" to ".XSnoed".
**                       Goto Run now opens a new run file if standard run filenames are used.
**                       Fixed bug which could cause crash while scanning a zdab file
**                       if there are more than 100 consecutive orphans.
** 3.9.5 (11/30/99) - PH Added Run Info Window.  Added "Show ZDAB Record Info" option.
**                       Changed trigger name EXT2 to HYDRO.
** 4.0.0 (12/13/99) - PH Added event labels to image windows. Added extra options to Print
**                       dialog.  Changed some menus.  Added "All" and "None" to Hits submenu.
**                       Added "Sudbury" time option and option to allow disabling of angle
**                       display.  Major internal changes to window update mechanism.
** 4.0.1 (12/14/99) - PH Added greyscale display option.  Moved "Print Image" up in File menu.
** 4.0.2 (12/17/99) - PH Changed window hierarchy to fix LESSTIF "dead window" problem (this
**                       results in a loss of window position settings saved by older versions).
**                       Changed resource version to 3.1 to accomodate related resource changes.
**                       Fixed some minor label update problems.  Added Printer/File selection
**                       to print dialog.  Small changes to load settings logic.  Switched
**                       positions of Settings OK and Cancel.  Show "-" instead of "H000-00"
**                       as the panel number for Neck, BUTTS and FECD channels.
** 4.0.3 (12/22/99) - PH Added label "big font" feature. Fixed bug which caused crash on
**                       startup if colors couldn't be allocated.
** 4.0.4 (12/23/99) - PH Fixed ordering of external trigger bit names.
** 4.0.5 (01/03/00) - PH Fixed window resize problem in Solaris versions.
** 4.0.6 (01/10/00) - PH Added ability to dump dispatched record info. Changed "Show ZDAB Info"
**                       to "Dump Record Info" because it now works for dispatched data too.
**                       Changed "Run Info" window to "Record Info" window, and added display
**                       of other record types.  Finally fixed time offset bug (introduced
**                       in version 3.9.0) that caused calibrated times to be negative.
** 4.0.7 (01/27/00) - PH Added warning dialog when overwriting Print file.  Added NO_FORK
**                       compiler option.  Reverted back to the window hierarchy of 4.0.1 due
**                       to problems on some systems.  Updated resource version to 3.3 because
**                       of the hierarchy change.  Changed histogram auto-scaling behaviour.
**                       Removed XSNOMAN-specific code from XSNOED release.  Added extra hit
**                       data sub-field to extended PmtEventRecord format, and added dynamic
**                       entries to Data menu and Hit Info window to support new data.  Added
**                       xsnoed_replace() library interface routine.  Added NO_FITTER_DAT
**                       compiler option.
** 4.0.8 (02/03/00) - PH Added Goto Time option.  Removed OPTICAL_CAL from standard release.
** 4.1.0 (02/11/00) - PH Added ASCII Output window.  Changed attachment of some text entry
**                       fields to allow them to grow in width with the window.  Added Tab
**                       feature to Event Control window NHIT and Trigger fields.  Added Sun
**                       Angle (%sa) to the list of label format specifications.
** 4.1.1 (02/24/00) - PH Added "-s" command line option.  Added code for DEMO_VERSION.
**                       Added "Echo Main Display" option for auxiliary displays.
** 4.1.2 (03/06/00) - PH Added Colors window.  Tweaked postscript output of 3-D image.
** 4.1.3 (03/10/00) - PH Fixed display of EPED type in Record Info window.  Draw directly to
**                       screen if memory not available for backing pixmap.  Changes to image
**                       update mechanism.  Minor changes to Colors window.
** 4.1.4 (03/13/00) - PH Fixed some bugs in the print feature which could intermittently cause
**                       the printing to fail.
** 4.1.5 (03/21/00) - PH Fixed bug which could cause crash if Hit Info window is opened while
**                       extra hit data is available (affects XSNOMAN version only).  Clear Hit
**                       Info when the mouse moves out of the projection window.  Added ability
**                       to display extra event data.  Changed length of extra hit data 'name'
**                       from 16 to 24 characters in the extended PmtEventRecord and added
**                       optional format specification.  Added sub-run number (%sr) to the list
**                       of label format specifications.  Changed all transient shells to top
**                       level shells in yet another attempt to fix the Lesstif "dead window"
**                       problem.  Changed color resource settings to use newer rgb syntax.
**                       Weighted greyscale levels by luminance instead of intensity.  Disabled
**                       ineffective menu items.  Added menu accelerators.  Changes to event
**                       window layout.  Changes to window geometry logic to fix quirks with
**                       some window managers.
** 4.1.6 (03/25/00) - PH Fixed bug in menu accelerators.  Changed Event Control window layout
**                       to fix ugliness on Solaris systems.  Patched problem where the
**                       restored window locations were offset by the window manager frame
**                       size on some systems.  Changed XSNOMAN version so windows specified
**                       in settings are opened at startup.  Changed resource version to 3.4
**                       to propagate changes to resource font settings.
** 4.1.7 (03/31/00) - PH Added ability to display all events with specific trigger bits set,
**                       regardless of the NHIT threshold (new Event Control '!' specifier).
** 4.1.8 (04/20/00) - PH Changed character to force specific triggers to be displayed from
**                       '!' to '*'.  Fixed color intensity slider behaviour for page up/down.
**                       Added "-d" command line option.  Added new trigger name "NONE" to
**                       distinguish true orphans from valid events with no trigger bits set.
** 4.1.9 (05/11/00) - PH Minor changes to fix some compiler incompatibilities.  Fixed bug
**                       which caused crash if the filename was empty when "Save" was pushed
**                       in the Event Control window.  Fixed bug which could cause crash
**                       when using "Goto" feature.
** 4.2.0 (05/16/00) - PH Added Dump Data window from Mark Howe's code.
** 4.2.1 (05/23/00) - PH Added display of total number of hits of each type to Event Info
**                       window and labels.  Changed histogram binning for integer data types.
**                       Changed makefiles to support new Linux compilers and newly available
**                       Open Group Motif for Linux.
** 4.2.2 (06/09/00) - PH Fixed bug where 'next' time sometimes wasn't updating properly.
**                       Save the filename for each event so the main window title always
**                       shows the proper filename for the currently displayed event.
** 4.2.3 (07/07/00) - PH Minor improvements to Monte Carlo data display.  Avoid unecessary
**                       updates of Event Info Prev/Next field.  Change continuous update
**                       strategy to always try to show next event matching trigger settings
**                       (this means that XSNOED may lag the live data by 100 events if
**                       the trigger threshold is low, but now it won't skip events in a
**                       burst).  Fixed initialization of log scale checkbox.
** 4.2.4 (07/10/00) - PH Un-do update change of 4.2.3 because display was falling too far
**                       behind during extended periods of high event rates.
** 4.2.5 (07/13/00) - PH Made hits on the front of the 3-D sphere solid instead of hollow.
**                       (OK, yes.  After 8 years I finally gave in and changed it!)  Added
**                       "load_trigger_settings" to resources to allow trigger settings to
**                       be loaded at startup.  Added handler for broken dispatcher pipes.
**                       Added THICK_SUN compiler option to draw double-width sun vector.
**                       Added Nathaniel's command line rotation option.
** 4.2.6 (08/29/00) - PH Fixed problem with non-zero orphan date/time in image labels.
** 4.3.0 (11/08/00) - PH Added new code to calibrate using official titles files, but left
**                       this feature disabled for now.  Changed lab rotation to new angle
**                       measured by gyrocompass.  Now reads and writes extended PMT event
**                       records (but only writes fit information).
** 4.3.1 (12/05/00) - PH Added SNODB Viewer window to ROOT version.
** 4.3.2 (02/15/01) - PH Clicking on image label now turns on/off label in individual images.
** 4.4.0 (05/16/01) - PH Added Animation window.
** 4.4.1 (05/17/01) - PH Added Apply Window checkbox to Animation window.
** 4.4.2 (08/08/01) - PH Fixed problem with colour scale when compiled with gcc 3.0
** 4.4.3 (10/26/01) - PH Added fit index to the fit name.  Added colour support for new
**                       hardware.
** 4.4.4 (01/13/03) - PH Fixed "endless resize" bug.  Added OS X 10.2 support.
** 4.4.5 (04/08/03) - PH Added source orientation readout to CAST record
** 4.5.0 (06/27/03) - PH Added NCD Info Window
** 4.5.1 (08/29/03) - PH Added NCD Scope Window and Trigger Scope Window
** 4.5.2 (10/10/03) - PH Updated header file to fix ZDAB read on on SWAP_BYTES systems
** 4.5.3 (10/16/03) - PH Fixed structure alignment crash bug in trigger scope code
** 4.5.4 (11/13/03) - PH Added synthetic NCD trigger bits.  FINALLY fixed Event Control
**                       window "dead button" problem!
** 4.5.5 (11/17/03) - PH Moved NCD logic event selection logic from trigger bits to nhit
** 4.6.0 (11/27/03) - PH Add NCD display to main 3D window
** 4.6.1 (12/12/03) - PH Changes to NCD mapping.  Fixed NCD Info display in sum mode.
**                       Fixed crash problem with dispatch data on Linux.
** 4.6.2 (01/13/04) - PH Fixed problem in count for NCD MUX threshold.  Attempt to fix
**                       crash of Linux version when viewing dispatched data.
** 4.6.3 (01/20/04) - PH Increase the number of NCD Shaper Slots (was 10, now 21)
** 4.7.0 (07/16/04) - PH Added NCD Histogram and NCD map windows.  Properly sum NCD hits.
**                       NCD Scope hits still not implemented.  Allow shift of histogram Y
**                       scales.  Changed trigger names: NCD->NCDMUX, EXT7->NCDSHAP.
** 4.7.1 (07/19/04) - PH Display latest NCD scope trace for all channels when in sum mode.
**                       Properly interpret NCD scope bits.  Improve NCD histogram scaling.
** 4.7.2 (07/22/04) - PH Added support for new NCD data types.
** 4.7.3 (07/23/04) - PH Changed ordering of Shaper/MUX/Scope entries in windows and menus.
**                       Changed handling of NCD hit mask bits to make more sense.
**                       Fixed problem when displaying multiple NCD scope traces.
** 4.7.4 (07/27/04) - PH Changes to histogram Y scales to fix arithmetic exception problem.
** 4.7.5 (07/29/04) - PH Clear NCD scope on Clear Event.  Fix handling of palette changes.
**                       Allow NCD Map to be displayed in main window.
** 4.8.0 (09/24/04) - PH Dynamically load PMT/NCD databases based on the event run number. 
**                       Added NCD Hit window.  Speed up drawing of NCD scope traces.
** 4.8.1 (09/30/04) - PH List contents of general NCD records, and add "General" hit type.
** 4.8.2 (10/14/04) - PH Added menu to NCD scope window.  Fixed maximum scale problem with
**                       histogram Y scales. Updated run and source bit names.
** 4.9.0 (10/18/04) - PH Added PMT/NCD logic to Event Control window.  Added PMT name to
**                       Hit Info window.  Save NCD scope menu settings and add scope scales
**                       to resources.  NCD data (but not summed NCD data) is now saved with
**                       a "Save" in the Event Control window.  Fixed problem in loading PMT
**                       barcode for neck tubes (database.dat.bin files should be rebuilt).
** 4.9.1 (11/02/04) - PH Fixed memory delete problem when closing Event Times window.
**                       Added new PMT and NCD database files and db_list.dat.  Fixed NCD
**                       positions in old default NCD database.  Updated printout of NCD
**                       livetime scalar record.
** 4.9.2 (11/10/04) - PH Changed definition of NCD livetime record start/middle/stop flags
** 4.9.3 (11/18/04) - PH Fixed bug in ZDAB reader which could cause the last few events in
**                       a file to be missed.
** 4.9.4 (12/03/04) - PH Fixed bug in Flat window which caused hits not to be displayed
**                       if a new database was loaded after opening the Flat window.
** 4.9.5 (01/04/05) - PH Fixed bug where NCD size gets reset if settings are cancelled.
**                       Added "Reset Event Sum" setting.
** 4.9.6 (02/09/05) - PH Fixed bug which could cause crash when resizing NCD Scope Window.
**                       Updated NCD tube maps.
** 4.9.7 (05/20/05) - PH Print proper scope trace when Print Image used on NCD Scope Window.
** 4.9.8 (01/03/06) - PH Added rope information to CAST record display
** 5.0.0 (09/08/11) - PH Added modifications for SNO+ version
** 5.0.1 (06/01/12) - PH Buffered dispatcher reading for higher data rates.
**                       Save CAEN Y scale limits and add CAEN channel labels to resources.
** 5.0.2 (11/07/12) - PH Clear CAEN histograms when "Clear Event" selected.
**                       Fixed problem with changing width of HitInfo window columns.
**                       Updated Sudbury DST changeover times (in Mar and Nov since 2007)
** 5.0.3 (03/24/14) - PH Added resource names to CAEN channel menu.  Added Full and Auto
**                       buttons to histogram scales dialog.  Fixed quirk with drawing bars
**                       of supressed-zero histogram.  
** 5.0.4 (12/19/16) - PH Added TUBII trigger info and hotkey for "Clear Event" menu item
** 5.0.5 (01/11/17) - PH Use SNO+ time zero of Jan 1, 2010
** 5.0.6 (04/07/17) - Ben Land - Patch race condition in dispatcher thread
** 5.0.7 (04/11/17) - PH Show RHDR GTID's in zdab dump
** 5.0.8 (05/18/17) - PH Update to SNO+ tube map
** 5.0.9 (05/31/17) - PH Added CAEN Scope summing feature
** 5.1.0 (06/13/17) - PH Fixed some potential crash problems
** 5.1.1 (07/19/17) - PH Fixed decoding of RHDR date/time for SNO+
** 5.1.2 (08/10/17) - PH Print CAST/SOSL sourceID as 2 16-bit integers
** 5.1.3 (08/11/17) - PH Added FECD trigger display to Event Info window
** 5.1.4 (08/17/17) - PH Added source name to CAST record display
** 5.1.5 (08/18/17) - PH Changed "EXT6" to "NO_CLK"
** 5.1.6 (11/21/17) - PH Updated manipulator rope names and added CAEN Log Color Sum option
*/

#define XSNOED_VERSION      "5.1.6"     // xsnoed version number

