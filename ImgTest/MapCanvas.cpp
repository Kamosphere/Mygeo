#include "MapCanvas.h"
#include <QtWidgets\QMessageBox>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QtWidgets\QGraphicsPixmapItem>
#include <QMatrix>
#include <QWheelEvent>
#include <QtWidgets\QScrollBar>
#include <opencv.hpp>
#include <gdal_priv.h>

MapCanvas::MapCanvas( QWidget *parent /*= 0 */ )
    : QGraphicsView( parent )
{
    poDataset = NULL;
    m_scaleFactor = 1.0;
    m_showColor = true;
    imgMetaModel = new QStandardItemModel;
    imgMetaModel->setColumnCount( 2 );
    fileListModel = new QStandardItemModel;
    QSizePolicy policy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    this->setSizePolicy( policy );
}

MapCanvas::~MapCanvas()
{

}

/*
QImage转换为Mat格式
输入：
QImage image：QImage格式图像
return：
Mat mat：Mat格式图像
*/
cv::Mat QImageToMat(QImage image)
{
	cv::Mat mat;
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
		cv::cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
		break;
	}
	return mat;
}

bool MapCanvas::isMatEmpty(){
	if (OriginalDataA.empty()){
		return true;
	}
	return false;
}

/*
读取图像文件
输入：
QString imgPath：图像文件路径
*/
void MapCanvas::ReadImg( const QString imgPath )
{
	
    GDALAllRegister();
    CPLSetConfigOption( "GDAL_FILENAME_IS_UTF8", "NO" );
    poDataset = ( GDALDataset* )GDALOpen( imgPath.toStdString().c_str(), GA_ReadOnly );
    if ( poDataset == NULL )
    {
		QMessageBox::critical(this, QStringLiteral("错误！"), QStringLiteral("无法打开文件 %1").arg(imgPath));
        return;
    }
    ShowFileList( imgPath );
    ShowImgInfor( imgPath );
    //如果图像文件是三个波段，全部显示
	if (poDataset->GetRasterCount() == 3)
	{
		m_showColor = true;
		QList<GDALRasterBand*> bandList;
		bandList.append(poDataset->GetRasterBand(1));
		bandList.append(poDataset->GetRasterBand(2));
		bandList.append(poDataset->GetRasterBand(3));
		QImage A=ImgProcess(&bandList,1);
		Imgview(A);
		OriginalDataA = QImageToMat(A);
	}
	//如果图像文件大于三个波段，取前三个波段和后三个波段显示
    else if ( poDataset->GetRasterCount() > 3)
    {
        m_showColor = true;
		int countRas = poDataset->GetRasterCount();
		QList<GDALRasterBand*> bandListA,bandListB;
		bandListA.append(poDataset->GetRasterBand(3));
		bandListA.append(poDataset->GetRasterBand(2));
		bandListA.append(poDataset->GetRasterBand(1));

		bandListB.append(poDataset->GetRasterBand(countRas));
		bandListB.append(poDataset->GetRasterBand(countRas-1));
		bandListB.append(poDataset->GetRasterBand(countRas-2));

		QImage B=ImgProcess(&bandListB,2);
		QImage A=ImgProcess(&bandListA,1);
		Imgview(A);
		OriginalDataB = QImageToMat(B);
		OriginalDataA = QImageToMat(A);
		cv::Mat wshed;
		addWeighted(OriginalDataA, 0.1, OriginalDataB, 0.9, 0, wshed);
    }
    //如果图像只有一个波段，则显示那个波段
    else
    {
		m_showColor = false;
		ShowBand(poDataset->GetRasterBand(1));
    }
    GDALClose( poDataset );
}

/*
关闭当前图像文件
*/
void MapCanvas::CloseCurrentImg()
{
    poDataset = NULL;
    imgMetaModel->clear();
    fileListModel->clear();
}

/*
显示单波段图像
输入：
GDALRasterBand* band：图像波段
*/
void MapCanvas::ShowBand( GDALRasterBand* band )
{
    if ( band == NULL )
    {
        return;
    }
    
    QList<GDALRasterBand*> myBand;
    myBand.append( band );
    myBand.append( band );
    myBand.append( band );
    
	ImgProcess(&myBand,1);
    
}

