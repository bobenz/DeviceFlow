import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Window {
    width: 400
    height: 300
    visible: true
    title: "Timeout Test"

    Column {
        anchors.centerIn: parent
        spacing: 20

        Text {
            id: statusText
            text: "Click button to test timeout"
            font.pixelSize: 16
        }

        Button {
            text: "Test Timeout (2 seconds)"
            onClicked: {
                statusText.text = "Waiting..."
                testSeq.run.fire()
            }
        }
    }

    Sequence {
        id: testSeq

        onEnter: {
            console.log("Sequence entered")
            statusText.text = "Waiting for signal that will never come..."

            // Wait for a signal that will never fire, with 2 second timeout
            var result = wait(testSeq, "nonExistentSignal()", 2000)

            console.log("Wait returned:", result)

            if (result === SequenceBase.Timeout) {
                statusText.text = "Timeout worked! ✓"
                console.log("SUCCESS: Timeout detected")
            } else if (result === SequenceBase.Finished) {
                statusText.text = "ERROR: Finished (should be timeout)"
                console.log("ERROR: Got Finished instead of Timeout")
            } else if (result === SequenceBase.Cancelled) {
                statusText.text = "ERROR: Cancelled"
                console.log("ERROR: Got Cancelled")
            } else if (result === SequenceBase.Error) {
                statusText.text = "ERROR: Error finding signal"
                console.log("ERROR: Signal not found")
            }
        }
    }

    Component.onCompleted: {
        console.log("Timeout test ready")
    }
}
