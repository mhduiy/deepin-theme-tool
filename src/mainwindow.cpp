#include "mainwindow.h"
#include "dcitoolsview.h"
#include <QLayout>
#include <QStandardItemModel>
#include <DVerticalLine>
#include <DTitlebar>
#include <QStackedWidget>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_navList(new DListView())
    , m_stackedWidget(new QStackedWidget())
    , m_dciToolsView(new DciToolsView())
{
    initUI();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    QWidget *mainWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);
    mainLayout->addWidget(m_navList);
    mainLayout->addWidget(new DVerticalLine());
    mainLayout->addWidget(m_stackedWidget);
    setCentralWidget(mainWidget);

    m_stackedWidget->setContentsMargins(QMargins(0, 0, 0, 0));

    QStandardItemModel *navModel = new QStandardItemModel();
    m_navList->setModel(navModel);
    m_navList->setFixedWidth(150);
    m_navList->setEditTriggers(DListView::EditTrigger::NoEditTriggers);

    navModel->appendRow(new DStandardItem("Dci工具"));
    m_stackedWidget->addWidget(m_dciToolsView);
    navModel->appendRow(new DStandardItem("主题工具"));
    m_stackedWidget->addWidget(new QLabel("开发中..."));

    navModel->appendRow(new DStandardItem("关于此工具"));
    QWidget *aboutThisToolWidget = new QWidget();
    QVBoxLayout *aboutThisToolLayout = new QVBoxLayout(aboutThisToolWidget);
    QPushButton *openSrcCodeBtn = new QPushButton("查看源码");
    aboutThisToolLayout->addWidget(openSrcCodeBtn);
    m_stackedWidget->addWidget(aboutThisToolWidget);
    connect(openSrcCodeBtn, &QPushButton::clicked, this, [](){
        QDesktopServices::openUrl(QUrl("https://github.com/mhduiy/deepin-theme-tool"));
    });

    QModelIndex index = m_navList->model()->index(0,0);
    m_navList->setCurrentIndex(index);
    m_stackedWidget->setCurrentIndex(0);

    connect(m_navList, &DListView::pressed, [this](const QModelIndex &index){
        m_stackedWidget->setCurrentIndex(index.row());
    });
}

void MainWindow::initTitlebar()
{

}