/*
显示内存中图像文件
输入：
QImage image：QImage格式图像
*/
void MapCanvas::Imgview(QImage image)
{
	QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	QGraphicsScene *myScene = new QGraphicsScene();
	myScene->addItem(imgItem);
	this->setScene(myScene);
}

/*
图像分波段处理
输入：
QList<GDALRasterBand*> *imgBand：list格式的各波段信息
int count：波段总数
return：
QImage ImgTemp：QImage格式图像
*/
QImage MapCanvas::ImgProcess( QList<GDALRasterBand*> *imgBand,int count)
{
	cv::Mat iimg;
	QImage iimg2;
    if ( imgBand->size() != 3 )
    {
        return iimg2;
    }
    int imgWidth = imgBand->at( 0 )->GetXSize();
    int imgHeight = imgBand->at( 0 )->GetYSize();
    
    m_scaleFactor = this->height() * 1.0 / imgHeight;
    
	int iScaleWidth = (int)(imgWidth);
	int iScaleHeight = (int)(imgHeight);
    
    GDALDataType dataType = imgBand->at( 0 )->GetRasterDataType();
    
    // 首先分别读取RGB三个波段
    float* rBand = new float[iScaleWidth * iScaleHeight];
    float* gBand = new float[iScaleWidth * iScaleHeight];
    float* bBand = new float[iScaleWidth * iScaleHeight];
    
    unsigned char *rBandUC, *gBandUC, *bBandUC;
    
    // 根据是否显示彩色图像，判断RGB三个波段的组成方式，并分别读取
    if ( m_showColor == true )
    {
        imgBand->at( 0 )->RasterIO( GF_Read, 0, 0, imgWidth, imgHeight, rBand , iScaleWidth, iScaleHeight, GDT_Float32, 0, 0 );
        imgBand->at( 1 )->RasterIO( GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0 );
        imgBand->at( 2 )->RasterIO( GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0 );
        
        // 分别拉伸每个波段并将Float转换为unsigned char
        rBandUC = ImgSketch( rBand, imgBand->at( 0 ), iScaleWidth * iScaleHeight, imgBand->at( 0 )->GetNoDataValue() );
        gBandUC = ImgSketch( gBand, imgBand->at( 1 ), iScaleWidth * iScaleHeight, imgBand->at( 1 )->GetNoDataValue() );
        bBandUC = ImgSketch( bBand, imgBand->at( 2 ), iScaleWidth * iScaleHeight, imgBand->at( 2 )->GetNoDataValue() );
    }
    else
    {
        imgBand->at( 0 )->RasterIO( GF_Read, 0, 0, imgWidth, imgHeight, rBand , iScaleWidth, iScaleHeight, GDT_Float32, 0, 0 );
        
        rBandUC = ImgSketch( rBand, imgBand->at( 0 ), iScaleWidth * iScaleHeight, imgBand->at( 0 )->GetNoDataValue() );
        gBandUC = rBandUC;
        bBandUC = rBandUC;
    }
    
    // 将三个波段组合起来
    int bytePerLine = ( iScaleWidth * 24 + 31 ) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight * 3];
    for( int h = 0; h < iScaleHeight; h++ )
    {
        for( int w = 0; w < iScaleWidth; w++ )
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBandUC[h * iScaleWidth + w];
        }
    }
	QImage ImgTemp = QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);
	return ImgTemp;
    // 构造图像
}

/*
显示图像基本信息
输入：
QString filename：图像文件名
*/
void MapCanvas::ShowImgInfor( const QString filename )
{
    if ( filename == "" || poDataset == NULL )
    {
        return;
    }
    int row = 0; // 用来记录数据模型的行号
    
    // 图像的格式信息
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("描述")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( poDataset->GetDriver()->GetDescription() ) );
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("元信息")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) ) ) ;
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("数据类型")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( GDALGetDataTypeName( ( poDataset->GetRasterBand( 1 )->GetRasterDataType() ) ) ) );
    
    // 图像的大小和波段个数
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("X坐标信息")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number( poDataset->GetRasterXSize() ) ) );
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("Y坐标信息")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number( poDataset->GetRasterYSize() ) ) );
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("波段数")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( QString::number( poDataset->GetRasterCount() ) ) );
    
    // 图像的投影信息
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("投影信息")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( poDataset->GetProjectionRef() ) );
    
    // 图像的坐标和分辨率信息
    double adfGeoTransform[6];
    QString origin = "";
    QString pixelSize = "";
    if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
    {
        origin = QString::number( adfGeoTransform[0] ) + ", " + QString::number( adfGeoTransform[3] );
        pixelSize = QString::number( adfGeoTransform[1] ) + ", " + QString::number( adfGeoTransform[5] );
    }
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("原始区域")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( origin ) );
	imgMetaModel->setItem(row, 0, new QStandardItem(QStringLiteral("分辨率")));
    imgMetaModel->setItem( row++, 1, new QStandardItem( pixelSize ) );
}

