#include "deviceflow.h"

DeviceFlow::DeviceFlow(QObject *parent)
    : StateBase{parent}
{}

QQmlListProperty<SequenceBase> DeviceFlow::sequences()
{
    return QQmlListProperty<SequenceBase>(this, &m_sequences,
                                          &DeviceFlow::appendSequence,
                                          &DeviceFlow::sequenceCount,
                                          &DeviceFlow::sequenceAt,
                                          &DeviceFlow::clearSequences);
}

void DeviceFlow::appendSequence(QQmlListProperty<SequenceBase>* list, SequenceBase* p)
{
    DeviceFlow* df = reinterpret_cast<DeviceFlow*>(list->object);
    df->m_sequences.append(p);
    connect(p, &StateBase::statusChanged, df, &StateBase::statusChanged);
    connect(p, &StateBase::activeChanged, df, &StateBase::activeChanged);
}

int DeviceFlow::sequenceCount(QQmlListProperty<SequenceBase>* list)
{
    return reinterpret_cast<DeviceFlow*>(list->object)->m_sequences.count();
}

SequenceBase* DeviceFlow::sequenceAt(QQmlListProperty<SequenceBase>* list, int index)
{
    return reinterpret_cast<DeviceFlow*>(list->object)->m_sequences.at(index);
}

void DeviceFlow::clearSequences(QQmlListProperty<SequenceBase>* list)
{
    DeviceFlow* df = reinterpret_cast<DeviceFlow*>(list->object);
    for (SequenceBase* p : df->m_sequences) {
        if (p) {
            disconnect(p, &StateBase::statusChanged, df, &StateBase::statusChanged);
            disconnect(p, &StateBase::activeChanged, df, &StateBase::activeChanged);
        }
    }
    df->m_sequences.clear();
}

StateBase::Status DeviceFlow::status() const
{
    if (m_sequences.isEmpty()) {
        return m_status;
    }
    
    bool allIdle = true;
    bool allCompleted = true;
    bool anyRunning = false;
    bool anyWaiting = false;
    
    for (SequenceBase* seq : m_sequences) {
        if (!seq) continue;
        Status s = seq->status();
        
        if (s != Idle) allIdle = false;
        if (s != Completed) allCompleted = false;
        if (s == Running) anyRunning = true;
        if (s == Waiting) anyWaiting = true;
    }
    
    if (anyRunning) return Running;
    if (anyWaiting) return Waiting; 
    if (allIdle) return Idle;
    if (allCompleted) return Completed;
    
    return m_status;
}

QVariantMap DeviceFlow::getProperties()
{
    QVariantMap map = StateBase::getProperties();
    
    QVariantList seqList;
    for (SequenceBase* seq : m_sequences) {
        if (seq) {
            seqList.append(seq->getProperties());
        }
    }
    
    map.insert(QStringLiteral("sequences"), seqList);
    return map;
}
