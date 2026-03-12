# MART POS - Payment Form (ver. 0.1.0)

Qt 6.5.2 · C++17 · Linux (x86_64)

A cash-payment terminal with a built-in macro recorder/replayer.
Developed as a test assignment.

---

## Building

```bash
# Make sure you are using the Qt6 qmake, not the system Qt5 one
~/Qt/6.5.2/gcc_64/bin/qmake payment_form.pro
make -j$(nproc)
./payment_form
```

---

## Project structure

```
.
├── main.cpp                    Entry point - creates QApplication, loads stylesheet, shows MainWindow
├── payment_form.pro            qmake project file - sources, headers, INCLUDEPATH, LIBS
├── resources.qrc               Qt resource bundle (embeds styles.qss)
├── styles.qss                  Single source of truth for ALL visual styling
├── check.csv                   Example product list - open via "Відкрити чек"
│
├── core/                       Pure business logic - zero Qt widget dependency
│   ├── Product/
│   │   └── Product.h           Plain struct: name, quantity, price, lineTotal()
│   │
│   ├── Cart/
│   │   ├── Cart.h              Holds product list; computes total; validates cash payment
│   │   └── Cart.cpp            validate() returns PaymentInfo{result, cash, change, missing}
│   │                           warningText() returns ready-made Ukrainian error string
│   │                           receiptBody() returns formatted line-item block for the receipt
│   │
│   ├── CartIO/
│   │   ├── CartIO.h            File I/O for product lists
│   │   └── CartIO.cpp          load() - parses UTF-8 CSV, handles quoted names, skips comments
│   │                           save() - writes CSV with comment header
│   │
│   └── macro/
│       ├── Scriptio/
│       │   ├── scriptio.h      InputEvent struct + ScriptIO class
│       │   └── scriptio.cpp    toScript() / fromScript() - text serialisation of events
│       │                       ScriptIO::save() / load() - .mrs file read/write
│       │                       Format: T=<ms> TYPE [X=x Y=y] [BTN=n] [CODE=n SYM=name]
│       │
│       ├── Recorder/
│       │   ├── recorder.h      XRecord-based recorder - runs in a dedicated QThread
│       │   └── recorder.cpp    startRecording() - opens control display, creates XRecord context
│       │                       run() - opens data display in recording thread; calls
│       │                               XRecordEnableContext (blocking); emits eventRecorded()
│       │                       stopRecording() - calls XRecordDisableContext from main thread
│       │                       Records absolute screen coords (x, y) and X11 keycodes
│       │                       Two displays required by XRecord: control (main thread) +
│       │                       data (recording thread) - each Display* must only be used
│       │                       from the thread that opened it
│       │
│       └── Player/
│           ├── player.h        XTest-based replayer - single-threaded, QTimer-driven
│           └── player.cpp      play() - opens X display, verifies XTest extension
│                               onTimer() - fires per event, calls injectEvent()
│                               injectEvent() - XTestFakeMotionEvent / XTestFakeButtonEvent /
│                                               XTestFakeKeyEvent, then XFlush
│                               scheduleNext() - delay = event.timestamp - elapsed - pauseOffset
│                               Supports loop (infinite or N times) and pause/resume
│
└── ui/                         All Qt widget code
    ├── AppStyles/
    │   ├── appstyles.h         AppStyles::global() declaration
    │   └── appstyles.cpp       Loads :/styles.qss from Qt resources and returns it as QString
    │
    ├── MainWindow/
    │   ├── mainwindow.h        Thin shell - owns Cart*, PosPanel*, MacroPanel*, badge QLabel*
    │   └── mainwindow.cpp      buildHeader() - logo, datetime, cashier, macro badge
    │                           buildSplitter() - creates both panels, wires statusChanged signal
    │                           Badge update uses dynamic property + unpolish/polish for QSS rules
    │
    ├── panels/
    │   ├── POSPanel/
    │   │   ├── pospanel.h      Left panel - table, summary box, numpad, pay/cancel buttons
    │   │   └── pospanel.cpp    onLoadClicked() - opens CSV via CartIO, populates table
    │   │                       onClientAmountChanged() - calls Cart::validate() on every keystroke
    │   │                       applyPaymentInfo() - pure switch on ValidationResult, no arithmetic
    │   │                       setPaymentSectionEnabled() - disables whole payment area until file loaded
    │   │                       QDoubleValidator on clientInput - blocks letter input at field level
    │   │
    │   └── MacroPanel/
    │       ├── macropanel.h    Right panel - recorder controls, player controls, log, file I/O
    │       └── macropanel.cpp  State machine: Idle -> Countdown -> Recording -> Idle
    │                                          Idle -> Playing <-> Paused -> Idle
    │                           statusChanged(text, stateKey) signal drives the header badge
    │                           stateKey is a plain string ("recording", "playing" …) matched
    │                           by QSS dynamic-property rules - no hex colours in C++ code
    │
    └── widgets/
        ├── NumpadWidget/
        │   ├── numpadwidget.h   On-screen numeric keypad widget
        │   └── numpadwidget.cpp Internal buffer; mirrors to a target QLineEdit via setTargetField()
        │                        appendChar() enforces: max 10 chars, one dot, 2 decimal places
        │                        valueConfirmed(QString) signal emitted on "Підтвердити"
        │
        └── ReceiptDialog/
            ├── receiptdialog.h   Modal receipt dialog - read-only, no state mutation
            └── receiptdialog.cpp buildReceiptText() is static - pure data-in / string-out
                                  Line items delegated to Cart::receiptBody()
                                  All styling via objectNames matched by styles.qss
```

