
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

class MapCanvas;

/// <summary>
/// Class ImgTest.
/// </summary>
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
private:
    /// <summary>
    /// 主窗口UI
    /// </summary>
    Ui::ImgTestClass ui;
    /// <summary>
    /// 图像显示窗口控件
    /// </summary>
    MapCanvas *myMap;
};

#endif // IMGTEST_H
