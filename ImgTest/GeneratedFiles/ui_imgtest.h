/********************************************************************************
** Form generated from reading UI file 'imgtest.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMGTEST_H
#define UI_IMGTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ImgTestClass
{
public:
    QAction *actionImage;
    QAction *actionExit;
    QAction *actionInformation;
    QAction *actionFile_List;
    QAction *actionAbout;
    QAction *actionClose;
    QAction *actionZoom_Out;
    QAction *actionZoom_In;
    QAction *actionFit_Window;
    QAction *actionNormal_Size;
    QWidget *centralWidget;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *FileListDockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QTreeView *fileTreeView;
    QDockWidget *inforDockWidget;
    QWidget *dockWidgetContents_2;
    QVBoxLayout *verticalLayout;
    QTableView *tableView;
    QMenuBar *menuBar;
    QMenu *menuOpen;
    QMenu *menuWindow;
    QMenu *menuAbout;
    QMenu *menuView_Operation;

    void setupUi(QMainWindow *ImgTestClass)
    {
        if (ImgTestClass->objectName().isEmpty())
            ImgTestClass->setObjectName(QStringLiteral("ImgTestClass"));
        ImgTestClass->resize(1024, 768);
        ImgTestClass->setStyleSheet(QStringLiteral(""));
        actionImage = new QAction(ImgTestClass);
        actionImage->setObjectName(QStringLiteral("actionImage"));
        actionExit = new QAction(ImgTestClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionInformation = new QAction(ImgTestClass);
        actionInformation->setObjectName(QStringLiteral("actionInformation"));
        actionFile_List = new QAction(ImgTestClass);
        actionFile_List->setObjectName(QStringLiteral("actionFile_List"));
        actionAbout = new QAction(ImgTestClass);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionClose = new QAction(ImgTestClass);
        actionClose->setObjectName(QStringLiteral("actionClose"));
        actionZoom_Out = new QAction(ImgTestClass);
        actionZoom_Out->setObjectName(QStringLiteral("actionZoom_Out"));
        actionZoom_In = new QAction(ImgTestClass);
        actionZoom_In->setObjectName(QStringLiteral("actionZoom_In"));
        actionFit_Window = new QAction(ImgTestClass);
        actionFit_Window->setObjectName(QStringLiteral("actionFit_Window"));
        actionNormal_Size = new QAction(ImgTestClass);
        actionNormal_Size->setObjectName(QStringLiteral("actionNormal_Size"));
        centralWidget = new QWidget(ImgTestClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ImgTestClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(ImgTestClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ImgTestClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ImgTestClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ImgTestClass->setStatusBar(statusBar);
        FileListDockWidget = new QDockWidget(ImgTestClass);
        FileListDockWidget->setObjectName(QStringLiteral("FileListDockWidget"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        fileTreeView = new QTreeView(dockWidgetContents);
        fileTreeView->setObjectName(QStringLiteral("fileTreeView"));
        fileTreeView->header()->setVisible(false);

        verticalLayout_2->addWidget(fileTreeView);

        FileListDockWidget->setWidget(dockWidgetContents);
        ImgTestClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), FileListDockWidget);
        inforDockWidget = new QDockWidget(ImgTestClass);
        inforDockWidget->setObjectName(QStringLiteral("inforDockWidget"));
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        verticalLayout = new QVBoxLayout(dockWidgetContents_2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tableView = new QTableView(dockWidgetContents_2);
        tableView->setObjectName(QStringLiteral("tableView"));
        tableView->horizontalHeader()->setVisible(true);
        tableView->verticalHeader()->setVisible(false);

        verticalLayout->addWidget(tableView);

        inforDockWidget->setWidget(dockWidgetContents_2);
        ImgTestClass->addDockWidget(static_cast<Qt::DockWidgetArea>(2), inforDockWidget);
        menuBar = new QMenuBar(ImgTestClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1024, 23));
        menuOpen = new QMenu(menuBar);
        menuOpen->setObjectName(QStringLiteral("menuOpen"));
        menuWindow = new QMenu(menuBar);
        menuWindow->setObjectName(QStringLiteral("menuWindow"));
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QStringLiteral("menuAbout"));
        menuView_Operation = new QMenu(menuBar);
        menuView_Operation->setObjectName(QStringLiteral("menuView_Operation"));
        ImgTestClass->setMenuBar(menuBar);

        mainToolBar->addAction(actionImage);
        mainToolBar->addAction(actionClose);
        mainToolBar->addAction(actionZoom_Out);
        mainToolBar->addAction(actionZoom_In);
        mainToolBar->addAction(actionFit_Window);
        mainToolBar->addAction(actionNormal_Size);
        mainToolBar->addAction(actionAbout);
        menuBar->addAction(menuOpen->menuAction());
        menuBar->addAction(menuView_Operation->menuAction());
        menuBar->addAction(menuWindow->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuOpen->addAction(actionImage);
        menuOpen->addAction(actionClose);
        menuOpen->addSeparator();
        menuOpen->addAction(actionExit);
        menuWindow->addAction(actionFile_List);
        menuWindow->addAction(actionInformation);
        menuAbout->addAction(actionAbout);
        menuView_Operation->addAction(actionZoom_Out);
        menuView_Operation->addAction(actionZoom_In);
        menuView_Operation->addAction(actionFit_Window);
        menuView_Operation->addAction(actionNormal_Size);

        retranslateUi(ImgTestClass);

        QMetaObject::connectSlotsByName(ImgTestClass);
    } // setupUi

    void retranslateUi(QMainWindow *ImgTestClass)
    {
        ImgTestClass->setWindowTitle(QApplication::translate("ImgTestClass", "ImgTest", 0));
        actionImage->setText(QApplication::translate("ImgTestClass", "image", 0));
#ifndef QT_NO_TOOLTIP
        actionImage->setToolTip(QApplication::translate("ImgTestClass", "Open image files", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        actionImage->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        actionImage->setShortcut(QApplication::translate("ImgTestClass", "Ctrl+O", 0));
        actionExit->setText(QApplication::translate("ImgTestClass", "Exit", 0));
        actionInformation->setText(QApplication::translate("ImgTestClass", "Information", 0));
        actionFile_List->setText(QApplication::translate("ImgTestClass", "File List", 0));
        actionAbout->setText(QApplication::translate("ImgTestClass", "About", 0));
        actionClose->setText(QApplication::translate("ImgTestClass", "Close", 0));
        actionZoom_Out->setText(QApplication::translate("ImgTestClass", "Zoom Out", 0));
        actionZoom_In->setText(QApplication::translate("ImgTestClass", "Zoom In", 0));
        actionFit_Window->setText(QApplication::translate("ImgTestClass", "Fit Window", 0));
        actionNormal_Size->setText(QApplication::translate("ImgTestClass", "Normal Size", 0));
        menuOpen->setTitle(QApplication::translate("ImgTestClass", "Open", 0));
        menuWindow->setTitle(QApplication::translate("ImgTestClass", "Window", 0));
        menuAbout->setTitle(QApplication::translate("ImgTestClass", "Help", 0));
        menuView_Operation->setTitle(QApplication::translate("ImgTestClass", "View Operation", 0));
    } // retranslateUi

};

namespace Ui {
    class ImgTestClass: public Ui_ImgTestClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMGTEST_H
