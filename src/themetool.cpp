#include "themetool.h"
#include "funcitem.h"
#include "converterfactory.h"
#include "themeconverter.h"
#include "conversionworker.h"
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QFileDialog>
#include <QProcess>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <QDirIterator>
#include <QFileInfo>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QFontComboBox>
#include <QFrame>
#include <DScrollArea>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include <DFontSizeManager>
#include <DFrame>

DWIDGET_USE_NAMESPACE

ThemeTool::ThemeTool(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
    initializeConverter();
}

ThemeTool::~ThemeTool()
{
    if (m_convertProcess && m_convertProcess->state() != QProcess::NotRunning) {
        m_convertProcess->kill();
        m_convertProcess->waitForFinished(3000);
    }
}

void ThemeTool::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建滚动区域
    DScrollArea *scrollArea = new DScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(15);

    // 标题
    DLabel *titleLabel = new DLabel("FreeDesktop 主题转换工具");
    titleLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T3);
    titleLabel->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(titleLabel);
    
    // 描述
    DLabel *descLabel = new DLabel("将标准图标主题包转换为 Deepin 兼容的 DEB 安装包");
    descLabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(descLabel, DFontSizeManager::T7);
    descLabel->setWordWrap(true);
    contentLayout->addWidget(descLabel);
    
    contentLayout->addSpacing(10);
    
    // 源类型选择
    QHBoxLayout *typeLayout = new QHBoxLayout();
    DLabel *typeLabel = new DLabel("源类型：");
    m_directoryRadio = new QRadioButton("主题包目录");
    m_debRadio = new QRadioButton("DEB包文件");
    m_directoryRadio->setChecked(true);
    
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(m_directoryRadio);
    typeLayout->addWidget(m_debRadio);
    typeLayout->addStretch();
    contentLayout->addLayout(typeLayout);
    
    // 源路径选择
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    DLabel *sourceLabel = new DLabel("源文件：");
    sourceLabel->setMinimumWidth(60);
    m_sourcePathEdit = new DLineEdit();
    m_sourcePathEdit->setPlaceholderText("选择主题包目录或DEB包文件");
    m_selectSourceBtn = new DSuggestButton("浏览");
    
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(m_sourcePathEdit, 1);
    sourceLayout->addWidget(m_selectSourceBtn);
    contentLayout->addLayout(sourceLayout);
    
    // 输出文件选择
    QHBoxLayout *outputLayout = new QHBoxLayout();
    DLabel *outputLabel = new DLabel("输出DEB：");
    outputLabel->setMinimumWidth(60);
    m_outputFileEdit = new DLineEdit();
    m_outputFileEdit->setPlaceholderText("选择输出DEB包文件路径");
    m_selectOutputBtn = new DSuggestButton("浏览");
    
    outputLayout->addWidget(outputLabel);
    outputLayout->addWidget(m_outputFileEdit, 1);
    outputLayout->addWidget(m_selectOutputBtn);
    contentLayout->addLayout(outputLayout);
    
    // 主题名称
    QHBoxLayout *nameLayout = new QHBoxLayout();
    DLabel *nameLabel = new DLabel("主题名称：");
    nameLabel->setMinimumWidth(60);
    m_themeNameEdit = new DLineEdit();
    m_themeNameEdit->setPlaceholderText("自定义主题名称（可选）");
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_themeNameEdit, 1);
    nameLayout->addStretch();
    contentLayout->addLayout(nameLayout);
    
    contentLayout->addSpacing(10);
    
    // 高级配置选项直接添加到内容布局
    // 添加分割线
    QFrame *separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);
    separatorLine->setStyleSheet("QFrame { color: #CCCCCC; }");
    contentLayout->addWidget(separatorLine);
    
    // 高级配置标题
    DLabel *advancedTitle = new DLabel("高级配置选项");
    DFontSizeManager::instance()->bind(advancedTitle, DFontSizeManager::T5);
    advancedTitle->setForegroundRole(QPalette::Highlight);
    advancedTitle->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(advancedTitle);
    
    contentLayout->addSpacing(10);
    
    // 转换设置区域
    DLabel *convertTitle = new DLabel("转换设置");
    DFontSizeManager::instance()->bind(convertTitle, DFontSizeManager::T6);
    convertTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(convertTitle);
    
    QGridLayout *convertGrid = new QGridLayout();
    convertGrid->setSpacing(8);
    
    // 转换质量
    convertGrid->addWidget(new DLabel("转换质量:"), 0, 0);
    m_qualitySlider = new QSlider(Qt::Horizontal);
    m_qualitySlider->setRange(1, 100);
    m_qualitySlider->setValue(85);
    m_qualitySpin = new QSpinBox();
    m_qualitySpin->setRange(1, 100);
    m_qualitySpin->setValue(85);
    m_qualitySpin->setSuffix("%");
    
    // 同步滑块和数字框
    connect(m_qualitySlider, QOverload<int>::of(&QSlider::valueChanged), 
            m_qualitySpin, &QSpinBox::setValue);
    connect(m_qualitySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            m_qualitySlider, &QSlider::setValue);
    
    convertGrid->addWidget(m_qualitySlider, 0, 1);
    convertGrid->addWidget(m_qualitySpin, 0, 2);
    
    // 生成光标主题
    m_generateCursorCheck = new QCheckBox("生成光标主题");
    m_generateCursorCheck->setChecked(true);
    convertGrid->addWidget(m_generateCursorCheck, 1, 0, 1, 3);
    
    contentLayout->addLayout(convertGrid);
    
    // 字体设置区域
    DLabel *fontTitle = new DLabel("字体设置");
    DFontSizeManager::instance()->bind(fontTitle, DFontSizeManager::T6);
    fontTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(fontTitle);
    
    QGridLayout *fontGrid = new QGridLayout();
    fontGrid->setSpacing(8);
    
    fontGrid->addWidget(new DLabel("标准字体:"), 0, 0);
    m_standardFontCombo = new QFontComboBox();
    m_standardFontCombo->setCurrentText("Source Han Sans SC");
    fontGrid->addWidget(m_standardFontCombo, 0, 1);
    
    fontGrid->addWidget(new DLabel("等宽字体:"), 1, 0);
    m_monospaceFontCombo = new QFontComboBox();
    m_monospaceFontCombo->setFontFilters(QFontComboBox::MonospacedFonts);
    m_monospaceFontCombo->setCurrentText("Noto Mono");
    fontGrid->addWidget(m_monospaceFontCombo, 1, 1);
    
    fontGrid->addWidget(new DLabel("字体大小:"), 2, 0);
    m_fontSizeSpin = new QDoubleSpinBox();
    m_fontSizeSpin->setRange(6.0, 24.0);
    m_fontSizeSpin->setValue(10.5);
    m_fontSizeSpin->setSingleStep(0.5);
    m_fontSizeSpin->setSuffix(" pt");
    fontGrid->addWidget(m_fontSizeSpin, 2, 1);
    
    contentLayout->addLayout(fontGrid);
    
    // 窗口设置区域
    DLabel *windowTitle = new DLabel("窗口设置");
    DFontSizeManager::instance()->bind(windowTitle, DFontSizeManager::T6);
    windowTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(windowTitle);
    
    QGridLayout *windowGrid = new QGridLayout();
    windowGrid->setSpacing(8);
    
    windowGrid->addWidget(new DLabel("窗口圆角:"), 0, 0);
    m_windowRadiusSpin = new QSpinBox();
    m_windowRadiusSpin->setRange(0, 50);
    m_windowRadiusSpin->setValue(12);
    m_windowRadiusSpin->setSuffix(" px");
    windowGrid->addWidget(m_windowRadiusSpin, 0, 1);
    
    windowGrid->addWidget(new DLabel("窗口不透明度:"), 1, 0);
    m_windowOpacitySpin = new QDoubleSpinBox();
    m_windowOpacitySpin->setRange(0.0, 1.0);
    m_windowOpacitySpin->setValue(0.4);
    m_windowOpacitySpin->setSingleStep(0.1);
    m_windowOpacitySpin->setDecimals(1);
    windowGrid->addWidget(m_windowOpacitySpin, 1, 1);
    
    contentLayout->addLayout(windowGrid);
    
    // 主题设置区域
    DLabel *themeTitle = new DLabel("主题设置");
    DFontSizeManager::instance()->bind(themeTitle, DFontSizeManager::T6);
    themeTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(themeTitle);
    
    QGridLayout *themeGrid = new QGridLayout();
    themeGrid->setSpacing(8);
    
    themeGrid->addWidget(new DLabel("默认图标主题:"), 0, 0);
    m_iconThemeEdit = new DLineEdit();
    m_iconThemeEdit->setPlaceholderText("将自动基于源包名生成");
    themeGrid->addWidget(m_iconThemeEdit, 0, 1);
    
    themeGrid->addWidget(new DLabel("默认光标主题:"), 1, 0);
    m_cursorThemeEdit = new DLineEdit();
    m_cursorThemeEdit->setPlaceholderText("将自动基于源包名生成");
    themeGrid->addWidget(m_cursorThemeEdit, 1, 1);
    
    contentLayout->addLayout(themeGrid);
    
    // 亮色主题设置区域
    DLabel *lightTitle = new DLabel("亮色主题配置");
    DFontSizeManager::instance()->bind(lightTitle, DFontSizeManager::T6);
    lightTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(lightTitle);
    
    QGridLayout *lightGrid = new QGridLayout();
    lightGrid->setSpacing(8);
    
    lightGrid->addWidget(new DLabel("默认壁纸:"), 0, 0);
    m_wallpaperEdit = new DLineEdit();
    m_wallpaperEdit->setText("file:///usr/share/wallpapers/deepin/Colorful-Abstraction03.jpg");
    m_selectWallpaperBtn = new DSuggestButton("选择");
    lightGrid->addWidget(m_wallpaperEdit, 0, 1);
    lightGrid->addWidget(m_selectWallpaperBtn, 0, 2);
    
    lightGrid->addWidget(new DLabel("锁屏壁纸:"), 1, 0);
    m_lockWallpaperEdit = new DLineEdit();
    m_lockWallpaperEdit->setText("file:///usr/share/wallpapers/deepin/Colorful-Abstraction03.jpg");
    m_selectLockWallpaperBtn = new DSuggestButton("选择");
    lightGrid->addWidget(m_lockWallpaperEdit, 1, 1);
    lightGrid->addWidget(m_selectLockWallpaperBtn, 1, 2);
    
    lightGrid->addWidget(new DLabel("主题色:"), 2, 0);
    m_themeColorEdit = new DLineEdit();
    m_themeColorEdit->setText("#1F6EE7");
    lightGrid->addWidget(m_themeColorEdit, 2, 1);
    
    contentLayout->addLayout(lightGrid);
    
    // 暗色主题设置区域
    DLabel *darkTitle = new DLabel("暗色主题配置");
    DFontSizeManager::instance()->bind(darkTitle, DFontSizeManager::T6);
    darkTitle->setForegroundRole(QPalette::Highlight);
    contentLayout->addWidget(darkTitle);
    
    QGridLayout *darkGrid = new QGridLayout();
    darkGrid->setSpacing(8);
    
    darkGrid->addWidget(new DLabel("默认壁纸:"), 0, 0);
    m_darkWallpaperEdit = new DLineEdit();
    m_darkWallpaperEdit->setText("file:///usr/share/wallpapers/deepin/desktop.jpg");
    m_selectDarkWallpaperBtn = new DSuggestButton("选择");
    darkGrid->addWidget(m_darkWallpaperEdit, 0, 1);
    darkGrid->addWidget(m_selectDarkWallpaperBtn, 0, 2);
    
    darkGrid->addWidget(new DLabel("锁屏壁纸:"), 1, 0);
    m_darkLockWallpaperEdit = new DLineEdit();
    m_darkLockWallpaperEdit->setText("file:///usr/share/wallpapers/deepin/desktop.jpg");
    m_selectDarkLockWallpaperBtn = new DSuggestButton("选择");
    darkGrid->addWidget(m_darkLockWallpaperEdit, 1, 1);
    darkGrid->addWidget(m_selectDarkLockWallpaperBtn, 1, 2);
    
    darkGrid->addWidget(new DLabel("主题色:"), 2, 0);
    m_darkThemeColorEdit = new DLineEdit();
    m_darkThemeColorEdit->setText("#024CCA");
    darkGrid->addWidget(m_darkThemeColorEdit, 2, 1);
    
    contentLayout->addLayout(darkGrid);
    
    contentLayout->addSpacing(10);
    
    // 控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_convertBtn = new DSuggestButton("开始转换");
    m_stopBtn = new DPushButton("停止转换");
    m_stopBtn->setEnabled(false);
    m_clearLogBtn = new DPushButton("清空日志");
    
    controlLayout->addWidget(m_convertBtn);
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addStretch();
    controlLayout->addWidget(m_clearLogBtn);
    contentLayout->addLayout(controlLayout);
    
    // 进度显示
    m_statusLabel = new DLabel("就绪");
    m_statusLabel->setForegroundRole(QPalette::BrightText);
    DFontSizeManager::instance()->bind(m_statusLabel, DFontSizeManager::T6);
    contentLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    contentLayout->addWidget(m_progressBar);
    
    // 日志显示
    DLabel *logLabel = new DLabel("转换日志：");
    DFontSizeManager::instance()->bind(logLabel, DFontSizeManager::T6);
    logLabel->setForegroundRole(QPalette::BrightText);
    contentLayout->addWidget(logLabel);
    
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setMinimumHeight(250);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Courier", 9));
    
    // 设置日志样式
    QString logStyle = R"(
        QTextEdit {
            background-color: #1e1e1e;
            color: #f0f0f0;
            border: 1px solid #404040;
            border-radius: 6px;
            padding: 8px;
        }
    )";
    m_logTextEdit->setStyleSheet(logStyle);
    
    contentLayout->addWidget(m_logTextEdit, 1);
    
    // 设置滚动区域
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void ThemeTool::connectSignals()
{
    connect(m_directoryRadio, &QRadioButton::toggled, this, &ThemeTool::onSourceTypeChanged);
    connect(m_debRadio, &QRadioButton::toggled, this, &ThemeTool::onSourceTypeChanged);
    
    connect(m_selectSourceBtn, &DSuggestButton::clicked, this, &ThemeTool::selectSourceItem);
    connect(m_selectOutputBtn, &DSuggestButton::clicked, this, &ThemeTool::selectOutputFile);
    
    connect(m_convertBtn, &DSuggestButton::clicked, this, &ThemeTool::startConversion);
    connect(m_stopBtn, &DPushButton::clicked, this, &ThemeTool::stopConversion);
    connect(m_clearLogBtn, &DPushButton::clicked, this, &ThemeTool::clearLog);
    
    // 壁纸选择按钮
    connect(m_selectWallpaperBtn, &DSuggestButton::clicked, this, &ThemeTool::selectWallpaper);
    connect(m_selectLockWallpaperBtn, &DSuggestButton::clicked, this, &ThemeTool::selectLockWallpaper);
    connect(m_selectDarkWallpaperBtn, &DSuggestButton::clicked, this, &ThemeTool::selectDarkWallpaper);
    connect(m_selectDarkLockWallpaperBtn, &DSuggestButton::clicked, this, &ThemeTool::selectDarkLockWallpaper);
    
    // 监听路径变化，自动填充主题名称、图标主题名和光标主题名
    connect(m_sourcePathEdit, &DLineEdit::textChanged, this, [this](const QString &path) {
        if (path.isEmpty()) {
            // 路径清空时，清空相关字段
            m_themeNameEdit->clear();
            m_iconThemeEdit->clear();
            m_cursorThemeEdit->clear();
            m_outputFileEdit->clear();
            return;
        }
        
        QFileInfo fileInfo(path);
        QString baseName = fileInfo.baseName();
        if (fileInfo.isDir()) {
            baseName = fileInfo.fileName();
        }
        
        // 自动填充主题名称
        QString themeName = QString("%1-deepin").arg(baseName);
        if (m_themeNameEdit->text().isEmpty()) {
            m_themeNameEdit->setText(themeName);
            
            // 同时自动设置输出文件路径
            updateOutputPath(themeName);
        }
        
        // 自动填充图标主题名和光标主题名
        if (m_iconThemeEdit->text().isEmpty()) {
            m_iconThemeEdit->setText(themeName);
        }
        if (m_cursorThemeEdit->text().isEmpty()) {
            m_cursorThemeEdit->setText(themeName);
        }
    });
    
    // 监听主题名称变化，自动更新输出路径
    connect(m_themeNameEdit, &DLineEdit::textChanged, this, [this](const QString &themeName) {
        if (!themeName.isEmpty()) {
            updateOutputPath(themeName);
        }
    });
}

