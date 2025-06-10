#pragma once

#include "themetool.h"

class ThemeConverter;

// 转换器工厂类
class ConverterFactory
{
public:
    static ThemeConverter* createConverter(ThemeTool::SourceType type, QObject *parent = nullptr);
}; 