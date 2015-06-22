/*******************************************************************************
*									       *
* histoscope.h  -- interface for client histoscope routines: functions called  *
*		   by users to communicate with the Histo-Scope process and    *
*		   allows users to book, fill, and display histograms, ntuples,*
*		   and indicators.					       *
*									       *
*		   This file contains prototypes for the following functions:  *
*									       *
*     For all users:						               *
*									       *
*	 hs_initialize       - Initialize the Histo-Scope connection software  *
*			       and set up a potential connection to a Histo-   *
*			       Scope process.				       *
*	 hs_update	     - Updates the Histo-Scope display.		       *
*	 hs_complete	     - Closes all connections w/Histo-Scope processes. *
*	 hs_complete_and_wait- Waits until all Histo-Scopes finish scoping and *
*			       then performs an hs_complete.		       *
*	 hs_histoscope	     - Invokes Histo-Scope as a sub-process.           *
*	 hs_histo_with_config- Invokes Histo-Scope with a configuration file.  *
*	 hs_load_config	     - Asks Histo-Scope to load a configuration file.  *
*									       *
*     For HBOOK users:						               *
*									       *
*	 hs_hbook_setup       - Sets up all HBOOK histograms and ntuples in a  *
*			        top directory for use with Histo-Scope.	       *
*	 hs_reset_hbook_setup - Call this routine if you have previously       *
*			        called hs_hbook_setup and have booked new      *
*			        histograms or ntuples, or deleted, renamed,    *
*			        rebinned, or resetted existing ones.	       *
*									       *
*     For Histo-Scope item users:				               *
*									       *
*	 hs_create_1d_hist   - Books a one-dimensional histogram.	       *
*	 hs_create_2d_hist   - Books a two-dimensional histogram.	       *
*	 hs_create_ntuple    - Defines an n-tuple.  N-tuples have a specified  *
*			       number of variables and automatic storage       *
*			       allocated as they grow.			       *
*	 hs_create_indicator - Creates an indicator (a scalar value).          *
*	 hs_create_control   - Creates a control (a scalar value set by HS).   *
*	 hs_fill_1d_hist     - Adds a value to a one-dimensional histogram.    *
*	 hs_fill_2d_hist     - Adds a value to a two-dimensional histogram.    *
*	 hs_fill_ntuple      - Adds an array of real values to an n-tuple.     *
*	 hs_set_indicator    - Sets the value of an indicator.		       *
*	 hs_read_control     - Reads a control (a scalar value set by HS).     *
*	 hs_set_1d_errors    - Copies a vector of real numbers as error info   *
*	 hs_set_2d_errors    - Copies an array of real numbers as error info   *
*	 hs_reset	     - Resets all of the bins of a histogram to 0, or  *
*			       removes all of the data from an n-tuple, or     *
*			       sets an indicator to 0.			       *
*	 hs_save_file	     - Saves all current histograms, n-tuples, and     *
*			       indicators in a Histo-Scope-format file.        *
*	 hs_save_file_items  - Saves some current histograms, n-tuples, by     *
*			       uid/Category identification.                    *
*	 hs_delete	     - Deletes a histogram, n-tuple, or indicator.     *
*	 hs_delete_items     - Deletes a list of histograms, n-tuples, etc.    *
*									       *
*	 hs_1d_hist_block_fill - Replaces all of the accumulated bin and error *
*			         data in a 1D histogram & clears overflow bins *
*	 hs_2d_hist_block_fill - Replaces all of the accumulated bin and error *
*			         data in a 2D histogram & clears overflow bins *
*	 hs_1d_hist_num_bins - Returns the number of bins in a 1D histogram.   *
*	 hs_2d_hist_num_bins - Returns the number of bins in a 2D histogram.   *
*	 hs_1d_hist_range    - Returns the minimum & maximum horizontal limits *
*	 hs_2d_hist_range    - Returns the minimum & maximum horizontal limits *
*	 hs_1d_hist_bin_contents - Returns the bin data from a 1d Histogram    *
*	 hs_2d_hist_bin_contents - Returns the bin data from a 2d Histogram    *
*	 hs_1d_hist_errors   - Returns the error bar data from a 1d Histogram  *
*	 hs_2d_hist_errors   - Returns the error bar data from a 2d Histogram  *
*	 hs_1d_hist_overflows- Returns the overflow data from a 1d Histogram   *
*	 hs_2d_hist_overflows- Returns the overflow data from a 2d Histogram   *
*	 hs_num_entries      - Returns the # of fill operations for an item    * 
*	 hs_1d_hist_x_value  - Returns the value in the histogram bin that     *
*			       would be filled by the value x.		       *
*	 hs_2d_hist_xy_value - Returns the value in the histogram bin that     *
*			       would be filled by the value x,y.	       *
*	 hs_1d_hist_bin_value- Returns the value in the histogram bin referred *
*			       to by bin (or channel) number.		       *
*	 hs_2d_hist_bin_value- Returns the value in the histogram bin referred *
*			       to by bin (or channel) number.		       *
*	 hs_sum_histograms   - Creates a new Histogram (1D or 2D) whose data   *
*			       is the sum, bin by bin, of two histograms.      *
*	 hs_multiply_histograms - Creates a new Histogram (1D or 2D) whose     *
*			          data is the multiplication, bin by bin, of   *
*			          two histograms.			       *
*	 hs_divide_histograms - Creates a new Histogram (1D or 2D) whose data  *
*			        is the multiplication, bin by bin, of two      *
*			        histograms.				       *
*        hs_1d_hist_derivative - Creates a new Histogram (1D) whose data is    *
*                                is the 1st order derivative of a given hist.  *		*
*	 hs_sum_category     - Creates a collection of new histograms or       *
*			       NTuples based on two existing categories        *
*	 hs_sum_file         - Reads all of the items from a file, sums histo- *
*	 		       grams and/or merges Ntuples with those existing *
*	 		       under that top category and stores all newly    *
*	 		       created items in another category.	       *
*	 hs_id		     - Returns the histoscope ID of an item given UID  *
*	 hs_id_from_title    - Returns the histoscope ID of an item given title*
*	 hs_list_items       - Return/Fill a list of histoscope ID numbers     *
*	 hs_uid		     - Returns the User ID of an item given the histo- *
*	 		       scope ID					       *
*	 hs_category         - Returns the category of an item given the ID    *
*	 hs_title            - Returns the title of an item given the histo ID *
*	 hs_type             - Returns the type of the item given the histo ID *
*	 hs_delete_category  - Removes all items in the named category.	       *
*	 hs_num_items        - Returns the number of items defined so far.     *
*	 hs_change_category  - Renames the Category of an item		       *
*	 hs_change_title     - Renames the Title of an item		       *
*	 hs_change_uid       - Renames the UID of an item		       *
*	 hs_read_file        - Reads all of the items from a file prefixing    *
*	 		       their category with a specified top category.   *
*	 hs_read_file_items  - Reads items from a specific category in a file  *
*	 		       prefixing their category with a specified top   *
*	 		       category. 				       *
*	 hs_save_file_items  - Saves all items in a specified category and/or  *
*	 		       list of User IDs into a Histo-Scope-format file *
*	 hs_num_variables    - Returns the number of variables in an ntuple.   *
*	 hs_variable_name    - Gives the name of a variable for an NTuple      *
*	 hs_variable_index   - Returns the index (i.e. a column number in the  *
*	 		       NTuple) corresponding to a given variable       *
*	 hs_ntuple_value     - Returns a value from an n-tuple given a row and *
*	 		       column index.				       *
*	 hs_ntuple_contents  - Returns all of the data in an n-tuple.	       *
*	 hs_row_contents     - Returns the contents of a specified row (entry) *
*	 		       of an NTuple.
*	 hs_column_contents  - Returns the contents of a specified column      *
*	 		       (variable) of an NTuple.
*	 hs_merge_entries    - Creates a new Ntuple from two existing NTuples. *
*	 hs_1d_hist_minimum  - Gives the coordinate and bin content where the  *
*	 		       1d histogram data reaches the minimum extremum. *
*	 hs_2d_hist_minimum  - Gives the coordinate and bin content where the  *
*	 		       2d histogram data reaches the minimum extremum. *
*	 hs_1d_hist_maximum  - Gives the coordinate and bin content where the  *
*	 		       1d histogram data reaches the maximum extremum. *
*	 hs_2d_hist_maximum  - Gives the coordinate and bin content where the  *
*	 		       2d histogram data reaches the maximum extremum. *
*	 hs_1d_hist_stats    - Calculates the mean and standard deviation.     *
*	 hs_2d_hist_stats    - Calculates the mean and standard deviation.     *
*	 hs_hist_integral    - Calculate the integral (the sum of the contents *
*	 		       times the binwidth)			       *
*	 hs_hist_set_gauss_errors - Calculate and store Gaussian errors.       *
*									       *
* Copyright (c) 1993, 1994 Universities Research Association, Inc.	       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warranty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* June 1, 1992								       *
*									       *
* Written by Joy Kyriakopulos						       *
*									       *
* Modifications for Histo-Scope V2:					       *
*	Added: hs_create_control, hs_read_control,			       *
*	       hs_create_trigger, hs_check_trigger,			       *
*	       hs_set_1d_errors, hs_set_2d_errors			       *
* 									       *
* Modifications for Histo-Scope V3 and Application interface by Paul Lebrun    *
*		hs_save_file_items and many others, see below		       *
*******************************************************************************/

