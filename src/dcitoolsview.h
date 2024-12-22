#pragma once

#include "funcitem.h"
#include <DLineEdit>
#include <DPushButton>

class DciToolsView : public QWidget
{
    Q_OBJECT
public:
    explicit DciToolsView(QWidget *parent = nullptr);

private:
    void initDciAnalysisWidget();
    void initDciConvertWidget();

private slots:
    void openDciDoc();
    void runDciDciTool();
    void installDciCli();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    DPushButton *m_readDciDocBtn = nullptr;
    QPushButton *m_installDciCliToolBtn = nullptr;
    QPushButton *m_runDciCliBtn = nullptr;
    // Dci 解析
    FuncItem *m_dciAnalysisItem = nullptr;
    DLineEdit *m_dciPathEdit = nullptr;
    DLineEdit *m_dciExtractEdit = nullptr;
    QPushButton *m_selectDciButton = nullptr;
    DPushButton *m_treeButton = nullptr;
    DPushButton *m_extraButton = nullptr;

    // Dci 转换
    FuncItem *m_dciConvert = nullptr;
    DLineEdit *m_srcConvertPathEdit = nullptr;
    DLineEdit *m_tarConvertPathEdit = nullptr;
    DPushButton *m_srcConvertSelectBtn = nullptr;
    DPushButton *m_tarConvertSelectBtn = nullptr;
    DPushButton *m_convertBtn = nullptr;
    
};