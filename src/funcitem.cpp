#include "funcitem.h"
#include <QLayout>
#include <DLabel>
#include <DFontSizeManager>
#include <qwindowdefs.h>

FuncItem::FuncItem(QWidget *parent, const QString &title, const QString &describtion)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins(10, 10, 10, 10));
    if (!title.isEmpty()) {
        DLabel *titleLabel = new DLabel(title);
        titleLabel->setForegroundRole(QPalette::Highlight);
        DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T4);
        mainLayout->addWidget(titleLabel);
    }
    if (!describtion.isEmpty()) {
        DLabel *describtionLabel = new DLabel(describtion);
        DFontSizeManager::instance()->bind(describtionLabel, DFontSizeManager::T8);
        mainLayout->addWidget(describtionLabel);
    }
}

void FuncItem::setWidget(QWidget *w)
{
    this->layout()->addWidget(w);
}

void FuncItem::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen;
    pen.setColor(palette().color(QPalette::Mid));
    pen.setWidth(2);
    painter.setPen(pen);
    QRect r = rect().marginsRemoved(QMargins(2, 2, 2, 2));
    painter.drawRoundedRect(r, 12, 12);
    QWidget::paintEvent(e);
}