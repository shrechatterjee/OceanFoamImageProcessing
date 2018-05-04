# OceanFoamImageProcessing
C++ code to find ROI's on Ocean surface for EPSRC project
Foamcam Code Description

---------------------functions.hpp---------------------
Contains the functions that do the image processing.
Output is done using 'OpData' objects. See documentation on 'opdata.h'.


Mat imreadRaw(string src_path);
	Reads in a raw .img file at the location specified by 'src_path'.
	Creates a Mat image object pixel by pixel using the data in the .img file.
	Returns the Mat object.

void convertRaw(string src_path, string op_path, string format);
	Converts a raw .img file at the location 'src_path' to a Mat object using the 'imreadRaw' function.
	Saves the Mat object as an image file at the location specified by 'op_path'.
	The the image file is of the file format  'format', if no format is given, the macro 'DEFAULT_OP_IMG_EXT' from opdata.h is used. Currently ".jpg".
	An 'OpData' object is used, as this allows the directory specified to be created if it does not exist. See 'opdata.h' documentation for more info.

void maskObjects(Mat& src);
	Takes input image 'src' and masks out static objects using the globally defined polys 'FRAME_POLY[]', and 'ROPE_POLY[]'.
	These polys are first used to create binary masks, which are then used to mask the source image.
	Masks the metal frame, and the ropes.
	
void extractWhitecaps(Mat& src, OpData& data);
	Takes input image 'src' and 'OpData' object 'data'.
	'data' is used to store any intermediate images, and output data. These are stored in 'data' to be written to disk at a later time.
	Blurs 'src' in order to make the function 'optimalThreshSubimgs' work better.
	'optimalThreshSubimgs' is used to extract the whitecaps from the image, creating a corresponding whitecap mask, false whitecaps may also be detected at this stage.
	The whitecap mask is morphologically closed in order to remove holes caused by raindrops.
	The function 'maskObjects' is used to remove false whitecaps caused by the static objects.
	The 'maskObjects' function inadvertently separates the sky into two separate regions with the rope poly.
	This is undesirable as it must be detected as one region in order to remove it later. In order to overcome this, a white line is drawn on the whitecap mask, bridging the two sky sections.
	This is not an elegant solution, but it works.
	The contours of the whitecap mask are found, in order to allow false whitecaps to be removed.
	Any contour containing less than 'min_contour_size' pixels is removed, as these contours are only rain and noise.
	The histograms of each region on the source image 'src' covered by a contour are calculated using  the function 'getHist'.
	The most common grey level for each histogram is calculated using 'getHistPeak'
	If this peak grey level is below the average, or below a minimum value 'peak_min', then the corresponding contour is removed.
	By this point the only contours remaining should correspond to whitecaps. The next step is to improve these contours.
	
	
void removeBarrelDist(Mat& src);
	Takes input image 'src' and uses OpenCV camera calibration to remove fisheye effect. ---------------------NOT YET IMPLIMENTED---------------------


void findSkeleton(Mat& src, Mat& dst);
	---------------------UNUSED---------------------
	Original purpose was to find the skeletons of the ropes  in order to create a mask for them.
	This method is no longer used, as it is now unnecessary.


void divideIntoSubimgs(Mat& src, vector<Mat>* op, int n, int m);
	Takes input image 'src' and divides it into sub-images, which are stored in the vector of images 'op'.
	The arguments n, and m specify the number of sub-images the 'src' should be split into, corresponding to number of sub-images in the x and y axis respectively.

	
void combineSubimgs(vector<Mat>* src, Mat& op, int n, int m); // the default argument is so that less args can be used for square images
	Combines the sub-images from the vector of images 'src' into a single image, which is then stored in 'op'.
	The arguments n, and m specify the number of sub-images 'src' is comprised of in the x and y axis respectively.
	To be used only to re-combine sub-images after the function 'divideIntoSubimgs' has been used to split an input image up.
	
	
void optimalThreshSubimgs(Mat& src, Mat& dst, int min_fg_bg_diff, int n, int m);
	Takes inpit image 'src' and splits it into sub-images using the function 'divideIntoSubimgs'.
	The arguments 'n' and 'm' are used by the functions 'divideIntoSubimgs' and 'combineSubimgs' and specify the number of sub-images that 'src' should be split into in the x and y axis respectively.
	Each sub-image has it's optimum threshold calculated using Otus's method, with the goal of splitting the sub-image into foreground (whitecap) and background (ocean).
	Mask images are created for the foreground and background, and the histogram of 'src' at the locations masked by the mask images are calculated using the function 'getHist'
	The function 'getHistPeak' is used on the foreground and background histograms in order to find their most common grey levels.
	If the most common foreground and background grey levels are different by less than 'min_fg_bg_diff' the sub-image is rejected, setting the flag 'ignore_subimg' true. 
	The goal of this is to prevent sub-images that do not contain whitecaps from affecting the output image 'dst' by showing patches of ocean as false whitecaps.
	The boundaries of the sub-images and their neighbours are then compared in order to look for sharp straight transitions along the boundary.		---------------------WORK IN PROGRESS---------------------
	Sharp transitions indicate false whitecaps, as real whitecaps tend not to have sharp, straight edges.
	Sub-images with such transitions are rejected, setting the flag 'ignore_subimg' true. 
	If the sub-image is to be rejected, (indicated by the 'ignore_subimg' flag) then it masked all black.
	The masked and filtered sub-images are then combined into a single image using the 'combineSubimgs' function, the result is stored in 'dst'.