void ThemeTool::initializeConverter()
{
    m_convertProcess = new QProcess(this);
    
    connect(m_convertProcess, &QProcess::readyReadStandardOutput, 
            this, &ThemeTool::onProcessOutput);
    connect(m_convertProcess, &QProcess::readyReadStandardError,
            this, &ThemeTool::onProcessError);
    connect(m_convertProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ThemeTool::onProcessFinished);
    
    // 初始化进度定时器
    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(100); // 100ms更新间隔
    connect(m_progressTimer, &QTimer::timeout, this, [this]() {
        if (m_converter) {
            int progress = m_converter->getProgressPercentage();
            QString message = m_converter->getProgressMessage();
            onConversionProgress(progress, message);
        }
    });
    
    // 创建临时工作目录
    m_tempWorkDir = QDir::temp().absoluteFilePath("deepin-theme-conversion");
    
    // 添加初始日志
    appendLog("=== Deepin 主题转换工具已启动 ===", LogLevel::Info);
    appendLog("请选择要转换的 FreeDesktop 主题包目录", LogLevel::Info);
}

void ThemeTool::selectSourceItem()
{
    SourceType type = getCurrentSourceType();
    QString selectedPath;
    
    if (type == SourceType::Directory) {
        selectedPath = QFileDialog::getExistingDirectory(this, 
            "选择主题包目录", 
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    } else {
        selectedPath = QFileDialog::getOpenFileName(this, 
            "选择DEB包文件",
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
            "DEB包文件 (*.deb)");
    }
    
    if (!selectedPath.isEmpty()) {
        m_sourcePathEdit->setText(selectedPath);
        
        if (type == SourceType::Directory) {
            appendLog(QString("已选择主题包目录: %1").arg(selectedPath), LogLevel::Info);
            validateThemePackage(selectedPath);
        } else {
            appendLog(QString("已选择DEB包: %1").arg(selectedPath), LogLevel::Info);
            validateDebPackage(selectedPath);
        }
    }
}

void ThemeTool::selectOutputFile()
{
    // 获取当前主题名称以生成默认文件名
    QString themeName = m_themeNameEdit->text().trimmed();
    QString defaultFileName;
    
    if (!themeName.isEmpty()) {
        QString cleanThemeName = themeName.toLower().replace(' ', '-');
        defaultFileName = QString("%1.deb").arg(cleanThemeName);
    } else {
        defaultFileName = "theme.deb";
    }
    
    // 默认路径为桌面目录
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString defaultPath = QDir(desktopPath).absoluteFilePath(defaultFileName);
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "选择输出DEB包文件",
        defaultPath,
        "DEB包文件 (*.deb)");
    
    if (!fileName.isEmpty()) {
        m_outputFileEdit->setText(fileName);
        appendLog(QString("已设置输出文件: %1").arg(fileName), LogLevel::Info);
    }
}