#define HS_TYPES_DEFINED

typedef enum _hsGroupType { 
    HS_INDIVIDUAL, HS_MULTI_PLOT, HS_OVERLAY_PLOT
} hsGroupType;

typedef enum _hsItemType {
    HS_1D_HISTOGRAM, HS_2D_HISTOGRAM, HS_NTUPLE, HS_INDICATOR, HS_CONTROL,
    HS_TRIGGER, HS_NONE
} hsItemType;

typedef enum _hsErrorType {
    HS_NO_ERRORS, HS_POS_ERRORS, HS_BOTH_ERRORS, HS_ITEMNOTFOUND_ERRORS
} hsErrorType;

typedef enum _hsErrorBars {
    HS_NO_ERROR_BARS, HS_DATA_ERROR_BARS, HS_GAUSSIAN_ERROR_BARS
} hsErrorBars;

void hs_initialize(char *id_string);
void hs_update(void);
void hs_complete(void);
void hs_complete_and_wait(void);
void hs_histoscope(int return_immediately);
void hs_histo_with_config(int return_immediately, char *config_file);
int hs_num_connected_scopes(void);
void hs_load_config_string(char *cfgStr);
void hs_load_config_file(char *cfgFile);
void hs_kill_histoscope(void);
int hs_create_1d_hist(int uid, char *title, char *category, 
			char *x_label, char *y_label,
			int n_bins, float min, float max);
