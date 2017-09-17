#include "SegmentationControl.h"
#include <QDebug>
#include "SLIC.h"

SegmentationControl::SegmentationControl(const Mat& IMG)
{
	assert(IMG.type() == CV_8UC3);
	_originalIMG = IMG;
	init();
}

SegmentationControl::~SegmentationControl()
{
}

Mat& SegmentationControl::getMatRef(int idx)
{
	return _vecAllSegmentationTypeMats[idx];
}
PtrSegments& SegmentationControl::getSegmentsPtrRef(int idx)
{
	return _vecAllSegmentationTypeSegments[idx];
}
int& SegmentationControl::getSegmentsNumRef(int idx)
{
	return _vecSegmentationSegmentsNum[idx];
}

void SegmentationControl::doMeanShiftSegmentation()
{
	//TODO
	const int idx = getSegmentationType0basedIdx(MEAN_SHIFT);
	Mat & mSegImg = getMatRef(idx);
	PtrSegments& pSegments = getSegmentsPtrRef(idx);
	int &iSegNum = getSegmentsNumRef(idx);

	qDebug() << "TODO add meanshift seg";
}

void SegmentationControl::doSlicSegmentation()
{
	int width = _originalIMG.cols;
	int height = _originalIMG.rows;
	int sz = width*height;
	unsigned int * IMG = NULL;
	int* labels = NULL;
	IMG = new unsigned int[width*height];
	labels = new int[sz];
	if (IMG == nullptr) { throw exception("not Enough Memory"); }
	slic::CopyMatToMem(_originalIMG, IMG, width, height);
	SLIC slic;
	int numlabels(0);
	double dummyM(0);
	slic.PerformSLICO_ForGivenStepSize(IMG, width, height, labels, numlabels, 20, dummyM);
	slic.DrawContoursAroundSegmentsTwoColors(IMG, labels, width, height);
	Mat outIMG;
	slic::CopyMemToMat(outIMG, IMG, width, height);
	cv::imshow("image", outIMG);
	cv::waitKey(1);
	int idx = getSegmentationType0basedIdx(segmentationType::SLIC_);
	getMatRef(idx) = outIMG;
	delete[]IMG;
	delete[]labels;
}

int SegmentationControl::getSegmentationTypeNum()
{
	return static_cast<int>(DUMMY_LAST - DUMMY_FIRST - 1);
}

void SegmentationControl::processSegmentation(segmentationType type)
{
	switch (type)
	{
	case MEAN_SHIFT:
		doMeanShiftSegmentation();
		break;
	default:
		break;
	}
}

void SegmentationControl::processSegmentations()
{
	doMeanShiftSegmentation();
	//TODO add more
}

int SegmentationControl::getSegmentationType0basedIdx(segmentationType type)
{
	return type - DUMMY_FIRST - 1;
}

Mat SegmentationControl::getSegmentationMatOfType(segmentationType type)
{
	int idx = getSegmentationType0basedIdx(type);
	return _vecAllSegmentationTypeMats[idx];
}

PtrSegments SegmentationControl::getSegmentationSegmentsOfType(segmentationType type)
{
	int idx = getSegmentationType0basedIdx(type);
	return _vecAllSegmentationTypeSegments[idx];
}

int SegmentationControl::getSegmentationSegmentsNumOfType(segmentationType type)
{
	int idx = getSegmentationType0basedIdx(type);
	return _vecSegmentationSegmentsNum[idx];
}

int SegmentationControl::getLabel(Mat Img, int x, int y)
{
	return Img.at<int>(y, x);
}

PtrSegmentPoints SegmentationControl::getSegmentOfType(segmentationType type, int segment_Idx)
{
	PtrSegments pSegs = getSegmentationSegmentsOfType(type);
	PtrSegmentPoints pSegPts = (*pSegs)[segment_Idx];
	return pSegPts;
}

bool SegmentationControl::init()
{
	int count = this->getSegmentationTypeNum();
	_vecAllSegmentationTypeMats.resize(count);
	_vecAllSegmentationTypeSegments.resize(count);
	_vecSegmentationSegmentsNum.resize(count);
	return true;
}

void SegmentationControl::setSegmentationTypeNum(segmentationType type, int count)
{
	int idx = getSegmentationType0basedIdx(type);
	_vecSegmentationSegmentsNum[idx] = count;
}
void SegmentationControl::setSegmentationMatOfType(segmentationType type, Mat& IMG)
{
	int idx = getSegmentationType0basedIdx(type);
	_vecAllSegmentationTypeMats[idx] = IMG;
}
void SegmentationControl::setSegmentationSegmentsOfType(segmentationType type, PtrSegments segs)
{
	int idx = getSegmentationType0basedIdx(type);
	_vecAllSegmentationTypeSegments[idx] = segs;
}
void SegmentationControl::setSegmentationNumOfType(segmentationType type, int num)
{
	int idx = getSegmentationType0basedIdx(type);
	_vecSegmentationSegmentsNum[idx] = num;
}
void SegmentationControl::setSegmentOfType(segmentationType type, int segment_Idx, PtrSegmentPoints segPts)
{
	PtrSegments pSegs = getSegmentationSegmentsOfType(type);
	(*pSegs)[segment_Idx] = segPts;
}

Mat& SegmentationControl::getSlicSegResultRef()
{
	int idx = getSegmentationType0basedIdx(segmentationType::SLIC_);
	return getMatRef(idx);
}

Mat& SegmentationControl::getMeanShiftSegResultRef()
{
	return Mat();
}

void SegmentationControl::updateSegmentsStorageWithLabelImage(segmentationType type, Mat& labelImg)
{

}
void SegmentationControl::updateSegmentsStorageWithLabelImage(segmentationType type, int* labelImg, int width, int height)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int label = labelImg[i*width + j];
		}
	}

}