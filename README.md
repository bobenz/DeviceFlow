# DeviceFlow

A Qt5/QML-based state machine framework for building visual programming flows with asynchronous state management and timeout support.

## Overview

DeviceFlow enables you to write synchronous-looking asynchronous code in QML. It provides a powerful `Sequence` component that can wait for conditions or signals while keeping the Qt event loop running, making it perfect for building complex interactive flows, device communication protocols, or multi-step user workflows.

## Features

- **Synchronous-style async code**: Write sequential logic that waits for conditions without blocking the UI
- **Timeout support**: All wait operations support optional timeouts
- **Cancellation**: Every sequence can be cancelled mid-execution
- **Flexible waiting**: Wait for boolean conditions or Qt signals
- **Event-driven triggers**: Built-in trigger system for flow control
- **QML-native**: Designed for declarative UI programming

## Installation

### Prerequisites
- Qt 5.15.2 or later
- C++ compiler with C++11 support

### Building

```bash
# Configure the project
/opt/Qt/5.15.2/gcc_64/bin/qmake DeviceFlow.pro

# Build
make

# Run
export LD_LIBRARY_PATH=/opt/Qt/5.15.2/gcc_64/lib:$LD_LIBRARY_PATH
./DeviceFlow
```

## Quick Start

```qml
import QtQuick 2.15
import FlowGraph 1.0

Sequence {
    id: mySequence

    onEnter: {
        // This runs when the sequence starts
        console.log("Starting sequence...")

        // Wait for a property to change
        wait(function() { return someValue > 10 })

        console.log("Condition met! Continuing...")

        // Do more work
        doSomething()
    }

    // Start the sequence
    Component.onCompleted: run.fire()
}
```

## Core Components

### Sequence

`Sequence` is the main building block for creating sequential async flows. It extends `StateBase` and provides a powerful `wait()` function.

#### Properties

- **`run`** (Trigger, read-only): Trigger to start the sequence
- **`cancel`** (Trigger, read-only): Trigger to cancel the sequence
- **`status`** (Status, read-only): Current status (IDLE, RUNNING, FINISHED, CANCELLED, ERROR)

#### Signals

- **`enter()`**: Emitted when the sequence starts
- **`exit()`**: Emitted when the sequence completes
- **`statusChanged()`**: Emitted when status changes

#### The `wait()` Function

The `wait()` function has two overloaded forms:

##### 1. Wait for Condition

```qml
WaitResult wait(function() { return booleanExpression }, timeout)
```

**Parameters:**
- `booleanExpression`: A function returning a boolean value
- `timeout` (optional): Timeout in milliseconds (0 = no timeout)

**Returns:** `WaitResult` enum value

**Example:**
```qml
Sequence {
    property int counter: 0

    onEnter: {
        // Wait up to 5 seconds for counter to reach 10
        var result = wait(function() { return counter >= 10 }, 5000)

        if (result === SequenceBase.Finished) {
            console.log("Counter reached 10!")
        } else if (result === SequenceBase.Timeout) {
            console.log("Timed out waiting for counter")
        } else if (result === SequenceBase.Cancelled) {
            console.log("User cancelled the wait")
        }
    }
}
```

##### 2. Wait for Signal

```qml
WaitResult wait(sender, "signalName", timeout)
```

**Parameters:**
- `sender`: The QObject that will emit the signal
- `signalName`: Name of the signal (with or without parentheses)
- `timeout` (optional): Timeout in milliseconds (0 = no timeout)

**Returns:** `WaitResult` enum value

**Example:**
```qml
Sequence {
    id: sequence

    Button {
        id: myButton
        text: "Click me"
    }

    onEnter: {
        console.log("Waiting for button click...")

        // Wait up to 10 seconds for button click
        var result = wait(myButton, "clicked()", 10000)

        if (result === SequenceBase.Finished) {
            console.log("Button was clicked!")
        } else if (result === SequenceBase.Timeout) {
            console.log("Nobody clicked the button")
        }
    }
}
```

#### WaitResult Enum

All `wait()` operations return a `WaitResult`:

- **`SequenceBase.Finished`**: Wait completed successfully
- **`SequenceBase.Cancelled`**: Wait was cancelled via the cancel trigger
- **`SequenceBase.Timeout`**: Wait timed out
- **`SequenceBase.Error`**: An error occurred (e.g., invalid signal name)

### Trigger

`Trigger` is an event source that can be enabled/disabled and fired to trigger actions.

#### Properties

