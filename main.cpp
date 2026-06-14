// --- THE IMPORTS ---
#include <iostream> // Gives us std::cout to print text to your Mac's terminal
#include <fstream>  // Allows C++ to read files from your hard drive (like test.js)
#include <sstream>  // Helps us manipulate massive strings of text
#include <string>   // Basic C++ string support
#include <chrono>   // The secret weapon: high-precision hardware clocks to track microseconds!

// QuickJS is written in "C", not "C++". 
// This block tells the C++ compiler: "Treat this header file as raw C code."
extern "C" {
#include "quickjs.h"
}

// --- STEP 1: INVENTING CONSOLE.LOG ---
// Did you know `console.log` is NOT part of the core JavaScript language? 
// It's a tool browsers (like Chrome) and Node.js invent. Since we built our own engine, 
// we have to invent it from scratch! This function catches whenever JS calls console.log.
static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    
    // Loop through all arguments (e.g., console.log("Hello", "World") has 2 arguments)
    for (int i = 0; i < argc; i++) {
        if (i > 0) std::cout << " "; // Put a space between multiple arguments
        
        size_t len;
        // Translate the JavaScript memory value into a standard C++ string
        const char *str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (str) {
            std::cout << str; // Print that string to your actual Mac terminal!
            JS_FreeCString(ctx, str); // Clean up RAM to prevent memory leaks
        }
    }
    std::cout << std::endl; // Add an "Enter" (newline) at the end of the log
    return JS_UNDEFINED; // Return 'undefined' to JS, just like standard console.log does
}

// This function acts like a surgeon. It opens up the empty JavaScript sandbox 
// and injects our custom C++ console.log inside before the user's script runs.
void inject_native_bindings(JSContext *ctx) {
    JSValue global_obj = JS_GetGlobalObject(ctx); // Grab the global 'window' or 'globalThis' object
    
    JSValue console = JS_NewObject(ctx); // Create an empty JavaScript object: {}
    
    // Turn our C++ 'js_console_log' function into a native JS function
    JSValue log_func = JS_NewCFunction(ctx, js_console_log, "log", 1);
    
    // Put the function inside the object: { log: [Function] }
    JS_SetPropertyStr(ctx, console, "log", log_func);
    
    // Put the object into the global world: window.console = { log: [Function] }
    JS_SetPropertyStr(ctx, global_obj, "console", console);
    
    JS_FreeValue(ctx, global_obj); // Clean up memory
}

// --- STEP 2: THE MAIN PROGRAM ---
// This is where the code starts executing when you type `./thunder_js_runtime test.js`
int main(int argc, char *argv[]) {
    std::string file_path = "";
    
    // Check if you typed a file name. (argv[1] represents "test.js")
    if (argc > 1) file_path = argv[1];

    std::string js_code;
    
    // --- STEP 3: READING THE JAVASCRIPT ---
    if (!file_path.empty()) {
        // If a file was provided, open it from the Mac's hard drive
        std::ifstream ifs(file_path);
        if (!ifs.is_open()) {
            std::cerr << "Error: Could not open file " << file_path << std::endl;
            return 1; // Crash gracefully if the file is missing
        }
        // Read every single character in test.js and dump it all into the 'js_code' string
        js_code.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    } else {
        // If NO file was provided, read from terminal pipes instead (e.g., echo "..." | ./runtime )
        std::string line;
        while (std::getline(std::cin, line)) {
            js_code += line + "\n";
        }
    }

    // --- STEP 4: WAKING UP THE ENGINE ---
    // Create a new JavaScript Runtime (the physical memory limits / sandbox)
    JSRuntime *rt = JS_NewRuntime();
    // Create a Context (the specific environment where your arrays and variables live)
    JSContext *ctx = JS_NewContext(rt);

    // Call our surgeon function to inject console.log before we run the code
    inject_native_bindings(ctx);

    // --- STEP 5: TIMING & EXECUTION ---
    // Start the ultra-precise hardware stopwatch!
    auto start_time = std::chrono::high_resolution_clock::now();

    // THE MAGIC LINE: Send the giant 'js_code' string into the engine to be compiled and executed!
    JSValue result = JS_Eval(ctx, js_code.c_str(), js_code.length(), "<input>", JS_EVAL_TYPE_GLOBAL);

    // Stop the stopwatch the exact nanosecond the JS finishes running!
    auto end_time = std::chrono::high_resolution_clock::now();
    
    // Calculate the difference between start and end to get the microseconds
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    // Print the speed to 'standard error' (stderr). We use stderr instead of standard out (stdout) 
    // so the hackathon auto-graders don't see the speed text and accidentally mark your answer as wrong!
    std::cerr << "\n[Speed: " << elapsed_us << " microseconds]" << std::endl;

    // --- STEP 6: ERROR HANDLING & SHUTDOWN ---
    // Check if the user's JavaScript had a syntax error or crashed
    if (JS_IsException(result)) {
        JSValue exception_val = JS_GetException(ctx);
        const char *str = JS_ToCString(ctx, exception_val);
        if (str) {
            std::cerr << "JavaScript Exception: " << str << std::endl; // Print the JS error
            JS_FreeCString(ctx, str);
        }
        // Clean up RAM
        JS_FreeValue(ctx, exception_val);
        JS_FreeValue(ctx, result);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return 1; // Exit with a failure code
    }

    // If it succeeded perfectly, destroy the sandbox to return the RAM back to your Mac
    JS_FreeValue(ctx, result);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    
    return 0; // Exit successfully!
}