# ThunderJS ⚡

A high-performance, native C++ runtime engine wrapper designed to execute JavaScript payloads with near-zero latency. Built for Thunder Hackathon 2.0.

## 🚀 The Innovation
Instead of writing a slow interpreter from scratch in a high-level language like Python or Java, ThunderJS embeds the ultra-lightweight **QuickJS** engine directly into a compiled C++ executable. 

* **Native C++ Performance:** Executes modern ES2020 JavaScript in roughly 600 microseconds, destroying standard interpreter speeds.
* **Hardware-Clock Profiling:** Injects sub-millisecond precision hardware clock tracking into the runtime, streaming execution speeds to standard error (`stderr`) to preserve exact `stdout` string matching for automated graders.
* **Modern Syntax Support:** Natively supports ES6+ features like the spread operator (`...arr`), higher-order array methods (`.map()`, `.reduce()`), and the `Math` object out of the box.

## 🛠️ Build Instructions (macOS / Linux)
To compile this project from source, you need a standard C/C++ build environment (`gcc` / `make`).

**1. Clone the official QuickJS repository to build the core engine static library:**
```bash
git clone https://github.com/bellard/quickjs.git
cd quickjs
make
cd ..
```

**2. Compile the C++ wrapper and link the engine:**
```bash
g++ -O3 main.cpp -I./quickjs -L./quickjs -lquickjs -o thunder_js_runtime
```

## ⚡ Execution Instructions
Pass JavaScript code directly via standard input (stdin) or by passing a file path.

**Using Standard Input:**
```bash
echo 'let arr = [1, 2, 3]; console.log("Reversed: " + [...arr].reverse().join(", "));' | ./thunder_js_runtime
```

**Using a File:**
```bash
./thunder_js_runtime test.js
```