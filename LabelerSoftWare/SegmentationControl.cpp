#include "SegmentationControl.h"
#include <QDebug>
#include "SLIC.h"
//#define COMPILE_TEST
#ifdef COMPILE_TEST
namespace test
{
	void testSegments(PtrSegments& segs,int width,int height)
	{
		Mat outPut(height,width,CV_8UC3);
		outPut.setTo(0);
		int labelNum = segs->size();
		int ptNum = 0;
#pragma omp parallel for
		for (int i = 0; i < labelNum; i++)
		{
			cv::Scalar color(cv::randu<uchar>(), cv::randu<uchar>(), cv::randu<uchar>());
			PtrSegmentPoints pPts = (*segs)[i];
			ptNum += pPts->size();
			for (int j = 0; j < pPts->size(); j++)
			{
				Point pt = (*pPts)[j];
				outPut.at<cv::Vec3b>(pt.y, pt.x)[0] = color[0];
				outPut.at<cv::Vec3b>(pt.y, pt.x)[1] = color[1];
				outPut.at<cv::Vec3b>(pt.y, pt.x)[2] = color[2];
			}
		}
		cv::imwrite("Test.bmp", outPut);
		qDebug() << "Total segments' points:" << ptNum;
	}
}
#endif // COMPILE_TEST

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
	_labelImg.create(height, width, CV_32S);
	labels = (int*)_labelImg.data;
	if (IMG == nullptr) { throw exception("not Enough Memory"); }
	slic::CopyMatToMem(_originalIMG, IMG, width, height);
	SLIC slic;
	int numlabels(0);
	double dummyM(0);
	slic.PerformSLICO_ForGivenStepSize(IMG, width, height, labels, numlabels, 20, dummyM);
	slic.DrawContoursAroundSegmentsTwoColors(IMG, labels, width, height);
	//updateSegmentsStorageWithLabelImage(segmentationType::SLIC_, labels, width, height);
	updateSegmentsStorageWithLabelImage(segmentationType::SLIC_, _labelImg);
	Mat outIMG;
	slic::CopyMemToMat(outIMG, IMG, width, height);
	//cv::imshow("image", outIMG);
	//cv::waitKey(1);
	int idx = getSegmentationType0basedIdx(segmentationType::SLIC_);
	getMatRef(idx) = outIMG;
	delete[]IMG;
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
	_segType = DUMMY_FIRST;
	int count = this->getSegmentationTypeNum();
	_vecAllSegmentationTypeMats.resize(count);
	_vecAllSegmentationTypeSegments.resize(count);
	_vecSegmentationSegmentsNum.resize(count);
	for (int i = 0; i < count; i++)
	{
		(_vecAllSegmentationTypeSegments[i]).reset(new Segments);
	}
	return true;
}

void SegmentationControl::setSegmentationType(SegmentationControl::segmentationType type)
{
	_segType = type;
}

SegmentationControl::segmentationType SegmentationControl::getSegmentationType()
{
	return _segType;
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
	int width = labelImg.cols;
	int height = labelImg.rows;
	int* plabelImg = (int*)labelImg.data;
	int idx = getSegmentationType0basedIdx(type);
	int maxlabelIndex = -1;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int label = plabelImg[i*width + j];
			if (label > maxlabelIndex)
			{
				maxlabelIndex = label;
			}
		}
	}
	this->setSegmentationNumOfType(type, maxlabelIndex + 1);
	PtrSegments pSegs = this->getSegmentsPtrRef(idx);
	assert(pSegs.get() != NULL);
	pSegs->resize(maxlabelIndex + 1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int label = plabelImg[i*width + j];
			if ((*pSegs)[label].get() == NULL)
			{
				(*pSegs)[label].reset(new SementPoints);
				(*pSegs)[label]->reserve(500);
			}
			(*pSegs)[label]->push_back(Point(j, i));
		}
	}
	for (int i = 0; i < maxlabelIndex + 1; i++)
	{
		(*pSegs)[i]->shrink_to_fit();
	}
#ifdef COMPILE_TEST
	test::testSegments(pSegs, width, height);
#endif//COMPILE_TEST
}
//void SegmentationControl::updateSegmentsStorageWithLabelImage(segmentationType type, int* plabelImg, int width, int height)
//{
//	int idx = getSegmentationType0basedIdx(type);
//	int maxlabelIndex = -1;
//	for (int i = 0; i < height; i++)
//	{
//		for (int j = 0; j < width; j++)
//		{
//			int label = plabelImg[i*width + j];
//			if (label > maxlabelIndex)
//			{
//				maxlabelIndex = label;
//			}
//		}
//	}
//	this->setSegmentationNumOfType(type, maxlabelIndex);
//	PtrSegments pSegs = this->getSegmentsPtrRef(idx);
//	assert(pSegs.get() != NULL);
//	pSegs->resize(maxlabelIndex + 1);
//	for (int i = 0; i < height; i++)
//	{
//		for (int j = 0; j < width; j++)
//		{
//			int label = plabelImg[i*width + j];
//			if ((*pSegs)[label].get() == NULL)
//			{
//				(*pSegs)[label].reset(new SementPoints);
//				(*pSegs)[label]->reserve(500);
//			}
//			(*pSegs)[label]->push_back(Point(j, i));
//		}
//	}
//	for (int i = 0; i < maxlabelIndex + 1; i++)
//	{
//		(*pSegs)[i]->shrink_to_fit();
//	}
//#ifdef COMPILE_TEST
//test::testSegments(pSegs, width, height);
//#endif//COMPILE_TEST	
//}

void SegmentationControl::slotReceivePts(vector<Point>* vecPts)
{
	_tempVecPts.clear();
	vector<int> labelCache;
	labelCache.reserve(30);
	segmentationType type = getSegmentationType();
	int idx = getSegmentationType0basedIdx(type);
	PtrSegments pSegs = _vecAllSegmentationTypeSegments[idx];
	for (size_t i = 0; i < vecPts->size(); i++)
	{
		Point pt = (*vecPts)[i];
		int label = _labelImg.at<int>(pt.y, pt.x);
		if (std::find(labelCache.begin(), labelCache.end(), label) != labelCache.end())
		{
			continue;
		}
		labelCache.push_back(label);
		PtrSegmentPoints pSegPts = (*pSegs)[label];
		for (size_t j = 0; j < pSegPts->size(); j++)
		{
			_tempVecPts.push_back((*pSegPts)[j]);
		}
	}
	emit signalSendPts(&_tempVecPts);
}
