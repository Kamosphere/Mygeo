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
	//int i = 0;  
		Mat tempA = myMapcopy->getMatDataA();
		//Mat tempB = myMap->getMatDataB();
		//std::string Aword = "oriA.jpg";
		//imwrite(Aword, tempA);
		//std::string Bword = "temp/oriB";
		//imwrite(Bword, tempB);
		WatershedMain water = WatershedMain(tempA);
		if (water.mainMethod() != 0){
			cout << "error!" << endl;
		}
}

/// <summary>
/// Initializes a new instance of the <see cref="ImgTest"/> class.
/// </summary>
/// <param name="ILogic &">业务逻辑类引用.</param>
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
	//connect(ui.actionSaveresult, SIGNAL(triggered()), this, SLOT(SaveResult()));
	connect(ui.actionFile_Save, SIGNAL(triggered()), this, SLOT(saveResult()));
    //connect( ui.actionInformation, SIGNAL( triggered() ), this, SLOT( ShowInforWindow() ) );
	connect(ui.actionExtract, SIGNAL(triggered()), this, SLOT(Extract()), Qt::QueuedConnection);
    connect( ui.actionZoom_In, SIGNAL( triggered() ), this->myMap, SLOT( ZoomIn() ) );
    connect( ui.actionZoom_Out, SIGNAL( triggered() ), this->myMap, SLOT( ZoomOut() ) );

	connect(my, SIGNAL(finished()), this, SLOT(newtab()));
	
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(progressBar()));
	timer->start(100);

}

void UIImgTest::progressBar()
{
		ui.progressBar->setValue(processView::viewer);
}

/// <summary>
/// Finalizes an instance of the <see cref="ImgTest" /> class.
/// </summary>
UIImgTest::~UIImgTest()
{

}

/// <summary>
/// 退出
/// </summary>
void UIImgTest::Exit()
{
    this->close();
}

/// <summary>
/// 选择文件
/// </summary>
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
	ui.label_2->setText(QStringLiteral("已就绪"));
}

/// <summary>
/// 保存提取结果
/// </summary>
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

/// <summary>
/// Shows the file list window
/// </summary>
void UIImgTest::ShowFileListWindow()
{
    ui.FileListDockWidget->toggleViewAction();
}

/// <summary>
/// Shows the information window.
/// </summary>
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
	//connect(thread1, SIGNAL(started()), my, SLOT(first(myMap)));
	
}

void UIImgTest::newtab()
{
	if (ifExtract == true){
		ifExtract = false;
		myMap2 = new MapCanvas(this);
		myMap2->ReadImg("wshed.jpg");
		QWidget *widget2(myMap2);
		this->tabWidget->addTab(widget2, QStringLiteral("提取结果"));
		ui.label_2->setText(QStringLiteral("处理完成"));
	}


}