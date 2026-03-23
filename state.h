#ifndef STATEBASE_H
#define STATEBASE_H

#include <QEventLoop>
#include <QObject>
#include <QVariantMap>
#include "trigger.h"



class StateBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isActive READ active NOTIFY activeChanged)
    Q_PROPERTY(int error READ error  NOTIFY errorChanged FINAL)
    Q_PROPERTY(Trigger *run READ run NOTIFY runChanged FINAL)
    Q_PROPERTY(Trigger *cancel READ cancel NOTIFY cancelChanged FINAL)

public:

    enum Status { Idle, Running, Waiting, Completed };
    Q_ENUM(Status)

    explicit StateBase(QObject *parent = nullptr);

    int error(){return m_errorcode;}

    void setError(int code)
    {
        if(m_errorcode == code) return;
        m_errorcode = code;
        emit errorChanged();
    }

    Q_INVOKABLE virtual QVariantMap getProperties();


    virtual Status status() const;
    bool active(){ return m_status == Status::Running || m_status == Status::Waiting;}
    void setStatus(const Status &newStatus);

    Trigger *run() const;
    Trigger *cancel() const;

signals:

    void statusChanged();
    void runChanged();
    void cancelChanged();
    void activeChanged();
    void errorChanged();


protected:
    Status m_status = Idle;
    int m_errorcode = 0;  //0 - ok, 1 - canceled
    mutable Trigger m_run;
    mutable Trigger m_cancel;
};

Q_DECLARE_METATYPE(StateBase::Status)

#endif // STATEBASE_H
