# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DeviceFlow is a Qt5/QML-based state machine framework for building visual programming flows with asynchronous state management. The project enables declarative state definitions in QML with C++ backend support for event loop integration and state transitions.

## Build System

This is a Qt5 qmake-based project using Qt 5.15.2. Use these commands:

**Build:**
```bash
/opt/Qt/5.15.2/gcc_64/bin/qmake DeviceFlow.pro
make
```

**Run:**
```bash
export LD_LIBRARY_PATH=/opt/Qt/5.15.2/gcc_64/lib:$LD_LIBRARY_PATH
./DeviceFlow
```

**Clean:**
```bash
make clean
```

Note: Build artifacts are placed in the `build/` directory. Generated files (moc_*.cpp, qrc_*.cpp, *.o) should not be committed.

## Architecture

### QML Module: FlowGraph

C++ types are registered in the `FlowGraph` QML module (main.cpp):
- `StateBase` - Base state class
- `Flow` - Flow controller (C++ class: DeviceFlow)
- `Trigger` - Event triggers

Import in QML: `import FlowGraph 1.0`

### Core Components

**DeviceFlow** (deviceflow.h/cpp): Central state machine controller
- QML type name: `Flow`
- Manages current and initial states via `StateBase*` properties
- Provides `run` and `cancel` triggers for flow control
- Handles state transitions via `setState()` which calls `_exit()` on old state and `_enter()` on new state
- Maintains bidirectional state linking through `prevState`

**StateBase** (state.h/cpp): Base class for all states
- Status enum: IDLE → RUNNING → FINISHED/CANCELLED/ERROR (registered as metatype)
- Key methods:
  - `beginWait()`: Q_INVOKABLE, blocks until `condition` property becomes true
  - `waitSignal(QObject* sender, QString name)`: Q_INVOKABLE, blocks until specified signal fires
    - Uses QMetaObject to search inheritance hierarchy for signals
    - Accepts signal names with or without "()" suffix
    - Uses index-based QMetaObject::connect for robust QML signal handling
  - `_enter()` / `_exit()`: Lifecycle hooks (emit `enter`/`exit` signals)
- The `condition` property is designed to work with QML bindings for reactive waiting

**Sequence.qml**: QML component extending StateBase
- Renamed from FlowState.qml for clarity
- Provides overloaded `wait()` function supporting two modes:
  1. **Condition mode**: `wait(function() { return expression })`
     - Evaluates condition immediately
     - Creates Qt.binding to continuously update C++ `condition` property
     - Calls `waitSignal(this, "conditionChanged()")` to block until condition is true
     - Cleans up binding when unblocked
  2. **Signal mode**: `wait(sender, "signalName()")`
     - Directly calls `waitSignal()` to wait for any Qt signal
- This pattern allows writing synchronous-looking async code in QML

**Trigger** (trigger.h): Event source for state transitions
- Properties: `enabled` (bool, default: true)
- Methods: `fire()` (Q_INVOKABLE, only succeeds if enabled)
- Signals: `fired()`, `enabledChanged()`
- Complete implementation with getter, setter, and member variable

**Action** (action.h): Currently a stub placeholder class

### QML Integration

The project uses Qt Resource System (qml.qrc) to embed QML files. Main entry point is main.qml with a test UI demonstrating both wait modes. State logic is composed using Sequence.qml components.

### Key Design Patterns

1. **Hybrid blocking model**: C++ uses `QEventLoop` to block execution while keeping Qt's event loop running, enabling bindings/signals to continue processing
2. **Property-driven conditions**: States wait on boolean conditions that QML can bind to any expression
3. **Signal-based waiting**: States can wait for any Qt signal using meta-object introspection
4. **Lifecycle management**: States track their status and previous state for complex flow control

## Development Notes

### Important Implementation Details

- **StateBase::Status enum** is registered as a Qt metatype in main.cpp to enable proper QML property handling
- **DeviceFlow::m_run and m_cancel** are marked `mutable` to allow returning pointers from const getter methods
- **Signal name format** for `waitSignal()`: accepts both `"signalName()"` and `"signalName"` (trailing parentheses optional)
- **Include order** in deviceflow.h: trigger.h must be included before state.h due to dependencies

### Wait Mechanism Details

The `wait()` function in Sequence.qml uses JavaScript's `arguments.length` to determine the call mode:
- 1 argument: Condition mode (boolean expression)
- 2 arguments: Signal mode (object + signal name)

The C++ `waitSignal()` implementation:
1. Searches through the entire QMetaObject hierarchy (from index 0, not methodOffset)
2. Uses index-based `QMetaObject::connect()` instead of string-based connection
3. This ensures compatibility with QML-generated signals and dynamic property signals

### Example Usage

```qml
import FlowGraph 1.0

Sequence {
    function myFlow() {
        // Wait for a condition
        wait(function() { return someProperty > 10 })

        // Wait for a signal
        wait(someObject, "propertyChanged()")

        // Continue with next step
        doSomething()
    }
}
```
