#include "conversionworker.h"
#include "themeconverter.h"
#include <QDebug>
#include <QApplication>

ConversionWorker::ConversionWorker(ThemeConverter *converter, const ConversionConfig &config, QObject *parent)
    : QThread(parent)
    , m_converter(converter)
    , m_config(config)
    , m_cancelled(false)
{
    // 连接转换器的信号到工作线程的信号
    if (m_converter) {
        connect(m_converter, &ThemeConverter::progressChanged, 
                this, &ConversionWorker::progressChanged, Qt::DirectConnection);
        connect(m_converter, &ThemeConverter::finished, 
                this, &ConversionWorker::finished, Qt::DirectConnection);
        connect(m_converter, &ThemeConverter::logMessage, 
                this, &ConversionWorker::logMessage, Qt::DirectConnection);
    }
}

void ConversionWorker::cancel()
{
    QMutexLocker locker(&m_mutex);
    m_cancelled = true;
    
    if (m_converter) {
        m_converter->cancel();
    }
}

bool ConversionWorker::isCancelled() const
{
    QMutexLocker locker(&m_mutex);
    return m_cancelled;
}

void ConversionWorker::run()
{
    if (!m_converter) {
        emit logMessage("转换器不可用", 3);
        emit finished(false, "转换器不可用");
        return;
    }
    
    try {
        emit logMessage("开始在后台线程中执行转换", 0);
        
        // 在工作线程中执行转换
        bool success = m_converter->convert(m_config);
        
        if (isCancelled()) {
            emit logMessage("转换被用户取消", 2);
            emit finished(false, "转换被用户取消");
            return;
        }
        
        if (!success) {
            QString errorMessage = m_converter->getProgressMessage();
            emit logMessage(QString("转换失败: %1").arg(errorMessage), 3);
            emit finished(false, errorMessage);
        } else {
            emit logMessage("转换成功完成", 1);
            // 注意：转换器本身也会发送finished信号，这里不重复发送
        }
        
    } catch (const std::exception &e) {
        QString errorMessage = QString("转换过程中发生异常: %1").arg(e.what());
        emit logMessage(errorMessage, 3);
        emit finished(false, errorMessage);
    } catch (...) {
        QString errorMessage = "转换过程中发生未知异常";
        emit logMessage(errorMessage, 3);
        emit finished(false, errorMessage);
    }
} 