- **`enabled`** (bool): Whether the trigger can fire (default: true)

#### Methods

- **`fire()`**: Fire the trigger (returns true if successful, false if disabled)

#### Signals

- **`fired()`**: Emitted when the trigger fires
- **`enabledChanged()`**: Emitted when enabled state changes

#### Example

```qml
import QtQuick 2.15
import FlowGraph 1.0

Item {
    Trigger {
        id: myTrigger
        enabled: true

        onFired: {
            console.log("Trigger fired!")
        }
    }

    Button {
        text: "Fire Trigger"
        enabled: myTrigger.enabled
        onClicked: myTrigger.fire()
    }

    CheckBox {
        text: "Enable Trigger"
        checked: myTrigger.enabled
        onCheckedChanged: myTrigger.enabled = checked
    }
}
```

## Examples

### Example 1: Simple Countdown

```qml
import QtQuick 2.15
import FlowGraph 1.0

Item {
    property int countdown: 10

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: if (countdown > 0) countdown--
    }

    Sequence {
        id: countdownSequence

        onEnter: {
            console.log("Waiting for countdown...")
            wait(function() { return countdown === 0 })
            console.log("Countdown complete!")
        }

        Component.onCompleted: run.fire()
    }

    Text {
        text: "Countdown: " + countdown
    }
}
```

### Example 2: User Input with Timeout

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Item {
    TextField {
        id: inputField
        placeholderText: "Enter your name..."
    }

    Button {
        id: submitButton
        text: "Submit"
    }

    Sequence {
        id: inputSequence

        onEnter: {
            console.log("Waiting for user input (30 seconds)...")

            var result = wait(submitButton, "clicked()", 30000)

            if (result === SequenceBase.Finished) {
                console.log("User entered:", inputField.text)
            } else if (result === SequenceBase.Timeout) {
                console.log("User took too long!")
                inputField.text = "TIMEOUT"
            }
        }

        Component.onCompleted: run.fire()
    }
}
```

### Example 3: Multi-Step Workflow with Cancellation

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Item {
    property bool step1Complete: false
    property bool step2Complete: false

    Column {
        Button {
            text: "Complete Step 1"
            onClicked: step1Complete = true
        }

        Button {
            text: "Complete Step 2"
            onClicked: step2Complete = true
        }

        Button {
            text: "Start Workflow"
            onClicked: workflow.run.fire()
        }

        Button {
            text: "Cancel Workflow"
            onClicked: workflow.cancel.fire()
        }

        Text {
            id: statusText
            text: "Status: Ready"
        }
    }

    Sequence {
        id: workflow

        onEnter: {
            statusText.text = "Status: Waiting for step 1..."
            var result = wait(function() { return step1Complete }, 10000)

            if (result === SequenceBase.Cancelled) {
                statusText.text = "Status: Cancelled by user"
                return
            } else if (result === SequenceBase.Timeout) {
                statusText.text = "Status: Step 1 timeout"
                return
            }

            statusText.text = "Status: Waiting for step 2..."
            result = wait(function() { return step2Complete }, 10000)

            if (result === SequenceBase.Cancelled) {
                statusText.text = "Status: Cancelled by user"
                return
            } else if (result === SequenceBase.Timeout) {
                statusText.text = "Status: Step 2 timeout"
                return
            }

            statusText.text = "Status: Workflow complete!"
        }
    }
}
```

### Example 4: Combining Multiple Wait Types

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Item {
    property int temperature: 20

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: temperature += Math.random() * 2 - 1
    }

    Button {
        id: overrideButton
        text: "Override Temperature Check"
    }

    Sequence {
        id: tempMonitor

        onEnter: {
            console.log("Monitoring temperature...")

            // Wait for temperature to exceed 25 OR user override
            var tempResult = wait(function() { return temperature > 25 }, 15000)
            var overrideResult = wait(overrideButton, "clicked()", 15000)

            // Start both waits (first one to complete wins)
            // Note: This example shows the pattern; actual parallel waiting
            // would require additional implementation

            if (temperature > 25) {
                console.log("Temperature threshold exceeded:", temperature)
            } else {
                console.log("Manual override activated")
            }
        }

        Component.onCompleted: run.fire()
    }
}
```

### Example 5: Retry Logic with Timeout

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import FlowGraph 1.0

Item {
    property bool connectionSuccessful: false
    property int attemptCount: 0

    Button {
        text: "Simulate Connection Success"
        onClicked: connectionSuccessful = true
    }

    Sequence {
        id: connectionRetry

        onEnter: {
            var maxAttempts = 3

            for (attemptCount = 1; attemptCount <= maxAttempts; attemptCount++) {
                console.log("Connection attempt", attemptCount, "of", maxAttempts)

                connectionSuccessful = false
                var result = wait(function() { return connectionSuccessful }, 5000)

                if (result === SequenceBase.Finished) {
                    console.log("Connection successful!")
                    return
                } else if (result === SequenceBase.Cancelled) {
                    console.log("Connection cancelled by user")
                    return
                } else if (result === SequenceBase.Timeout) {
                    console.log("Attempt", attemptCount, "timed out")
                    if (attemptCount < maxAttempts) {
                        console.log("Retrying...")
                    }
                }
            }

            console.log("All connection attempts failed")
        }

        Component.onCompleted: run.fire()
    }
}
```

