import QtQuick 2.15
import FlowGraph 1.0

StateBase {
    id: root

    // Overloaded wait function:
    // 1. wait(booleanExpression) - waits for boolean expression to become true
    // 2. wait(sender, "signalName()") - waits for a specific signal
    function wait(arg1, arg2) {
        if (arguments.length === 1) {
            // Boolean expression case
            // Evaluate immediately in case it's already true
            if (arg1()) {
                return;
            }

            // Assign expression as binding to condition property
            root.condition = Qt.binding(arg1);

            // Wait for conditionChanged signal
            root.waitSignal(root, "conditionChanged()");

            // Clean up binding
            root.condition = false;
        } else if (arguments.length === 2) {
            // Signal case: wait for specific signal
            // arg1 = sender object, arg2 = signal name
            root.waitSignal(arg1, arg2);
        }
    }
}
