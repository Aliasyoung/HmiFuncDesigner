#ifndef QSLIDERHOST_H
#define QSLIDERHOST_H

#include "qabstractsliderhost.h"

class QSliderHost: public QAbstractSliderHost
{
    Q_OBJECT
public:
    Q_INVOKABLE QSliderHost(QAbstractHost *parent = 0);

    static QString getShowName();
    static QString getShowIcon();
    static QString getShowGroup();

protected:
    void initProperty() override;

protected:
    void createObject() override;
    // 控件支持的功能事件
    QStringList supportFuncEvents() override;
};

#endif // QSLIDERHOST_H