int hs_create_2d_hist(int uid, char *title, char *category,
			char *x_label, char *y_label,
			char *z_label, int x_bins, int y_bins, float x_min,
			float x_max, float y_min, float y_max);
int hs_create_ntuple(int uid, char *title, char *category,
			 int n_variables,char **names);
int hs_create_indicator(int uid, char *title, char *category,
			 float min, float max);
int hs_create_control(int uid, char *title, char *category, 
			float min, float max, float default_value);
int hs_create_trigger(int uid, char *title, char *category);
int hs_create_group(int uid, char *title, char *category, int groupType,
			int numItems, int *itemId, int *errsDisp);
void hs_fill_1d_hist(int id, float x, float weight);
void hs_fill_2d_hist(int id, float x, float y, float weight);
int hs_fill_ntuple(int id, float *values);
void hs_set_indicator(int id, float value);
void hs_read_control(int id, float *value);
int hs_check_trigger(int id);
void hs_set_1d_errors(int id, float *err_valsP, float *err_valsM);
void hs_set_2d_errors(int id, float *err_valsP, float *err_valsM);
void hs_reset(int id);
int hs_save_file(char *name);
void hs_delete(int id);
void hs_delete_items(int *ids, int num_ids);
void hs_hbook_setup(char *topDirectory);
void hs_reset_hbook_setup(char *topDirectory);
/***** API for HistoScope Client/Users. *****/
void hs_change_uid(int id, int newuid);
void hs_change_category(int id, char *newcategory);
void hs_change_title(int id, char *newtitle);
int hs_id(int uid, char *category);
int hs_id_from_title(char *title, char *category);
int hs_list_items(char *title, char * category, int *ids, int num, 
		  int matchFlg);
