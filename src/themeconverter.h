#pragma once

#include <QObject>
#include <QString>
#include "conversionconfig.h"

// 转换器抽象基类
class ThemeConverter : public QObject
{
    Q_OBJECT

public:
    explicit ThemeConverter(QObject *parent = nullptr);
    virtual ~ThemeConverter();
    
    virtual bool convert(const ConversionConfig &config) = 0;
    virtual void cancel() = 0;
    virtual QString getProgressMessage() const = 0;
    virtual int getProgressPercentage() const = 0;

signals:
    void progressChanged(int percentage, const QString &message);
    void finished(bool success, const QString &message);
    void logMessage(const QString &message, int level);

protected:
    virtual bool prepareWorkspace(const QString &workDir) = 0;
    virtual bool extractIcons(const QString &sourcePath, const QString &workDir) = 0;
    virtual bool convertToDci(const QString &workDir, const QString &outputDir, int quality) = 0;
    virtual bool generateThemeConfig(const QString &outputDir, const ConversionConfig &config) = 0;
    virtual bool createDebPackage(const QString &packageDir, const QString &outputFile) = 0;
    virtual void cleanup() = 0;
    
    QString m_workDir;
    QString m_currentMessage;
    int m_currentProgress = 0;
    bool m_cancelled = false;
}; 