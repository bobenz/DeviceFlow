#ifndef TRIGGER_H
#define TRIGGER_H

#include <QObject>
#include <QQmlEngine>

class Trigger : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
public:
    explicit Trigger(QObject *parent = nullptr);
    Q_INVOKABLE bool fire()
    {
        if(!m_enabled) return false;
        emit fired();
        return true;
    }
signals:
    void fired();
};

#endif // TRIGGER_H
