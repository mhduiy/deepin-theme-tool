#include "dcitoolsview.h"
#include "funcitem.h"
#include <QLayout>
#include <QPainter>
#include <DScrollArea>
#include <DSuggestButton>
#include <QDesktopServices>
#include <QProcess>

static auto constexpr DciDocUrl = R"(https://github.com/linuxdeepin/deepin-specifications/blob/master/unstable/%E5%9B%BE%E6%A0%87%E6%96%87%E4%BB%B6%E8%A7%84%E8%8C%83.md)";

DciToolsView::DciToolsView(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins(0, 0, 0, 0));
    DScrollArea *scrollArea = new DScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    layout->addWidget(scrollArea);

    QWidget *mainWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(mainWidget);
    m_mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    scrollArea->setWidget(mainWidget);

    QHBoxLayout *hBoxLayout = new QHBoxLayout();
    m_readDciDocBtn = new DSuggestButton("阅读Dci规范文档");
    hBoxLayout->addWidget(m_readDciDocBtn);
    m_installDciCliToolBtn = new DSuggestButton("安装Dci命令行工具");
    hBoxLayout->addWidget(m_installDciCliToolBtn);
    m_runDciCliBtn = new DSuggestButton("在终端中启动命令行工具");
    hBoxLayout->addWidget(m_runDciCliBtn);

    m_mainLayout->addLayout(hBoxLayout);

    initDciAnalysisWidget();
    initDciConvertWidget();

    connect(m_readDciDocBtn, &DSuggestButton::clicked, this, &DciToolsView::openDciDoc);
    connect(m_installDciCliToolBtn, &DSuggestButton::clicked, this, &DciToolsView::installDciCli);
    connect(m_runDciCliBtn, &DSuggestButton::clicked, this, &DciToolsView::runDciDciTool);


    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));
    m_mainLayout->addWidget(new FuncItem(this, "TextTest", "TextTest"));

    m_mainLayout->addStretch();
}

void DciToolsView::initDciAnalysisWidget()
{
    m_dciAnalysisItem = new FuncItem(this, "Dci解析", "树形解析，解压");

    QWidget *widget = new QWidget();
    m_dciPathEdit = new DLineEdit();
    m_dciExtractEdit = new DLineEdit();
    m_selectDciButton = new DSuggestButton("选择Dci");
    m_treeButton = new DSuggestButton("解析为树形结构");
    m_extraButton = new DSuggestButton("解压导出");

    m_dciPathEdit->setPlaceholderText("单个dci图标的路径");
    m_dciExtractEdit->setPlaceholderText("dci解压导出的路径");

    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setColumnStretch(0, 3);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->addWidget(m_dciPathEdit, 0, 0);
    gridLayout->addWidget(m_selectDciButton, 0, 1);
    gridLayout->addWidget(m_dciExtractEdit, 1, 0);
    gridLayout->addWidget(m_extraButton, 1, 1);
    gridLayout->addWidget(m_treeButton, 2, 1);

    m_mainLayout->addWidget(m_dciAnalysisItem);
    m_dciAnalysisItem->setWidget(widget);
}

void DciToolsView::initDciConvertWidget()
{
    m_dciConvert = new FuncItem(this, "Dci转换", "转换svg, png等图标为dci图标");

    QWidget *widget = new QWidget();
    m_srcConvertPathEdit = new DLineEdit();
    m_tarConvertPathEdit = new DLineEdit();
    m_srcConvertSelectBtn = new DSuggestButton("选择");
    m_tarConvertSelectBtn = new DSuggestButton("选择");
    m_convertBtn = new DSuggestButton("开始转换");

    m_srcConvertPathEdit->setPlaceholderText("待转换路径或文件");
    m_tarConvertPathEdit->setPlaceholderText("导出路径");

    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setColumnStretch(0, 3);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->addWidget(m_srcConvertPathEdit, 0, 0);
    gridLayout->addWidget(m_srcConvertSelectBtn, 0, 1);
    gridLayout->addWidget(m_tarConvertPathEdit, 1, 0);
    gridLayout->addWidget(m_tarConvertSelectBtn, 1, 1);
    gridLayout->addWidget(m_convertBtn, 2, 1);

    m_mainLayout->addWidget(m_dciConvert);
    m_dciConvert->setWidget(widget);
}

void DciToolsView::openDciDoc()
{
    QDesktopServices::openUrl(QUrl(DciDocUrl));
}

void DciToolsView::runDciDciTool()
{
    QStringList args;
    args << "-w" << "/usr/libexec/dtk6/DGui/bin";
    QProcess::startDetached("/usr/bin/deepin-terminal", args);
}

void DciToolsView::installDciCli()
{
    QStringList args;
    args << "-e" << "sudo apt install libdtk6gui-bin";
    QProcess::startDetached("/usr/bin/deepin-terminal", args);
}
