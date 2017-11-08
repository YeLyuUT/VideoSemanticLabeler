#include "labelersoftware.h"
#include <QtWidgets/QApplication>
#include <qcommandlineparser.h>
#include "ProcessSettingReader.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include "DataType.h"
#include "ProcessControl.h"
#include "Surface.h"
#include "ClassSelection.h"
#include <LabelingTaskControl.h>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	do 
	{
		//string xmlPath = "E:\\WorkingDirectory\\MY\\ProjectDataCollection\\UAVBenchMark\\Encapsulator\\LabelerSoftWare\\LabelerSoftWare\\processSetting.xml";
		//string xmlPath = "C:\\Work\\VSSolutions\\labelersoftware\\example\\processSetting.xml";
		//string xmlPath = "D:\\UAVDataCapturing\\German\\Gronau\\processSetting.xml";
		string xmlPath = "./processSetting.xml";
		try
		{
			ProcessSettingReader r(xmlPath);
			if (r.parse())
			{
		
				MetaData metaData = r.getMetaData();
				//QImage Img("C:\\Users\\lvye\\Desktop\\unnamed.jpg");
				
				//DrawTaskControl::getDrawControl(Img, selection);
				
				//selection->show();
				ProcessControl *pCon = new ProcessControl(metaData.filePath, metaData.outputDir, metaData.skipFrameNum, metaData.labelList);
				pCon->process();
			}
			else
			{
				QMessageBox::critical(NULL, "Error", QString("Cannot parse xml file!\n"), QMessageBox::StandardButton::Cancel);
				return 0;
			}			
		}
		catch (std::exception &e)
		{
			QMessageBox::critical(NULL, "Error", QString("XML file content wrong!\n")+ QString(e.what()), QMessageBox::StandardButton::Cancel);
			return 0;
		}
		catch (...)
		{
			QMessageBox::critical(NULL, "Error", "Cannot read XML file!\nPlease make sure the content is correct!", QMessageBox::StandardButton::Cancel);			
			return 0;
		}
	} while(false);

	return a.exec();
}
