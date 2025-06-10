#pragma once

#include "themeconverter.h"
#include <QMap>
#include <QStringList>

// 目录转换器
class DirectoryConverter : public ThemeConverter
{
    Q_OBJECT

public:
    explicit DirectoryConverter(QObject *parent = nullptr);
    ~DirectoryConverter() override;
    
    bool convert(const ConversionConfig &config) override;
    void cancel() override;
    QString getProgressMessage() const override;
    int getProgressPercentage() const override;

protected:
    bool prepareWorkspace(const QString &workDir) override;
    bool extractIcons(const QString &sourcePath, const QString &workDir) override;
    bool convertToDci(const QString &workDir, const QString &outputDir, int quality) override;
    bool generateThemeConfig(const QString &outputDir, const ConversionConfig &config) override;
    bool createDebPackage(const QString &packageDir, const QString &outputFile) override;
    void cleanup() override;

private:
    bool reorganizeIcons(const QString &sourcePath, const QString &workDir);
    bool runDciIconTheme(const QString &inputDir, const QString &outputDir, int quality);
    QStringList findIconCategories(const QString &sourcePath);
    QMap<QString, QStringList> groupIconsByName(const QString &sourcePath);
    QString extractResolutionFromPath(const QString &path);
    bool generateIndexTheme(const QString &configPath, const ConversionConfig &config);
    bool generateDebianControl(const QString &debianDir, const ConversionConfig &config);
    bool copyCursorTheme(const QString &sourcePath, const QString &outputDir, const ConversionConfig &config);
    bool copyDirectoryRecursively(const QString &source, const QString &target);
    void updateCursorThemeName(const QString &filePath, const QString &themeName);
    void createCursorThemeFile(const QString &filePath, const QString &themeName);
}; 