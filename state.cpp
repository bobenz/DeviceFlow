#include "state.h"
//#include "deviceflow.h"
#include <QEventLoop>
#include <QDebug>
#include <QMetaMethod>
#include <QTimer>


StateBase::StateBase(QObject *parent)
    : QObject{parent}
{
    // Connect run trigger to _enter()
    connect(&m_run, &Trigger::fired, this, &StateBase::_enter);
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

Trigger *StateBase::run() const
{
    return &m_run;
}

Trigger *StateBase::cancel() const
{
    return &m_cancel;
}

StateBase::WaitResult StateBase::waitCondition(int timeout)
{
    WaitResult result{WaitResult::Error};
    if (m_condition) {
        return WaitResult::Finished;
    }
    QEventLoop loop;
    // Connect cancel trigger to exit loop with Cancelled result
    connect(&m_cancel, &Trigger::fired, &loop, [&]() {
        result = WaitResult::Cancelled;
        loop.quit();
    });


    // Setup timeout if specified
    QTimer *timer = nullptr;
    if (timeout > 0) {
        timer = new QTimer(&loop);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, &loop, [&]() {
            result = WaitResult::Timeout;
            loop.quit();
        });
        timer->start(timeout);
    }

    connect(this, &StateBase::conditionChanged, &loop, [&]() {
        if (m_condition) {
            m_condition = false;
            loop.quit();

        }
    });
    loop.exec();
    return result;
}

StateBase::WaitResult StateBase::waitSignal(QObject *sender, QString name, int timeout)
{
    if (!sender)
        return WaitResult::Error;

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
        return WaitResult::Error;
    }

    QEventLoop loop;
    WaitResult result = WaitResult::Finished;

    // Connect cancel trigger to exit loop with Cancelled result
    connect(&m_cancel, &Trigger::fired, &loop, [&]() {
        result = WaitResult::Cancelled;
        loop.quit();
    });

    // Setup timeout if specified
    QTimer *timer = nullptr;
    if (timeout > 0) {
        timer = new QTimer(&loop);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, &loop, [&]() {
            result = WaitResult::Timeout;
            loop.quit();
        });
        timer->start(timeout);
    }

    // Use QMetaObject::connect for more robust connection
    if (!QMetaObject::connect(sender, signalIndex, &loop, loop.metaObject()->indexOfSlot("quit()"))) {
        qWarning() << "waitSignal: Failed to connect to signal" << name;
        return WaitResult::Error;
    }

    loop.exec();

    return result;
}
