﻿#include "MainWindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPluginLoader>
#include <QProcess>
#include <QSettings>
#include <QStandardItem>
#include <QStringList>
#include <QTime>
#include <QDir>
#include "AboutDialog.h"
#include "ConfigUtils.h"
#include "Helper.h"
#include "NewProjectDialog.h"
#include "NewVariableGroupDialog.h"
#include "ProjectData.h"
#include "ProjectDownloadDialog.h"
#include "ProjectUploadDialog.h"
#include "RealTimeDatabaseChild.h"
#include "ScriptManageChild.h"
#include "ProjectData.h"
#include "TagManagerChild.h"
#include "MainWindow.h"
#include "Helper.h"
#include "ProjectData.h"
#include "DrawListUtils.h"
#include "ProjectInfoManager.h"
#include "ProjectData.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"
#include "variantmanager.h"
#include "variantfactory.h"
#include "GraphPage.h"
#include "ChildInterface.h"
#include "SystemParametersChild.h"
#include "CommunicationDeviceChild.h"
#include "TagManagerChild.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QFileInfo>
#include <QRect>
#include <QGraphicsView>
#include <QFileDialog>
#include <QScrollArea>
#include <QToolBar>
#include <QInputDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_szCurItem(""),
      m_szCurTreeViewItem("")
{
    initUI();

}


/**
 * @brief MainWindow::initUI
 * @details 初始化UI
 */