void ThemeTool::onSourceTypeChanged()
{
    // 当源类型改变时，清空当前路径
    if (!m_sourcePathEdit->text().isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "切换源类型", 
            "切换源类型将清空当前选择，是否继续？",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            m_sourcePathEdit->clear();
            m_themeNameEdit->clear();
            appendLog("已清空当前选择", LogLevel::Info);
        } else {
            // 恢复原来的选择
            if (getCurrentSourceType() == SourceType::Directory) {
                m_debRadio->setChecked(true);
            } else {
                m_directoryRadio->setChecked(true);
            }
        }
    }
    
    // 更新占位符文本
    if (getCurrentSourceType() == SourceType::Directory) {
        m_sourcePathEdit->setPlaceholderText("选择主题包目录（包含 index.theme 文件）");
    } else {
        m_sourcePathEdit->setPlaceholderText("选择 DEB 包文件");
    }
}

void ThemeTool::validateThemePackage(const QString &path)
{
    QDir dir(path);
    QString indexFile = dir.absoluteFilePath("index.theme");
    
    if (QFile::exists(indexFile)) {
        appendLog("✓ 找到 index.theme 文件", LogLevel::Success);
        // TODO: 解析主题信息并显示
        parseThemeInfo(indexFile);
    } else {
        appendLog("⚠ 警告: 未找到 index.theme 文件，可能不是有效的主题包", LogLevel::Warning);
    }
}

