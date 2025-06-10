#pragma once

#include <DMainWindow>
#include <QStackedLayout>
#include <DListView>
#include "dcitoolsview.h"
#include "themetool.h"

DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUI();
    void initTitlebar();

private:
    DListView *m_navList = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;

    DciToolsView *m_dciToolsView = nullptr;
    ThemeTool *m_themeTool = nullptr;
};