void MainWindow::initUI()
{
    m_pCentralWidgetObj = new QWidget(this);
    QVBoxLayout *centralWidgetLayout = new QVBoxLayout(m_pCentralWidgetObj);
    centralWidgetLayout->setSpacing(0);
    centralWidgetLayout->setContentsMargins(1, 1, 1, 1);

    m_pGraphPageTabWidgetObj = new QTabWidget(m_pCentralWidgetObj);
    QSizePolicy sizePolicyGraphPageTabWidget(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicyGraphPageTabWidget.setHorizontalStretch(0);
    sizePolicyGraphPageTabWidget.setVerticalStretch(0);
    m_pGraphPageTabWidgetObj->setSizePolicy(sizePolicyGraphPageTabWidget);
    m_pGraphPageTabWidgetObj->installEventFilter(this);
    centralWidgetLayout->addWidget(m_pGraphPageTabWidgetObj);

    m_pMdiAreaObj = new MdiArea(m_pCentralWidgetObj);
    QSizePolicy sizePolicyMdiArea(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicyMdiArea.setHorizontalStretch(0);
    sizePolicyMdiArea.setVerticalStretch(0);
    m_pMdiAreaObj->setSizePolicy(sizePolicyMdiArea);
    //m_pMdiAreaObj->setLineWidth(3);
    //m_pMdiAreaObj->setFrameShape(QFrame::Panel);
    //m_pMdiAreaObj->setFrameShadow(QFrame::Sunken);
    //m_pMdiAreaObj->setViewMode(QMdiArea::TabbedView);
    ((MdiArea*)m_pMdiAreaObj)->setupMdiArea();
    QObject::connect(m_pMdiAreaObj, SIGNAL(tabCloseRequested(int)), this, SLOT(onSlotTabCloseRequested(int)));
    qApp->installEventFilter(this);
    m_pMdiAreaObj->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pMdiAreaObj->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    connect(m_pMdiAreaObj, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(onSlotUpdateMenus()));
    m_windowMapper = new QSignalMapper(this);
    connect(m_windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(onSlotSetActiveSubWindow(QWidget*)));
    centralWidgetLayout->addWidget(m_pMdiAreaObj);

    m_pCentralWidgetObj->setLayout(centralWidgetLayout);
    this->setCentralWidget(m_pCentralWidgetObj);

    // 工程管理器停靠控件
    m_pDockProjectMgrObj = new QDockWidget(this);
    m_pDockProjectMgrObj->setWindowTitle(tr("工程管理器"));
    this->addDockWidget(Qt::LeftDockWidgetArea, m_pDockProjectMgrObj);
    QWidget *dockWidgetContents = new QWidget();
    QVBoxLayout *dockWidgetContentsLayout = new QVBoxLayout(dockWidgetContents);
    dockWidgetContentsLayout->setSpacing(0);
    dockWidgetContentsLayout->setContentsMargins(0, 0, 0, 0);
    m_pTabProjectMgrObj = new QTabWidget(dockWidgetContents);

    QWidget *projectTabWidget = new QWidget();
    QVBoxLayout *projectTabWidgetLayout = new QVBoxLayout(projectTabWidget);
    projectTabWidgetLayout->setSpacing(0);
    projectTabWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pProjectTreeViewObj = new ProjectTreeView(projectTabWidget);
    m_pProjectTreeViewObj->updateUI();
    connect(m_pProjectTreeViewObj, &ProjectTreeView::sigNotifyClicked, this, &MainWindow::onSlotTreeProjectViewClicked);
    connect(m_pProjectTreeViewObj, &ProjectTreeView::sigNotifySetWindowSetTitle, this, &MainWindow::onSlotSetWindowSetTitle);
    projectTabWidgetLayout->addWidget(m_pProjectTreeViewObj);

    m_pTabProjectMgrObj->addTab(projectTabWidget, QString(tr("工程")));
    QWidget *graphPageWidget = new QWidget();
    QVBoxLayout *graphPageWidgetLayout = new QVBoxLayout(graphPageWidget);
    graphPageWidgetLayout->setSpacing(0);
    graphPageWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // 画面名称列表
    m_pListWidgetGraphPagesObj = new GraphPageListWidget(graphPageWidget);
    connect(m_pListWidgetGraphPagesObj, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(onSlotListWidgetGraphPagesCurrentTextChanged(const QString &)));

    graphPageWidgetLayout->addWidget(m_pListWidgetGraphPagesObj);
    m_pTabProjectMgrObj->addTab(graphPageWidget, QString(tr("画面")));
    dockWidgetContentsLayout->addWidget(m_pTabProjectMgrObj);
    m_pTabProjectMgrObj->setCurrentIndex(0);

    // 图形元素停靠控件
    m_pDockElemetsObj = new QDockWidget(this);
    m_pDockElemetsObj->setWindowTitle(tr("图形元素"));
    this->addDockWidget(Qt::RightDockWidgetArea, m_pDockElemetsObj);
    QWidget *dockElemetsWidget = new QWidget();
    QVBoxLayout *dockElemetsLayout = new QVBoxLayout(dockElemetsWidget);
    dockElemetsLayout->setSpacing(0);
    dockElemetsLayout->setContentsMargins(0, 0, 0, 0);
    m_pElemetsLayoutObj = new QVBoxLayout();
    m_pElemetsLayoutObj->setSpacing(0);
    dockElemetsLayout->addLayout(m_pElemetsLayoutObj);
    m_pDockElemetsObj->setWidget(dockElemetsWidget);


    // 属性停靠控件
    m_pDockPropertyObj = new QDockWidget(this);
    m_pDockPropertyObj->setWindowTitle(tr("属性编辑"));
    this->addDockWidget(Qt::RightDockWidgetArea, m_pDockPropertyObj);
    QWidget *dockPropertyWidget = new QWidget();
    QVBoxLayout *propertyWidgetLayout = new QVBoxLayout(dockPropertyWidget);
    propertyWidgetLayout->setSpacing(0);
    propertyWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pPropertyLayoutObj = new QVBoxLayout();
    m_pPropertyLayoutObj->setSpacing(0);
    propertyWidgetLayout->addLayout(m_pPropertyLayoutObj);
    m_pDockPropertyObj->setWidget(dockPropertyWidget);
    m_pDockProjectMgrObj->setWidget(dockWidgetContents);

    QSizePolicy dockPropertySizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    dockPropertySizePolicy.setHorizontalStretch(0);
    dockPropertySizePolicy.setVerticalStretch(0);
    dockWidgetContents->setSizePolicy(dockPropertySizePolicy);

    m_pUndoGroupObj = new QUndoGroup(this);

    // 创建状态栏
    createStatusBar();
    // 创建动作
    createActions();
    // 创建菜单
    createMenus();
    // 创建工具条
    createToolbars();

    m_pCurrentGraphPageObj = Q_NULLPTR;
    m_pCurrentViewObj = Q_NULLPTR;
    m_bGraphPageGridVisible = true;
    m_iCurrentGraphPageIndex = 0;

    setContextMenuPolicy(Qt::DefaultContextMenu); // 右键菜单生效
    readSettings();  // 初始窗口时读取窗口设置信息
    loadRecentProjectList();

    //--------------------------------------------------------------------------

    m_pElementWidgetObj = new ElementLibraryWidget();
    this->m_pElemetsLayoutObj->addWidget(m_pElementWidgetObj);

    m_pVariantEditorFactoryObj = new VariantFactory(this);

    //propertyEditor_ = new QtTreePropertyBrowser(dockProperty);
    m_pPropertyEditorObj = new QtTreePropertyBrowser(this);
    m_pPropertyEditorObj->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_pPropertyEditorObj->setHeaderLabels(QStringList() << tr("属性") << tr("值"));
    //propertyEditor_->setColumnWidth(0, 60);
    //propertyEditor_->setColumnWidth(1, 200);


    this->m_pPropertyLayoutObj->addWidget(m_pPropertyEditorObj);

    VariantManager *pVariantManager  = new VariantManager(this);
    m_pVariantPropertyMgrObj = pVariantManager;
    pVariantManager->setPropertyEditor(m_pPropertyEditorObj);
    m_pPropertyEditorObj->setFactoryForManager(m_pVariantPropertyMgrObj, m_pVariantEditorFactoryObj);

    connect(m_pVariantPropertyMgrObj, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
            this, SLOT(propertyValueChanged(QtProperty *, const QVariant &)));


    slotUpdateActions();
    connect(m_pGraphPageTabWidgetObj, SIGNAL(currentChanged(int)), SLOT(slotChangeGraphPage(int)));

    QDesktopWidget * pDesktopWidget = QApplication::desktop();
    QRect rect = pDesktopWidget->screenGeometry();
    int screenWidth = rect.width();
    int screenHeight = rect.height();
    this->resize(screenWidth*3/4, screenHeight*3/4);

    Helper::WidgetMoveCenter(this);

    DrawListUtils::setProjectPath(ProjectData::getInstance()->szProjPath_);

    m_pListWidgetGraphPagesObj->setContextMenuPolicy(Qt::DefaultContextMenu);


    //    setWindowState(Qt::WindowMaximized);
    setWindowTitle(tr("HmiFuncDesigner组态软件"));

    // 当多文档区域的内容超出可视区域后，出现滚动条
    this->m_pMdiAreaObj->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->m_pMdiAreaObj->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //    this->m_pMdiAreaObj->setLineWidth(3);
    //    this->m_pMdiAreaObj->setFrameShape(QFrame::Panel);
    //    this->m_pMdiAreaObj->setFrameShadow(QFrame::Sunken);
    //    this->m_pMdiAreaObj->setViewMode(QMdiArea::TabbedView);

    this->m_pStatusBarObj->showMessage(tr("欢迎使用HmiFuncDesigner组态软件"));

    connect(m_pTabProjectMgrObj, SIGNAL(currentChanged(int)), SLOT(onSlotTabProjectMgrCurChanged(int)));
    onSlotTabProjectMgrCurChanged(0);
}


MainWindow::~MainWindow()
{
    DEL_OBJ(m_pVariantPropertyMgrObj);
    DEL_OBJ(m_pPropertyEditorObj);
    DEL_OBJ(m_pVariantEditorFactoryObj);
}


/**
 * @brief MainWindow::createStatusBar
 * @details 创建状态栏
 */
void MainWindow::createStatusBar()
{
    m_pStatusBarObj = new QStatusBar(this);
    this->setStatusBar(m_pStatusBarObj);
}


/**
 * @brief MainWindow::createActions
 * @details 创建动作
 */
void MainWindow::createActions()
{
    // 新建工程
    m_pActNewProjObj = new QAction(QIcon(":/images/newproject.png"), tr("新建工程"), this);
    m_pActNewProjObj->setShortcut(QString("Ctrl+N"));
    connect(m_pActNewProjObj, &QAction::triggered, this, &MainWindow::onNewPoject);

    //  打开工程
    m_pActOpenProjObj = new QAction(QIcon(":/images/openproject.png"), tr("打开工程"), this);
    m_pActOpenProjObj->setShortcut(QString("Ctrl+O"));
    connect(m_pActOpenProjObj, &QAction::triggered, this, &MainWindow::onOpenProject);

    // 关闭工程
    m_pActCloseProjObj = new QAction(QIcon(":/images/projectexit.png"), tr("关闭"), this);
    connect(m_pActCloseProjObj, &QAction::triggered, this, &MainWindow::onCloseProject);

    // 保存工程
    m_pActSaveProjObj = new QAction(QIcon(":/images/saveproject.png"), tr("保存"), this);
    m_pActSaveProjObj->setShortcut(QString("Ctrl+S"));
    connect(m_pActSaveProjObj, &QAction::triggered, this, &MainWindow::onSaveProject);

    // 最近打开的工程
    m_pActRecentProjListObj = new QAction(tr("最近打开的工程"), this);

    m_pActExitObj = new QAction(QIcon(":/images/programexit.png"), tr("退出"), this);
    m_pActExitObj->setShortcut(QString("Ctrl+Q"));
    connect(m_pActExitObj, &QAction::triggered, this, &MainWindow::onExit);

    //-----------------------------<视图>---------------------------------------

    // 视图工具栏
    m_pActToolBarObj = new QAction(tr("视图工具栏"), this);
    m_pActToolBarObj->setCheckable(true);

    // 状态栏
    m_pActStatusBarObj = new QAction(tr("状态栏"), this);
    m_pActStatusBarObj->setCheckable(true);

    // 工作区
    m_pActWorkSpaceObj = new QAction(tr("工作区"), this);
    m_pActWorkSpaceObj->setCheckable(true);

    // 显示区
    m_pActDisplayAreaObj = new QAction(tr("显示区"), this);
    m_pActDisplayAreaObj->setCheckable(true);

    //-----------------------------<画面编辑器>----------------------------------

    // 窗口.图形元素
    m_pActShowGraphObj = new QAction(tr("图形元素"), this);
    m_pActShowGraphObj->setCheckable(true);
    m_pActShowGraphObj->setChecked(true);
    connect(m_pActShowGraphObj, SIGNAL(triggered(bool)), SLOT(onSlotShowGraphObj(bool)));

    // 窗口.属性编辑器
    m_pActShowPropEditorObj = new QAction(tr("属性编辑器"), this);
    m_pActShowPropEditorObj->setCheckable(true);
    m_pActShowPropEditorObj->setChecked(true);
    connect(m_pActShowPropEditorObj, SIGNAL(triggered(bool)), SLOT(onSlotShowPropEditor(bool)));

    // 画面.新建
    m_pActNewGraphPageObj = new QAction(QIcon(":/DrawAppImages/filenew.png"), tr("新建"), this);
    m_pActNewGraphPageObj->setShortcut(QString("Ctrl+N"));
    connect(m_pActNewGraphPageObj, SIGNAL(triggered()), SLOT(onSlotNewGraphPage()));

    // 画面.打开
    m_pActOpenObj = new QAction(QIcon(":/DrawAppImages/fileopen.png"), tr("打开"), this);
    m_pActOpenObj->setShortcut(QString("Ctrl+O"));
    connect(m_pActOpenObj, SIGNAL(triggered()), SLOT(onSlotEditOpen()));

    // 画面.保存
    m_pActSaveGraphPageObj = new QAction(QIcon(":/DrawAppImages/saveproject.png"), tr("保存"), this);
    m_pActSaveGraphPageObj->setShortcut(QKeySequence::Save);
    connect(m_pActSaveGraphPageObj, SIGNAL(triggered()), SLOT(onSlotSaveGraphPage()));

    // 关闭画面
    m_pActCloseGraphPageObj = new QAction(tr("关闭"), this);
    connect(m_pActCloseGraphPageObj, SIGNAL(triggered()), SLOT(onSlotCloseGraphPage()));

    // 关闭所有画面
    m_pActCloseAllObj = new QAction(tr("关闭所有"), this);
    connect(m_pActCloseAllObj, SIGNAL(triggered()), SLOT(onSlotCloseAll()));

    // 显示栅格
    m_pActShowGridObj = new QAction(QIcon(":/DrawAppImages/showgrid.png"), tr("显示栅格"), this);
    m_pActShowGridObj->setCheckable(true);
    m_pActShowGridObj->setChecked(m_bGraphPageGridVisible);
    connect(m_pActShowGridObj, SIGNAL(triggered(bool)), SLOT(onSlotShowGrid(bool)));

    // 画面放大
    m_pActZoomInObj = new QAction(QIcon(":/DrawAppImages/zoom-in.png"), tr("放大"), this);
    connect(m_pActZoomInObj, SIGNAL(triggered()), SLOT(onSlotZoomIn()));

    // 画面缩小
    m_pActZoomOutObj = new QAction(QIcon(":/DrawAppImages/zoom-out.png"), tr("缩小"), this);
    connect(m_pActZoomOutObj, SIGNAL(triggered()), SLOT(onSlotZoomOut()));

    // 撤销
    m_pActUndoObj = m_pUndoGroupObj->createUndoAction(this);
    m_pActUndoObj->setIcon(QIcon(":/DrawAppImages/undo.png"));
    m_pActUndoObj->setText(tr("撤销"));
    m_pActUndoObj->setShortcut(QKeySequence::Undo);

    // 重做
    m_pActRedoObj = m_pUndoGroupObj->createRedoAction(this);
    m_pActRedoObj->setText(tr("重做"));
    m_pActRedoObj->setIcon(QIcon(":/DrawAppImages/redo.png"));
    m_pActRedoObj->setShortcut(QKeySequence::Redo);

    // 删除画面
    m_pActDeleteObj = new QAction(QIcon(":/DrawAppImages/delete.png"), tr("删除"));
    m_pActDeleteObj->setShortcut(QKeySequence::Delete);
    connect(m_pActDeleteObj, SIGNAL(triggered()), SLOT(onSlotEditDelete()));

    // 拷贝画面
    m_pActCopyObj = new QAction(QIcon(":/DrawAppImages/editcopy.png"),tr("拷贝"));
    m_pActCopyObj->setShortcut(QKeySequence::Copy);
    connect(m_pActCopyObj, SIGNAL(triggered()), SLOT(onSlotEditCopy()));

    // 粘贴画面
    m_pActPasteObj = new QAction(QIcon(":/DrawAppImages/editpaste.png"),tr("粘贴"));
    m_pActPasteObj->setShortcut(QKeySequence::Paste);
    connect(m_pActPasteObj, SIGNAL(triggered()), SLOT(onSlotEditPaste()));

    // 顶部对齐
    m_pActAlignTopObj = new QAction(QIcon(":/DrawAppImages/align-top.png"), tr("顶部对齐"));
    m_pActAlignTopObj->setData(Qt::AlignTop);
    connect(m_pActAlignTopObj, SIGNAL(triggered()), SLOT(onSlotAlignElements()));

    // 底部对齐
    m_pActAlignDownObj = new QAction(QIcon(":/DrawAppImages/align-bottom.png"), tr("底部对齐"));
    m_pActAlignDownObj->setData(Qt::AlignBottom);
    connect(m_pActAlignDownObj, SIGNAL(triggered()), SLOT(onSlotAlignElements()));

    // 右对齐
    m_pActAlignRightObj = new QAction(QIcon(":/DrawAppImages/align-right.png"), tr("右对齐"));
    m_pActAlignRightObj->setData(Qt::AlignRight);
    connect(m_pActAlignRightObj, SIGNAL(triggered()), SLOT(onSlotAlignElements()));

    // 左对齐
    m_pActAalignLeftObj = new QAction(QIcon(":/DrawAppImages/align-left.png"), tr("左对齐"));
    m_pActAalignLeftObj->setData(Qt::AlignLeft);
    connect(m_pActAalignLeftObj, SIGNAL(triggered()), SLOT(onSlotAlignElements()));

    // 水平均匀分布
    m_pActHUniformDistributeObj = new QAction(QIcon(":/DrawAppImages/align_hsame.png"), tr("水平均匀分布"));
    connect(m_pActHUniformDistributeObj, SIGNAL(triggered()), SLOT(onSlotHUniformDistributeElements()));

    // 垂直均匀分布
    m_pActVUniformDistributeObj = new QAction(QIcon(":/DrawAppImages/align_vsame.png"), tr("垂直均匀分布"));
    connect(m_pActVUniformDistributeObj, SIGNAL(triggered()), SLOT(onSlotVUniformDistributeElements()));

    // 设置选中控件大小一致
    m_pActSetTheSameSizeObj = new QAction(QIcon(":/DrawAppImages/the-same-size.png"), tr("大小一致"));
    connect(m_pActSetTheSameSizeObj, SIGNAL(triggered()), SLOT(onSlotSetTheSameSizeElements()));

    // 上移一层
    m_pActUpLayerObj = new QAction(QIcon(":/DrawAppImages/posfront.png"), tr("上移一层"));
    connect(m_pActUpLayerObj, SIGNAL(triggered()), SLOT(onSlotUpLayerElements()));

    // 下移一层
    m_pActDownLayerObj = new QAction(QIcon(":/DrawAppImages/posback.png"), tr("下移一层"));
    connect(m_pActDownLayerObj, SIGNAL(triggered()), SLOT(onSlotDownLayerElements()));

    //-----------------------------<工具菜单>----------------------------------
    // 模拟仿真
    m_pActSimulateObj = new QAction(QIcon(":/images/offline.png"), tr("模拟仿真"));
    m_pActSimulateObj->setEnabled(false);
    connect(m_pActSimulateObj, SIGNAL(triggered()), SLOT(onSlotSimulate()));

    // 运行工程
    m_pActRunObj = new QAction(QIcon(":/images/online.png"), tr("运行"));
    m_pActRunObj->setEnabled(true);
    connect(m_pActSimulateObj, SIGNAL(triggered()), SLOT(onSlotRunProject()));

    // 下载工程
    m_pActDownloadObj = new QAction(QIcon(":/images/download.png"), tr("下载"));
    connect(m_pActDownloadObj, SIGNAL(triggered()), SLOT(onSlotDownloadProject()));

    // 上载工程
    m_pActUpLoadObj = new QAction(QIcon(":/images/upload.png"), tr("上载"));
    connect(m_pActUpLoadObj, SIGNAL(triggered()), SLOT(onSlotUpLoadProject()));


    //-----------------------------<窗口菜单>----------------------------------

    m_pActCloseWndObj = new QAction(tr("关闭"));
    m_pActCloseWndObj->setStatusTip(tr("关闭活动窗口"));
    connect(m_pActCloseWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::closeActiveSubWindow);

    m_pActCloseAllWndObj = new QAction(tr("关闭所有"));
    m_pActCloseAllWndObj->setStatusTip(tr("关闭所有窗口"));
    connect(m_pActCloseAllWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::closeAllSubWindows);

    m_pActTileWndObj = new QAction(tr("平铺窗口"));
    m_pActTileWndObj->setStatusTip(tr("平铺所有窗口"));
    connect(m_pActTileWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::tileSubWindows);

    m_pActCascadeWndObj = new QAction(tr("层叠窗口"), this);
    m_pActCascadeWndObj->setStatusTip(tr("层叠所有窗口"));
    connect(m_pActCascadeWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::cascadeSubWindows);

    m_pActNextWndObj = new QAction(tr("下一窗口"), this);
    m_pActNextWndObj->setShortcuts(QKeySequence::NextChild);
    m_pActNextWndObj->setStatusTip(tr("移动焦点至下一下窗口"));
    connect(m_pActNextWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::activateNextSubWindow);

    m_pActPreviousWndObj = new QAction(tr("前一窗口"), this);
    m_pActPreviousWndObj->setShortcuts(QKeySequence::PreviousChild);
    m_pActPreviousWndObj->setStatusTip(tr("移动焦点至前一下窗口"));
    connect(m_pActPreviousWndObj, &QAction::triggered, m_pMdiAreaObj, &QMdiArea::activatePreviousSubWindow);

    m_pActWindowMenuSeparatorObj = new QAction(this);
    m_pActWindowMenuSeparatorObj->setSeparator(true);

    //-----------------------------<帮助菜单>----------------------------------
    // 帮助
    m_pActHelpObj = new QAction(tr("帮助"));
    connect(m_pActHelpObj, SIGNAL(triggered()), SLOT(onSlotHelp()));

    // 关于
    m_pActAboutObj = new QAction(tr("关于"));
    connect(m_pActAboutObj, SIGNAL(triggered()), SLOT(onSlotAbout()));

}


/**
 * @brief MainWindow::createMenus
 * @details 创建菜单
 */
void MainWindow::createMenus()
{
    // 工程菜单
    m_pMenuProjectObj = this->menuBar()->addMenu(tr("工程"));
    m_pMenuProjectObj->addAction(m_pActNewProjObj);
    m_pMenuProjectObj->addAction(m_pActOpenProjObj);
    m_pMenuProjectObj->addAction(m_pActCloseProjObj);
    m_pMenuProjectObj->addAction(m_pActSaveProjObj);
    m_pMenuProjectObj->addSeparator();
    m_pMenuProjectObj->addAction(m_pActRecentProjListObj);
    m_pMenuProjectObj->addSeparator();
    m_pMenuProjectObj->addAction(m_pActExitObj);

    // 视图菜单
    m_pMenuViewObj = this->menuBar()->addMenu(tr("视图"));
    m_pMenuViewObj->addAction(m_pActToolBarObj);
    m_pMenuViewObj->addAction(m_pActStatusBarObj);
    m_pMenuViewObj->addAction(m_pActWorkSpaceObj);
    m_pMenuViewObj->addAction(m_pActDisplayAreaObj);

    //-----------------------------<画面编辑器>----------------------------------
    QMenu *filemenu = this->menuBar()->addMenu(tr("画面"));
#if 0  // for test we need this
    filemenu->addAction(m_pActNewGraphPageObj);
    filemenu->addAction(m_pActOpenObj);
#endif
    filemenu->addAction(m_pActSaveGraphPageObj);
    filemenu->addSeparator();
    filemenu->addAction(m_pActCloseGraphPageObj); // 画面.关闭
    filemenu->addAction(m_pActCloseAllObj); // 画面.关闭所有
    filemenu->addSeparator();


    QMenu *windowMenu = this->menuBar()->addMenu(tr("窗口"));
    windowMenu->addAction(m_pActShowGraphObj);
    windowMenu->addAction(m_pActShowPropEditorObj);

    //-----------------------------<工具菜单>----------------------------------
    m_pMenuToolsObj = this->menuBar()->addMenu(tr("工具"));
    m_pMenuToolsObj->addAction(m_pActSimulateObj); // 模拟仿真
    m_pMenuToolsObj->addAction(m_pActRunObj); // 运行工程
    m_pMenuToolsObj->addAction(m_pActDownloadObj); // 下载工程
    m_pMenuToolsObj->addAction(m_pActUpLoadObj); // 上传工程

    //-----------------------------<窗口菜单>----------------------------------
    m_pActWindowMenuObj = this->menuBar()->addMenu(tr("窗口"));
    connect(m_pActWindowMenuObj, &QMenu::aboutToShow, this, &MainWindow::onSlotUpdateWindowMenu);
    m_pActWindowMenuSeparatorObj = new QAction(this);
    m_pActWindowMenuSeparatorObj->setSeparator(true);
    onSlotUpdateWindowMenu();

    menuBar()->addSeparator();

    //-----------------------------<帮助菜单>----------------------------------
    m_pMenuHelpObj = this->menuBar()->addMenu(tr("帮助"));
    m_pMenuHelpObj->addAction(m_pActHelpObj); // 帮助
    m_pMenuHelpObj->addAction(m_pActAboutObj); // 关于
}


/**
    * @brief MainWindow::createToolbars
    * @details 创建工具条
    */
void MainWindow::createToolbars()
{
    //-----------------------------<工程工具栏>-----------------------------------
    m_pToolBarProjectObj = new QToolBar(this);
    m_pToolBarProjectObj->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pToolBarProjectObj->addAction(m_pActNewProjObj);
    m_pToolBarProjectObj->addAction(m_pActOpenProjObj);
    m_pToolBarProjectObj->addAction(m_pActCloseProjObj);
    m_pToolBarProjectObj->addAction(m_pActSaveProjObj);
    m_pToolBarProjectObj->addAction(m_pActExitObj);

    //-----------------------------<画面编辑器>----------------------------------
    m_pToolBarGraphPageEditObj = new QToolBar(this);
    //m_pToolBarGraphPageEditObj->addAction(m_pActSaveGraphPageObj);
    m_pToolBarGraphPageEditObj->addSeparator();
    m_pToolBarGraphPageEditObj->addAction(m_pActShowGridObj); // 显示栅格
    m_pToolBarGraphPageEditObj->addAction(m_pActZoomOutObj); //画面缩小
    m_pToolBarGraphPageEditObj->addAction(m_pActZoomInObj); // 画面放大
    m_pToolBarGraphPageEditObj->addSeparator();
    m_pToolBarGraphPageEditObj->addAction(m_pActUndoObj); // 撤销
    m_pToolBarGraphPageEditObj->addAction(m_pActRedoObj); // 重做
    m_pToolBarGraphPageEditObj->addSeparator();
    m_pToolBarGraphPageEditObj->addAction(m_pActCopyObj); // 拷贝画面
    m_pToolBarGraphPageEditObj->addAction(m_pActPasteObj); // 粘贴画面
    m_pToolBarGraphPageEditObj->addAction(m_pActDeleteObj); // 删除画面
    m_pToolBarGraphPageEditObj->addSeparator();
    m_pToolBarGraphPageEditObj->addAction(m_pActAlignTopObj); // 顶部对齐
    m_pToolBarGraphPageEditObj->addAction(m_pActAlignDownObj); // 底部对齐
    m_pToolBarGraphPageEditObj->addAction(m_pActAalignLeftObj); // 左对齐
    m_pToolBarGraphPageEditObj->addAction(m_pActAlignRightObj); // 右对齐
    m_pToolBarGraphPageEditObj->addAction(m_pActHUniformDistributeObj); // 水平均匀分布
    m_pToolBarGraphPageEditObj->addAction(m_pActVUniformDistributeObj); // 垂直均匀分布
    m_pToolBarGraphPageEditObj->addAction(m_pActSetTheSameSizeObj); // 设置选中控件大小一致
    m_pToolBarGraphPageEditObj->addAction(m_pActUpLayerObj); // 上移一层
    m_pToolBarGraphPageEditObj->addAction(m_pActDownLayerObj); // 下移一层
    m_pToolBarGraphPageEditObj->addSeparator();

    //-----------------------------<工具>----------------------------------

    m_pToolBarToolsObj = new QToolBar(this);
    m_pToolBarToolsObj->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pToolBarToolsObj->addAction(m_pActSimulateObj); // 模拟仿真
    m_pToolBarToolsObj->addAction(m_pActRunObj); // 运行工程
    m_pToolBarToolsObj->addAction(m_pActDownloadObj); // 下载工程
    m_pToolBarToolsObj->addAction(m_pActUpLoadObj); // 上传工程



    this->addToolBar(Qt::TopToolBarArea, m_pToolBarProjectObj);
    this->addToolBar(Qt::TopToolBarArea, m_pToolBarToolsObj);
    //addToolBarBreak();
    this->addToolBar(Qt::TopToolBarArea, m_pToolBarGraphPageEditObj);
}

/**
 * @brief MainWindow::onSlotUpdateWindowMenu
 * @details 更新窗口菜单
 */
void MainWindow::onSlotUpdateWindowMenu()
{
    m_pActWindowMenuObj->clear();
    m_pActWindowMenuObj->addAction(m_pActCloseWndObj);
    m_pActWindowMenuObj->addAction(m_pActCloseAllWndObj);
    m_pActWindowMenuObj->addSeparator();
    m_pActWindowMenuObj->addAction(m_pActTileWndObj);
    m_pActWindowMenuObj->addAction(m_pActCascadeWndObj);
    m_pActWindowMenuObj->addSeparator();
    m_pActWindowMenuObj->addAction(m_pActNextWndObj);
    m_pActWindowMenuObj->addAction(m_pActPreviousWndObj);
    m_pActWindowMenuObj->addAction(m_pActWindowMenuSeparatorObj);

    QList<QMdiSubWindow*> windows = m_pMdiAreaObj->subWindowList();
    m_pActWindowMenuSeparatorObj->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        QWidget* pWidgwtObj = windows.at(i)->widget();
        if(!pWidgwtObj) continue;

        ChildInterface* pIFaceChildObj = qobject_cast<ChildInterface*>(pWidgwtObj);
        if(!pIFaceChildObj) continue;

        QString szText = "";
        if (i < 9) {
            szText = tr("&%1 %2").arg(i + 1).arg(pIFaceChildObj->wndTitle());
        } else {
            szText = tr("%1 %2").arg(i + 1).arg(pIFaceChildObj->wndTitle());
        }

        QAction* pActObj  = m_pActWindowMenuObj->addAction(szText);
        pActObj->setCheckable(true);
        pActObj->setChecked(pWidgwtObj == activeMdiChild());
        connect(pActObj, SIGNAL(triggered()), m_windowMapper, SLOT(map()));
        m_windowMapper->setMapping(pActObj, windows.at(i));
    }
}


QWidget *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = this->m_pMdiAreaObj->activeSubWindow()) {
        return qobject_cast<QWidget *>(activeSubWindow->widget());
    }
    return Q_NULLPTR;
}