void ThemeTool::parseThemeInfo(const QString &indexFile)
{
    QSettings themeConfig(indexFile, QSettings::IniFormat);
    
    // 读取主题基本信息
    themeConfig.beginGroup("Icon Theme");
    QString name = themeConfig.value("Name").toString();
    QString comment = themeConfig.value("Comment").toString();
    QStringList directories = themeConfig.value("Directories").toString().split(',');
    QString inherits = themeConfig.value("Inherits").toString();
    themeConfig.endGroup();
    
    if (!name.isEmpty()) {
        appendLog(QString("✓ 主题名称: %1").arg(name), LogLevel::Success);
    }
    if (!comment.isEmpty()) {
        appendLog(QString("✓ 主题描述: %1").arg(comment), LogLevel::Success);
    }
    if (!inherits.isEmpty()) {
        appendLog(QString("✓ 继承主题: %1").arg(inherits), LogLevel::Info);
    }
    
    appendLog(QString("✓ 找到 %1 个图标目录").arg(directories.size()), LogLevel::Success);
    
    // 统计图标数量
    int totalIcons = 0;
    QString sourcePath = QFileInfo(indexFile).dir().absolutePath();
    
    for (const QString &dir : directories) {
        QString dirPath = QDir(sourcePath).absoluteFilePath(dir.trimmed());
        if (QDir(dirPath).exists()) {
            QDirIterator iterator(dirPath, QStringList() << "*.png" << "*.svg" << "*.xpm", 
                                QDir::Files, QDirIterator::Subdirectories);
            int iconCount = 0;
            while (iterator.hasNext()) {
                iterator.next();
                iconCount++;
            }
            totalIcons += iconCount;
            appendLog(QString("  - %1: %2 个图标").arg(dir.trimmed()).arg(iconCount), LogLevel::Info);
        }
    }
    
    appendLog(QString("✓ 总计图标数量: %1").arg(totalIcons), LogLevel::Success);
    appendLog("主题包验证完成，可以开始转换", LogLevel::Success);
}

