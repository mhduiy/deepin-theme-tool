#pragma once

#include <QString>

// 转换配置结构体
struct ConversionConfig {
    QString sourcePath;
    QString outputPath;
    QString themeName;
    int quality = 85;
    bool generateCursor = true;
    QString standardFont = "Source Han Sans SC";
    QString monospaceFont = "Noto Mono";
    double fontSize = 10.5;
    int windowRadius = 12;
    double windowOpacity = 0.4;
    QString iconTheme = "bloom";
    QString cursorTheme = "bloom";
    
    // 亮色主题配置
    QString lightWallpaper;
    QString lightLockWallpaper;
    QString lightThemeColor = "#1F6EE7";
    
    // 暗色主题配置
    QString darkWallpaper;
    QString darkLockWallpaper;
    QString darkThemeColor = "#024CCA";
}; 