void MainWindow::onSlotSetActiveSubWindow(QWidget *window)
{
    if (!window) return;

    QMdiSubWindow *pMdiSubWndObj = qobject_cast<QMdiSubWindow *>(window);
    if(pMdiSubWndObj) {
        QWidget *pWidgetObj = qobject_cast<QWidget *>(pMdiSubWndObj->widget());
        if(pWidgetObj) {
            m_szCurItem = window->windowTitle();
        }
    }
    this->m_pMdiAreaObj->setActiveSubWindow(pMdiSubWndObj);

    QList<QMdiSubWindow *> subWindowList = this->m_pMdiAreaObj->subWindowList(QMdiArea::ActivationHistoryOrder);
    int iSubWndSize = subWindowList.size();
    QWidget *pLastActiveMdiChildObj = Q_NULLPTR;
    if(iSubWndSize > 1) {
        pLastActiveMdiChildObj = subWindowList.at(iSubWndSize-2)->widget();
        if (ChildInterface* pIFaceChildOldObj = qobject_cast<ChildInterface*>(pLastActiveMdiChildObj)) {
            setUpdatesEnabled(false);
            pIFaceChildOldObj->removeUserInterface(this);
            setUpdatesEnabled(true);
        }
    }

    QWidget* pChildWndObj = Q_NULLPTR;
    pChildWndObj = subWindowList.at(iSubWndSize-1)->widget();;
    if (ChildInterface* pIFaceChildNowObj = qobject_cast<ChildInterface*>(pChildWndObj)) {
        m_childCurrent = pChildWndObj;
        setUpdatesEnabled(false);
        pIFaceChildNowObj->buildUserInterface(this);
        setUpdatesEnabled(true);
    }
}