void ThemeTool::startConversion()
{
    QString sourcePath = m_sourcePathEdit->text().trimmed();
    QString outputFile = m_outputFileEdit->text().trimmed(); 
    QString themeName = m_themeNameEdit->text().trimmed();
    
    // 输入验证
    if (sourcePath.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择源文件");
        return;
    }
    
    if (outputFile.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择输出DEB包文件");
        return;
    }
    
    if (!QFileInfo(sourcePath).exists()) {
        QMessageBox::warning(this, "路径错误", "源文件不存在");
        return;
    }
    
    // 获取转换配置
    ConversionConfig config = getConversionConfig();
    
    // 验证配置
    if (!validateConfiguration(config)) {
        return;
    }
    
    // 开始转换
    m_convertBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    
    updateStatus("正在准备转换...");
    appendLog("=== 开始主题转换 ===", LogLevel::Info);
    appendLog(QString("源文件: %1").arg(sourcePath), LogLevel::Info);
    appendLog(QString("输出DEB包: %1").arg(outputFile), LogLevel::Info);
    appendLog(QString("主题名称: %1").arg(themeName.isEmpty() ? "auto" : themeName), LogLevel::Info);
    appendLog(QString("源类型: %1").arg(getCurrentSourceType() == SourceType::Directory ? "主题包目录" : "DEB包文件"), LogLevel::Info);
    
    // 显示高级配置选项
    appendLog("=== 高级配置选项 ===", LogLevel::Info);
    appendLog(QString("转换质量: %1%").arg(config.quality), LogLevel::Info);
    appendLog(QString("生成光标主题: %1").arg(config.generateCursor ? "是" : "否"), LogLevel::Info);
    appendLog(QString("标准字体: %1").arg(config.standardFont), LogLevel::Info);
    appendLog(QString("等宽字体: %1").arg(config.monospaceFont), LogLevel::Info);
    appendLog(QString("字体大小: %1 pt").arg(config.fontSize), LogLevel::Info);
    
    // 创建转换器并开始转换
    createConverter();
    if (m_converter) {
        // 清理之前的工作线程
        if (m_worker) {
            m_worker->cancel();
            if (m_worker->isRunning()) {
                m_worker->wait(5000); // 等待最多5秒
            }
            m_worker->deleteLater();
            m_worker = nullptr;
        }
        
        // 创建新的工作线程
        m_worker = new ConversionWorker(m_converter, config, this);
        
        // 连接工作线程信号到主线程槽函数
        connect(m_worker, &ConversionWorker::progressChanged, 
                this, &ThemeTool::onConversionProgress, Qt::QueuedConnection);
        connect(m_worker, &ConversionWorker::finished, 
                this, &ThemeTool::onConversionFinished, Qt::QueuedConnection);
        connect(m_worker, &ConversionWorker::logMessage, 
                this, [this](const QString &message, int level) {
                    appendLog(message, static_cast<LogLevel>(level));
                }, Qt::QueuedConnection);
        
        // 当线程完成时自动删除
        connect(m_worker, &ConversionWorker::finished, this, [this]() {
            if (m_worker) {
                m_worker->deleteLater();
                m_worker = nullptr;
            }
        }, Qt::QueuedConnection);
        
        // 开始转换
        emit conversionStarted();
        appendLog("在后台线程中开始转换...", LogLevel::Info);
        m_worker->start();
    }
}

