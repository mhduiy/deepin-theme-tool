#pragma once

#include <QWidget>
#include <QDateTime>
#include <QProcess>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QFontComboBox>
#include <QGroupBox>
#include <QDir>
#include <QTimer>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include "conversionconfig.h"

class QVBoxLayout;
class QTextEdit;
class QProgressBar;

DWIDGET_USE_NAMESPACE

// 前向声明
class ThemeConverter;

class ThemeTool : public QWidget
{
    Q_OBJECT

public:
    enum class SourceType {
        Directory,
        DebPackage
    };
    
    explicit ThemeTool(QWidget *parent = nullptr);
    ~ThemeTool();

signals:
    void conversionStarted();
    void conversionFinished(bool success);
    void conversionProgress(int percentage, const QString &message);
    void logMessage(const QString &message, int level);

private slots:
    void selectSourceItem();
    void selectOutputFile();
    void onSourceTypeChanged();
    void startConversion();
    void stopConversion();
    void clearLog();
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void selectWallpaper();
    void selectLockWallpaper();
    void selectDarkWallpaper();
    void selectDarkLockWallpaper();
    
    // 新增转换相关槽函数
    void onConversionProgress(int percentage, const QString &message);
    void onConversionFinished(bool success, const QString &message);

private:
    enum class LogLevel {
        Info,
        Success,
        Warning,
        Error
    };

    void setupUI();
    void connectSignals();
    void initializeConverter();
    void validateThemePackage(const QString &path);
    void validateDebPackage(const QString &debPath);
    void parseThemeInfo(const QString &indexFile);
    void analyzeThemeDirectory(const QString &themePath);
    void analyzeDebPackage(const QString &debPath);
    void simulateConversion();
    void resetConversionState();
    void updateStatus(const QString &status);
    void appendLog(const QString &message, LogLevel level);
    SourceType getCurrentSourceType() const;
    
    // 新增转换相关方法
    ConversionConfig getConversionConfig() const;
    bool validateConfiguration(const ConversionConfig &config);
    void createConverter();
    void cleanupTempFiles();

private:
    // 基本UI组件
    QRadioButton *m_directoryRadio = nullptr;
    QRadioButton *m_debRadio = nullptr;
    DLineEdit *m_sourcePathEdit = nullptr;
    DLineEdit *m_outputFileEdit = nullptr;
    DLineEdit *m_themeNameEdit = nullptr;
    DSuggestButton *m_selectSourceBtn = nullptr;
    DSuggestButton *m_selectOutputBtn = nullptr;
    DSuggestButton *m_convertBtn = nullptr;
    DPushButton *m_stopBtn = nullptr;
    DPushButton *m_clearLogBtn = nullptr;
    
    QProgressBar *m_progressBar = nullptr;
    DLabel *m_statusLabel = nullptr;
    QTextEdit *m_logTextEdit = nullptr;
    
    // 高级配置选项
    
    // 转换设置
    QSlider *m_qualitySlider = nullptr;
    QSpinBox *m_qualitySpin = nullptr;
    QCheckBox *m_generateCursorCheck = nullptr;
    
    // 字体设置
    QFontComboBox *m_standardFontCombo = nullptr;
    QFontComboBox *m_monospaceFontCombo = nullptr;
    QDoubleSpinBox *m_fontSizeSpin = nullptr;
    
    // 窗口设置
    QSpinBox *m_windowRadiusSpin = nullptr;
    QDoubleSpinBox *m_windowOpacitySpin = nullptr;
    
    // 主题设置
    DLineEdit *m_iconThemeEdit = nullptr;
    DLineEdit *m_cursorThemeEdit = nullptr;
    
    // 亮色主题设置
    DLineEdit *m_wallpaperEdit = nullptr;
    DLineEdit *m_lockWallpaperEdit = nullptr;
    DLineEdit *m_themeColorEdit = nullptr;
    DSuggestButton *m_selectWallpaperBtn = nullptr;
    DSuggestButton *m_selectLockWallpaperBtn = nullptr;
    
    // 暗色主题设置
    DLineEdit *m_darkWallpaperEdit = nullptr;
    DLineEdit *m_darkLockWallpaperEdit = nullptr;
    DLineEdit *m_darkThemeColorEdit = nullptr;
    DSuggestButton *m_selectDarkWallpaperBtn = nullptr;
    DSuggestButton *m_selectDarkLockWallpaperBtn = nullptr;
    
    // 转换进程和管理
    QProcess *m_convertProcess = nullptr;
    ThemeConverter *m_converter = nullptr;
    QTimer *m_progressTimer = nullptr;
    QString m_tempWorkDir;
};

// 前向声明工厂类
class ConverterFactory;
