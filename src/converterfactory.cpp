#include "converterfactory.h"
#include "directoryconverter.h"
#include "debconverter.h"

ThemeConverter* ConverterFactory::createConverter(ThemeTool::SourceType type, QObject *parent)
{
    switch (type) {
    case ThemeTool::SourceType::Directory:
        return new DirectoryConverter(parent);
    case ThemeTool::SourceType::DebPackage:
        return new DebConverter(parent);
    default:
        return nullptr;
    }
} 