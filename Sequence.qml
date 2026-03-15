import QtQuick 2.15
import FlowGraph 1.0

StateBase {
    id: root

    // Overloaded wait function:
    // 1. wait(booleanExpression[, timeout]) - waits for boolean expression to become true
    // 2. wait(sender, "signalName()"[, timeout]) - waits for a specific signal
    // Returns: WaitResult (Finished, Cancelled, Timeout, Error)
    function wait(arg1, arg2, arg3) {
        var timeout = -1;

        if (arguments.length === 1 || (arguments.length === 2 && typeof arg2 === "number")) {
            // Boolean expression case: wait(expression) or wait(expression, timeout)
            timeout = (arguments.length === 2) ? arg2 : 0;
            root.condition = Qt.binding(arg1);
            return root.waitCondition(timeout);

        } else if (arguments.length === 2 || (arguments.length === 3  && typeof arg2 === "number")) {
            // Signal case: wait(sender, signal) or wait(sender, signal, timeout)
            // arg1 = sender object, arg2 = signal name, arg3 = optional timeout
            timeout = (arguments.length === 3) ? arg3 :0;
            return root.waitSignal(arg1, arg2, timeout);
        }

        return StateBase.Error;
    }
}