QWidget *MainWindow::getActiveSubWindow()
{
    return this->m_pMdiAreaObj->activeSubWindow()->widget();
}


QMdiSubWindow *MainWindow::findMdiChild(const QString &szWndTitle)
{
    foreach (QMdiSubWindow *wnd, this->m_pMdiAreaObj->subWindowList()) {
        ChildInterface *ifChild = qobject_cast<ChildInterface *>(wnd->widget());
        if (ifChild && ifChild->wndTitle() == szWndTitle) return wnd;
    }
    return Q_NULLPTR;
}


/*
   * 新建工程时，创建缺省IO变量组
   */
void MainWindow::CreateDefaultIOTagGroup()
{
    if (this->m_pProjectTreeViewObj->getDevTagGroupCount() == 0) {
        TagIOGroup &tagIOGroup = ProjectData::getInstance()->tagIOGroup_;
        TagIOGroupDBItem *pObj = new TagIOGroupDBItem();
        pObj->m_id = 1;
        pObj->m_szGroupName = QString("group%1").arg(pObj->m_id);
        pObj->m_szShowName = QString(tr("IO设备"));
        tagIOGroup.listTagIOGroupDBItem_.append(pObj);
        UpdateDeviceVariableTableGroup();
    }
}


bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    switch(event->type()) {
    case QEvent::ChildRemoved :
        if (QChildEvent* pEvent= static_cast<QChildEvent*>(event)) {
            if (qobject_cast<QMdiSubWindow*>(pEvent->child()) && m_childCurrent &&
                    m_pMdiAreaObj->subWindowList().size() == 1) {
                if (ChildInterface* pIFaceChildObj = qobject_cast<ChildInterface*>(m_childCurrent))
                    pIFaceChildObj->removeUserInterface(this);
                m_childCurrent = Q_NULLPTR;
            }
        }
        break;
    default:
        break;
    }
    return QMainWindow::eventFilter(obj, event);
}


/**
    * @brief MainWindow::closeEvent  关闭事件
    * @param event
    */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QString strFile = Helper::AppDir() + "/lastpath.ini";
    if (this->m_pProjectTreeViewObj->getProjectName() != tr("未创建工程"))
        ConfigUtils::setCfgStr(strFile, "PathInfo", "Path", ProjectData::getInstance()->szProjPath_);
    this->m_pMdiAreaObj->closeAllSubWindows();
    writeSettings();
    m_pMdiAreaObj->closeAllSubWindows();
    if (m_pMdiAreaObj->currentSubWindow()) {
        event->ignore();
    } else {
        event->accept();
    }


#if 0
    bool unsaved = false;

    QListIterator<GraphPage*> it(GraphPageManager::getInstance()->getGraphPageList());

    while (it.hasNext()) {
        GraphPage *graphPage = it.next();
        if (!graphPage->undoStack()->isClean() || graphPage->getUnsavedFlag()) {
            unsaved = true;
        }
    }

    if (unsaved) {
        int ret = exitResponse();

        if (ret == QMessageBox::Yes) {
            slotSaveGraphPage();
            event->accept();
        } else if (ret == QMessageBox::No) {
            event->accept();
        }
    } else {
        event->accept();
    }
#endif
}

/**
    * @brief MainWindow::writeSettings 写入窗口设置
    */
void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

/**
    * @brief MainWindow::readSettings 读取窗口设置
    */
void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() * 3 / 4, availableGeometry.height() * 3 / 4);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}


/**
    * @brief MainWindow::onNewPoject
    * @details 新建工程
    */
void MainWindow::onNewPoject()
{
    if (this->m_pProjectTreeViewObj->getProjectName() != tr("未创建工程")) {
        QMessageBox::information(this,
                                 tr("系统提示"),
                                 tr("工程文件已建立，请手动关闭当前工程文件后重新建立！"));
        return;
    }

    NewProjectDialog *pNewProjectDlg = new NewProjectDialog(this);
    if (pNewProjectDlg->exec() == QDialog::Accepted) {
        UpdateProjectName(ProjectData::getInstance()->szProjFile_);
        pNewProjectDlg->save();
        updateRecentProjectList(ProjectData::getInstance()->szProjFile_);
        CreateDefaultIOTagGroup();
        UpdateDeviceVariableTableGroup();
    }
}

void MainWindow::doOpenProject(QString proj)
{
    QFile fileProj(proj);
    if(!fileProj.exists()) {
        QMessageBox::information(this, tr("系统提示"), tr("工程：") + proj + tr("不存在！"));
        return;
    }

    ProjectData::getInstance()->szProjFile_ = proj;
    UpdateProjectName(proj);
    QString strFile = Helper::AppDir() + "/lastpath.ini";
    ConfigUtils::setCfgStr(strFile, "PathInfo", "Path", ProjectData::getInstance()->szProjPath_);

    ProjectData::getInstance()->szProjPath_ = ProjectData::getInstance()->getProjectPath(ProjectData::getInstance()->szProjFile_);
    ProjectData::getInstance()->szProjName_ = ProjectData::getInstance()->getProjectNameWithOutSuffix(ProjectData::getInstance()->szProjFile_);
    ProjectData::getInstance()->openFromXml(ProjectData::getInstance()->szProjFile_);

    // 加载设备变量组信息
    UpdateDeviceVariableTableGroup();
    updateRecentProjectList(proj);

    // 初始化画面名称列表控件
    initGraphPageListWidget();
}

/**
    * @brief MainWindow::onOpenProject
    * @details 打开工程
    */
void MainWindow::onOpenProject()
{
    QString strFile = QCoreApplication::applicationDirPath() + "/lastpath.ini";
    QString path = ConfigUtils::getCfgStr(strFile, "PathInfo", "Path", "C:/");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("选择工程文件"),
                                                    path,
                                                    tr("project file (*.pdt)"));
    if(fileName != Q_NULLPTR) {
        doOpenProject(fileName);
    }
}

void MainWindow::on_actionWorkSpace_triggered(bool checked)
{
    this->m_pDockProjectMgrObj->setVisible(checked);
}


/**
    * @brief MainWindow::onSlotProjectTreeViewClicked
    * @details 工程树节点被单击
    * @param index
    */
