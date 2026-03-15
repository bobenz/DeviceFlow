#ifndef TRIGGER_H
#define TRIGGER_H

#include <QObject>
#include <QQmlEngine>

class Trigger : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit Trigger(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled)
    {
        if (m_enabled == enabled)
            return;
        m_enabled = enabled;
        emit enabledChanged();
    }

    Q_INVOKABLE bool fire()
    {
        if (!m_enabled)
            return false;
        QMetaObject::invokeMethod(this, &Trigger::fired, Qt::QueuedConnection);
        return true;
    }

signals:
    void fired();
    void enabledChanged();

private:
    bool m_enabled = true;
};

#endif // TRIGGER_H