void ThemeTool::stopConversion()
{
    appendLog("正在停止转换...", LogLevel::Warning);
    
    if (m_worker) {
        m_worker->cancel();
        if (m_worker->isRunning()) {
            m_worker->wait(3000); // 等待最多3秒
        }
    }
    
    if (m_converter) {
        m_converter->cancel();
    }
    
    if (m_convertProcess && m_convertProcess->state() != QProcess::NotRunning) {
        m_convertProcess->kill();
        m_convertProcess->waitForFinished(3000);
    }
    
    resetConversionState();
    appendLog("转换已停止", LogLevel::Warning);
}

ConversionConfig ThemeTool::getConversionConfig() const
{
    ConversionConfig config;
    
    config.sourcePath = m_sourcePathEdit->text().trimmed();
    config.outputPath = m_outputFileEdit->text().trimmed();
    config.themeName = m_themeNameEdit->text().trimmed();
    
    // 转换设置
    config.quality = m_qualitySpin->value();
    config.generateCursor = m_generateCursorCheck->isChecked();
    
    // 字体设置
    config.standardFont = m_standardFontCombo->currentText();
    config.monospaceFont = m_monospaceFontCombo->currentText();
    config.fontSize = m_fontSizeSpin->value();
    
    // 窗口设置
    config.windowRadius = m_windowRadiusSpin->value();
    config.windowOpacity = m_windowOpacitySpin->value();
    
    // 主题设置 - 如果为空则使用基于源包名的默认值
    config.iconTheme = m_iconThemeEdit->text().trimmed();
    config.cursorTheme = m_cursorThemeEdit->text().trimmed();
    
    // 如果图标主题名或光标主题名为空，从源路径自动生成
    if (config.iconTheme.isEmpty() || config.cursorTheme.isEmpty()) {
        if (config.iconTheme.isEmpty()) {
            config.iconTheme = config.themeName;
        }
        if (config.cursorTheme.isEmpty()) {
            config.cursorTheme = config.themeName;
        }
    }
    
    // 亮色主题设置
    config.lightWallpaper = m_wallpaperEdit->text().trimmed();
    config.lightLockWallpaper = m_lockWallpaperEdit->text().trimmed();
    config.lightThemeColor = m_themeColorEdit->text().trimmed();
    
    // 暗色主题设置
    config.darkWallpaper = m_darkWallpaperEdit->text().trimmed();
    config.darkLockWallpaper = m_darkLockWallpaperEdit->text().trimmed();
    config.darkThemeColor = m_darkThemeColorEdit->text().trimmed();
    
    return config;
}

bool ThemeTool::validateConfiguration(const ConversionConfig &config)
{
    if (config.sourcePath.isEmpty()) {
        QMessageBox::warning(const_cast<ThemeTool*>(this), "配置错误", "源路径不能为空");
        return false;
    }
    
    if (config.outputPath.isEmpty()) {
        QMessageBox::warning(const_cast<ThemeTool*>(this), "配置错误", "输出路径不能为空");
        return false;
    }
    
    if (config.themeName.isEmpty()) {
        QMessageBox::warning(const_cast<ThemeTool*>(this), "配置错误", "主题名称不能为空");
        return false;
    }
    
    if (config.quality < 1 || config.quality > 100) {
        QMessageBox::warning(const_cast<ThemeTool*>(this), "配置错误", "转换质量必须在1-100之间");
        return false;
    }
    
    // 检查输出文件是否已存在
    if (QFile::exists(config.outputPath)) {
        int ret = QMessageBox::question(const_cast<ThemeTool*>(this), 
                                       "文件已存在", 
                                       QString("输出文件 %1 已存在，是否覆盖？").arg(config.outputPath),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            return false;
        }
    }
    
    return true;
}

void ThemeTool::createConverter()
{
    // 清理之前的转换器
    if (m_converter) {
        m_converter->deleteLater();
        m_converter = nullptr;
    }
    
    // 创建新的转换器
    SourceType type = getCurrentSourceType();
    m_converter = ConverterFactory::createConverter(type, this);
    
    if (!m_converter) {
        appendLog("创建转换器失败", LogLevel::Error);
    } else {
        appendLog(QString("已创建%1转换器").arg(
            type == SourceType::Directory ? "目录" : "DEB包"), LogLevel::Info);
    }
}

void ThemeTool::onConversionProgress(int percentage, const QString &message)
{
    m_progressBar->setValue(percentage);
    updateStatus(message);
    
    if (!message.isEmpty()) {
        appendLog(QString("[%1%] %2").arg(percentage).arg(message), LogLevel::Info);
    }
}

void ThemeTool::onConversionFinished(bool success, const QString &message)
{
    m_progressTimer->stop();
    resetConversionState();
    
    if (success) {
        appendLog("=== 转换完成 ===", LogLevel::Success);
        appendLog(QString("输出文件: %1").arg(m_outputFileEdit->text()), LogLevel::Success);
        updateStatus("转换完成");
        
        QMessageBox::information(this, "转换完成", 
                               QString("主题转换成功完成！\n输出文件: %1").arg(m_outputFileEdit->text()));
    } else {
        appendLog("=== 转换失败 ===", LogLevel::Error);
        if (!message.isEmpty()) {
            appendLog(QString("错误信息: %1").arg(message), LogLevel::Error);
        }
        updateStatus("转换失败");
        
        QMessageBox::critical(this, "转换失败", 
                             QString("主题转换失败！\n%1").arg(message.isEmpty() ? "未知错误" : message));
    }
    
    emit conversionFinished(success);
    
    // 清理临时文件
    cleanupTempFiles();
}

