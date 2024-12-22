#pragma once

#include <DLabel>

DWIDGET_USE_NAMESPACE

class FuncItem : public QWidget
{
    Q_OBJECT
public:
    explicit FuncItem(QWidget *parent = nullptr, const QString &title = "", const QString &describtion = "");
    void setWidget(QWidget *w);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
};