#include "state.h"
#include "deviceflow.h"
#include <QEventLoop>
#include <QDebug>
#include <QMetaMethod>

StateBase::StateBase(QObject *parent)
    : QObject{parent}
{}

StateBase *StateBase::prevState() const
{
    return m_prevState;
}

void StateBase::setPrevState(StateBase *prev)
{
    m_prevState = prev;
}

StateBase::Status StateBase::status() const
{
    return m_status;
}

void StateBase::setStatus(const Status &newStatus)
{
    if (m_status == newStatus)
        return;
    m_status = newStatus;
    emit statusChanged();
}

void StateBase::waitSignal(QObject *sender, QString name)
{
    if (!sender)
        return;

    // Remove trailing () if present
    if (name.endsWith("()")) {
        name = name.left(name.length() - 2);
    }

    // Try to find the signal in the meta-object
    const QMetaObject *mo = sender->metaObject();
    int signalIndex = -1;

    // Search through ALL methods (including inherited ones) starting from 0
    for (int i = 0; i < mo->methodCount(); ++i) {
        QMetaMethod method = mo->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QString methodName = QString::fromLatin1(method.name());
            if (methodName == name) {
                signalIndex = i;
                break;
            }
        }
    }

    if (signalIndex == -1) {
        qWarning() << "waitSignal: Signal" << name << "not found on object" << sender->metaObject()->className();
        return;
    }

    QEventLoop loop;

    // Use QMetaObject::connect for more robust connection
    if (!QMetaObject::connect(sender, signalIndex, &loop, loop.metaObject()->indexOfSlot("quit()"))) {
        qWarning() << "waitSignal: Failed to connect to signal" << name;
        return;
    }

    loop.exec();
}
