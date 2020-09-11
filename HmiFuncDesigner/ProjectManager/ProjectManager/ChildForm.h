﻿#ifndef CHILDFORM_H
#define CHILDFORM_H

#include <QWidget>

#include "SystemParametersWin.h"
#include "CommunicationDeviceWin.h"
#include "TagManagerWin.h"
#include "DrawPageWin.h"
#include "RealTimeDatabaseWin.h"
#include "ScriptManageWin.h"

namespace Ui {
class ChildForm;
}

enum PAGE_FLOWTYPE{
    PAGE_NONE = 0,
    PAGE_SYSTEM_PARAMETER,
    PAGE_COMMUNICATE_DEVICE,
    PAGE_VARIABLE_MANAGER,
    PAGE_DRAW_PAGE,
    PAGE_RTDB,
    PAGE_SCRIPT_MANAGER,
};
Q_DECLARE_METATYPE(PAGE_FLOWTYPE)

class ChildForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChildForm(QWidget *parent = Q_NULLPTR, const QString &projName = "");
    ~ChildForm();

    void SetTitle(const QString &title);
    QString getItemName() const;

    // 页面切换
    void switchPage(PAGE_FLOWTYPE page);

    QString getProjectName() const;
    void setProjectName(const QString & s);
    // 设置文件修改标志
    void setModifiedFlag(bool b);
    // 获取文件修改标志
    bool getModifiedFlag();

    // 增加变量标签
    void addVariableTag();
    // 追加变量标签
    void appendVariableTag();
    // 行拷贝变量标签
    void rowCopyVariableTag();
    // 修改变量标签
    void modifyVariableTag();
    // 删除变量标签
    void deleteVariableTag();
    // 变量标签导出csv
    void variableTagExportToCsv(const QString &path);
    // 从csv导入变量标签
    void variableTagImportFromCsv(const QString &file);


    // 新建设备
    void newDevice();
    // 修改设备
    void modifyDevice();
    // 删除设备
    void deleteDevice();

    // 打开文件
    virtual void open();
    // 保存文件
    virtual void save();
    // 显示大图标
    virtual void showLargeIcon();
    // 显示小图标
    virtual void showSmallIcon();

private:

public slots:
    void treeItemClicked(const QString &itemText);

private:
    Ui::ChildForm *ui;
    PAGE_FLOWTYPE m_currPageFlow; // 当前页面
    bool m_bModifiedFlag;
    QString m_strProjectName;
    SystemParametersWin *m_sysParamWinPtr; // 系统参数设置
    CommunicationDeviceWin *m_communicationDeviceWinPtr; // 通讯设备
    TagManagerWin *m_tagManagerWinPtr; // 变量管理
    DrawPageWin *m_drawPageWinPtr; // 画面管理
    RealTimeDatabaseWin *m_rtdbWinPtr; // 实时数据库
    ScriptManageWin *m_scriptManageWinPtr; // 脚本编辑器

};

#endif // CHILDFORM_H
