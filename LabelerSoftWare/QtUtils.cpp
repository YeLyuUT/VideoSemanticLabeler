#include <QPainter>
void copyQImageToQImage(QImage&src, QImage&dst,bool bDeep)
{
	if (bDeep)
	{
		if (dst.size() != src.size())
		{
			dst = src.copy();
		}
		else
		{
			QPainter painter(&dst);
			painter.drawImage(QPoint(0, 0), src);
		}
	}
	else
	{
		dst = src;
	}
}