void MainWindow::onSlotTreeProjectViewClicked(const QString &szItemText)
{
    if(ProjectData::getInstance()->szProjFile_ == "") return;

    QString szWndTittle = szItemText;

    if(szItemText == tr("变量管理") || szItemText == tr("设备变量")) return;

    bool isTagWndFound = false;
    if(szItemText == tr("中间变量") || szItemText == tr("系统变量")) {
        szWndTittle = szItemText;
        isTagWndFound = true;
    } else {
        // 设备变量
        TagIOGroup &tagIOGroup = ProjectData::getInstance()->tagIOGroup_;
        foreach (TagIOGroupDBItem *pObj, tagIOGroup.listTagIOGroupDBItem_) {
            if (szItemText == pObj->m_szShowName) {
                szWndTittle = QString("%1%2%3").arg(tr("设备变量")).arg("-").arg(szItemText);
                isTagWndFound = true;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////

    QMdiSubWindow *pWndObj = findMdiChild(szWndTittle);
    if(pWndObj == Q_NULLPTR) {
        ChildInterface *pIFaceChildObj = Q_NULLPTR;
        if(szItemText == tr("系统参数")) {
            SystemParametersChild *pObj = new SystemParametersChild(this);
            pWndObj = this->m_pMdiAreaObj->addSubWindow(pObj);
            pObj->setWindowTitle(szWndTittle);
            pObj->showMaximized();
            pIFaceChildObj = pObj;
            pIFaceChildObj->m_szProjectName = ProjectData::getInstance()->szProjFile_;
            pIFaceChildObj->m_szItemName = szItemText;
        } else if(szItemText == tr("通讯设备") || szItemText == tr("串口设备") ||
                  szItemText == tr("网络设备")) {
            CommunicationDeviceChild *pObj = new CommunicationDeviceChild(this);
            pWndObj = this->m_pMdiAreaObj->addSubWindow(pObj);
            pObj->setWindowTitle(szWndTittle);
            pObj->showMaximized();
            pIFaceChildObj = pObj;
            pIFaceChildObj->m_szProjectName = ProjectData::getInstance()->szProjFile_;
            pIFaceChildObj->m_szItemName = szItemText;
        } else if(isTagWndFound) { // 标签变量
            TagManagerChild *pObj = new TagManagerChild(this);
            pWndObj = this->m_pMdiAreaObj->addSubWindow(pObj);
            pObj->setWindowTitle(szWndTittle);
            pObj->showMaximized();
            pIFaceChildObj = pObj;
            pIFaceChildObj->m_szProjectName = ProjectData::getInstance()->szProjFile_;
            pIFaceChildObj->m_szItemName = szItemText;
        } else if(szItemText == tr("实时数据库")) {
            RealTimeDatabaseChild *pObj = new RealTimeDatabaseChild(this);
            pWndObj = this->m_pMdiAreaObj->addSubWindow(pObj);
            pObj->setWindowTitle(szWndTittle);
            pObj->showMaximized();
            pIFaceChildObj = pObj;
            pIFaceChildObj->m_szProjectName = ProjectData::getInstance()->szProjFile_;
            pIFaceChildObj->m_szItemName = szItemText;
        } else if(szItemText == tr("脚本编辑器")) {
            ScriptManageChild *pObj = new ScriptManageChild(this);
            pWndObj = this->m_pMdiAreaObj->addSubWindow(pObj);
            pObj->setWindowTitle(szWndTittle);
            pObj->showMaximized();
            pIFaceChildObj = pObj;
            pIFaceChildObj->m_szProjectName = ProjectData::getInstance()->szProjFile_;
            pIFaceChildObj->m_szItemName = szItemText;
        }
    }

    onSlotSetActiveSubWindow(pWndObj);
}


/**
 * @brief MainWindow::UpdateProjectName
 * @details 更新工程名称
 * @param szName 工程名称
 */
void MainWindow::UpdateProjectName(const QString &szName)
{
    if(!szName.isEmpty()) {
        QString szNameTmp = szName.mid(szName.lastIndexOf("/") + 1, szName.indexOf(".") - szName.lastIndexOf("/") - 1);
        this->m_pProjectTreeViewObj->setProjectName(szNameTmp);
        this->m_pActRunObj->setEnabled(true);
    } else {
        ProjectData::getInstance()->szProjFile_ = "";
        ProjectData::getInstance()->szProjPath_ = "";
        this->m_pActRunObj->setEnabled(false);
        this->m_pProjectTreeViewObj->updateUI();
    }
}

/*
    * 更新设备变量组
    */
void MainWindow::UpdateDeviceVariableTableGroup()
{
    this->m_pProjectTreeViewObj->updateDeviceTagGroup();
}


/**
    * @brief MainWindow::onSaveProject
    * @details 保存工程
    */
void MainWindow::onSaveProject()
{
    //m_szProjFile
    ProjectData::getInstance()->saveToXml(ProjectData::getInstance()->szProjFile_);
    foreach (QMdiSubWindow* window, this->m_pMdiAreaObj->subWindowList()) {
        ChildInterface *pIFaceChildObj = qobject_cast<ChildInterface *>(window->widget());
        //if(pIFaceChildObj != Q_NULLPTR) pIFaceChildObj->save();
    }

    // 保存画面
    onSlotSaveGraphPage();
}


/**
    * @brief MainWindow::onCloseProject
    * @details 关闭工程
    */
void MainWindow::onCloseProject()
{
    if(this->m_pProjectTreeViewObj->getProjectName() == tr("未创建工程"))
        return;
    foreach (QMdiSubWindow* window, this->m_pMdiAreaObj->subWindowList()) {
        ChildInterface *pIFaceChildObj = qobject_cast<ChildInterface *>(window->widget());
        //if(pIFaceChildObj != Q_NULLPTR) pIFaceChildObj->save();
        window->close();
    }
    UpdateProjectName(QString());

    // 清空画面列表控件
    clearGraphPageListWidget();

    // 关闭所有画面
    onSlotCloseAll();
    onSlotTabProjectMgrCurChanged(0);
    m_pTabProjectMgrObj->setCurrentIndex(0);
}


/**
    * @brief MainWindow::onExit
    * @details 退出
    */
void MainWindow::onExit()
{
    onSaveProject();
    qApp->exit();
}


/**
    * @brief MainWindow::onSlotSimulate
    * @details 模拟仿真
    */
void MainWindow::onSlotSimulate()
{

}

/**
    * @brief MainWindow::onSlotRunProject
    * @details 运行工程
    */
void MainWindow::onSlotRunProject()
{
    QString fileRuntimeApplication = "";
#ifdef Q_OS_WIN
    fileRuntimeApplication = QDir::cleanPath(Helper::AppDir() + "/../../HmiRunTimeBin/HmiRunTime.exe");
#endif

#ifdef Q_OS_LINUX
    fileRuntimeApplication = QDir::cleanPath(Helper::AppDir() + "/../../HmiRunTimeBin/HmiRunTime");
#endif

    QFile file(fileRuntimeApplication);
    if (file.exists()) {
        QProcess *process = new QProcess();
        process->setWorkingDirectory(Helper::AppDir() + "/");
        QStringList argv;
        argv << ProjectData::getInstance()->szProjPath_;
        process->startDetached(fileRuntimeApplication, argv);
    }
}

/**
    * @brief MainWindow::onSlotUpLoadProject
    * @details 上载工程
    */
void MainWindow::onSlotUpLoadProject()
{
    // 创建tmp目录
    QString tmpDir = QCoreApplication::applicationDirPath() + "/UploadProjects/tmp";
    QDir dir(tmpDir);
    if (!dir.exists()) {
        dir.mkpath(tmpDir);
    }

    ProjectUploadDialog *pDlg = new ProjectUploadDialog(this, ProjectData::getInstance()->szProjFile_);
    if (pDlg->exec() == QDialog::Accepted) {
        QString desDir = pDlg->getProjectPath();
        QString program = QCoreApplication::applicationDirPath() + "/tar/tar.exe";
        QFile programFile(program);
        if (!programFile.exists()) {
            QMessageBox::information(this, "系统提示", "命令：" + program + "不存在！");
            return;
        }
        QProcess *tarProc = new QProcess;
        // 设置进程工作目录
        tarProc->setWorkingDirectory(QCoreApplication::applicationDirPath() + "/tar");
        QStringList arguments;
        arguments << "-xvf"
                  << "../UploadProjects/RunProject.tar"
                  << "-C"
                  << "../UploadProjects/tmp";
        tarProc->start(program, arguments);
        if (tarProc->waitForStarted()) {
            if (tarProc->waitForFinished()) {
                QString strSrc = QCoreApplication::applicationDirPath() +
                        "/UploadProjects/tmp/RunProject";

                Helper::CopyDir(strSrc, desDir, true);
                Helper::DeleteDir(tmpDir);

                QString tarProj = QCoreApplication::applicationDirPath() +
                        "/UploadProjects/RunProject.tar";
                QFile tarProjFile(tarProj);
                if (tarProjFile.exists()) {
                    tarProjFile.remove();
                }
            }
        } else {
            QMessageBox::information(this, "系统提示", "解压缩工程失败！");
        }

        delete tarProc;
    }
    delete pDlg;
}


/**
    * @brief MainWindow::onSlotDownloadProject
    * @details 下载工程
    */
void MainWindow::onSlotDownloadProject()
{
    if(ProjectData::getInstance()->szProjFile_ == Q_NULLPTR) return;

    // 创建tmp目录
    QString tmpDir = QCoreApplication::applicationDirPath() + "/tmp";
    QDir dir(tmpDir);
    if(!dir.exists()) {
        dir.mkpath(tmpDir);
    }

    // 拷贝工程到tmp目录
    QString desDir = QCoreApplication::applicationDirPath() + "/tmp/RunProject";
    Helper::CopyRecursively(ProjectData::getInstance()->szProjPath_, desDir);

    // 打包工程到tmp目录
    QString program = QCoreApplication::applicationDirPath() + "/tar/tar.exe";
    QFile programFile(program);
    if(!programFile.exists()) {
        QMessageBox::information(this, "系统提示", "命令：" + program + "不存在！");
        return;
    }

    QProcess *tarProc = new QProcess;
    // 设置进程工作目录
    tarProc->setWorkingDirectory(QCoreApplication::applicationDirPath() + "/tar");
    QStringList arguments;
    arguments << "-cvf"
              << "../tmp/RunProject.tar"
              << "-C"
              << "../tmp"
              << "RunProject";
    tarProc->start(program, arguments);
    if (tarProc->waitForStarted()) {
        if (tarProc->waitForFinished(-1)) {
            // 压缩完成准备传输文件

        }
        if (tarProc->exitStatus() == QProcess::NormalExit) {

        } else {  // QProcess::CrashExit

        }
    } else {
        QMessageBox::information(this, "系统提示", "压缩工程失败！");
    }

    QDir dirRunProj(desDir);
    if (dirRunProj.exists()) {
        Helper::DeleteDir(desDir);
    }

    delete tarProc;

    ProjectDownloadDialog *pDlg = new ProjectDownloadDialog(this, ProjectData::getInstance()->szProjFile_);
    pDlg->setProjFileName(QCoreApplication::applicationDirPath() + "/tmp/RunProject.tar");
    if (pDlg->exec() == QDialog::Accepted) {
    }
    delete pDlg;
}


/**
    * @brief MainWindow::onSlotHelp
    * @details 帮助
    */
void MainWindow::onSlotHelp()
{

}


/**
    * @brief MainWindow::onSlotAbout
    * @details 关于
    */
void MainWindow::onSlotAbout()
{
    AboutDialog *pDlg = new AboutDialog(this);
    if(pDlg->exec() == QDialog::Accepted) {

    }
    delete pDlg;
}

/*
    * 加载最近打开的工程列表
    */
void MainWindow::loadRecentProjectList()
{
    QString iniRecentProjectFileName = Helper::AppDir() + "/RecentProjectList.ini";
    QFile fileCfg(iniRecentProjectFileName);
    if(fileCfg.exists()) {
        QStringList slist;
        ConfigUtils::getCfgList(iniRecentProjectFileName, "RecentProjects", "project", slist);

        for (int i=0; i<slist.count(); i++) {
            QAction *pAct = new QAction(slist.at(i), this);
            pAct->setStatusTip(tr(""));
            connect(pAct, &QAction::triggered, this, [=]{
                this->doOpenProject(pAct->text());
            });
            this->m_pMenuProjectObj->insertAction(this->m_pActRecentProjListObj, pAct);
        }
        this->m_pMenuProjectObj->removeAction(this->m_pActRecentProjListObj);
        return;
    }

    QList<QAction *> listActRemove;
    QList<QAction *> listAct = this->m_pMenuProjectObj->actions();
    for (int i = 0; i<listAct.size(); ++i) {
        QAction *pAct = listAct.at(i);
        if(pAct->isSeparator())
            listActRemove.append(pAct);
        if(pAct->text() == tr("最近文件列表"))
            listActRemove.append(pAct);
    }
    for (int j = 0; j<listActRemove.size(); ++j) {
        this->m_pMenuProjectObj->removeAction(listActRemove.at(j));
    }
}

/*
    * 更新最近打开的工程列表
    */
void MainWindow::updateRecentProjectList(QString newProj)
{
    bool bStart = false;
    bool bEnd = false;

    QString iniRecentProjectFileName = Helper::AppDir() + "/RecentProjectList.ini";
    QFile fileCfg(iniRecentProjectFileName);
    if(fileCfg.exists()) {
        QStringList slist;
        ConfigUtils::getCfgList(iniRecentProjectFileName, "RecentProjects", "project", slist);

        for (int i = 0; i < slist.count(); i++) {
            if (newProj == slist.at(i)) {
                return;
            }
        }

        if (slist.count() >= 5) slist.removeLast();

        slist.push_front(newProj);
        ConfigUtils::writeCfgList(iniRecentProjectFileName, "RecentProjects", "project", slist);

        QList<QAction *> listActRemove;
        QList<QAction *> listAct = this->m_pMenuProjectObj->actions();
        for (int i = 0; i < listAct.size(); ++i) {
            QAction *pAct = listAct.at(i);
            if(pAct->isSeparator() && bStart == false && bEnd == false) {
                bStart = true;
                continue;
            }
            if(pAct->isSeparator() && bStart && bEnd == false) {
                bEnd = true;
                bStart = false;
                break;
            }
            if(bStart && bEnd == false) {
                listActRemove.append(pAct);
            }
            if(pAct->text() == tr("最近文件列表"))
                listActRemove.append(pAct);
        }
        for (int j = 0; j<listActRemove.size(); ++j) {
            this->m_pMenuProjectObj->removeAction(listActRemove.at(j));
        }

        /////////////////////////////////////////////////////

        bStart = bEnd = false;
        QAction *pActPos = Q_NULLPTR;
        listAct.clear();
        listAct = this->m_pMenuProjectObj->actions();
        for (int i = 0; i < listAct.size(); ++i) {
            QAction *pAct = listAct.at(i);
            if(pAct->isSeparator() && bStart == false && bEnd == false) {
                bStart = true;
                continue;
            }
            if(pAct->isSeparator() && bStart && bEnd == false) {
                bEnd = true;
                pActPos = pAct;
                bStart = false;
                break;
            }
        }

        for (int i=0; i<slist.count(); i++) {
            QAction *pAct = new QAction(slist.at(i), this);
            pAct->setStatusTip(tr(""));
            connect(pAct, &QAction::triggered, this, [=]{
                this->doOpenProject(pAct->text());
            });
            this->m_pMenuProjectObj->insertAction(pActPos, pAct);
        }
    } else {
        QStringList slist;
        ConfigUtils::writeCfgList(iniRecentProjectFileName, "RecentProjects", "project", slist);
    }
}


//------------------------------------------------------------------------------


/**
    * @brief openGraphPage
    * @details 打开画面
    * @param pagePath 画面路径
    * @param pagePath 画面名称
    */
void MainWindow::openGraphPage(const QString &szProjPath,
                               const QString &szProjName,
                               const QString &szPageName)
{
    DrawListUtils::loadDrawList(szProjPath);
    foreach(QString szPageId, DrawListUtils::drawList_) {
        this->m_pListWidgetGraphPagesObj->addItem(szPageId);
        QString fileName = szProjPath + "/" + szPageId + ".drw";
        if (fileName.toLower().endsWith(".drw")) {
            QGraphicsView *view = createTabView();
            if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
                delete view;
                return;
            }
            GraphPage *graphPage = new GraphPage(QRectF(), m_pVariantPropertyMgrObj, m_pPropertyEditorObj);
            if (!createDocument(graphPage, view, fileName)) return;
            if((szPageId == szPageName) || (szPageName == tr("") && this->m_pListWidgetGraphPagesObj->count() == 1)) {
                m_pCurrentGraphPageObj = graphPage;
                m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(view);
            }
            graphPage->setProjectPath(szProjPath);
            graphPage->setProjectName(szProjName);
            graphPage->setGridVisible(m_bGraphPageGridVisible);
            graphPage->loadAsXML(fileName);
            view->setFixedSize(graphPage->getGraphPageWidth(), graphPage->getGraphPageHeight());
            graphPage->setFileName(szPageId + ".drw");
            graphPage->setGraphPageId(szPageId);
            graphPage->fillGraphPagePropertyModel();
        }
    }

    QList<QListWidgetItem*> listWidgetItem = this->m_pListWidgetGraphPagesObj->findItems(szPageName, Qt::MatchCaseSensitive);
    if ( listWidgetItem.size() > 0 ) {
        this->m_pListWidgetGraphPagesObj->setCurrentItem(listWidgetItem.at(0));
    } else {
        this->m_pListWidgetGraphPagesObj->setCurrentRow(0);
    }
}


/**
    * @brief MainWindow::onSlotNewGraphPage
    * @details [画面.新建] 动作响应函数
    */
void MainWindow::onSlotNewGraphPage()
{
    addNewGraphPage();
}

QString MainWindow::fixedWindowTitle(const QGraphicsView *viewGraphPage) const
{
    QString title = m_pCurrentGraphPageObj->getGraphPageId();

    if (title.isEmpty()) {
        title = "Untitled";
    } else {
        title = QFileInfo(title).fileName();
    }

    QString result;

    for (int i = 0; ;++i) {
        result = title;

        if (i > 0) {
            result += QString::number(i);
        }

        bool unique = true;

        for (int j = 0; j < m_pGraphPageTabWidgetObj->count(); ++j) {
            const QWidget *widget = m_pGraphPageTabWidgetObj->widget(j);

            if (widget == viewGraphPage) {
                continue;
            }

            if (result == m_pGraphPageTabWidgetObj->tabText(j)) {
                unique = false;
                break;
            }
        }

        if (unique) {
            break;
        }
    }

    return result;
}

bool MainWindow::isGraphPageOpen(const QString &filename)
{
    QListIterator <GraphPage*> it(GraphPageManager::getInstance()->getGraphPageList());

    while (it.hasNext()) {
        if (filename == it.next()->getFileName()) {
            return true;
        }
    }

    return false;
}

void MainWindow::addNewGraphPage()
{
    QGraphicsView *view = createTabView();

    if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
        delete view;
        return;
    }

    GraphPage *graphPage = new GraphPage(QRectF(), m_pVariantPropertyMgrObj, m_pPropertyEditorObj);
    graphPage->setProjectPath(ProjectData::getInstance()->szProjPath_);
    graphPage->setProjectName(ProjectData::getInstance()->szProjFile_);
    graphPage->setGridVisible(m_bGraphPageGridVisible);
    m_pCurrentGraphPageObj = graphPage;
    view->setScene(graphPage);
    view->setFixedSize(graphPage->getGraphPageWidth(), graphPage->getGraphPageHeight());
    m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(view);
    QString title = fixedWindowTitle(view);
    graphPage->setFileName(title + ".drw");
    graphPage->setGraphPageId(title);
    m_pGraphPageTabWidgetObj->addTab(m_pCurrentViewObj, title);
    m_pGraphPageTabWidgetObj->setCurrentWidget(m_pCurrentViewObj);
    GraphPageManager::getInstance()->addGraphPage(graphPage);

    m_pUndoGroupObj->addStack(graphPage->undoStack());
    m_pUndoGroupObj->setActiveStack(graphPage->undoStack());

    connectGraphPage(graphPage);
}

void MainWindow::connectGraphPage(GraphPage *graphPage)
{
    connect(graphPage->undoStack(), SIGNAL(indexChanged(int)), SLOT(slotUpdateActions()));
    connect(graphPage->undoStack(), SIGNAL(cleanChanged(bool)), SLOT(slotUpdateActions()));
    connect(graphPage, SIGNAL(newElementAdded()), SLOT(slotNewElementAdded()));
    connect(graphPage, SIGNAL(elementsDeleted()), SLOT(slotElementsDeleted()));
    connect(graphPage, SIGNAL(elementIdChanged()), SLOT(slotElementIdChanged()));
    connect(graphPage, SIGNAL(changeGraphPageName()), SLOT(slotChangeGraphPageName()));
    connect(graphPage, SIGNAL(selectionChanged()), SLOT(slotUpdateActions()));
    connect(graphPage, SIGNAL(elementPropertyChanged()), SLOT(slotUpdateActions()));
    connect(graphPage, SIGNAL(GraphPagePropertyChanged()), SLOT(slotUpdateActions()));
    connect(graphPage, SIGNAL(GraphPageSaved()), SLOT(slotUpdateActions()));
}

void MainWindow::disconnectGraphPage(GraphPage *graphPage)
{
    disconnect(graphPage->undoStack(), SIGNAL(indexChanged(int)), this, SLOT(slotUpdateActions()));
    disconnect(graphPage->undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(slotUpdateActions()));
    disconnect(graphPage, SIGNAL(newElementAdded()), this, SLOT(slotNewElementAdded()));
    disconnect(graphPage, SIGNAL(elementsDeleted()), this, SLOT(slotElementsDeleted()));
    disconnect(graphPage, SIGNAL(elementIdChanged()), this, SLOT(slotElementIdChanged()));
    disconnect(graphPage, SIGNAL(changeGraphPageName()), this, SLOT(slotChangeGraphPageName()));
    disconnect(graphPage, SIGNAL(selectionChanged()), this, SLOT(slotUpdateActions()));
    disconnect(graphPage, SIGNAL(elementPropertyChanged()), this, SLOT(slotUpdateActions()));
    disconnect(graphPage, SIGNAL(GraphPagePropertyChanged()), this, SLOT(slotUpdateActions()));
    disconnect(graphPage, SIGNAL(GraphPageSaved()), this, SLOT(slotUpdateActions()));
}

QGraphicsView *MainWindow::createTabView()
{
    QGraphicsView *view = new QGraphicsView();
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    return view;
}

void MainWindow::slotUpdateActions()
{
    static const QIcon unsaved(":/DrawAppImages/filesave.png");

    for (int i = 0; i < m_pGraphPageTabWidgetObj->count(); i++) {
        QGraphicsView *view = dynamic_cast<QGraphicsView*>(m_pGraphPageTabWidgetObj->widget(i));
        if (!dynamic_cast<GraphPage *>(view->scene())->undoStack()->isClean() ||
                dynamic_cast<GraphPage *>(view->scene())->getUnsavedFlag()) {
            m_pGraphPageTabWidgetObj->setTabIcon(m_pGraphPageTabWidgetObj->indexOf(view), unsaved);
        } else {
            m_pGraphPageTabWidgetObj->setTabIcon(m_pGraphPageTabWidgetObj->indexOf(view), QIcon());
        }
    }

    m_pActZoomInObj->setEnabled(m_pGraphPageTabWidgetObj->count() ? true : false);
    m_pActZoomOutObj->setEnabled(m_pGraphPageTabWidgetObj->count() ? true : false);
    m_pActShowGridObj->setEnabled(m_pGraphPageTabWidgetObj->count() ? true : false);

    if (!m_pCurrentGraphPageObj) {
        return;
    }

    m_pUndoGroupObj->setActiveStack(m_pCurrentGraphPageObj->undoStack());

    if (!m_pCurrentGraphPageObj->undoStack()->isClean() || m_pCurrentGraphPageObj->getUnsavedFlag()) {
        m_pActSaveGraphPageObj->setEnabled(true);
    } else {
        m_pActSaveGraphPageObj->setEnabled(false);
    }
}

void MainWindow::slotChangeGraphPage(int iGraphPageNum)
{
    if (iGraphPageNum == -1) return;

    if(iGraphPageNum != this->m_pListWidgetGraphPagesObj->currentRow())
        this->m_pListWidgetGraphPagesObj->setCurrentRow(iGraphPageNum);

    for (int i = 0; i < m_pGraphPageTabWidgetObj->count(); i++) {
        QGraphicsView *view = dynamic_cast<QGraphicsView *>(m_pGraphPageTabWidgetObj->widget(i));
        dynamic_cast<GraphPage *>(view->scene())->setActive(false);
    }

    m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(m_pGraphPageTabWidgetObj->widget(iGraphPageNum));
    m_pCurrentGraphPageObj = dynamic_cast<GraphPage *>(m_pCurrentViewObj->scene());
    m_pCurrentGraphPageObj->setActive(true);
    //currentGraphPage_->fillGraphPagePropertyModel();
    m_iCurrentGraphPageIndex = iGraphPageNum;
    slotUpdateActions();
}

void MainWindow::slotChangeGraphPageName()
{
    m_pGraphPageTabWidgetObj->setTabText(m_iCurrentGraphPageIndex, m_pCurrentGraphPageObj->getGraphPageId());
    //int index = GraphPageManager::getInstance()->getIndexByGraphPage(currentGraphPage_);
}


void MainWindow::slotElementIdChanged()
{

}

void MainWindow::slotElementPropertyChanged()
{

}

void MainWindow::slotGraphPagePropertyChanged()
{

}

void MainWindow::propertyValueChanged(QtProperty *property, const QVariant &value)
{
    Q_UNUSED(property)
    Q_UNUSED(value)
}

void MainWindow::slotNewElementAdded()
{

}

void MainWindow::slotElementsDeleted()
{

}


/**
    * @brief MainWindow::onSlotShowGrid
    * @details 显示栅格
    * @param on true-显示，false-隐藏
    */
void MainWindow::onSlotShowGrid(bool on)
{
    QListIterator <GraphPage*> iter(GraphPageManager::getInstance()->getGraphPageList());

    while (iter.hasNext()) {
        iter.next()->setGridVisible(on);
    }

    m_bGraphPageGridVisible = on;
}


/**
    * @brief MainWindow::onSlotShowGraphObj
    * @details [窗口.图形元素] 动作响应函数; 图形元素停靠控件的显示或隐藏
    * @param on true-显示, false-隐藏
    */
void MainWindow::onSlotShowGraphObj(bool on)
{
    this->m_pDockElemetsObj->setVisible(on);
}


/**
    * @brief MainWindow::slotShowPropEditor
    * @details [窗口.属性编辑器] 动作响应函数; 属性编辑器停靠控件的显示或隐藏
    * @param on true-显示, false-隐藏
    */
void MainWindow::onSlotShowPropEditor(bool on)
{
    this->m_pDockPropertyObj->setVisible(on);
}


/**
    * @brief MainWindow::onSlotCloseAll
    * @details 关闭所有画面
    */
void MainWindow::onSlotCloseAll()
{
    while (m_pGraphPageTabWidgetObj->count()) {
        QGraphicsView *view = static_cast<QGraphicsView*>(m_pGraphPageTabWidgetObj->widget(m_pGraphPageTabWidgetObj->currentIndex()));
        removeGraphPage(view);
        delete view;
    }

    m_pCurrentViewObj = Q_NULLPTR;
    m_pCurrentGraphPageObj = Q_NULLPTR;
    slotUpdateActions();
}

void MainWindow::removeGraphPage(QGraphicsView *view)
{
    int index = m_pGraphPageTabWidgetObj->indexOf(view);
    GraphPage *graphPage = static_cast<GraphPage*>(view->scene());

    if (index == -1)
        return;

    if (!graphPage->undoStack()->isClean()) {
        int ret = exitResponse();

        if (ret == QMessageBox::Yes) {
            onSlotSaveGraphPage();
        }
    }

    m_pGraphPageTabWidgetObj->removeTab(index);
    m_pUndoGroupObj->removeStack(graphPage->undoStack());
    GraphPageManager::getInstance()->removeGraphPage(graphPage);
    disconnectGraphPage(graphPage);
    delete graphPage;
}


/**
    * @brief MainWindow::onSlotCloseGraphPage
    * @details 关闭画面
    */
void MainWindow::onSlotCloseGraphPage()
{
    QGraphicsView *view = m_pCurrentViewObj;
    removeGraphPage(view);
    delete view;

    if (m_pGraphPageTabWidgetObj->count() == 0) {
        m_pCurrentGraphPageObj = Q_NULLPTR;
        m_pCurrentViewObj = Q_NULLPTR;
    }

    slotUpdateActions();
}


/**
    * @brief MainWindow::onSlotEditOpen
    * @details [画面.打开] 动作响应函数
    */
void MainWindow::onSlotEditOpen()
{
    const QString &filename = QFileDialog::getOpenFileName(this,
                                                           tr("Open"),
                                                           ".",
                                                           tr("GraphPage (*.drw)"));
    if (filename.isEmpty())
        return;
#if 0
    if (filename.toLower().endsWith(".drwb")) {

        QGraphicsView *view = createTabView();

        if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
            delete view;
            return;
        }

        GraphPage *graphPage = new GraphPage(QRectF());
        if (!createDocument(graphPage,view,filename)) {
            return;
        }
        graphPage->loadAsBinary(filename);
    }
#endif
    if (filename.toLower().endsWith(".drw")) {

        QGraphicsView *view = createTabView();

        if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
            delete view;
            return;
        }

        GraphPage *graphPage = new GraphPage(QRectF(), m_pVariantPropertyMgrObj, m_pPropertyEditorObj);
        if (!createDocument(graphPage, view, filename)) {
            return;
        }

        m_pCurrentGraphPageObj = graphPage;
        m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(view);
        graphPage->setProjectPath(ProjectData::getInstance()->szProjPath_);
        graphPage->setProjectName(ProjectData::getInstance()->szProjFile_);
        graphPage->loadAsXML(filename);
        int pos = filename.lastIndexOf("/");
        QString pageFileName = "";
        if (pos != -1) {
            pageFileName = filename.right(filename.length() - pos - 1);
        }
        graphPage->setFileName(pageFileName);
        graphPage->setGraphPageId(pageFileName.left(pageFileName.length() - 4));
    }
}