int hs_uid(int id);
int hs_category(int id, char *category_string);
int hs_title(int id, char *title_string);
int hs_type(int id);
int hs_read_file(char *filename, char *prefix);
int hs_read_file_items(char *filename, char *prefix, char *category,
	int *uids, int n_uids);
int hs_save_file_items(char *filename, char *category,
	int *uids, int n_uids);
void hs_delete_category(char *category);
int hs_num_items(void);
void hs_1d_hist_block_fill(int id, float *data, float *err, float *err_m);
void hs_2d_hist_block_fill(int id, float *data, float *err, float *err_m);
int hs_1d_hist_num_bins(int id);
void hs_2d_hist_num_bins(int id, int *num_x_bins, int *num_y_bins);
void hs_1d_hist_range(int id, float *min, float *max);
void hs_2d_hist_range(int id, float *x_min, float *x_max, float *y_min,
	float *y_max);
int hs_num_entries(int id);
void hs_1d_hist_bin_contents(int id, float *data);
int hs_1d_hist_errors(int id, float *err, float *err_m);
void hs_2d_hist_bin_contents(int id, float *data);
int hs_2d_hist_errors(int id, float *err, float *err_m);
void hs_1d_hist_overflows(int id, float *underflow, float *overflow);
void hs_2d_hist_overflows(int id, float overflows[3][3]);
float hs_1d_hist_x_value(int id, float x);
float hs_2d_hist_xy_value(int id, float x, float y);
float hs_1d_hist_bin_value(int id, int bin_num);
float hs_2d_hist_bin_value(int id, int x_bin_num, int y_bin_num);
void hs_1d_hist_minimum(int id, float *x, int *bin_num, float *value);
void hs_2d_hist_minimum(int id, float *x, float *y, int *x_bin_num,
	int *y_bin_num, float *value);
void hs_1d_hist_maximum(int id, float *x, int *bin_num, float *value);
void hs_2d_hist_maximum(int id, float *x, float *y, int *x_bin_num,
	int *y_bin_num, float *value);
void hs_1d_hist_stats(int id, float *mean, float *std_dev);
void hs_2d_hist_stats(int id, float *x_mean, float *y_mean, float *x_std_dev,
	float *y_std_dev);
float hs_hist_integral(int id);
void hs_hist_set_gauss_errors(int id);
int hs_sum_histograms(int uid, char *title, char *category,
                      int id1, int id2, float const1, float const2);
int hs_multiply_histograms(int uid, char *title, char *category,
                           int id1, int id2, float consta);
int  hs_divide_histograms(int uid, char *title, char *category,
                          int id1, int id2, float consta);
int hs_1d_hist_derivative(int uid, char *title, char *category, int id);
void hs_1d_hist_set_overflows(int id, float underflow, float overflow);
void hs_2d_hist_set_overflows(int id, float overflows[3][3]);
int hs_num_variables(int id);
int hs_variable_name(int id, int column, char *name);
int hs_variable_index(int id, char *name);
float hs_ntuple_value(int id, int row, int column);
void hs_ntuple_contents(int id, float *data);
void hs_row_contents(int id, int row, float *data);
void hs_column_contents(int id, int column, float *data);
int hs_merge_entries(int uid, char *title, char *category,
                                           int id1, int id2);
void hs_sum_category(char *cat_top1, char *cat_top2, char *prefixsum); 
void hs_sum_file(char *file, char *cat_top, char *prefixsum);
