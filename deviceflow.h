#ifndef DEVICEFLOW_H
#define DEVICEFLOW_H

#include <QObject>
#include "trigger.h"
#include "state.h"

class DeviceFlow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(StateBase *currentState READ currentState WRITE setCurrentState NOTIFY
                   currentStateChanged FINAL)
    Q_PROPERTY(StateBase *initialState READ initialState WRITE setInitialState NOTIFY
                   initialStateChanged FINAL)
    Q_PROPERTY(Trigger *run READ run NOTIFY runChanged FINAL)
    Q_PROPERTY(Trigger *cancel READ cancel NOTIFY cancelChanged FINAL)
public:
    explicit DeviceFlow(QObject *parent = nullptr);
    Q_INVOKABLE void setState(StateBase *state);
    StateBase *currentState() const;
    void setCurrentState(StateBase *newCurrentState);

    StateBase *initialState() const;
    void setInitialState(StateBase *newInitialState);

    Trigger *run() const;

    Trigger *cancel() const;

signals:

    void currentStateChanged();
    void initialStateChanged();

    void runChanged();

    void cancelChanged();

private:
    StateBase *m_currentState = nullptr;
    StateBase *m_initialState = nullptr;
    mutable Trigger m_run;
    mutable Trigger m_cancel;
};

#endif // DEVICEFLOW_H