bool MainWindow::createDocument(GraphPage *graphPage,
                                QGraphicsView *view,
                                const QString &filename)
{
    if (isGraphPageOpen(filename)) {
        QMessageBox::information(this,
                                 tr("打开文件错误"),
                                 tr("文件已打开"),
                                 QMessageBox::Ok);
        delete graphPage;
        delete view;
        return false;
    }

    graphPage->setGridVisible(m_bGraphPageGridVisible);
    view->setScene(graphPage);
    view->setFixedSize(graphPage->getGraphPageWidth(), graphPage->getGraphPageHeight());
    m_pGraphPageTabWidgetObj->addTab(view, graphPage->getGraphPageId());
    m_pGraphPageTabWidgetObj->setCurrentWidget(view);
    GraphPageManager::getInstance()->addGraphPage(graphPage);

    m_pUndoGroupObj->addStack(graphPage->undoStack());
    m_pUndoGroupObj->setActiveStack(graphPage->undoStack());

    connectGraphPage(graphPage);

    graphPage->undoStack()->setClean();

    return true;
}


QString MainWindow::getFileName()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save as"),
                                                    QString("./%1").arg(m_pCurrentGraphPageObj->getGraphPageId()),
                                                    tr("GraphPage(*.drw)"));
    return filename;
}

