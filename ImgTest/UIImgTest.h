
// ***********************************************************************
// <copyright file="UIImgTest.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary>主UI类。</summary>
// ***********************************************************************
#ifndef IMGTEST_H
#define IMGTEST_H

#include <QtWidgets\QMainWindow>
#include "ui_imgtest.h"
#include<qthread.h>
#include <iostream>
#include <streambuf>
#include <string>

#include "qtextedit.h"

class MapCanvas;

/// <summary>
/// Class ImgTest.
/// </summary>


class MyThread : public QThread
{
	Q_OBJECT
public:
	explicit MyThread(MapCanvas *MyMap,QObject *parent = 0);

protected:
	void run();

private:
	MapCanvas* myMapcopy;

signals:
	public slots :
};


class UIImgTest : public QMainWindow
{
    Q_OBJECT
    
public:
    UIImgTest( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
    ~UIImgTest();
public slots:
    void PickOpenFile();
    void Exit();
    void ShowFileListWindow();
    void ShowInforWindow();
	void Extract();
	void newtab();
	void saveResult();
	void progressBar();
private:
    /// 主窗口UI
    Ui::ImgTestClass ui;
    /// 图像显示窗口控件
	QTabWidget *tabWidget;
    MapCanvas *myMap;
	MapCanvas *myMap2;
	MapCanvas *myMap3;
	MapCanvas *myMap4;
	MapCanvas *myMap5;
	MyThread *my;
	QTimer *timer;
	bool ifExtract = false;
};

#endif // IMGTEST_H