## Best Practices

### 1. Always Check WaitResult

Always check the return value of `wait()` to handle timeouts and cancellations properly:

```qml
var result = wait(function() { return someCondition }, 5000)
if (result !== SequenceBase.Finished) {
    // Handle timeout or cancellation
    return
}
// Continue with normal flow
```

### 2. Use Appropriate Timeouts

Choose timeouts based on the expected user response time or system behavior:
- User interactions: 10-30 seconds
- Quick animations: 1-5 seconds
- Network operations: 30-60 seconds
- No timeout: Pass 0 or omit the parameter

### 3. Provide Cancel Options

Always give users a way to cancel long-running sequences:

```qml
Button {
    text: "Cancel"
    onClicked: mySequence.cancel.fire()
}
```

### 4. Use Functions for Complex Conditions

For complex wait conditions, use a separate function for better readability:

```qml
function isSystemReady() {
    return temperature > 20 && pressure < 100 && !errorState
}

onEnter: {
    wait(isSystemReady, 10000)
}
```

### 5. Clean Up on Cancellation

Always handle the `Cancelled` result to clean up resources:

```qml
onEnter: {
    startResource()
    var result = wait(function() { return resourceReady }, 10000)

    if (result === SequenceBase.Cancelled) {
        cleanupResource()
        return
    }

    useResource()
}
```

## API Reference

### Sequence (extends StateBase)

| Member | Type | Description |
|--------|------|-------------|
| `run` | Trigger | Start the sequence |
| `cancel` | Trigger | Cancel the sequence |
| `status` | Status | Current status (IDLE, RUNNING, FINISHED, CANCELLED, ERROR) |
| `wait(condition, timeout)` | function | Wait for condition to become true |
| `wait(sender, signal, timeout)` | function | Wait for signal to fire |
| `enter()` | signal | Sequence started |
| `exit()` | signal | Sequence completed |

### Trigger

| Member | Type | Description |
|--------|------|-------------|
| `enabled` | bool | Can the trigger fire |
| `fire()` | function | Fire the trigger (returns bool) |
| `fired()` | signal | Trigger was fired |
| `enabledChanged()` | signal | Enabled state changed |

### Enums

#### StateBase.Status
- `IDLE`: Not running
- `RUNNING`: Currently executing
- `FINISHED`: Completed successfully
- `CANCELLED`: Cancelled by user
- `ERROR`: Error occurred

#### SequenceBase.WaitResult
- `Finished`: Wait completed successfully
- `Cancelled`: Wait was cancelled
- `Timeout`: Wait timed out
- `Error`: Error occurred during wait

## Troubleshooting

### Wait Never Returns

**Problem:** The `wait()` function never returns even though the condition should be true.

**Solution:** Ensure the condition is reactive. Use properties that emit change signals:

```qml
// Good - property change triggers signal
property int value: 0
wait(function() { return value > 10 })

// Bad - JavaScript variable doesn't emit signals
var value = 0
wait(function() { return value > 10 })  // Won't work!
```

### Signal Not Found Error

**Problem:** `waitSignal: Signal 'xxx' not found on object`

**Solution:**
1. Verify the signal exists on the object
2. Check signal name spelling (case-sensitive)
3. Signal names can include or omit the `()`: both `"clicked"` and `"clicked()"` work

### Sequence Doesn't Start

**Problem:** The `onEnter` handler never runs.

**Solution:** Make sure to fire the `run` trigger:

```qml
Sequence {
    id: seq
    onEnter: { /* ... */ }

    // Don't forget to start it!
    Component.onCompleted: run.fire()
}
```

## License

[Your license here]

## Contributing

[Contributing guidelines here]