void getHist(Mat& src, Mat& dst, Mat& mask);
	Computes the histogram of the input image 'src' and stores it in 'dst'.
	The input mask 'masp' can be used to specify the region in 'src' for which the histogram should be calculated.
	
	
void showHist(Mat& hist);
	Debug tool.
	Used to graphically display the input histogram 'hist'.
	
	
float getHistVariance(Mat& hist);
	---------------------UNUSED---------------------
	Calculates the variance of the input histogram 'hist' and returns it.
	Originally used in 'optimalThreshSubimgs' to detect whether a sub-image contained a whitecap or not, by looking at it's variance.
	This method was found to be too unreliable, and is no longer used.
	
	
int getHistPeak(Mat& hist);
	Calculates the most common grey level of the input histogram 'hist' and returns it.
	

Mat overlayContours(Mat& src, vector<vector<Point> >* contours);
	Debug tool/ output image generator.
	Takes the vector of of contours 'contours' and draws each of them on a colour (bgr) copy of the the input image 'src', in red.
	Uses the function 'findContourCentroids' in order to draw the centroids of each contour on the colour (bgr) copy of the the input image 'src', in blue.
	returns the resulting annotated input image copy.
	The copy is created so that the original image 'src' is not modified.

	
void path(Mat& img, Point* pts, int npts, const Scalar& color, int thickness, int lineType, int shift);
	Debug tool.
	Draws a path on the input image 'img' between the points in the input poly 'pts'.
	'npts' specifies the number of points in the 'img'.
	Every other argument is required by the OpenCV function 'line' which draws a line between two points.
	Used to make errors in the pre-defined polys 'FRAME_POLY[]', and 'ROPE_POLY[]' easier to find.
	
	
void labelPolyPoints(Mat& img, Point* pts, int npts, const Scalar& color, double text_scale);
	Debug tool.
	Marks every point in the input poly 'pts' with a circle, and writes the index of the point in the poly on input image 'img'.
	Every other argument is required by the OpenCV function 'circle' which draws a circle on an input image.
	Used to make errors in the pre-defined polys 'FRAME_POLY[]', and 'ROPE_POLY[]' easier to find.
	
	
void showMaskOverlay(Mat& src, Mat& mask);
	Debug tool/ output image generator.
	Draws the outline of regions in the input mask 'mask' on a copy of the input image 'src', in red.
	Used to show show which regions are covered by 'mask on the input image 'src'.
	
	
void findContourCentroids(vector<vector<Point> >& contours, vector<Point>& output_array);
	Finds the centroid points of each contour in the vector of contours 'contours' and stores them in the vector of points 'output_array'.
	The convex hull of each centroid is used to calculate it's centroid.
	
	
bool testIfProcessable(Mat& img);
---------------------WORK IN PROGRESS---------------------
	Test's whether the input image 'img' can be processed or not.
	Uses the function 'removeBarrelDist' to remove the fisheye distortion in the image. The horizon is now straight, rather than curved.
	Finds the gradient of the fisheye corrected image.
	Thresholds the gradient image in order to only keep sharp transitions in grey level in the fisheye corrected image.
	These are definite edges, one of which being the horizon.
	Uses a Hough Line Transform in order to detect the horizon edge.
	If this edge is found correctly the image is deemed processable, otherwise the horizon is obscured, and the image is deemed not processable. 
	returns true if 'img' is deemed processable, and false otherwise.
	
	
---------------------opdata.h---------------------
OpData objects are used in order to store output images and data.
They also save this data to disk, and create the file structure required for the TEABAG GUI program.
Directory creation is platform dependant. Currently only the code has only been tested on Windows, but code is written to implement this feature on unix based systems.

--------public--------

OpData(string new_src_img_path);
OpData(string new_src_img_path, Mat& new_src_img);
	Constructor, 'new_src_img_path' takes the path to the raw .img file that is being proccessed. This is required in order to create the file structure and output files.
	Optionally an image 'new_src_img' can be used. This allows the OpData object to also save a copy of the source image (in a specified format) in the same directory as it's output data.


~OpData();
	Standard destructor.
	Destroys all the stored Mat image objects and data fields, then destroys all private variables.


void addImg(string name, Mat& img);
	Stores the image 'img' with the file name 'name'.


void addField(string field, string value);
	Stores the field 'field' with the file value 'value'.

void addField(string field, int value);
	Same as other 'addField' function, except that takes an integer, rather than string value.
	
	
bool save(string dest_dir = "", string format = DEFAULT_OP_IMG_EXT);
	Saves all stored data and images.
	Optionally the  destination directory can be specified with "dest_dir", if not the directory of the source .img file is used.
	Optionally the image format can be specified, if not the default 'DEFAULT_OP_IMG_EXT' is used. DEFAULT_OP_IMG_EXT = ".jpg"
	

bool saveImg(string dest_dir = "", string format = DEFAULT_OP_IMG_EXT);
	Same functionality as 'save' function, except that it doesn't create save the data fields, only only the image 'new_src_img' given in the constructor.
	Primarily used for the 'convertRaw' function in 'functions.hpp'
