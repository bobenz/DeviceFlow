#ifndef SEQUENCEBASE_H
#define SEQUENCEBASE_H

#include <QObject>
#include "state.h"



class SequenceBase : public StateBase
{
    Q_OBJECT
    Q_PROPERTY(bool condition READ condition WRITE setCondition NOTIFY conditionChanged)
    Q_PROPERTY(QString stepDescription  READ stepDescription NOTIFY stepDescriptionChanged)
    Q_PROPERTY(int stepIndex READ stepIndex NOTIFY stepIndexChanged)
public:
    enum WaitResult { Finished, Cancelled, Timeout, Error };
    Q_ENUM(WaitResult)

    explicit SequenceBase(QObject *parent = nullptr);

    Q_INVOKABLE WaitResult waitCondition(int timeout = 0);

    Q_INVOKABLE WaitResult waitSignal(QObject* sender, QString name, int timeout = 0);



    int stepIndex() const;
    QString stepDescription() const;

    bool condition() const { return m_condition; }
    void setCondition(bool newCondition)
    {
        if (m_condition == newCondition)
            return;
        m_condition = newCondition;
        emit conditionChanged();
    }
private slots:
    void _enter()
    {
        if (m_status != Status::Idle)
            return;
        setStatus(Status::Running);
        emit enter();
        setStatus(Status::Completed);
        emit exit();
    }

    void _exit()
    {
        setStatus(Status::Completed);
        emit exit();
    }
signals:
    void enter();
    void exit();
    void conditionChanged();
    void stepIndexChanged();
    void stepDescriptionChanged();

private:
    bool m_condition = false;
    int m_stepIndex;
    QString m_stepDescription;
};
Q_DECLARE_METATYPE(SequenceBase::WaitResult)

#endif // SEQUENCEBASE_H
