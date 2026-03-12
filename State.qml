import QtQuick 2.15

StateBase {
    id: root

    // We can define the wait behavior directly in QML using Javascript and bindings
    function wait(conditionFunc) {
        // 1. Evaluate immediately just in case it's already true
        if (conditionFunc()) {
            return; 
        }

        // 2. Tell QML engine to track dependencies in this lambda
        //    and continually push the result to our C++ condition property.
        root.condition = Qt.binding(conditionFunc);

        // 3. Drop into C++. This will block the JS execution on this line,
        //    but the QML event loop keeps running in the background, updating bindings!
        root.beginWait();

        // 4. We are unblocked! Clean up the binding so it stops evaluating.
        root.condition = false; // Break the binding
    }
}