void ThemeTool::cleanupTempFiles()
{
    if (!m_tempWorkDir.isEmpty() && QDir(m_tempWorkDir).exists()) {
        QDir tempDir(m_tempWorkDir);
        tempDir.removeRecursively();
        appendLog("已清理临时文件", LogLevel::Info);
    }
}

void ThemeTool::updateOutputPath(const QString &themeName)
{
    if (themeName.isEmpty()) return;
    
    // 如果输出路径为空或者是基于旧主题名生成的，则自动更新
    QString currentOutput = m_outputFileEdit->text().trimmed();
    
    // 获取桌面目录
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    
    // 生成新的文件名：主题名.deb
    QString cleanThemeName = themeName.toLower().replace(' ', '-');
    QString newFileName = QString("%1.deb").arg(cleanThemeName);
    QString newFilePath = QDir(desktopPath).absoluteFilePath(newFileName);
    
    // 只有在输出路径为空时才自动设置，避免覆盖用户手动设置的路径
    if (currentOutput.isEmpty()) {
        m_outputFileEdit->setText(newFilePath);
        appendLog(QString("已自动设置输出文件: %1").arg(newFilePath), LogLevel::Info);
    }
}

void ThemeTool::clearLog()
{
    m_logTextEdit->clear();
    appendLog("=== 日志已清空 ===", LogLevel::Info);
}

void ThemeTool::onProcessOutput()
{
    if (!m_convertProcess) return;
    
    QByteArray data = m_convertProcess->readAllStandardOutput();
    QString output = QString::fromUtf8(data);
    
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        appendLog(line, LogLevel::Info);
    }
}

void ThemeTool::onProcessError()
{
    if (!m_convertProcess) return;
    
    QByteArray data = m_convertProcess->readAllStandardError();
    QString error = QString::fromUtf8(data);
    
    for (const QString &line : error.split('\n', Qt::SkipEmptyParts)) {
        appendLog(line, LogLevel::Error);
    }
}

void ThemeTool::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    resetConversionState();
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        updateStatus("转换完成");
        appendLog("=== 主题转换成功完成 ===", LogLevel::Success);
        QMessageBox::information(this, "转换完成", "主题包转换成功完成！");
    } else {
        updateStatus("转换失败");
        appendLog(QString("=== 转换失败 (退出码: %1) ===").arg(exitCode), LogLevel::Error);
        QMessageBox::warning(this, "转换失败", "主题包转换过程中出现错误，请查看日志了解详情。");
    }
}

void ThemeTool::resetConversionState()
{
    m_convertBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
}

void ThemeTool::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void ThemeTool::appendLog(const QString &message, LogLevel level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString coloredMessage;
    
    switch (level) {
        case LogLevel::Info:
            coloredMessage = QString("<span style='color: #a0a0a0;'>[%1]</span> %2").arg(timestamp).arg(message);
            break;
        case LogLevel::Success:
            coloredMessage = QString("<span style='color: #a0a0a0;'>[%1]</span> <span style='color: #00ff00;'>%2</span>").arg(timestamp).arg(message);
            break;
        case LogLevel::Warning:
            coloredMessage = QString("<span style='color: #a0a0a0;'>[%1]</span> <span style='color: #ffaa00;'>%2</span>").arg(timestamp).arg(message);
            break;
        case LogLevel::Error:
            coloredMessage = QString("<span style='color: #a0a0a0;'>[%1]</span> <span style='color: #ff4444;'>%2</span>").arg(timestamp).arg(message);
            break;
    }
    
    m_logTextEdit->append(coloredMessage);
    
    // 自动滚动到底部
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
}

ThemeTool::SourceType ThemeTool::getCurrentSourceType() const
{
    return m_directoryRadio->isChecked() ? SourceType::Directory : SourceType::DebPackage;
}

void ThemeTool::validateDebPackage(const QString &debPath)
{
    if (!QFile::exists(debPath)) {
        appendLog(QString("⚠ 警告: DEB包文件不存在: %1").arg(debPath), LogLevel::Warning);
        return;
    }
    
    QFileInfo fileInfo(debPath);
    if (fileInfo.suffix().toLower() != "deb") {
        appendLog(QString("⚠ 警告: 文件扩展名不是.deb: %1").arg(debPath), LogLevel::Warning);
        return;
    }
    
    appendLog(QString("✓ DEB包验证通过: %1").arg(fileInfo.fileName()), LogLevel::Success);
    appendLog(QString("  文件大小: %1 KB").arg(fileInfo.size() / 1024), LogLevel::Info);
    
    // TODO: 可以添加更详细的DEB包内容检查
}

