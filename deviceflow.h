#ifndef DEVICEFLOW_H
#define DEVICEFLOW_H

#include <QObject>
#include "trigger.h"
#include "state.h"

class DeviceFlow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Trigger *run READ run NOTIFY runChanged FINAL)
    Q_PROPERTY(Trigger *cancel READ cancel NOTIFY cancelChanged FINAL)
public:
    explicit DeviceFlow(QObject *parent = nullptr);

    Trigger *run() const;

    Trigger *cancel() const;

signals:


    void runChanged();

    void cancelChanged();

private:
    mutable Trigger m_run;
    mutable Trigger m_cancel;
};

#endif // DEVICEFLOW_H
