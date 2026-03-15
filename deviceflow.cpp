#include "deviceflow.h"

DeviceFlow::DeviceFlow(QObject *parent)
    : QObject{parent}
{}




Trigger *DeviceFlow::run() const
{
    return  &m_run;
}

Trigger *DeviceFlow::cancel() const
{
    return &m_cancel;
}
