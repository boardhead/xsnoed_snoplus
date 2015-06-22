$ ! This DCL command file will compile C anc C++ files
$ ! with the correct switches to make the 'standalone' xsnoed
$ ! on the VAX system.
$ !
$ ! N. Tagg, Guelph
$
$ c_defs = "(FITTR, OPTICAL_CAL, VAX, NO_HELP, SWAP_BYTES, USE_FTIME,"
$ c_defs = c_defs + "NO_DISPATCH,IGNORE_DISP_ERRORS, STAT_BUG "
$ c_defs = c_defs + ")"
$
$ c_flags =  "/debug=all /noOptimize /define="+c_defs
$ c_flags = c_flags + -
"/include_directory=([tagg.xsnoed.disp_include],"+-
"[tagg.xsnoed.qsno_include],"+-
"[tagg.xsnoed.root_include]) " + -
"/warnings=(disable=FOUNDCR)"
$
$ cc_flags = "/standard=common"
$
$   
$   file = f$search(p1)
$   if file .eqs. "" then goto done
$   spec = f$parse(file,,,"TYPE")
$
$   write sys$output "Compiling " + file
$
$   if (spec .eqs. ".CXX") 
$   then
$     cxx 'c_flags' 'file'
$   endif
$
$   if (spec .eqs. ".C")
$   then
$     cc 'c_flags' 'cc_flags' 'file'
$   endif
$
$
