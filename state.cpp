#include "state.h"
#include "deviceflow.h"
StateBase::StateBase(QObject *parent)
    : QObject{parent}
{}

StateBase *StateBase::prevState() const
{
    return m_prevState;
}

void StateBase::setPrevState(StateBase* prev)
{
    m_prevState = prev;
}


Status StateBase::status() const
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
