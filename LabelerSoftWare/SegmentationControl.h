#pragma once
#include <QObject>
#include "opencv.hpp"
#include <vector>
#include <memory>
using std::shared_ptr;
using cv::Mat;
using std::vector;
using cv::Point;
typedef vector<Point> SementPoints;
typedef shared_ptr<SementPoints> PtrSegmentPoints;
typedef vector<PtrSegmentPoints> Segments;
typedef shared_ptr<vector<PtrSegmentPoints> > PtrSegments;
class SegmentationControl:public QObject
{
	Q_OBJECT
public:
	/*segmentation type*/
	enum segmentationType { DUMMY_FIRST = 0, MEAN_SHIFT, SLIC_, DUMMY_LAST }_segType;

public:
	explicit SegmentationControl(const Mat& IMG,int slic_pixel_width=10);
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

	void setSegmentationType(segmentationType type);
	segmentationType getSegmentationType();
	void processSegmentation(segmentationType type);
	void processSegmentations();

signals:
	void signalSendPts(vector<PtrSegmentPoints>* vecPts);
public slots:
void slotReceivePts(vector<Point>* vecPts);

private:
	bool init();
	void updateSegmentsStorageWithLabelImage(segmentationType type, Mat& labelImg);//update _vecAllSegmentationTypeSegments and _vecSegmentationSegmentsNum
	//void updateSegmentsStorageWithLabelImage(segmentationType type, int* plabelImg, int width, int height);//update _vecAllSegmentationTypeSegments and _vecSegmentationSegmentsNum
	Mat& getMatRef(int idx);
	PtrSegments& getSegmentsPtrRef(int idx);
	int& getSegmentsNumRef(int idx);
private:
	Mat _originalIMG;
	Mat _labelImg;
	vector<Mat> _vecAllSegmentationTypeMats;
	vector<PtrSegments> _vecAllSegmentationTypeSegments;
	vector<int> _vecSegmentationSegmentsNum;
	vector<PtrSegmentPoints> _tempVecPts;
	int _slic_pixel_width;
};