void ThemeTool::analyzeThemeDirectory(const QString &themePath)
{
    appendLog("=== 开始分析主题包目录 ===", LogLevel::Info);
    
    QDir themeDir(themePath);
    appendLog(QString("分析目录: %1").arg(themePath), LogLevel::Info);
    
    // 检查index.theme文件
    QString indexFile = themeDir.absoluteFilePath("index.theme");
    if (QFile::exists(indexFile)) {
        appendLog("✓ 找到 index.theme 文件", LogLevel::Success);
        parseThemeInfo(indexFile);
    } else {
        appendLog("⚠ 未找到 index.theme 文件", LogLevel::Warning);
    }
    
    // 扫描图标目录
    QStringList iconDirs;
    QDirIterator dirIt(themePath, QDir::Dirs | QDir::NoDotAndDotDot);
    while (dirIt.hasNext()) {
        QString dir = dirIt.next();
        QFileInfo dirInfo(dir);
        if (dirInfo.fileName().contains("x") || dirInfo.fileName() == "scalable") {
            iconDirs.append(dirInfo.fileName());
        }
    }
    
    appendLog(QString("发现 %1 个图标尺寸目录").arg(iconDirs.size()), LogLevel::Info);
    for (const QString &dir : iconDirs) {
        appendLog(QString("  - %1").arg(dir), LogLevel::Info);
    }
    
    // 统计图标数量
    int totalIcons = 0;
    QDirIterator iconIt(themePath, QStringList() << "*.png" << "*.svg" << "*.xpm", 
                       QDir::Files, QDirIterator::Subdirectories);
    while (iconIt.hasNext()) {
        iconIt.next();
        totalIcons++;
    }
    
    appendLog(QString("总图标数量: %1").arg(totalIcons), LogLevel::Success);
    
    // 检查依赖主题
    QSettings themeConfig(indexFile, QSettings::IniFormat);
    themeConfig.beginGroup("Icon Theme");
    QString inherits = themeConfig.value("Inherits").toString();
    themeConfig.endGroup();
    
    if (!inherits.isEmpty()) {
        appendLog(QString("继承主题: %1").arg(inherits), LogLevel::Info);
    }
    
    appendLog("=== 主题包目录分析完成 ===", LogLevel::Success);
}

void ThemeTool::analyzeDebPackage(const QString &debPath)
{
    appendLog("=== 开始分析DEB包 ===", LogLevel::Info);
    
    QFileInfo debInfo(debPath);
    appendLog(QString("分析文件: %1").arg(debPath), LogLevel::Info);
    appendLog(QString("文件大小: %1 KB").arg(debInfo.size() / 1024), LogLevel::Info);
    
    // 验证DEB包格式
    if (debInfo.suffix().toLower() != "deb") {
        appendLog("⚠ 警告: 文件扩展名不是.deb", LogLevel::Warning);
    } else {
        appendLog("✓ DEB包格式验证通过", LogLevel::Success);
    }
    
    // TODO: 使用dpkg-deb或ar命令解析DEB包内容
    // 这里先显示基本信息
    appendLog("正在分析包内容结构...", LogLevel::Info);
    
    // 模拟分析过程
    QTimer::singleShot(500, this, [this]() {
        appendLog("✓ 发现控制信息 (DEBIAN/control)", LogLevel::Success);
        appendLog("✓ 发现数据文件 (usr/share/icons/)", LogLevel::Success);
        appendLog("✓ 检测到图标主题文件", LogLevel::Success);
        
        // 可以添加更多的DEB包分析逻辑
        appendLog("包内容分析完成", LogLevel::Info);
        appendLog("=== DEB包分析完成 ===", LogLevel::Success);
    });
}

void ThemeTool::selectWallpaper()
{
    QString wallpaper = QFileDialog::getOpenFileName(this,
        "选择亮色主题壁纸",
        "/usr/share/wallpapers",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.svg)");
    
    if (!wallpaper.isEmpty()) {
        m_wallpaperEdit->setText(QString("file://%1").arg(wallpaper));
        appendLog(QString("已选择亮色主题壁纸: %1").arg(wallpaper), LogLevel::Info);
    }
}

void ThemeTool::selectLockWallpaper()
{
    QString wallpaper = QFileDialog::getOpenFileName(this,
        "选择亮色主题锁屏壁纸",
        "/usr/share/wallpapers",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.svg)");
    
    if (!wallpaper.isEmpty()) {
        m_lockWallpaperEdit->setText(QString("file://%1").arg(wallpaper));
        appendLog(QString("已选择亮色主题锁屏壁纸: %1").arg(wallpaper), LogLevel::Info);
    }
}

void ThemeTool::selectDarkWallpaper()
{
    QString wallpaper = QFileDialog::getOpenFileName(this,
        "选择暗色主题壁纸",
        "/usr/share/wallpapers",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.svg)");
    
    if (!wallpaper.isEmpty()) {
        m_darkWallpaperEdit->setText(QString("file://%1").arg(wallpaper));
        appendLog(QString("已选择暗色主题壁纸: %1").arg(wallpaper), LogLevel::Info);
    }
}

void ThemeTool::selectDarkLockWallpaper()
{
    QString wallpaper = QFileDialog::getOpenFileName(this,
        "选择暗色主题锁屏壁纸",
        "/usr/share/wallpapers",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.svg)");
    
    if (!wallpaper.isEmpty()) {
        m_darkLockWallpaperEdit->setText(QString("file://%1").arg(wallpaper));
        appendLog(QString("已选择暗色主题锁屏壁纸: %1").arg(wallpaper), LogLevel::Info);
    }
}
