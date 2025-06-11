#ifndef CONVERSIONWORKER_H
#define CONVERSIONWORKER_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "conversionconfig.h"

// 前向声明
class ThemeConverter;

class ConversionWorker : public QThread
{
    Q_OBJECT

public:
    explicit ConversionWorker(ThemeConverter *converter, const ConversionConfig &config, QObject *parent = nullptr);
    
    void cancel();
    bool isCancelled() const;

signals:
    void progressChanged(int percentage, const QString &message);
    void finished(bool success, const QString &message);
    void logMessage(const QString &message, int level);

protected:
    void run() override;

private:
    ThemeConverter *m_converter;
    ConversionConfig m_config;
    mutable QMutex m_mutex;
    bool m_cancelled;
};

#endif // CONVERSIONWORKER_H 