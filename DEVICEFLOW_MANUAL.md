# DeviceFlow User Manual

## Table of Contents

1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Core Concepts](#core-concepts)
4. [Class Reference](#class-reference)
   - [StateBase](#statebase)
   - [SequenceBase](#sequencebase)
   - [DeviceFlow (Flow)](#deviceflow-flow)
   - [Trigger](#trigger)
5. [QML Component: Sequence](#qml-component-sequence)
6. [Wait Mechanism](#wait-mechanism)
7. [Status & Lifecycle](#status--lifecycle)
8. [QML Module Registration](#qml-module-registration)
9. [Usage Examples](#usage-examples)
10. [Integration Guide](#integration-guide)
11. [Design Notes](#design-notes)

---

## Overview

**DeviceFlow** is a Qt5/QML state machine framework for writing multi-step asynchronous flows in a synchronous style. It is designed for applications that need to:

- Execute sequential logic (open → query → execute → close) without callback nesting
- Wait for hardware events, timers, or user input while keeping the Qt event loop running
- Cancel or time out any step mid-execution
- Expose flow state to QML bindings for live UI updates

**Typical use cases:** ATM transaction flows, device initialization sequences, multi-step protocol handlers, interactive test scripts.

The framework provides four C++ types registered in QML, plus one QML component (`Sequence.qml`) that wraps the C++ base with a convenient `wait()` API.

---

## Getting Started

### Build

```bash
cd deviceflow
qmake DeviceFlow.pro
make
```

The module compiles to a static library or is included directly via `deviceflow.pri` (see [Integration Guide](#integration-guide)).

### Minimum Requirements

| Requirement | Detail |
|---|---|
| Qt | 5.15.2 (`QML_ELEMENT` and `Qt.binding` require 5.15+) |
| Build system | qmake |
| Compiler | GCC or MSVC (32-bit for XFS host — see parent CLAUDE.md) |

---

## Core Concepts

### Flow and Sequences

A **Flow** (`DeviceFlow`) is a container that holds one or more **Sequences** (`SequenceBase`). Each Sequence is an independent step that runs when its `run` trigger fires, executes its `onEnter` handler, and finishes when the handler returns.

```
Flow
 ├─ Sequence 1   (hardware init)
 ├─ Sequence 2   (user interaction)
 └─ Sequence 3   (cleanup)
```

### Synchronous-Looking Async Code

Inside a Sequence's `onEnter` handler, you can call `wait()` to pause execution until a condition becomes true or a signal fires — without blocking the Qt event loop. Other QML bindings, timers, and signals continue to process during the wait.

```qml
Sequence {
    onEnter: {
        wait(function() { return hardwareReady })   // pause here
        doNextStep()                                // resumes when ready
    }
}
```

### Triggers

A **Trigger** is a simple fire-once event source. `run` and `cancel` triggers on every state give external code (buttons, other sequences, timers) a way to start or stop a sequence without direct method calls.

---

## Class Reference

### StateBase

Base class for all state objects. Provides status tracking, error state, and `run`/`cancel` triggers.

**QML type name:** `StateBase`
**Module:** `FlowGraph 1.0`

#### Properties

| Property | Type | Access | Description |
|---|---|---|---|
| `status` | `Status` | read-only | Current status enum value |
| `isActive` | `bool` | read-only | `true` when status is `Running` or `Waiting` |
| `error` | `int` | read/write | Error code: `0` = none, `1` = cancelled, `>1` = error |
| `run` | `Trigger*` | read-only | Fire to start this state |
| `cancel` | `Trigger*` | read-only | Fire to cancel this state |

#### Status Enum

| Value | Meaning |
|---|---|
| `StateBase.Idle` | Not started |
| `StateBase.Running` | Actively executing |
| `StateBase.Waiting` | Paused inside a `wait()` call |
| `StateBase.Completed` | Finished successfully |

#### Signals

| Signal | Emitted when |
|---|---|
| `statusChanged()` | Status changes |
| `activeChanged()` | `isActive` transitions true ↔ false |
| `errorChanged()` | Error code changes |

#### Methods

| Method | Description |
|---|---|
| `QVariantMap getProperties()` | Returns all public properties as a map with `value`, `type`, and `access` fields. Used internally by FlowConsole for automatic UI generation. |

---

### SequenceBase

Extends `StateBase` with blocking wait capabilities. This is the C++ base for all executable steps.

**QML type name:** (not directly instantiable in QML — use `Sequence` component)

#### Additional Properties

| Property | Type | Access | Description |
|---|---|---|---|
| `condition` | `bool` | read/write | Reactive condition for `waitCondition()` |
| `stepDescription` | `QString` | read-only | Human-readable description (shown in FlowConsole) |
| `stepIndex` | `int` | read-only | Current step index within a multi-step sequence |

#### Additional Signals

| Signal | Emitted when |
|---|---|
| `enter()` | Sequence starts executing (run trigger fires) |
| `exit()` | Sequence finishes |
| `conditionChanged()` | `condition` property changes |
| `stepIndexChanged()` | Step index changes |
| `stepDescriptionChanged()` | Description changes |

#### Wait Methods (Q_INVOKABLE)

Both methods block the sequence (via an internal `QEventLoop`) while allowing the Qt event loop to continue processing.

---

**`waitCondition(int timeout = 0) → WaitResult`**

Pauses until the `condition` property becomes `true`.

| Parameter | Description |
|---|---|
| `timeout` | Milliseconds before automatic `Timeout` result. `0` = no timeout. |

Returns immediately if `condition` is already `true`.

---

**`waitSignal(QObject* sender, QString name, int timeout = 0) → WaitResult`**

Pauses until the named signal fires on `sender`.

| Parameter | Description |
|---|---|
| `sender` | Any `QObject` (including QML objects) |
| `name` | Signal name with or without `"()"` suffix |
| `timeout` | Milliseconds before automatic `Timeout`. `0` = no timeout. |

Returns `WaitResult.Error` if the signal is not found on `sender`.

---

#### WaitResult Enum

| Value | Meaning |
|---|---|
| `SequenceBase.Finished` | Condition met or signal received |
| `SequenceBase.Cancelled` | `cancel.fire()` was called during wait |
| `SequenceBase.Timeout` | Timeout elapsed before condition/signal |
| `SequenceBase.Error` | Signal not found or connection failed |

#### Lifecycle Hooks

| Signal / Handler | When |
|---|---|
| `onEnter` | Fires when `run` trigger fires; your main logic goes here |
| `onExit` | Fires after `onEnter` returns |

---

### DeviceFlow (Flow)

Container for sequences. Aggregates the status of all child sequences and exposes them as a QML list property.

**QML type name:** `Flow`
**Module:** `FlowGraph 1.0`

Inherits all `StateBase` properties and signals.

#### Additional Properties

| Property | Type | Access | Description |
|---|---|---|---|
| `sequences` | `QQmlListProperty<SequenceBase>` | read-only | List of child sequences (also the default property — add sequences as direct children in QML) |

#### Status Aggregation

The Flow's `status` is computed from its children:

| Child states | Flow status |
|---|---|
| Any child is `Running` | `Running` |
| Any child is `Waiting`, none `Running` | `Waiting` |
| All children are `Idle` | `Idle` |
| All children are `Completed` | `Completed` |

---

### Trigger

Simple fire-once event source. Can be enabled or disabled.

**QML type name:** `Trigger`
**Module:** `FlowGraph 1.0`

#### Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `enabled` | `bool` | `true` | If `false`, `fire()` does nothing |

#### Methods

| Method | Returns | Description |
|---|---|---|
| `fire()` | `bool` | Fire the trigger. Returns `true` if queued, `false` if disabled. The `fired()` signal is emitted on the next event loop iteration (queued connection). |

#### Signals

| Signal | Emitted when |
|---|---|
| `fired()` | Trigger fires (asynchronous — next event loop tick) |
| `enabledChanged()` | `enabled` changes |

---

## QML Component: Sequence

`Sequence.qml` is a QML component extending `SequenceBase` that provides the user-friendly `wait()` function.

**Import:** available after `include(deviceflow.pri)` and adding the component directory to the QML engine's import paths.

### `wait()` Function

```js
wait(conditionFn)
wait(conditionFn, timeoutMs)
wait(sender, "signalName()")
wait(sender, "signalName()", timeoutMs)
```

**Condition mode** (1 or 2 arguments where second is a number):

```qml
var result = wait(function() { return someProperty > 10 })
var result = wait(function() { return deviceReady }, 5000)
```

Internally creates a `Qt.binding` from the function and assigns it to `condition`, then calls `waitCondition(timeout)`. The binding is cleaned up (set to `false`) after the wait completes.

**Signal mode** (2 or 3 arguments where second is a string):

```qml
var result = wait(someObject, "readyChanged()")
var result = wait(someObject, "readyChanged()", 3000)
```

Internally calls `waitSignal(sender, signalName, timeout)`.

**Return value:** a `WaitResult` value (`Finished`, `Cancelled`, `Timeout`, or `Error`).

### Checking Wait Results

```qml
Sequence {
    onEnter: {
        var result = wait(function() { return sensor.ready }, 10000)

        if (result === SequenceBase.Timeout) {
            error = 2
            return
        }
        if (result === SequenceBase.Cancelled) {
            return
        }
        // result === SequenceBase.Finished — proceed
        processSensorData()
    }
}
```

---

## Wait Mechanism

### How Blocking Works

Both wait methods create a `QEventLoop` and call `.exec()`. This suspends the Sequence's call stack but **does not freeze the UI** — Qt continues delivering events, running timers, and updating bindings during the wait.

```
Sequence onEnter called
    ↓
wait() called
    ↓
QEventLoop::exec()  ← execution pauses here
    │
    ├─ Qt delivers other events (UI repaints, timers, signals)
    │
    ├─ condition becomes true → conditionChanged → loop.quit()
    ├─ OR target signal fires → loop.quit()
    ├─ OR timeout timer fires → loop.quit()
    └─ OR cancel.fire() → loop.quit()
    ↓
QEventLoop::exec() returns
    ↓
wait() returns WaitResult
    ↓
onEnter continues
```

### Concurrent Sequences

Multiple sequences can be in a waiting state simultaneously. Each has its own independent `QEventLoop`. Starting `seq2.run.fire()` while `seq1` is blocked in `wait()` is valid — the event loop processes both.

### Cancellation During Wait

```qml
// Cancel from a button:
Button {
    onClicked: mySeq.cancel.fire()
}

// The wait() call inside onEnter returns SequenceBase.Cancelled
Sequence {
    id: mySeq
    onEnter: {
        var result = wait(function() { return slowOperation.done })
        if (result === SequenceBase.Cancelled) {
            cleanup()
        }
    }
}
```

---

## Status & Lifecycle

### Sequence Lifecycle

```
Idle
 │
 ├─ run.fire() ──────────────────────────► Running
 │                                           │
 │                                      wait() called
 │                                           │
 │                                        Waiting
 │                                           │
 │                              condition/signal/timeout
 │                                           │
 │                                        Running (resumed)
 │                                           │
 │                                    onEnter returns
 │                                           │
 └─────────────────────────────────────── Completed
```

Cancellation can interrupt `Running` or `Waiting` at any point. Error state is set manually by writing to the `error` property.

### Monitoring Status in QML

```qml
Text {
    text: mySeq.status === SequenceBase.Running  ? "Running"   :
          mySeq.status === SequenceBase.Waiting  ? "Waiting"   :
          mySeq.status === SequenceBase.Completed ? "Done"     : "Idle"
}

// Or use isActive for a simple busy check:
BusyIndicator { running: mySeq.isActive }
```

---

## QML Module Registration

Register types in `main.cpp` before loading QML:

```cpp
#include "state.h"
#include "sequencebase.h"
#include "deviceflow.h"
#include "trigger.h"

// In main():
qRegisterMetaType<StateBase::Status>("Status");
qRegisterMetaType<StateBase::Status>("StateBase::Status");
qRegisterMetaType<SequenceBase::WaitResult>("WaitResult");
qRegisterMetaType<SequenceBase::WaitResult>("SequenceBase::WaitResult");

qmlRegisterType<StateBase>  ("FlowGraph", 1, 0, "StateBase");
qmlRegisterType<DeviceFlow> ("FlowGraph", 1, 0, "Flow");
qmlRegisterType<Trigger>    ("FlowGraph", 1, 0, "Trigger");
qmlRegisterType<SequenceBase>("FlowGraph", 1, 0, "SequenceBase");
```

Then add the directory containing `Sequence.qml` to the engine's import path so it's found at runtime:

```cpp
engine.addImportPath("path/to/deviceflow/");
```

In QML:

```qml
import FlowGraph 1.0
```

---

## Usage Examples

### Basic Two-Sequence Flow

```qml
import FlowGraph 1.0

Flow {
    id: myFlow

    Sequence {
        id: initSeq
        stepDescription: "Hardware Init"

        onEnter: {
            device.open()
            wait(device, "openComplete()", 5000)
            logWindow.append("✔ device open")
        }
    }

    Sequence {
        id: dispenseSeq
        stepDescription: "Dispense Cash"

        property int amount: 2000

        onEnter: {
            device.dispense(amount)
            var r = wait(device, "dispenseComplete()", 30000)
            if (r === SequenceBase.Timeout) {
                error = 2
                logWindow.append("✖ dispense timeout")
                return
            }
            logWindow.append("✔ dispensed " + amount)
        }
    }
}

// Start the flow:
Button { onClicked: initSeq.run.fire() }
```

### Condition-Based Wait

```qml
Sequence {
    onEnter: {
        // Wait until a property crosses a threshold
        wait(function() { return pressureGauge.value >= 100 })
        logWindow.append("✔ pressure reached")
    }
}
```

### Multi-Step Progress Tracking

```qml
Sequence {
    id: seq
    property var steps: ["Connect", "Authenticate", "Query", "Disconnect"]
    property int stepIndex: 0

    onEnter: {
        for (var i = 0; i < steps.length; i++) {
            stepIndex = i
            wait(function() { return backend.stepDone }, 5000)
            if (!backend.stepDone) { error = 2; return }
        }
    }
}
```

`stepIndex` and `steps` are automatically picked up by FlowConsole and rendered as a progress bar.

### Chaining Sequences

```qml
Flow {
    id: flow

    Sequence {
        id: step1
        onEnter: {
            doStepOne()
            step2.run.fire()   // hand off to next step
        }
    }

    Sequence {
        id: step2
        onEnter: {
            doStepTwo()
        }
    }
}
```

### External Trigger from a Button

```qml
Flow {
    id: flow
    Sequence {
        id: mySeq
        onEnter: { /* ... */ }
    }
}

Button {
    text: "Start"
    enabled: !mySeq.isActive
    onClicked: mySeq.run.fire()
}

Button {
    text: "Cancel"
    enabled: mySeq.isActive
    onClicked: mySeq.cancel.fire()
}
```

### Disabling a Trigger Conditionally

```qml
Sequence {
    id: seq
    Component.onCompleted: {
        run.enabled = Qt.binding(function() { return hardwareReady })
    }
}
```

---

## Integration Guide

### Embedding via `.pri`

Add to your `.pro` file:

```qmake
include(path/to/deviceflow/deviceflow.pri)
```

Then register types in your `main.cpp` as shown in [QML Module Registration](#qml-module-registration).

### Using with FlowConsole

FlowConsole automatically discovers and renders UI for any `Flow` object. Pass it via the `flow` property:

```qml
FlowConsole {
    flow: myFlow
}
```

FlowConsole calls `flow.getProperties()` to enumerate all public properties and generates appropriate widgets. To make a property visible in FlowConsole:

- Declare it as a `Q_PROPERTY` on a C++ subclass, or
- Declare it as a plain QML `property` on a `Sequence`

To make it editable in FlowConsole, ensure the property has a write accessor.

### Adding Custom Properties to a Sequence

In QML (directly on a `Sequence` instance):

```qml
Sequence {
    property string deviceName: "CDM30"
    property bool deviceReady: false
    property int retryCount: 0
}
```

In C++ (for shared behavior across multiple flow types):

```cpp
class MySequence : public SequenceBase {
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)
    // ...
};
```

---

## Design Notes

### Why QEventLoop Instead of Promises/Async?

QML lacks native async/await. `QEventLoop::exec()` achieves the same effect: the call stack is suspended but Qt's scheduler continues. This avoids callback nesting while staying compatible with all Qt signal/slot and binding machinery.

### Why Queued Connection for Trigger::fire()?

`Trigger::fire()` emits `fired()` via `Qt::QueuedConnection`. This ensures the caller's stack unwinds before the connected handler runs. If `fired()` were direct, a handler that calls `fire()` again could cause re-entrant issues.

### Signal Lookup from Index 0

`waitSignal()` searches the `QMetaObject` starting at index 0 (not `methodOffset()`). This ensures QML-generated signals (which appear in the inherited range) are found correctly. String-based `SIGNAL()` macros are not used because they fail silently for QML-defined signals.

### Condition Binding Lifecycle

When `Sequence.wait(fn)` is called, it assigns `Qt.binding(fn)` to the `condition` property. After `waitCondition()` returns, the binding is replaced with `false` to prevent it from continuing to evaluate and potentially re-triggering an unexpected state change.

### Mutable Trigger Members

`StateBase` declares `m_run` and `m_cancel` as `mutable` so the `const` getter methods `run()` and `cancel()` can return non-const `Trigger*` pointers. This is necessary because QML properties require non-const pointers for binding.
