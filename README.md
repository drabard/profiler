# Simple C++ RDTSC intrusive profiler

Allows for quickly getting relative performance metrics for a section of code.

Low overhead and as simple as possible. 

This is a standard implementation of the idea, done many times before. This repository exists mostly so that I have a minimal implementation where I have a full understanding and can easily control how stuff is done. 

## Usage

1. Include `profiler_inline.cpp` in the file you want to profile.
   ```
   #include "profiler_inline.cpp"
   ```
   
2. Place BeginProfiling() and FinishProfiling() statements to delineate the code you want to profile.
    ~~~ C++
    BeginProfiling()
    
    // Profiled code goes here.
    
    FinishProfiling()
    ~~~

3. Within the delineated code section, place PROFILE_BLOCK statements to mark sections you're interested in.
   ~~~ C++
   void MyFunction() {
     PROFILE_BLOCK("my function");
   
     // ...
   
     for(int i = 0; i < 10; ++i) {
       PROFILE_BLOCK("inside the loop");
   
       // ...
   
     }
   }
   ~~~
   
4. After `FinishProfiling`, call `PrintProfilingResults` to print the results to standard output. You'll get something like this:
   ~~~
    ==== Profile results ====
    9.08% total | 9.08% exclusive - my function [hits: 1]
    90.92% total | 90.92% exclusive - inside the loop [hits: 10]
    ==== End of profile results ====
   ~~~
## Capabilities

- Works with recursive functions.

## Limitations

- POSIX only (tested only on Linux, relies on POSIX headers).
- x86/amd64 only (relies on RDTSC instruction being available through `x86intrin.h`).
- Doesn't allow for profiling across compilation units (due to use of `__COUNTER__` preprocessor macro).
