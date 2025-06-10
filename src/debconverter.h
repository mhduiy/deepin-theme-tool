#pragma once

#include "themeconverter.h"

// DEB转换器
class DebConverter : public ThemeConverter
{
    Q_OBJECT

public:
    explicit DebConverter(QObject *parent = nullptr);
    ~DebConverter() override;
    
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
    bool extractDebPackage(const QString &debPath, const QString &extractDir);
    QString findThemeDirectory(const QString &extractDir);
}; 