#include "OpencvUtils.h"
#include <qglobal.h>
namespace CV_Utils
{
	cv::Rect trimRect(cv::Rect rect, int min_x, int min_y, int max_x, int max_y)
	{
		cv::Rect r;
		int rbX = rect.x + rect.width;
		int rbY = rect.y + rect.height;
		r.x = qMin(qMax(min_x, rect.x), max_x);
		r.y = qMin(qMax(min_y, rect.y), max_y);
		rbX = qMin(qMax(min_x, rbX), max_x);
		rbY = qMin(qMax(min_y, rbY), max_y);
		r.width = rbX - r.x;
		r.height = rbY - r.y;
		return r;
	}
}