---

## Key design decisions

### Core / UI separation
`core/` has zero widget includes. The UI only calls `Cart::validate()` and
reads `PaymentInfo` - arithmetic never leaks into widget code.

### Single stylesheet
`styles.qss` is the only place colours and fonts live. Widgets set `objectName`
and the stylesheet does the rest. Dynamic state is driven by a Qt dynamic property:
```cpp
badge->setProperty("macroState", "recording");
badge->style()->unpolish(badge);
badge->style()->polish(badge);
```
matched in QSS as:
```css
QLabel#macroStateBadge[macroState="recording"] { color: #EF4444; }
```

### Macro recorder - XRecord + XTest
The recorder runs in a dedicated `QThread`. Two separate X11 display connections
are required by the XRecord protocol:
- **control display** - opened on the main thread; used to create/disable the context
- **data display** - opened inside `run()`; used by `XRecordEnableContext` (blocking call)

Each `Display*` must only ever be used from the thread that opened it - this is
an X11 rule, not a Qt one.

Playback uses `XTestFakeMotionEvent`, `XTestFakeButtonEvent`, and
`XTestFakeKeyEvent` via a single-threaded `QTimer`-driven loop. Each event fires
at its recorded timestamp: `delay = event.timestamp - elapsed - pauseOffset`.

### CSV format
```
# comment lines are ignored
Молоко 2.5% 1л,2,42.50
"Хліб, Бородинський",1,28.00   ← quoted names support commas
Яблука Гала кг,1.5,58.00       ← fractional quantities supported
```

---

## Include path convention
`INCLUDEPATH` in `.pro` points to the *parent* of each class folder:
```
$$PWD/core          ->  #include <Cart/Cart.h>
$$PWD/core/macro    ->  #include <Recorder/recorder.h>
$$PWD/ui/panels     ->  #include <MacroPanel/macropanel.h>
$$PWD/ui/widgets    ->  #include <NumpadWidget/numpadwidget.h>
```
No `../` traversal anywhere in the codebase.

---

## Dependencies

| Library          | Used for                                         |
|------------------|--------------------------------------------------|
| Qt 6.5.2 Widgets | All UI                                           |
| Qt 6.5.2 Core    | QTimer, QElapsedTimer, QFileDialog, QThread etc. |
| libX11           | XOpenDisplay, XRecord setup                      |
| libXtst          | XRecord (capture) + XTest (playback injection)   |

Link flags in `.pro`: `-lX11 -lXtst -lXext`

---

## Known bugs

| Bug              | Description                                                                                                                                                                                                                                                                                                                                                                                                                  |
|------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Relative recording | XRecord captures raw **screen coordinates**. If the window moves, is resized, or the layout shifts between recording and replay (e.g. a warning label appears and pushes buttons down), injected clicks land on the wrong widget. Could be fixed by storing widget identity (objectName path + local offset) instead of absolute coords, or by ensuring the window is in an identical state and position before replaying. |
| Pop-up recording | When a file dialog (pop-up) is opened during recording, it relies on the system's 'last opened directory' state. During replay, if this underlying system state has changed, the dialog will open in a completely different directory than it did during the original recording.                                                                                                                                             |                                                                                                                                                                                                                                                                                                                                                                                                                            