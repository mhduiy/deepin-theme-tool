#include "directoryconverter.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

DirectoryConverter::DirectoryConverter(QObject *parent)
    : ThemeConverter(parent)
{
}

DirectoryConverter::~DirectoryConverter()
{
    cleanup();
}

bool DirectoryConverter::convert(const ConversionConfig &config)
{
    m_cancelled = false;
    m_currentProgress = 0;
    m_currentMessage = "开始转换...";
    
    emit logMessage("=== 开始目录转换 ===", 0);
    emit progressChanged(0, m_currentMessage);
    
    // 第1步：准备工作空间 (0-10%)
    m_currentMessage = "准备工作空间...";
    emit progressChanged(5, m_currentMessage);
    if (!prepareWorkspace(QDir::temp().absoluteFilePath("deepin-theme-conversion"))) {
        emit finished(false, "准备工作空间失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第2步：提取图标 (10-30%)
    m_currentMessage = "分析并重组织图标文件...";
    emit progressChanged(15, m_currentMessage);
    if (!extractIcons(config.sourcePath, m_workDir)) {
        emit finished(false, "图标提取失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第3步：转换为DCI (30-70%)
    m_currentMessage = "转换图标为DCI格式...";
    emit progressChanged(40, m_currentMessage);
    QString dciOutputDir = QDir(m_workDir).absoluteFilePath("dci-output");
    if (!convertToDci(QDir(m_workDir).absoluteFilePath("reorganized"), dciOutputDir, config.quality)) {
        emit finished(false, "DCI转换失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第4步：生成主题配置 (70-75%)
    m_currentMessage = "生成Deepin主题配置...";
    emit progressChanged(70, m_currentMessage);
    QString packageDir = QDir(m_workDir).absoluteFilePath("package");
    if (!generateThemeConfig(packageDir, config)) {
        emit finished(false, "主题配置生成失败");
        return false;
    }
    
    if (m_cancelled) return false;
    
    // 第4.5步：处理光标主题 (75-80%)
    if (config.generateCursor) {
        m_currentMessage = "处理光标主题...";
        emit progressChanged(75, m_currentMessage);
        if (!copyCursorTheme(config.sourcePath, packageDir, config)) {
            emit logMessage("光标主题处理失败，继续其他步骤", 2);
        }
    }
    
    if (m_cancelled) return false;
    
    // 第5步：复制DCI文件到包目录
    m_currentMessage = "复制DCI文件到包目录...";
    emit progressChanged(80, m_currentMessage);
    QString targetDciDir = QDir(packageDir).absoluteFilePath(QString("usr/share/dsg/icons/%1").arg(config.themeName));
    QDir().mkpath(targetDciDir);
    
    // 递归搜索所有DCI文件（因为现在每个图标都有自己的子目录）
    QDirIterator dciIt(dciOutputDir, QStringList() << "*.dci", QDir::Files, QDirIterator::Subdirectories);
    int dciCount = 0;
    while (dciIt.hasNext()) {
        QString dciFile = dciIt.next();
        QFileInfo info(dciFile);
        QString targetFile = QDir(targetDciDir).absoluteFilePath(info.fileName());
        
        // 如果目标文件已存在，尝试不同的名称
        int counter = 1;
        QString originalTargetFile = targetFile;
        while (QFile::exists(targetFile)) {
            QString baseName = info.completeBaseName();
            QString extension = info.suffix();
            targetFile = QDir(targetDciDir).absoluteFilePath(QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
            counter++;
        }
        
        if (!QFile::copy(dciFile, targetFile)) {
            emit logMessage(QString("复制DCI文件失败: %1 -> %2").arg(info.fileName()).arg(QFileInfo(targetFile).fileName()), 3);
        } else {
            dciCount++;
            if (targetFile != originalTargetFile) {
                emit logMessage(QString("文件重名，重命名为: %1").arg(QFileInfo(targetFile).fileName()), 2);
            }
        }
    }
    
    emit logMessage(QString("已复制 %1 个DCI文件").arg(dciCount), dciCount > 0 ? 1 : 3);
    
    if (m_cancelled) return false;
    
    // 第6步：创建DEB包 (85-100%)
    m_currentMessage = "创建DEB安装包...";
    emit progressChanged(90, m_currentMessage);
    if (!createDebPackage(packageDir, config.outputPath)) {
        emit finished(false, "DEB包创建失败");
        return false;
    }
    
    // 完成
    m_currentProgress = 100;
    m_currentMessage = "转换完成";
    emit progressChanged(100, m_currentMessage);
    emit logMessage("=== 转换成功完成 ===", 1);
    emit finished(true, "转换完成");
    
    return true;
}

void DirectoryConverter::cancel()
{
    m_cancelled = true;
    m_currentMessage = "正在取消转换...";
    emit logMessage("用户取消转换", 2);
}

QString DirectoryConverter::getProgressMessage() const
{
    return m_currentMessage;
}

int DirectoryConverter::getProgressPercentage() const
{
    return m_currentProgress;
}

bool DirectoryConverter::prepareWorkspace(const QString &workDir)
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
    QStringList subDirs = {"reorganized", "dci-output", "package", "package/DEBIAN", 
                          "package/usr/share/deepin-themes", "package/usr/share/dsg/icons"};
    
    for (const QString &subDir : subDirs) {
        if (!dir.mkpath(QDir(workDir).absoluteFilePath(subDir))) {
            emit logMessage(QString("无法创建子目录: %1").arg(subDir), 3);
            return false;
        }
    }
    
    emit logMessage(QString("工作空间已准备: %1").arg(workDir), 0);
    return true;
}

bool DirectoryConverter::extractIcons(const QString &sourcePath, const QString &workDir)
{
    return reorganizeIcons(sourcePath, QDir(workDir).absoluteFilePath("reorganized"));
}

bool DirectoryConverter::reorganizeIcons(const QString &sourcePath, const QString &workDir)
{
    emit logMessage("开始重组织图标文件结构...", 0);
    
    // 查找图标分类
    QStringList categories = findIconCategories(sourcePath);
    emit logMessage(QString("发现 %1 个图标分类").arg(categories.size()), 0);
    
    // 按图标名称分组
    QMap<QString, QStringList> iconGroups = groupIconsByName(sourcePath);
    emit logMessage(QString("发现 %1 个唯一图标").arg(iconGroups.size()), 0);
    
    int processedIcons = 0;
    int totalIcons = iconGroups.size();
    
    // 为每个图标创建多分辨率目录结构
    QMapIterator<QString, QStringList> it(iconGroups);
    while (it.hasNext()) {
        it.next();
        
        if (m_cancelled) return false;
        
        QString iconName = it.key();
        QStringList iconFiles = it.value();
        
        QString iconDir = QDir(workDir).absoluteFilePath(iconName);
        QDir().mkpath(iconDir);
        
        // 为每个文件创建分辨率子目录
        for (const QString &iconFile : iconFiles) {
            QFileInfo fileInfo(iconFile);
            QString fileName = fileInfo.fileName();
            
            // 从路径中提取分辨率信息
            QString resolution = extractResolutionFromPath(iconFile);
            if (resolution.isEmpty()) resolution = "48"; // 默认48x48
            
            QString resolutionDir = QDir(iconDir).absoluteFilePath(resolution);
            QDir().mkpath(resolutionDir);
            
            QString targetFile = QDir(resolutionDir).absoluteFilePath(fileName);
            if (!QFile::copy(iconFile, targetFile)) {
                emit logMessage(QString("复制图标文件失败: %1").arg(fileName), 2);
            }
        }
        
        processedIcons++;
        int progress = 15 + (processedIcons * 15 / totalIcons); // 15%-30%
        m_currentProgress = progress;
        
        if (processedIcons % 50 == 0) { // 每50个图标更新一次进度
            emit progressChanged(progress, QString("已处理 %1/%2 个图标").arg(processedIcons).arg(totalIcons));
        }
    }
    
    emit logMessage(QString("图标重组织完成，处理了 %1 个图标").arg(processedIcons), 1);
    return true;
}

QStringList DirectoryConverter::findIconCategories(const QString &sourcePath)
{
    QStringList categories;
    QDirIterator dirIt(sourcePath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (dirIt.hasNext()) {
        QString dir = dirIt.next();
        QFileInfo dirInfo(dir);
        QString categoryName = dirInfo.fileName();
        
        // 识别常见的图标分类
        if (categoryName == "actions" || categoryName == "apps" || categoryName == "categories" ||
            categoryName == "devices" || categoryName == "emblems" || categoryName == "emotes" ||
            categoryName == "mimetypes" || categoryName == "panel" || categoryName == "places" ||
            categoryName == "status" || categoryName == "symbolic") {
            
            if (!categories.contains(categoryName)) {
                categories.append(categoryName);
            }
        }
    }
    
    return categories;
}

QMap<QString, QStringList> DirectoryConverter::groupIconsByName(const QString &sourcePath)
{
    QMap<QString, QStringList> iconGroups;
    
    QDirIterator iconIt(sourcePath, QStringList() << "*.png" << "*.svg" << "*.xpm", 
                       QDir::Files, QDirIterator::Subdirectories);
    
    while (iconIt.hasNext()) {
        QString iconFile = iconIt.next();
        QFileInfo fileInfo(iconFile);
        QString baseName = fileInfo.completeBaseName(); // 不包含扩展名
        
        // 处理符号链接
        if (fileInfo.isSymLink()) {
            QString linkTarget = fileInfo.symLinkTarget();
            if (QFileInfo(linkTarget).exists()) {
                baseName = QFileInfo(linkTarget).completeBaseName();
            }
        }
        
        iconGroups[baseName].append(iconFile);
    }
    
    return iconGroups;
}

QString DirectoryConverter::extractResolutionFromPath(const QString &path)
{
    // 从路径中提取分辨率信息，如 "16x16", "48x48" 等
    QStringList pathParts = path.split('/');
    for (const QString &part : pathParts) {
        if (part.contains('x') && part.split('x').size() == 2) {
            QStringList dimensions = part.split('x');
            bool ok1, ok2;
            dimensions[0].toInt(&ok1);
            dimensions[1].toInt(&ok2);
            if (ok1 && ok2) {
                return dimensions[0]; // 返回宽度作为目录名
            }
        }
        if (part == "scalable") {
            return "scalable";
        }
    }
    return QString();
}

bool DirectoryConverter::convertToDci(const QString &workDir, const QString &outputDir, int quality)
{
    emit logMessage("开始DCI转换...", 0);
    
    QDir inputDir(workDir);
    QStringList iconDirs = inputDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    if (iconDirs.isEmpty()) {
        emit logMessage("警告: 没有找到需要转换的图标目录", 2);
        return true; // 不算错误，可能只是没有图标
    }
    
    emit logMessage(QString("找到 %1 个图标目录需要转换").arg(iconDirs.size()), 0);
    
    int processedDirs = 0;
    int successCount = 0;
    int totalDirs = iconDirs.size();
    
    for (const QString &iconDirName : iconDirs) {
        if (m_cancelled) return false;
        
        QString iconPath = inputDir.absoluteFilePath(iconDirName);
        
        // 检查图标目录是否包含有效文件
        QDir iconDir(iconPath);
        QStringList resolutionDirs = iconDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        
        if (resolutionDirs.isEmpty()) {
            emit logMessage(QString("跳过空图标目录: %1").arg(iconDirName), 2);
            processedDirs++;
            continue;
        }
        
        // 为每个图标创建独立的输出目录
        QString iconOutputDir = QDir(outputDir).absoluteFilePath(iconDirName);
        
        emit logMessage(QString("转换图标: %1 (%2个分辨率)").arg(iconDirName).arg(resolutionDirs.size()), 0);
        
        // 调用DCI转换工具
        if (runDciIconTheme(iconPath, iconOutputDir, quality)) {
            successCount++;
            emit logMessage(QString("✓ %1 转换成功").arg(iconDirName), 1);
        } else {
            emit logMessage(QString("✗ %1 转换失败").arg(iconDirName), 2);
            // 继续处理其他图标，不中断整个过程
        }
        
        processedDirs++;
        int progress = 40 + (processedDirs * 30 / totalDirs); // 40%-70%
        m_currentProgress = progress;
        
        if (processedDirs % 5 == 0 || processedDirs == totalDirs) {
            emit progressChanged(progress, QString("已转换 %1/%2 个图标").arg(processedDirs).arg(totalDirs));
        }
    }
    
    emit logMessage(QString("DCI转换完成: 成功 %1/%2 个图标").arg(successCount).arg(processedDirs), 
                   successCount > 0 ? 1 : 3);
    
    // 只要有部分成功就认为整体成功
    return successCount > 0;
}

bool DirectoryConverter::runDciIconTheme(const QString &inputDir, const QString &outputDir, int quality)
{
    // 确保输出目录的父目录存在，但目标目录本身不存在
    QDir parentDir = QFileInfo(outputDir).dir();
    if (!parentDir.exists()) {
        parentDir.mkpath(".");
    }
    
    // 如果输出目录已存在，删除它（dci-icon-theme要求目录不存在）
    if (QDir(outputDir).exists()) {
        QDir(outputDir).removeRecursively();
    }
    
    QProcess process;
    
    // 设置环境变量，包括DISPLAY和正确的locale
    QStringList env = QProcess::systemEnvironment();
    env << "DISPLAY=:0";
    env << "LC_ALL=zh_CN.UTF-8";
    env << "LANG=zh_CN.UTF-8";
    process.setEnvironment(env);
    
    QStringList arguments;
    arguments << inputDir
              << "-o" << outputDir
              << "-O" << QString("3=%1").arg(quality);
    
    QString program = "/usr/libexec/dtk6/DGui/bin/dci-icon-theme";
    
    emit logMessage(QString("执行: %1 %2").arg(program).arg(arguments.join(" ")), 0);
    emit logMessage(QString("输入目录: %1").arg(inputDir), 0);
    emit logMessage(QString("输出目录: %1").arg(outputDir), 0);
    
    process.start(program, arguments);
    if (!process.waitForStarted(5000)) {
        emit logMessage(QString("启动DCI转换工具失败: %1").arg(process.errorString()), 3);
        return false;
    }
    
    if (!process.waitForFinished(30000)) { // 30秒超时
        emit logMessage("DCI转换超时", 3);
        process.kill();
        return false;
    }
    
    // 获取输出信息
    QString standardOutput = process.readAllStandardOutput();
    QString standardError = process.readAllStandardError();
    
    if (!standardOutput.isEmpty()) {
        emit logMessage(QString("DCI工具输出: %1").arg(standardOutput), 0);
    }
    
    if (process.exitCode() != 0) {
        emit logMessage(QString("DCI转换失败，退出码: %1").arg(process.exitCode()), 3);
        if (!standardError.isEmpty()) {
            // 过滤掉常见的Qt警告信息
            QStringList errorLines = standardError.split('\n');
            QStringList filteredErrors;
            for (const QString &line : errorLines) {
                if (!line.contains("setHighDpiScaleFactorRoundingPolicy") && 
                    !line.contains("Detected locale") &&
                    !line.trimmed().isEmpty()) {
                    filteredErrors.append(line);
                }
            }
            if (!filteredErrors.isEmpty()) {
                emit logMessage(QString("错误详情: %1").arg(filteredErrors.join("; ")), 3);
            }
        }
        return false;
    }
    
    // 检查输出目录是否创建成功并包含DCI文件
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        emit logMessage("DCI转换完成但输出目录不存在", 3);
        return false;
    }
    
    QStringList dciFiles = outDir.entryList(QStringList() << "*.dci", QDir::Files);
    if (dciFiles.isEmpty()) {
        emit logMessage("DCI转换完成但未生成DCI文件", 2);
        return false;
    }
    
    emit logMessage(QString("DCI转换成功，生成了 %1 个DCI文件").arg(dciFiles.size()), 1);
    return true;
}

bool DirectoryConverter::generateThemeConfig(const QString &outputDir, const ConversionConfig &config)
{
    // 创建主题配置目录
    QString themeConfigDir = QDir(outputDir).absoluteFilePath(QString("usr/share/deepin-themes/%1").arg(config.themeName));
    QDir().mkpath(themeConfigDir);
    
    // 生成index.theme文件
    QString indexThemePath = QDir(themeConfigDir).absoluteFilePath("index.theme");
    if (!generateIndexTheme(indexThemePath, config)) {
        return false;
    }
    
    // 创建DEBIAN控制文件目录
    QString debianDir = QDir(outputDir).absoluteFilePath("DEBIAN");
    if (!generateDebianControl(debianDir, config)) {
        return false;
    }
    
    emit logMessage("主题配置文件生成完成", 1);
    return true;
}

bool DirectoryConverter::generateIndexTheme(const QString &configPath, const ConversionConfig &config)
{
    QFile themeFile(configPath);
    if (!themeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit logMessage(QString("无法创建主题配置文件: %1").arg(configPath), 3);
        return false;
    }
    
    QTextStream out(&themeFile);
    out.setEncoding(QStringConverter::Utf8);
    
    // [Deepin Theme] 节
    out << "[Deepin Theme]\n";
    out << "Name=" << config.themeName << "\n";
    out << "Comment=Converted from FreeDesktop theme\n";
    out << "Version=1.0\n";
    out << "Author=Deepin Theme Tool\n";
    out << "CreateTime=" << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    out << "\n";
    
    // [DefaultTheme] 节 - 亮色主题
    out << "[DefaultTheme]\n";
    out << "StandardFont=" << config.standardFont << "\n";
    out << "MonospaceFont=" << config.monospaceFont << "\n";
    out << "FontSize=" << config.fontSize << "\n";
    out << "WindowRadius=" << config.windowRadius << "\n";
    out << "WindowOpacity=" << config.windowOpacity << "\n";
    out << "IconTheme=" << config.iconTheme << "\n";
    if (config.generateCursor) {
        out << "CursorTheme=" << config.cursorTheme << "\n";
    }
    if (!config.lightWallpaper.isEmpty()) {
        out << "Wallpaper=" << config.lightWallpaper << "\n";
    }
    if (!config.lightLockWallpaper.isEmpty()) {
        out << "LockWallpaper=" << config.lightLockWallpaper << "\n";
    }
    out << "ThemeColor=" << config.lightThemeColor << "\n";
    out << "\n";
    
    // [DarkTheme] 节 - 暗色主题
    out << "[DarkTheme]\n";
    out << "StandardFont=" << config.standardFont << "\n";
    out << "MonospaceFont=" << config.monospaceFont << "\n";
    out << "FontSize=" << config.fontSize << "\n";
    out << "WindowRadius=" << config.windowRadius << "\n";
    out << "WindowOpacity=" << config.windowOpacity << "\n";
    out << "IconTheme=" << config.iconTheme << "\n";
    if (config.generateCursor) {
        out << "CursorTheme=" << config.cursorTheme << "\n";
    }
    if (!config.darkWallpaper.isEmpty()) {
        out << "Wallpaper=" << config.darkWallpaper << "\n";
    }
    if (!config.darkLockWallpaper.isEmpty()) {
        out << "LockWallpaper=" << config.darkLockWallpaper << "\n";
    }
    out << "ThemeColor=" << config.darkThemeColor << "\n";
    
    themeFile.close();
    
    emit logMessage(QString("已生成主题配置文件: %1").arg(configPath), 0);
    return true;
}

bool DirectoryConverter::generateDebianControl(const QString &debianDir, const ConversionConfig &config)
{
    QString controlPath = QDir(debianDir).absoluteFilePath("control");
    QFile controlFile(controlPath);
    
    if (!controlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit logMessage(QString("无法创建control文件: %1").arg(controlPath), 3);
        return false;
    }
    
    QTextStream out(&controlFile);
    out.setEncoding(QStringConverter::Utf8);
    
    out << "Package: deepin-theme-" << config.themeName.toLower().replace(' ', '-') << "\n";
    out << "Version: 1.0.0\n";
    out << "Section: x11\n";
    out << "Priority: optional\n";
    out << "Architecture: all\n";
    out << "Depends: deepin-desktop-environment-base\n";
    out << "Maintainer: Deepin Theme Tool <support@deepin.org>\n";
    out << "Description: " << config.themeName << " theme for Deepin Desktop\n";
    out << " A theme package converted from FreeDesktop theme format\n";
    out << " to Deepin theme format using Deepin Theme Tool.\n";
    
    controlFile.close();
    
    emit logMessage(QString("已生成Debian control文件: %1").arg(controlPath), 0);
    return true;
}

bool DirectoryConverter::copyCursorTheme(const QString &sourcePath, const QString &outputDir, const ConversionConfig &config)
{
    emit logMessage("开始处理光标主题...", 0);
    
    // 查找源目录中的光标主题
    QDir sourceDir(sourcePath);
    QStringList cursorDirs;
    
    // 查找cursors目录
    QDirIterator it(sourcePath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dir = it.next();
        if (QFileInfo(dir).fileName() == "cursors") {
            cursorDirs << dir;
        }
    }
    
    if (cursorDirs.isEmpty()) {
        emit logMessage("源目录中未找到光标主题", 2);
        return false;
    }
    
    // 选择第一个找到的cursors目录
    QString sourceCursorDir = cursorDirs.first();
    QString sourceParentDir = QFileInfo(sourceCursorDir).absolutePath();
    
    emit logMessage(QString("找到光标主题目录: %1").arg(sourceCursorDir), 0);
    
    // 创建目标光标主题目录
    QString targetIconDir = QDir(outputDir).absoluteFilePath(QString("usr/share/icons/%1").arg(config.cursorTheme));
    QDir().mkpath(targetIconDir);
    
    // 复制cursors目录
    QString targetCursorDir = QDir(targetIconDir).absoluteFilePath("cursors");
    if (!copyDirectoryRecursively(sourceCursorDir, targetCursorDir)) {
        emit logMessage("复制cursors目录失败", 3);
        return false;
    }
    
    // 查找并复制cursor.theme文件
    QString sourceCursorTheme = QDir(sourceParentDir).absoluteFilePath("cursor.theme");
    QString targetCursorTheme = QDir(targetIconDir).absoluteFilePath("cursor.theme");
    
    if (QFile::exists(sourceCursorTheme)) {
        if (!QFile::copy(sourceCursorTheme, targetCursorTheme)) {
            emit logMessage("复制cursor.theme文件失败", 2);
        } else {
            // 修改cursor.theme文件中的主题名称
            updateCursorThemeName(targetCursorTheme, config.cursorTheme);
        }
    } else {
        // 创建新的cursor.theme文件
        createCursorThemeFile(targetCursorTheme, config.cursorTheme);
    }
    
    // 统计复制的文件数量
    QDir cursorDir(targetCursorDir);
    int cursorCount = cursorDir.entryList(QDir::Files).size();
    
    emit logMessage(QString("光标主题处理完成，复制了 %1 个光标文件").arg(cursorCount), 1);
    return true;
}

bool DirectoryConverter::copyDirectoryRecursively(const QString &source, const QString &target)
{
    QDir sourceDir(source);
    if (!sourceDir.exists()) {
        return false;
    }
    
    QDir targetDir;
    if (!targetDir.mkpath(target)) {
        return false;
    }
    
    QDirIterator it(source, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString sourceFile = it.next();
        QFileInfo fileInfo(sourceFile);
        QString relativePath = sourceDir.relativeFilePath(sourceFile);
        QString targetFile = QDir(target).absoluteFilePath(relativePath);
        
        // 确保目标目录存在
        QDir().mkpath(QFileInfo(targetFile).absolutePath());
        
        if (!QFile::copy(sourceFile, targetFile)) {
            emit logMessage(QString("复制文件失败: %1").arg(relativePath), 2);
        }
    }
    
    return true;
}

void DirectoryConverter::updateCursorThemeName(const QString &filePath, const QString &themeName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QString content = file.readAll();
    file.close();
    
    // 替换主题名称
    content.replace(QRegularExpression("Name=.*"), QString("Name=%1").arg(themeName));
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << content;
        file.close();
    }
}

void DirectoryConverter::createCursorThemeFile(const QString &filePath, const QString &themeName)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << "[Icon Theme]\n";
        out << "Name=" << themeName << "\n";
        file.close();
    }
}

bool DirectoryConverter::createDebPackage(const QString &packageDir, const QString &outputFile)
{
    emit logMessage("开始创建DEB包...", 0);
    
    QProcess process;
    QStringList arguments;
    arguments << "--build" << packageDir << outputFile;
    
    QString program = "dpkg-deb";
    
    emit logMessage(QString("执行: %1 %2").arg(program).arg(arguments.join(" ")), 0);
    
    process.start(program, arguments);
    if (!process.waitForStarted(5000)) {
        emit logMessage(QString("启动dpkg-deb失败: %1").arg(process.errorString()), 3);
        return false;
    }
    
    if (!process.waitForFinished(60000)) { // 60秒超时
        emit logMessage("DEB包创建超时", 3);
        process.kill();
        return false;
    }
    
    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        emit logMessage(QString("DEB包创建失败，退出码: %1, 错误: %2").arg(process.exitCode()).arg(errorOutput), 3);
        return false;
    }
    
    emit logMessage(QString("DEB包创建成功: %1").arg(outputFile), 1);
    return true;
}

void DirectoryConverter::cleanup()
{
    if (!m_workDir.isEmpty() && QDir(m_workDir).exists()) {
        QDir(m_workDir).removeRecursively();
        emit logMessage("已清理工作目录", 0);
    }
} 