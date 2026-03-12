#include "deviceflow.h"

DeviceFlow::DeviceFlow(QObject *parent)
    : QObject{parent}
{


}

StateBase *DeviceFlow::currentState() const
{
    return m_currentState;
}

void DeviceFlow::setCurrentState(StateBase *newState)
{
    if (m_currentState == newState)
        return;
    m_currentState = newState;
    emit currentStateChanged();
}

StateBase *DeviceFlow::initialState() const
{
    return m_initialState;
}

void DeviceFlow::setInitialState(StateBase *newInitialState)
{
    if (m_initialState == newInitialState)
        return;
    if(m_initialState() != nullptr)
    {
       // disconnect
    }
    m_initialState = newInitialState;

    if(m_initialState() != nullptr)
        connect(&m_run, &Trigger::fired, m_initialState->_enter());
    emit initialStateChanged();
}

void DeviceFlow::setState(StateBase* state)
{

    if(m_currentState)
    {
        m_currentState->_exit();
        state->setPrevState(m_currentState);
    }

    setCurrentState(state);
      state->_enter();
}

Trigger *DeviceFlow::run() const
{
    return &m_run;
}



Trigger *DeviceFlow::cancel() const
{
    return &m_cancel;
}