void MainWindow::updateGraphPageViewInfo(const QString &fileName)
{
    int index = m_pGraphPageTabWidgetObj->indexOf(m_pCurrentViewObj);
    QFileInfo file(fileName);
    m_pCurrentGraphPageObj->setGraphPageId(file.baseName());
    m_pGraphPageTabWidgetObj->setTabText(index,file.baseName());
    slotChangeGraphPageName();
}

/**
    * @brief MainWindow::onSlotEditOpen
    * @details [画面.保存] 动作响应函数
    */
void MainWindow::onSlotSaveGraphPage()
{
    if (!m_pCurrentGraphPageObj) return;

    for (;;) {
        QString fileName = m_pCurrentGraphPageObj->getFileName();
        if (fileName.isEmpty()) fileName = getFileName();
        if (fileName.isEmpty()) break;
        m_pCurrentGraphPageObj->setFileName(fileName);
        updateGraphPageViewInfo(fileName);
        m_pCurrentGraphPageObj->saveAsXML(ProjectData::getInstance()->szProjPath_ + "/" + fileName);
#if 0
        if (fileName.toLower().endsWith(".drw")) {
            QString binaryFileName = fileName.toLower()+ "b"; // ".drw"==>".drwb"
            currentGraphPage->saveAsBinary(szProjPath_ + "/" + binaryFileName);
        }
#endif
        break;
    }
}


int MainWindow::exitResponse()
{
    int ret = QMessageBox::information(this,
                                       tr("退出程序"),
                                       tr("文件已修改。是否保存?"),
                                       QMessageBox::Yes | QMessageBox::No);
    return ret;
}

/**
    * @brief MainWindow::onSlotZoomIn
    * @details 画面放大
    */
void MainWindow::onSlotZoomIn()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        int width = m_pCurrentGraphPageObj->getGraphPageWidth();
        int height = m_pCurrentGraphPageObj->getGraphPageHeight();
        m_pCurrentGraphPageObj->setGraphPageWidth(static_cast<int>(width * 1.25));
        m_pCurrentGraphPageObj->setGraphPageHeight(static_cast<int>(height * 1.25));
        m_pCurrentGraphPageObj->setGridVisible(m_pCurrentGraphPageObj->isGridVisible());
    }
    if (m_pCurrentViewObj != Q_NULLPTR) {
        m_pCurrentViewObj->scale(1.25, 1.25);
        m_pCurrentViewObj->setFixedSize(m_pCurrentGraphPageObj->getGraphPageWidth(), m_pCurrentGraphPageObj->getGraphPageHeight());
    }
}


/**
    * @brief MainWindow::onSlotZoomOut
    * @details 画面缩小
    */
void MainWindow::onSlotZoomOut()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        int width = m_pCurrentGraphPageObj->getGraphPageWidth();
        int height = m_pCurrentGraphPageObj->getGraphPageHeight();
        m_pCurrentGraphPageObj->setGraphPageWidth(static_cast<int>(width * 1/1.25));
        m_pCurrentGraphPageObj->setGraphPageHeight(static_cast<int>(height * 1/1.25));
        m_pCurrentGraphPageObj->setGridVisible(m_pCurrentGraphPageObj->isGridVisible());
    }
    if (m_pCurrentViewObj != Q_NULLPTR) {
        m_pCurrentViewObj->scale(1/1.25, 1/1.25);
        m_pCurrentViewObj->setFixedSize(m_pCurrentGraphPageObj->getGraphPageWidth(), m_pCurrentGraphPageObj->getGraphPageHeight());
    }
}


/**
    * @brief MainWindow::onSlotAlignElements
    * @details 顶部对齐, 底部对齐, 右对齐, 左对齐
    */
void MainWindow::onSlotAlignElements()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    Qt::Alignment alignment = static_cast<Qt::Alignment>(action->data().toInt());
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onAlignElements(alignment, items);
    }
}


/**
    * @brief MainWindow::onSlotHUniformDistributeElements
    * @details 水平均匀分布
    */
void MainWindow::onSlotHUniformDistributeElements()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onHUniformDistributeElements(items);
    }
}


/**
    * @brief MainWindow::onSlotVUniformDistributeElements
    * @details 垂直均匀分布
    */
void MainWindow::onSlotVUniformDistributeElements()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onVUniformDistributeElements(items);
    }
}


/**
    * @brief MainWindow::onSlotSetTheSameSizeElements
    * @details 设置选中控件大小一致
    */
void MainWindow::onSlotSetTheSameSizeElements()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onSetTheSameSizeElements(items);
    }
}


/**
    * @brief MainWindow::onSlotUpLayerElements
    * @details 上移一层
    */
void MainWindow::onSlotUpLayerElements()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onUpLayerElements(items);
    }
}


/**
    * @brief MainWindow::onSlotDownLayerElements
    * @details 下移一层
    */
void MainWindow::onSlotDownLayerElements()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onDownLayerElements(items);
    }
}


/**
    * @brief MainWindow::slotEditDelete
    * @details 删除画面
    */
void MainWindow::onSlotEditDelete()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onEditDelete(items);
    }
}


/**
    * @brief MainWindow::slotEditCopy
    * @details 拷贝画面
    */
void MainWindow::onSlotEditCopy()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onEditCopy(items);
    }
}


/**
    * @brief MainWindow::slotEditPaste
    * @details 粘贴画面
    */
void MainWindow::onSlotEditPaste()
{
    if(m_pCurrentGraphPageObj != Q_NULLPTR) {
        QList<QGraphicsItem*> items = m_pCurrentGraphPageObj->selectedItems();
        m_pCurrentGraphPageObj->onEditPaste();
    }
}


/**
    * @brief MainWindow::onSlotListWidgetGraphPagesCurrentTextChanged
    * @details 画面名称被单击
    * @param item
    */
void MainWindow::onSlotListWidgetGraphPagesCurrentTextChanged(const QString &szText)
{
    if(szText == tr("")) return;
    if(this->m_pListWidgetGraphPagesObj->currentRow() != this->m_pGraphPageTabWidgetObj->currentIndex());
    m_pGraphPageTabWidgetObj->setCurrentIndex(this->m_pListWidgetGraphPagesObj->currentRow());
}


/**
    * @brief MainWindow::createEmptyGraphpage
    * @details 创建空的画面页
    * @param projPath 工程路径
    * @param graphPageName 画面名称
    * @param width 画面宽度
    * @param height 画面高度
    */
void MainWindow::createEmptyGraphpage(const QString &projPath,
                                      const QString &graphPageName,
                                      int width,
                                      int height)
{
    QString fileName = projPath + "/" + graphPageName + ".drw";
    QString szContent = QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<graphPage fileName=\"%1.drw\" graphPageId=\"%1\" "
                "width=\"%2\" height=\"%3\" background=\"#ffffff\">\n"
                "</graphPage>")
            .arg(graphPageName)
            .arg(QString::number(width))
            .arg(QString::number(height));

    Helper::writeString(fileName, szContent);
}


/**
    * @brief MainWindow::onNewGraphPage
    * @details 新建画面
    */
void MainWindow::onNewGraphPage()
{
    int last = DrawListUtils::getMaxDrawPageNum("draw");
    QString szGraphPageName = QString("draw%1").arg(last);

    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("画面名称"));
    dlg.setLabelText(tr("请输入画面名称"));
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setTextValue(szGraphPageName);

