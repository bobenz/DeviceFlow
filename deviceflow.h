#ifndef DEVICEFLOW_H
#define DEVICEFLOW_H
#include <QList>
#include <QQmlListProperty>
#include "state.h"
#include "sequencebase.h"

class DeviceFlow : public StateBase
{
    Q_OBJECT
    // 1. Mark the property as the DefaultProperty for QML
    Q_CLASSINFO("DefaultProperty", "sequences")
    Q_PROPERTY(QQmlListProperty<SequenceBase> sequences READ sequences NOTIFY sequencesChanged)

public:
    explicit DeviceFlow(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap getProperties() override;
    Status status() const override;

    QQmlListProperty<SequenceBase> sequences();

signals:
    void sequencesChanged();

private:
    // 2. Static callbacks required by QQmlListProperty
    static void appendSequence(QQmlListProperty<SequenceBase>* list, SequenceBase* p);
    static int sequenceCount(QQmlListProperty<SequenceBase>* list);
    static SequenceBase* sequenceAt(QQmlListProperty<SequenceBase>* list, int index);
    static void clearSequences(QQmlListProperty<SequenceBase>* list);

    // Internal storage
    QList<SequenceBase*> m_sequences;
};

#endif // DEVICEFLOW_H
