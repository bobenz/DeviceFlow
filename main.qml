import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

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
                onClicked: testState.runFlow()
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

        function runFlow() {
            statusText.text = "Status: Waiting for counter >= 5..."

            // Wait for counter to reach 5
            wait(function() { return counter >= 5 })

            statusText.text = "Status: Counter reached 5! Now waiting for testCondition..."

            // Wait for testCondition to become true
            wait(win, "testConditionChanged()");

            statusText.text = "Status: Flow Complete! ✓"
        }
    }

    Component.onCompleted: {
        console.log("DeviceFlow Test Window Ready")
    }
}
