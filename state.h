#ifndef STATEBASE_H
#define STATEBASE_H

#include <QEventLoop>
#include <QObject>
#include "trigger.h"


class StateBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool condition READ condition WRITE setCondition NOTIFY conditionChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(Trigger *run READ run NOTIFY runChanged FINAL)
    Q_PROPERTY(Trigger *cancel READ cancel NOTIFY cancelChanged FINAL)
public:
    enum Status { IDLE, RUNNING, FINISHED, CANCELLED, ERROR };
    Q_ENUM(StateBase::Status)
    enum WaitResult{ Finished, Cancelled, Timeout, Error};
    Q_ENUM(StateBase::WaitResult)

    explicit StateBase(QObject *parent = nullptr);

    bool condition() const { return m_condition; }

    void setCondition(bool newCondition)
    {
        if (m_condition == newCondition)
            return;
        m_condition = newCondition;
        emit conditionChanged();
    }

    Q_INVOKABLE WaitResult waitCondition(int timeout = 0);

    Q_INVOKABLE WaitResult waitSignal(QObject* sender, QString name, int timeout = 0);



    Status status() const;
    void setStatus(const Status &newStatus);

    Trigger *run() const;
    Trigger *cancel() const;

signals:
    void conditionChanged();
    void enter();
    void exit();
    void statusChanged();
    void runChanged();
    void cancelChanged();

private slots:
    void _enter()
    {
        if (m_status != Status::IDLE)
            return;
        setStatus(Status::RUNNING);
        emit enter();
    }
    void _exit()
    {
        setStatus(Status::FINISHED);
        emit exit();
    }
private:
    bool m_condition = false;
    Status m_status = IDLE;
    mutable Trigger m_run;
    mutable Trigger m_cancel;
};

Q_DECLARE_METATYPE(StateBase::Status)
Q_DECLARE_METATYPE(StateBase::WaitResult)

#endif // STATEBASE_H
