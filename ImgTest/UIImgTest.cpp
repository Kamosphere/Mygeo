#include "UIImgTest.h"
#include <QtWidgets\QFileDialog>
#include <QtWidgets\QMessageBox>
#include "MapCanvas.h"
#include <opencv.hpp>
#include <QtWidgets\QHBoxLayout>
#include <QDebug>  
#include <QThread>  
#include<qtimer.h>
#include "WatershedMain.h"

using namespace cv;

MyThread::MyThread(MapCanvas *MyMap,QObject *parent) : /* 构造函数 */
QThread(parent)
{
	myMapcopy = MyMap;
}

void MyThread::run()  /* start()函数默认会调用run()函数 */
{
		Mat tempA = myMapcopy->getMatDataA();

		WatershedMain water = WatershedMain(tempA);
		if (water.mainMethod() != 0){
			cout << "error!" << endl;
		}
}

UIImgTest::UIImgTest( QWidget *parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags )
{
    ui.setupUi( this );
    this->showMaximized();
    
    myMap = new MapCanvas( this );
    myMap->setMinimumSize( 1024, 768 );

	my = new MyThread(myMap);

	tabWidget = new QTabWidget(); 
	QWidget *widget(myMap);
	tabWidget->addTab(widget, QStringLiteral("原始图像"));
	tabWidget->showMaximized();
	this->setCentralWidget(tabWidget);

	
    ui.tableView->setModel( myMap->ImgMetaModel() );// 初始化图像元数据模型
    ui.tableView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    ui.fileTreeView->setModel( myMap->FileListModel() );// 初始化文件列表
    ui.fileTreeView->setEditTriggers( QAbstractItemView::NoEditTriggers );
	ui.progressBar->setRange(0, 3);
	ui.progressBar->setValue(processView::viewer);
	ui.label_2->setText(QStringLiteral("未就绪"));

    // 连接信号、槽
    connect( ui.actionAbout, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );
    connect( ui.actionImage, SIGNAL( triggered() ), this, SLOT( PickOpenFile() ) );
    connect( ui.actionExit, SIGNAL( triggered() ), this, SLOT( Exit() ) );
	connect( ui.actionFile_Save, SIGNAL(triggered()), this, SLOT(saveResult()));
	connect( ui.actionExtract, SIGNAL(triggered()), this, SLOT(Extract()), Qt::QueuedConnection);
    connect( ui.actionZoom_In, SIGNAL( triggered() ), this->myMap, SLOT( ZoomIn() ) );
    connect( ui.actionZoom_Out, SIGNAL( triggered() ), this->myMap, SLOT( ZoomOut() ) );

	connect(my, SIGNAL(finished()), this, SLOT(newtab()));
	
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(progressBar()));
	timer->start(100);
	
}

UIImgTest::~UIImgTest()
{

}

/*
进度条显示
*/
void UIImgTest::progressBar()
{
	ui.progressBar->setValue(processView::viewer);
}

/*
退出
*/
void UIImgTest::Exit()
{
    this->close();
}

/*
选择文件
*/
void UIImgTest::PickOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                           this,
						   QStringLiteral("选择要打开的遥感影像"),
                           QDir::currentPath(),
                           tr( "tif(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)" ) );
    if ( !fileName.isNull() )
    {
        myMap->ReadImg( fileName );
    }
    ui.fileTreeView->expandAll();
	processView::viewer = 0;
	ui.label_2->setText(QStringLiteral("已就绪"));
}

/*
保存提取结果
*/
void UIImgTest::saveResult()
{
	QString fileName = QFileDialog::getSaveFileName(
		this,
		QStringLiteral("选择保存路径"),
		QDir::currentPath(),
		tr("tif(*.tif);;jpg(*.jpg);;img(*.img);;All files(*.*)"));
	cout << fileName.toStdString() << "" << myMap2->isMatEmpty()<<endl;
	if (!fileName.isNull()&&!myMap2->isMatEmpty())
	{
		Mat save = myMap2->getMatDataA();
		imwrite(fileName.toStdString(),save);
		ui.label_2->setText(QStringLiteral("已保存"));
	}
	
}

/*
显示文件列表
*/
void UIImgTest::ShowFileListWindow()
{
    ui.FileListDockWidget->toggleViewAction();
}

/*
显示信息列表
*/
void UIImgTest::ShowInforWindow()
{
    ui.inforDockWidget->toggleViewAction();
}
void UIImgTest::Extract()
{
	if (ifExtract == true){
		QMessageBox msgBox;
		QString stringI = QStringLiteral("已经有处理的事务");
		QString stringJ = QStringLiteral("正在提取中，请等待");
		msgBox.setText(stringI);
		msgBox.setInformativeText(stringJ);
		msgBox.setStandardButtons(QMessageBox::Cancel);
		msgBox.exec();
	}
	else{
		ifExtract = true;
		ui.label_2->setText(QStringLiteral("正在处理"));
		this->my->start();
	}
	
}

/*
创建新标签以显示提取结果
*/
void UIImgTest::newtab()
{
	if (ifExtract == true){
		ifExtract = false;

		QString fileName = "log.txt";
		QFile file(fileName);
		if (!file.open(QIODevice::ReadWrite))
			return;
		QTextStream out(&file);
		while (!file.atEnd())
		{
			ui.textEdit->setText(out.readAll());
		}

		myMap2 = new MapCanvas(this);
		myMap2->ReadImg("aftermeanshift.jpg");
		QWidget *widget2(myMap2);
		this->tabWidget->addTab(widget2, QStringLiteral("meanshift后"));

		myMap3 = new MapCanvas(this);
		myMap3->ReadImg("Aftercolorfill.jpg");
		QWidget *widget3(myMap3);
		this->tabWidget->addTab(widget3, QStringLiteral("分水岭分割"));

		myMap4 = new MapCanvas(this);
		myMap4->ReadImg("regionalGrowthImage1.jpg");
		QWidget *widget4(myMap4);
		this->tabWidget->addTab(widget4, QStringLiteral("种子生长"));

		myMap5 = new MapCanvas(this);
		myMap5->ReadImg("wshed.jpg");
		QWidget *widget5(myMap5);
		this->tabWidget->addTab(widget5, QStringLiteral("提取结果"));

		ui.label_2->setText(QStringLiteral("处理完成"));
	}


}