import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("DeviceFlow Test")
    id:win
    property int counter: 0
    property bool testCondition: false

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Counter: " + counter
            font.pixelSize: 24
        }

        Text {
            text: "Test Condition: " + (testCondition ? "TRUE" : "FALSE")
            font.pixelSize: 20
            color: testCondition ? "green" : "red"
        }

        Text {
            id: statusText
            text: "Status: Ready"
            font.pixelSize: 18
        }

        Row {
            spacing: 10

            Button {
                text: "Start Flow"
                onClicked: testState.run.fire()
            }

            Button {
                text: "Cancel Flow"
                onClicked: testState.cancel.fire()
            }

            Button {
                text: "Increment Counter"
                onClicked: counter++
            }

            Button {
                text: "Toggle Condition"
                onClicked: testCondition = !testCondition
            }

            Button {
                text: "Reset"
                onClicked: {
                    counter = 0
                    testCondition = false
                    statusText.text = "Status: Reset"
                }
            }
        }
    }

    Sequence {
        id: testState

        onEnter: {
            statusText.text = "Status: Waiting for counter >= 5 (5 sec timeout)..."

            // Wait for counter to reach 5 with 5 second timeout
            var result = wait(function() { return counter >= 5 })

            console.log("First wait result:", result)

            if (result === SequenceBase.Timeout) {
                statusText.text = "Status: Timed out waiting for counter! ✗"
                return
            } else if (result === SequenceBase.Cancelled) {
                statusText.text = "Status: Cancelled by user ✗"
                return
            }

            statusText.text = "Status: Counter reached 5! Now waiting for testCondition (10 sec timeout)..."

            // Wait for testCondition to become true with 10 second timeout
            result = wait(function() { return testCondition }, 10000)

            console.log("Second wait result:", result)

            if (result === SequenceBase.Timeout) {
                statusText.text = "Status: Timed out waiting for condition! ✗"
                return
            } else if (result === SequenceBase.Cancelled) {
                statusText.text = "Status: Cancelled by user ✗"
                return
            }

            statusText.text = "Status: Flow Complete! ✓"
        }
    }

    Component.onCompleted: {
        console.log("DeviceFlow Test Window Ready")
    }
}
