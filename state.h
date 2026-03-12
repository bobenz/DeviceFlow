#ifndef STATEBASE_H
#define STATEBASE_H

#include <QEventLoop>
#include <QObject>

class StateBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool condition READ condition WRITE setCondition NOTIFY conditionChanged)
    Q_PROPERTY(StateBase *prevState READ prevState NOTIFY prevStateChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
public:
    enum Status { IDLE, RUNNING, FINISHED, CANCELLED, ERROR };
    Q_ENUM(StateBase::Status)
    explicit StateBase(QObject *parent = nullptr);

    bool condition() const { return m_condition; }

    void setCondition(bool newCondition)
    {
        if (m_condition == newCondition)
            return;
        m_condition = newCondition;
        emit conditionChanged();
    }

    Q_INVOKABLE void beginWait()
    {
        if (m_condition) {
            return;
        }

        QEventLoop loop;
        connect(this, &StateBase::conditionChanged, &loop, [&]() {
            if (m_condition) {
                loop.quit();
            }
        });
        loop.exec();
    }

    Q_INVOKABLE void waitSignal(QObject* sender, QString name);

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
    StateBase *prevState() const;
    void setPrevState(StateBase *);
    Status status() const;
    void setStatus(const Status &newStatus);

signals:
    void conditionChanged();
    void enter();
    void exit();

    void prevStateChanged();

    void statusChanged();

private:
    bool m_condition = false;
    StateBase *m_prevState = nullptr;
    Status m_status = IDLE;
};

Q_DECLARE_METATYPE(StateBase::Status)

#endif // STATEBASE_H
