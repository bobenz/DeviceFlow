#include "state.h"
//#include "deviceflow.h"
#include <QEventLoop>
#include <QDebug>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QTimer>
#include <QVariantMap>

StateBase::StateBase(QObject *parent)
    : QObject{parent}
{
    qDebug() << "STATE BASE";
}

StateBase::Status StateBase::status() const
{
    return m_status;
}

void StateBase::setStatus(const Status &newStatus)
{
    if (m_status == newStatus)
        return;
    bool _active = active();
    m_status = newStatus;
    if (_active != active())
        emit activeChanged();
    emit statusChanged();

}

Trigger *StateBase::run() const
{
    return m_run;
}

Trigger *StateBase::cancel() const
{
    return m_cancel;
}

QVariantMap StateBase::getProperties()
{
    QVariantMap map;
    const QMetaObject *mo = metaObject();
    
    int startIndex = StateBase::staticMetaObject.propertyOffset();
    
    for (int i = startIndex; i < mo->propertyCount(); ++i) {
        QMetaProperty prop = mo->property(i);
        QVariantMap propDetails;
        propDetails.insert(QStringLiteral("value"), prop.read(this));
        propDetails.insert(QStringLiteral("type"), QString::fromLatin1(prop.typeName()));
        propDetails.insert(QStringLiteral("access"), prop.isWritable() ? QStringLiteral("readwrite") : QStringLiteral("readonly"));
        map.insert(QString::fromLatin1(prop.name()), propDetails);
    }

    for (const QByteArray &propName : dynamicPropertyNames()) {
        QVariant val = property(propName);
        QVariantMap propDetails;
        propDetails.insert(QStringLiteral("value"), val);
        propDetails.insert(QStringLiteral("type"), QString::fromLatin1(val.typeName()));
        propDetails.insert(QStringLiteral("access"), QStringLiteral("readwrite"));
        map.insert(QString::fromLatin1(propName), propDetails);
    }
    
    return map;
}

void StateBase::setCancel(Trigger *newCancel)
{
    if (m_cancel == newCancel)
        return;
    m_cancel = newCancel;
    emit cancelChanged();
}

void StateBase::setRun(Trigger *newRun)
{
    if (m_run == newRun)
        return;
    m_run = newRun;
    emit runChanged();
}
