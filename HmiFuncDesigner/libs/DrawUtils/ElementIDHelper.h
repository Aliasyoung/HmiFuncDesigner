#ifndef ELEMENTIDHELPER_H
#define ELEMENTIDHELPER_H

#include <QObject>

class ElementIDHelper : public QObject
{
    Q_OBJECT
public:
    // 设置工程路径
    static void setProjectPath(const QString &path);
    // 取得工程路径
    static QString getProjectPath();
    //获取工程所有控件的ID名称
    static void getAllElementIDName(const QString &proj_path, QStringList &idList);

signals:

public slots:

private:
    explicit ElementIDHelper(QObject *parent = Q_NULLPTR);
    ~ElementIDHelper();

private:
    static QString szProjectPath_;

    Q_DISABLE_COPY(ElementIDHelper)
};

#endif // ELEMENTIDHELPER_H


