#include "debconverter.h"
#include "directoryconverter.h"
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QFileInfo>

DebConverter::DebConverter(QObject *parent)
    : ThemeConverter(parent)
{
}

DebConverter::~DebConverter()
{
    cleanup();
}

bool DebConverter::convert(const ConversionConfig &config)
{
    m_cancelled = false;
    m_currentProgress = 0;
    m_currentMessage = "开始DEB包转换...";
    
    emit logMessage("=== 开始DEB包转换 ===", 0);
    emit progressChanged(0, m_currentMessage);
    
    // 第1步：准备工作空间 (0-10%)
    m_currentMessage = "准备工作空间...";
    emit progressChanged(5, m_currentMessage);
    if (!prepareWorkspace(QDir::temp().absoluteFilePath("deepin-theme-conversion-deb"))) {
        emit finished(false, "准备工作空间失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第2步：解压DEB包 (10-30%)
    m_currentMessage = "解压DEB包...";
    emit progressChanged(15, m_currentMessage);
    QString extractDir = QDir(m_workDir).absoluteFilePath("extracted");
    if (!extractDebPackage(config.sourcePath, extractDir)) {
        emit finished(false, "DEB包解压失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第3步：查找主题目录 (30-40%)
    m_currentMessage = "查找主题目录...";
    emit progressChanged(35, m_currentMessage);
    QString themeDir = findThemeDirectory(extractDir);
    if (themeDir.isEmpty()) {
        emit finished(false, "未找到有效的主题目录");
        return false;
    }
    
    emit logMessage(QString("找到主题目录: %1").arg(themeDir), 0);
    
    if (m_cancelled) return false;
    
    // 第4步：使用DirectoryConverter进行转换 (40-100%)
    m_currentMessage = "转换主题内容...";
    emit progressChanged(45, m_currentMessage);
    
    // 创建修改后的配置，使用解压出的主题目录作为源
    ConversionConfig modifiedConfig = config;
    modifiedConfig.sourcePath = themeDir;
    
    // 创建目录转换器来处理实际转换
    DirectoryConverter dirConverter(this);
    
    // 连接信号
    connect(&dirConverter, &ThemeConverter::progressChanged, this, [this](int percentage, const QString &message) {
        // 将40-100%的进度映射到45-100%
        int adjustedProgress = 45 + (percentage * 55 / 100);
        m_currentProgress = adjustedProgress;
        m_currentMessage = message;
        emit progressChanged(adjustedProgress, message);
    });
    
    connect(&dirConverter, &ThemeConverter::logMessage, this, &ThemeConverter::logMessage);
    
    // 执行转换
    bool success = dirConverter.convert(modifiedConfig);
    
    if (success) {
        m_currentProgress = 100;
        m_currentMessage = "转换完成";
        emit progressChanged(100, m_currentMessage);
        emit logMessage("=== DEB包转换成功完成 ===", 1);
        emit finished(true, "转换完成");
    } else {
        emit finished(false, "主题转换失败");
    }
    
    return success;
}

void DebConverter::cancel()
{
    m_cancelled = true;
    m_currentMessage = "正在取消转换...";
    emit logMessage("用户取消DEB包转换", 2);
}

QString DebConverter::getProgressMessage() const
{
    return m_currentMessage;
}

int DebConverter::getProgressPercentage() const
{
    return m_currentProgress;
}

bool DebConverter::prepareWorkspace(const QString &workDir)
{
    m_workDir = workDir;
    
    // 清理并创建工作目录
    QDir dir(workDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    
    if (!dir.mkpath(workDir)) {
        emit logMessage(QString("无法创建工作目录: %1").arg(workDir), 3);
        return false;
    }
    
    // 创建子目录
    QStringList subDirs = {"extracted", "theme-extracted"};
    
    for (const QString &subDir : subDirs) {
        if (!dir.mkpath(QDir(workDir).absoluteFilePath(subDir))) {
            emit logMessage(QString("无法创建子目录: %1").arg(subDir), 3);
            return false;
        }
    }
    
    emit logMessage(QString("DEB包转换工作空间已准备: %1").arg(workDir), 0);
    return true;
}

bool DebConverter::extractIcons(const QString &sourcePath, const QString &workDir)
{
    // DEB转换器不直接提取图标，而是通过extractDebPackage来处理
    return true;
}

bool DebConverter::extractDebPackage(const QString &debPath, const QString &extractDir)
{
    emit logMessage(QString("开始解压DEB包: %1").arg(debPath), 0);
    
    QProcess process;
    QStringList arguments;
    arguments << "-x" << debPath << extractDir;
    
    QString program = "dpkg-deb";
    
    emit logMessage(QString("执行: %1 %2").arg(program).arg(arguments.join(" ")), 0);
    
    process.start(program, arguments);
    if (!process.waitForStarted(5000)) {
        emit logMessage(QString("启动dpkg-deb失败: %1").arg(process.errorString()), 3);
        return false;
    }
    
    if (!process.waitForFinished(30000)) { // 30秒超时
        emit logMessage("DEB包解压超时", 3);
        process.kill();
        return false;
    }
    
    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        emit logMessage(QString("DEB包解压失败，退出码: %1, 错误: %2").arg(process.exitCode()).arg(errorOutput), 3);
        return false;
    }
    
    emit logMessage("DEB包解压完成", 1);
    return true;
}

QString DebConverter::findThemeDirectory(const QString &extractDir)
{
    emit logMessage("查找主题目录...", 0);
    
    // 常见的图标主题路径
    QStringList possiblePaths = {
        "usr/share/icons",
        "share/icons",
        "icons"
    };
    
    for (const QString &path : possiblePaths) {
        QString fullPath = QDir(extractDir).absoluteFilePath(path);
        QDir themeDir(fullPath);
        
        if (!themeDir.exists()) continue;
        
        // 查找包含index.theme文件的子目录
        QStringList themeDirs = themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &themeDirName : themeDirs) {
            QString themePathCandidate = themeDir.absoluteFilePath(themeDirName);
            QString indexThemeFile = QDir(themePathCandidate).absoluteFilePath("index.theme");
            
            if (QFile::exists(indexThemeFile)) {
                emit logMessage(QString("找到有效的主题目录: %1").arg(themePathCandidate), 1);
                return themePathCandidate;
            }
        }
    }
    
    // 如果没找到index.theme文件，尝试查找包含大量图标文件的目录
    for (const QString &path : possiblePaths) {
        QString fullPath = QDir(extractDir).absoluteFilePath(path);
        QDir themeDir(fullPath);
        
        if (!themeDir.exists()) continue;
        
        QStringList themeDirs = themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &themeDirName : themeDirs) {
            QString themePathCandidate = themeDir.absoluteFilePath(themeDirName);
            
            // 计算图标文件数量
            QDirIterator iconIt(themePathCandidate, QStringList() << "*.png" << "*.svg" << "*.xpm", 
                               QDir::Files, QDirIterator::Subdirectories);
            int iconCount = 0;
            while (iconIt.hasNext()) {
                iconIt.next();
                iconCount++;
                if (iconCount > 10) break; // 找到足够多的图标就认为是有效的
            }
            
            if (iconCount > 10) {
                emit logMessage(QString("找到包含图标的目录: %1 (包含 %2+ 个图标)").arg(themePathCandidate).arg(iconCount), 1);
                return themePathCandidate;
            }
        }
    }
    
    emit logMessage("未找到有效的主题目录", 3);
    return QString();
}

bool DebConverter::convertToDci(const QString &workDir, const QString &outputDir, int quality)
{
    // DEB转换器将此任务委托给DirectoryConverter
    return true;
}

bool DebConverter::generateThemeConfig(const QString &outputDir, const ConversionConfig &config)
{
    // DEB转换器将此任务委托给DirectoryConverter
    return true;
}

bool DebConverter::createDebPackage(const QString &packageDir, const QString &outputFile)
{
    // DEB转换器将此任务委托给DirectoryConverter
    return true;
}

void DebConverter::cleanup()
{
    if (!m_workDir.isEmpty() && QDir(m_workDir).exists()) {
        QDir(m_workDir).removeRecursively();
        emit logMessage("已清理DEB转换工作目录", 0);
    }
} 