#pragma once
#include "opencv.hpp"
#include <vector>
#include <memory>
using std::shared_ptr;
using cv::Mat;
using std::vector;
using cv::Point;
typedef shared_ptr<vector<Point> > PtrSegmentPoints;
typedef shared_ptr<vector<PtrSegmentPoints> > PtrSegments;
class SegmentationControl
{
public:
	/*segmentation type*/
	enum segmentationType { DUMMY_FIRST = 0, MEAN_SHIFT, SLIC_, DUMMY_LAST };

public:
	explicit SegmentationControl(const Mat& IMG);
	~SegmentationControl();
public:
	/*various segmentations*/
	void doMeanShiftSegmentation();
	void doSlicSegmentation();

	Mat& getMeanShiftSegResultRef();
	Mat& getSlicSegResultRef();
	/***********************/

	int getSegmentationType0basedIdx(segmentationType type);
	int getSegmentationTypeNum();
	Mat getSegmentationMatOfType(segmentationType type);
	PtrSegments getSegmentationSegmentsOfType(segmentationType type);
	int getSegmentationSegmentsNumOfType(segmentationType type);
	PtrSegmentPoints getSegmentOfType(segmentationType type, int segment_Idx);
	int getLabel(Mat Img, int x, int y);

	void setSegmentationTypeNum(segmentationType type, int count);
	void setSegmentationMatOfType(segmentationType type, Mat& IMG);
	void setSegmentationSegmentsOfType(segmentationType type, PtrSegments segs);
	void setSegmentationNumOfType(segmentationType type, int num);
	void setSegmentOfType(segmentationType type, int segment_Idx, PtrSegmentPoints segPts);

	void processSegmentation(segmentationType type);
	void processSegmentations();

private:
	bool init();
	void updateSegmentsStorageWithLabelImage(segmentationType type, Mat& labelImg);//update _vecAllSegmentationTypeSegments and _vecSegmentationSegmentsNum
	void updateSegmentsStorageWithLabelImage(segmentationType type, int* labelImg, int width, int height);//update _vecAllSegmentationTypeSegments and _vecSegmentationSegmentsNum
	Mat& getMatRef(int idx);
	PtrSegments& getSegmentsPtrRef(int idx);
	int& getSegmentsNumRef(int idx);
private:
	Mat _originalIMG;
	vector<Mat> _vecAllSegmentationTypeMats;
	vector<PtrSegments> _vecAllSegmentationTypeSegments;
	vector<int> _vecSegmentationSegmentsNum;

};

