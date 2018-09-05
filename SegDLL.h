#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SEGDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SEGDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SEGDLL_EXPORTS
#define SEGDLL_API __declspec(dllexport)
#else
#define SEGDLL_API __declspec(dllimport)
#endif

// This class is exported from the SegDLL.dll
class SEGDLL_API CSegDLL {
public:
	CSegDLL(void);
	// TODO: add your methods here.
};

SEGDLL_API int fnSegDLL(uchar *b, uchar *g, uchar *r, uchar *map_counts, uchar *map_lifetime, float lifetime, int min_thres, int max_thres, int rin, float *center_x, float *center_y, float *rout);
int limit_thresholds(int value);
bool compareContourAreas(vector<Point> contour1, vector<Point> contour2);