reInput:
    if ( dlg.exec() == QDialog::Accepted ) {
        szGraphPageName = dlg.textValue();
        if ( szGraphPageName == "" ) {
            goto reInput;
        }

        QList<GraphPage*> listGraphPage = GraphPageManager::getInstance()->getGraphPageList();

        int width = 480;
        int height = 272;
        if ( listGraphPage.size() > 0 ) {
            GraphPage* pGraphPage = listGraphPage.at(0);
            width = pGraphPage->getGraphPageWidth();
            height = pGraphPage->getGraphPageHeight();
        }

        createEmptyGraphpage(ProjectData::getInstance()->szProjPath_, szGraphPageName, width, height);
        DrawListUtils::drawList_.append(szGraphPageName);
        DrawListUtils::saveDrawList(ProjectData::getInstance()->szProjPath_);

        this->m_pListWidgetGraphPagesObj->addItem(szGraphPageName);
        QString fileName = ProjectData::getInstance()->szProjPath_ + "/" + szGraphPageName + ".drw";

        if (fileName.toLower().endsWith(".drw")) {
            QGraphicsView *view = createTabView();

            if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
                delete view;
                return;
            }

            GraphPage *graphPage = new GraphPage(QRectF(), m_pVariantPropertyMgrObj, m_pPropertyEditorObj);
            if (!createDocument(graphPage, view, fileName)) {
                return;
            }

            m_pCurrentGraphPageObj = graphPage;
            m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(view);
            graphPage->setProjectPath(ProjectData::getInstance()->szProjPath_);
            graphPage->setProjectName(ProjectData::getInstance()->szProjFile_);
            graphPage->loadAsXML(fileName);
            view->setFixedSize(graphPage->getGraphPageWidth(), graphPage->getGraphPageHeight());
            graphPage->setFileName(szGraphPageName + ".drw");
            graphPage->setGraphPageId(szGraphPageName);
        }

        QList<QListWidgetItem*> listWidgetItem = this->m_pListWidgetGraphPagesObj->findItems(szGraphPageName, Qt::MatchCaseSensitive);
        if ( listWidgetItem.size() > 0 ) {
            this->m_pListWidgetGraphPagesObj->setCurrentItem(listWidgetItem.at(0));
            m_pGraphPageTabWidgetObj->setCurrentIndex(this->m_pListWidgetGraphPagesObj->currentRow());
        }
    }
}


/**
    * @brief MainWindow::onRenameGraphPage
    * @details 修改画面名称
    */
void MainWindow::onRenameGraphPage()
{
    QString szOldGraphPageName = this->m_pListWidgetGraphPagesObj->currentItem()->text();

    QInputDialog dlg(this);
    dlg.setWindowTitle(tr("修改画面名称"));
    dlg.setLabelText(tr("请输入画面名称"));
    dlg.setOkButtonText(tr("确定"));
    dlg.setCancelButtonText(tr("取消"));
    dlg.setTextValue(szOldGraphPageName);

reInput:
    if (dlg.exec() == QDialog::Accepted) {
        QString szNewGraphPageName = dlg.textValue();

        if (szNewGraphPageName == "") {
            goto reInput;
        }

        for (int i = 0; i < DrawListUtils::drawList_.count(); i++) {
            if ( szOldGraphPageName == DrawListUtils::drawList_.at(i) ) {
                DrawListUtils::drawList_.replace(i, szNewGraphPageName);
                QString szOldName = ProjectData::getInstance()->szProjPath_ + "/" + szOldGraphPageName + ".drw";
                QString szNewName = ProjectData::getInstance()->szProjPath_ + "/" + szNewGraphPageName + ".drw";
                QFile::rename(szOldName, szNewName);
                DrawListUtils::saveDrawList(ProjectData::getInstance()->szProjPath_);
                this->m_pListWidgetGraphPagesObj->currentItem()->setText(szNewGraphPageName);
                GraphPage *pGraphPage = GraphPageManager::getInstance()->getGraphPageById(szOldGraphPageName);
                pGraphPage->setFileName(szNewGraphPageName + ".drw");
                pGraphPage->setGraphPageId(szNewGraphPageName);
                m_pGraphPageTabWidgetObj->setTabText(m_pGraphPageTabWidgetObj->currentIndex(), szNewGraphPageName);
                m_pCurrentGraphPageObj->setUnsavedFlag(true);
                slotUpdateActions();
                break;
            }
        }
    }
}


/**
    * @brief MainWindow::onDeleteGraphPage
    * @details 删除画面
    */
void MainWindow::onDeleteGraphPage()
{
    QString szGraphPageName = this->m_pListWidgetGraphPagesObj->currentItem()->text();

    for (int i = 0; i < DrawListUtils::drawList_.count(); i++) {
        if ( szGraphPageName == DrawListUtils::drawList_.at(i) ) {
            DrawListUtils::drawList_.removeAt(i);

            QString fileName = ProjectData::getInstance()->szProjPath_ + "/" + szGraphPageName + ".drw";
            QFile file(fileName);
            if (file.exists()) {
                file.remove();
            }

            m_pGraphPageTabWidgetObj->removeTab(this->m_pListWidgetGraphPagesObj->currentRow());

            GraphPage *pGraphPageObj = GraphPageManager::getInstance()->getGraphPageById(szGraphPageName);
            if ( pGraphPageObj != Q_NULLPTR ) {
                GraphPageManager::getInstance()->removeGraphPage(pGraphPageObj);
                delete pGraphPageObj;
                pGraphPageObj = Q_NULLPTR;
            }

            DrawListUtils::saveDrawList(ProjectData::getInstance()->szProjPath_);

            this->m_pListWidgetGraphPagesObj->clear();
            foreach(QString szPageId, DrawListUtils::drawList_) {
                this->m_pListWidgetGraphPagesObj->addItem(szPageId);
            }

            if (this->m_pListWidgetGraphPagesObj->count() > 0) {
                this->m_pListWidgetGraphPagesObj->setCurrentRow(0);
                m_pGraphPageTabWidgetObj->setCurrentIndex(0);
            }

            break;
        }
    }
}


/**
    * @brief MainWindow::onCopyGraphPage
    * @details 复制画面
    */
void MainWindow::onCopyGraphPage()
{
    m_szCopyGraphPageFileName = this->m_pListWidgetGraphPagesObj->currentItem()->text();
}


/**
    * @brief MainWindow::onPasteGraphPage
    * @details 粘贴画面
    */
void MainWindow::onPasteGraphPage()
{
    int iLast = 0;

reGetNum:
    iLast = DrawListUtils::getMaxDrawPageNum(m_szCopyGraphPageFileName);
    QString strDrawPageName = m_szCopyGraphPageFileName + QString("-%1").arg(iLast);
    if ( DrawListUtils::drawList_.contains(strDrawPageName )) {
        m_szCopyGraphPageFileName = strDrawPageName;
        goto reGetNum;
    }

    this->m_pListWidgetGraphPagesObj->addItem(strDrawPageName);
    DrawListUtils::drawList_.append(strDrawPageName);
    DrawListUtils::saveDrawList(ProjectData::getInstance()->szProjPath_);
    QString szFileName = ProjectData::getInstance()->szProjPath_ + "/" + m_szCopyGraphPageFileName + ".drw";
    QFile file(szFileName);
    QString szPasteFileName = ProjectData::getInstance()->szProjPath_ + "/" + strDrawPageName + ".drw";
    file.copy(szPasteFileName);

    if (szPasteFileName.toLower().endsWith(".drw")) {
        QGraphicsView *view = createTabView();

        if (m_pGraphPageTabWidgetObj->indexOf(view) != -1) {
            delete view;
            return;
        }

        GraphPage *graphPage = new GraphPage(QRectF(), m_pVariantPropertyMgrObj, m_pPropertyEditorObj);
        if (!createDocument(graphPage, view, szPasteFileName)) {
            return;
        }

        m_pCurrentGraphPageObj = graphPage;
        m_pCurrentViewObj = dynamic_cast<QGraphicsView *>(view);
        graphPage->setProjectPath(ProjectData::getInstance()->szProjPath_);
        graphPage->setProjectName(ProjectData::getInstance()->szProjFile_);
        graphPage->loadAsXML(szPasteFileName);
        view->setFixedSize(graphPage->getGraphPageWidth(), graphPage->getGraphPageHeight());
        graphPage->setFileName(strDrawPageName + ".drw");
        graphPage->setGraphPageId(strDrawPageName);
    }

    QList<QListWidgetItem*> listWidgetItem = this->m_pListWidgetGraphPagesObj->findItems(strDrawPageName, Qt::MatchCaseSensitive);
    if ( listWidgetItem.size() > 0 ) {
        this->m_pListWidgetGraphPagesObj->setCurrentItem(listWidgetItem.at(0));
        m_pGraphPageTabWidgetObj->setCurrentIndex(this->m_pListWidgetGraphPagesObj->currentRow());
    }

    m_pCurrentGraphPageObj->setUnsavedFlag(true);
    slotUpdateActions();
}


/**
    * @brief MainWindow::initGraphPageListWidget
    * @details 初始化画面名称列表控件
    */
void MainWindow::initGraphPageListWidget()
{
    this->m_pListWidgetGraphPagesObj->clear();
    this->m_pListWidgetGraphPagesObj->setProjectPath(ProjectData::getInstance()->szProjPath_);
    this->openGraphPage(ProjectData::getInstance()->szProjPath_, ProjectData::getInstance()->szProjFile_, tr(""));
}


/**
    * @brief MainWindow::clearGraphPageListWidget
    * @details 清空画面列表控件
    */
void MainWindow::clearGraphPageListWidget()
{
    this->m_pListWidgetGraphPagesObj->clear();
    m_pListWidgetGraphPagesObj->setProjectPath(QString());
}


/**
    * @brief MainWindow::onSlotSetWindowSetTitle
    * @details 设置窗口标题
    * @param szTitle 标题
    */
void MainWindow::onSlotSetWindowSetTitle(const QString &szTitle)
{
    //    ChildForm *window = findMdiChild(this->m_szCurItem);
    //    if(window != Q_NULLPTR) {
    //        window->SetTitle(szTitle);
    //    }
}


/**
    * @brief MainWindow::onSlotTabProjectMgrCurChanged
    * @details 工程管理器标签改变
    * @param index 0：工程标签页, 1：画面标签页
    */
void MainWindow::onSlotTabProjectMgrCurChanged(int index)
{
    if(ProjectData::getInstance()->szProjFile_ == "") {
        m_pMdiAreaObj->setVisible(true);
        m_pGraphPageTabWidgetObj->setVisible(false);
        m_pDockPropertyObj->setVisible(false); // 属性停靠控件
        m_pDockElemetsObj->setVisible(false); // 图形元素停靠控件
        m_pToolBarGraphPageEditObj->setVisible(false); // 画面编辑工具条
    } else {
        m_pMdiAreaObj->setVisible(index == 0);
        m_pGraphPageTabWidgetObj->setVisible(index == 1);
        m_pDockPropertyObj->setVisible(index == 1); // 属性停靠控件
        m_pDockElemetsObj->setVisible(index == 1); // 图形元素停靠控件
        m_pToolBarGraphPageEditObj->setVisible(index == 1);
    }
}

/**
    * @brief MainWindow::onSlotTabCloseRequested
    * @details 子窗口关闭请求
    */
void MainWindow::onSlotTabCloseRequested(int index)
{
    QList<QMdiSubWindow*> list = m_pMdiAreaObj->subWindowList();
    if (index < 0 || index >= list.count()) return;
    QMdiSubWindow* mdiSubWindow = list[index];
    mdiSubWindow->close();
}


/**
    * @brief MainWindow::onSlotUpdateMenus
    * @details 更新菜单
    */
void MainWindow::onSlotUpdateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    m_pActSaveProjObj->setEnabled(hasMdiChild);
    m_pActCloseProjObj->setEnabled(hasMdiChild);

    m_pActCloseWndObj->setEnabled(hasMdiChild);
    m_pActCloseAllWndObj->setEnabled(hasMdiChild);
    m_pActTileWndObj->setEnabled(hasMdiChild);
    m_pActCascadeWndObj->setEnabled(hasMdiChild);
    m_pActNextWndObj->setEnabled(hasMdiChild);
    m_pActPreviousWndObj->setEnabled(hasMdiChild);
    m_pActWindowMenuSeparatorObj->setVisible(hasMdiChild);
}