/*
显示文件结构树
输入：
QString filename：图像文件名
*/
void MapCanvas::ShowFileList( const QString filename )
{
    if ( filename == "" || poDataset == NULL )
    {
        return;
    }
    QFileInfo fileInfo( filename );
    QStandardItem *rootItem = new QStandardItem( fileInfo.fileName() );
    for ( int i = 0; i < poDataset->GetRasterCount(); i++ )
    {
		QStandardItem *childItem = new QStandardItem(QStringLiteral("波段 %1").arg(i + 1));
        rootItem->setChild( i, childItem );
    }
    fileListModel->setItem( 0, rootItem );
}

/*
图像线性拉伸
输入：
float* buffer 图像缓存
GDALRasterBand* currentBand 当前波段
int bandSize 波段数
double noValue 图像中的异常值
return：
unsigned char* resBuffer 经过拉伸的8位图像缓存
*/
unsigned char* MapCanvas::ImgSketch( float* buffer , GDALRasterBand* currentBand, int bandSize, double noValue )
{
    unsigned char* resBuffer = new unsigned char[bandSize];
    double max, min;
    double minmax[2];
    
    currentBand->ComputeRasterMinMax( 1, minmax );
    min = minmax[0];
    max = minmax[1];
    if( min <= noValue && noValue <= max )
    {
        min = 0;
    }
    for ( int i = 0; i < bandSize; i++ )
    {
        if ( buffer[i] > max )
        {
            resBuffer[i] = 255;
        }
        else if ( buffer[i] <= max && buffer[i] >= min )
        {
            resBuffer[i] = static_cast<uchar>( 255 - 255 * ( max - buffer[i] ) / ( max - min ) );
        }
        else
        {
            resBuffer[i] = 0;
        }
    }
    
    return resBuffer;
}

/*
控件大小
return：
QSize：规定的分辨率
*/
QSize MapCanvas::sizeHint() const
{
    return QSize( 1024, 768 );
}

/*
鼠标滚轮事件，实现图像缩放
输入：
QWheelEvent *event：滚轮事件
*/
void MapCanvas::wheelEvent( QWheelEvent *event )
{
    // 滚轮向上滑动，放大图像
    if ( event->delta() > 0 )
    {
        ZoomIn();
    }
    // 滚轮向下滑动，缩小图像
    if ( event->delta() < 0 )
    {
        ZoomOut();
    }
}

/*
鼠标按键按下事件
输入：
QMouseEvent *event：鼠标事件
*/
void MapCanvas::mousePressEvent( QMouseEvent *event )
{
    // 滚轮键按下，平移图像
    if ( event->button() == Qt::MidButton )
    {
        this->setDragMode( QGraphicsView::ScrollHandDrag );
        this->setInteractive( false );
        lastEventCursorPos = event->pos();
    }
}

/*
鼠标移动事件
输入：
QMouseEvent *event：鼠标事件
*/
void MapCanvas::mouseMoveEvent( QMouseEvent *event )
{
    if ( this->dragMode() == QGraphicsView::ScrollHandDrag )
    {
        QPoint delta = ( event->pos() - lastEventCursorPos ) / 10;
        this->horizontalScrollBar()->setValue( this->horizontalScrollBar()->value() + ( isRightToLeft() ? delta.x() : -delta.x() ) );
        this->verticalScrollBar()->setValue( this->verticalScrollBar()->value() - delta.y() );
        this->viewport()->setCursor( Qt::ClosedHandCursor );
    }
    
}

/*
鼠标按键释放事件
输入：
QMouseEvent *event：鼠标事件
*/
void MapCanvas::mouseReleaseEvent( QMouseEvent *event )
{
    if ( event->button() == Qt::MidButton )
    {
        this->setDragMode( QGraphicsView::NoDrag );
    }
}

