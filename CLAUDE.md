# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DeviceFlow is a Qt5/QML-based state machine framework for building visual programming flows with asynchronous state management. The project enables declarative state definitions in QML with C++ backend support for event loop integration and state transitions.

## Build System

This is a Qt5 qmake-based project. Use these commands:

**Build:**
```bash
qmake DeviceFlow.pro
make
```

**Run:**
```bash
./DeviceFlow
```

**Clean:**
```bash
make clean
```

Note: The project uses Qt 5.15. Build artifacts are placed in the `build/` directory.

## Architecture

### Core Components

**DeviceFlow** (deviceflow.h/cpp): Central state machine controller
- Manages current and initial states via `StateBase*` properties
- Provides `run` and `cancel` triggers for flow control
- Handles state transitions via `setState()` which calls `_exit()` on old state and `_enter()` on new state
- Maintains bidirectional state linking through `prevState`

**StateBase** (state.h/cpp): Base class for all states
- Implements blocking wait mechanism using `QEventLoop` for condition polling
- Status enum: IDLE → RUNNING → FINISHED/CANCELLED/ERROR
- Key methods:
  - `beginWait()`: Blocks until `condition` property becomes true
  - `waitSignal()`: Template method to block until a Qt signal fires
  - `_enter()` / `_exit()`: Lifecycle hooks (emit `enter`/`exit` signals)
- The `condition` property is designed to work with QML bindings for reactive waiting

**State.qml**: QML component extending StateBase
- Provides `wait(conditionFunc)` helper that:
  1. Evaluates condition immediately
  2. Creates a Qt.binding to continuously update the C++ `condition` property
  3. Calls `beginWait()` to block JS execution while QML event loop continues
  4. Cleans up binding when unblocked
- This pattern allows writing synchronous-looking async code in QML

**Trigger** (trigger.h): Event source for state transitions
- Exposes `fire()` method (only succeeds if `enabled`)
- Emits `fired()` signal when activated
- QML_ELEMENT registered for direct QML usage

**Action** (action.h): Currently a stub placeholder class

### QML Integration

The project uses Qt Resource System (qml.qrc) to embed QML files. Main entry point is main.qml with a basic Window. State logic is intended to be composed using State.qml components.

### Key Design Patterns

1. **Hybrid blocking model**: C++ uses `QEventLoop` to block execution while keeping Qt's event loop running, enabling bindings/signals to continue processing
2. **Property-driven conditions**: States wait on boolean conditions that QML can bind to any expression
3. **Signal-based transitions**: DeviceFlow connects triggers to state entry points
4. **Lifecycle management**: States track their status and previous state for complex flow control

## Development Notes

- DeviceFlow.pro line 29 has a likely bug: `m_initialState()` should be `m_initialState` (it's a pointer, not a function)
- DeviceFlow.cpp line 35 has an incomplete implementation: `connect(&m_run, &Trigger::fired, m_initialState->_enter())` - the third argument should be a slot/lambda, not a function call
- The Status enum in state.cpp references `Status` but should reference `StateBase::Status` for proper scoping
- Trigger class has an `enabled` property declared but `m_enabled` member is never defined or initialized
