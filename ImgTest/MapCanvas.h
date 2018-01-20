
//图像显示窗口类，负责图像的读取和显示等功能。
#ifndef MAPCANVAS_H
#define MAPCANVAS_H

#include <QtWidgets\QGraphicsView>
#include <QStandardItemModel>
#include <gdal_priv.h>
#include <opencv.hpp>

class MapCanvas : public QGraphicsView
{
	Q_OBJECT

public:
	MapCanvas(QWidget *parent = 0);
	~MapCanvas();
	bool isMatEmpty();
	void ReadImg(const QString imgPath);
	void CloseCurrentImg();
	/// 返回图像元数据信息模型.
	QStandardItemModel* ImgMetaModel()
	{
		return imgMetaModel;
	};
	/// 设置图像元数据信息模型
	void SetMetaModel(QStandardItemModel* model)
	{
		this->imgMetaModel = model;
	};
	/// 返回文件列表数据模型
	QStandardItemModel* FileListModel()
	{
		return fileListModel;
	};
	/// 设置fileListModel图像文件列表数据模型
	void SetFileListModel(QStandardItemModel* model)
	{
		this->fileListModel = model;
	};

	QSize sizeHint() const;

	public slots:
	/// 放大图像
	void ZoomIn()
	{
		ScaleImg(1.2);
	};
	/// 缩小图像
	void ZoomOut()
	{
		ScaleImg(0.8);
	};
	cv::Mat getMatDataA(){
		return OriginalDataA;
	}
	cv::Mat getMatDataB(){
		return OriginalDataB;
	}
protected:
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	cv::Mat OriginalDataA;
	cv::Mat OriginalDataB;
	void ShowBand(GDALRasterBand* band);
	QImage ImgProcess(QList<GDALRasterBand*> *imgBand,int count);
	void Imgview(QImage processer);
	void ShowImgInfor(const QString filename);
	void ShowFileList(const QString filename);
	cv::Mat GDAL2Mat(const QString filename);

	unsigned char* ImgSketch(float* buffer, GDALRasterBand* currentBand, int size, double noValue);
	/// 图像缩放
	/// <"factor">缩放因子
	void ScaleImg(double factor)
	{
		m_scaleFactor *= factor;
		QMatrix matrix;
		matrix.scale(m_scaleFactor, m_scaleFactor);
		this->setMatrix(matrix);
	};
	/// 图像元数据模型
	QStandardItemModel *imgMetaModel;
	/// 图像数据集
	GDALDataset *poDataset;
	/// 文件列表数据模型
	QStandardItemModel *fileListModel;
	/// 缩放系数
	float m_scaleFactor;
	/// 判断是否显示RGB彩色图像
	bool m_showColor;
	/// 上一个鼠标事件触发时鼠标的位置
	QPoint lastEventCursorPos;
};

#endif // MAPCANVAS_H
