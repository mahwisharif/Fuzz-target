# Fuzz-target
Generation of fuzz target by prompting LLM 

## Prompt
Your task is to write a fuzz target for the provided function signature using `LLVMFuzzerTestOneInput()` to work with libFuzzer.

The function under test is as follows:

```
unsigned kTest_bug(KTest *bo) {
   unsigned i, res = 0;
   for (i = 0; i < bo->numObjects; i++)
      res += bo->objects[i].numBytes;
   return (bo->numObjects != 3) || (100 / (res - 7) != 3);
}
```
     
Additional Context:

The `kTest_bug` function is located in the kTest.c file.
Its signature is declared in the kTest.h file, which also contains the following struct definitions:

```
typedef struct KTestObject KTestObject;
struct KTestObject {
  char *name;
  unsigned numBytes;
  unsigned char *bytes;
};

typedef struct KTest KTest;
struct KTest {
   unsigned version;
   unsigned numArgs;
   char **args;
   unsigned symArgvs;
   unsigned symArgvLen;
   unsigned numObjects;
   KTestObject *objects;
};
```

Your goal:   

Write a fuzz target in the style of libFuzzer. The fuzz target should:
- Be implemented in the function int `LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)`.
- Convert the input data into a structure or format suitable for calling `kTest_bug`.
- Ensure that the generated fuzz target  builds and compiles correctly with the target code provided.

Further Guidelines:
- Carefully analyse the code to determine the type of vulnerability it may have, and how it can be triggered best 
- Return only the code , any comments should only be included as c-style comments.
- Exclude `extern "C"` from your response.
- The code should be self-contained and compatible with the provided function and struct definitions.

## Fuzz Target Generated by ChatGPT
```

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "KTest.h"


int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < sizeof(unsigned)) {
        // Not enough data to parse numObjects
        return 0;
    }

    // Extract numObjects from the input data
    unsigned numObjects = *(const unsigned *)Data;
    Data += sizeof(unsigned);
    Size -= sizeof(unsigned);

    // Allocate memory for KTest and initialize
    KTest bo;
    bo.version = 0;
    bo.numArgs = 0;
    bo.args = NULL;
    bo.symArgvs = 0;
    bo.symArgvLen = 0;
    bo.numObjects = numObjects;

    bo.objects = (KTestObject *)malloc(numObjects * sizeof(KTestObject));
    if (bo.objects == NULL) {
        return 0; // Memory allocation failed
    }

    // Initialize objects with available data
    for (unsigned i = 0; i < numObjects; i++) {
        if (Size < sizeof(unsigned)) {

            // Not enough data to parse numBytes
            bo.objects[i].name = NULL;
            bo.objects[i].numBytes = 0;
            bo.objects[i].bytes = NULL;
            continue;
        }

        // Extract numBytes from the input data
        unsigned numBytes = *(const unsigned *)Data;
        Data += sizeof(unsigned);
        Size -= sizeof(unsigned);

        bo.objects[i].name = NULL; // Name is unused in kTest_bug
        bo.objects[i].numBytes = numBytes;

        // Allocate and fill bytes array if enough data is available
        if (Size >= numBytes) {
            bo.objects[i].bytes = (unsigned char *)malloc(numBytes);
            if (bo.objects[i].bytes != NULL) {
                memcpy(bo.objects[i].bytes, Data, numBytes);
            }
            Data += numBytes;
            Size -= numBytes;
        } else {
            bo.objects[i].bytes = NULL;
        }
    }

    // Call the function under test
    (void)kTest_bug(&bo);

    // Free allocated memory
    for (unsigned i = 0; i < numObjects; i++) {
        free(bo.objects[i].bytes);
    }
    free(bo.objects);

    return 0;
}
```
## Changes to Fuzz Target 
The fuzz target geneeratd by ChatGPT reported out-of-memory issues when run with libfuzzer. One of the things I tried was to increase the limit of `-rss_limit_mb` to 4096 (which was set to 2048 by default). However, that did not help. The next change was made to fuzz target file to limit the number of objects and bytes per object to avoid memory allocation issues. 

The changes made to fuzz target are below:
```
#define MAX_NUM_OBJECTS 10         /* Limit the number of objects to prevent memory allocation issue (out-of-memory) */
#define MAX_NUM_BYTES_PER_OBJECT 1024 /* Limit the number of bytes per object */
```

Once numObjects is extracted from the input data `Data`, we place a check to see that the numObject is less than the max limit, and if not, we clamp it down.

```
if (numObjects > MAX_NUM_OBJECTS) {
    numObjects = MAX_NUM_OBJECTS;
}
```
Similarly, when numBytes are extracted from input data, they are also capped to max limit

```
if (numBytes > MAX_NUM_BYTES_PER_OBJECT) {
   numBytes = MAX_NUM_BYTES_PER_OBJECT;
}

```
The final file is given in FuzzTarget.c

## Build and Run instructions
### Build: 

Build either using the Makefile included in this repo, or using the following build command: 

```
make
```

OR


```
clang -g -fsanitize=fuzzer -o FuzzTarget FuzzTarget.c KTest.c
```

### Run: 

```
./FuzzTarget
```

## Crash report with time

The following output is genertae when running the fuzz target. It shows a floating-point exception FPE (caused by divide by zero) at line 233 of KTest.c. The input that generated this crash is shown as `0x3,0x0,0x0,0x0,0x7,0x0,0x0,0x0` in little endian format. it shows a numObject value of 3, and numBytes value of 7 for the first object to be responsible for triggering the divide by zero exception. The time it took for running the testfuzzer is shown at the end of the output. The crash input file `crash-58190086f63587680efac1cbd1cf51209367361d` generated by fuzzer is included in the repo. 
 
```
UndefinedBehaviorSanitizer:DEADLYSIGNAL
==1934521==ERROR: UndefinedBehaviorSanitizer: FPE on unknown address 0x0000004db67d (pc 0x0000004db67d bp 0x7ffe904ea050 sp 0x7ffe904ea020 T1934521)
    #0 0x4db67d in kTest_bug /home/ma843/Fuzz-target/KTest.c:233:40
    #1 0x4da188 in LLVMFuzzerTestOneInput /home/ma843/Fuzz-target/FuzzTarget.c:75:11
    #2 0x41fa9f in fuzzer::Fuzzer::ExecuteCallback(unsigned char const*, unsigned long) /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerLoop.cpp:599
    #3 0x421a80 in fuzzer::Fuzzer::RunOne(unsigned char const*, unsigned long, bool, fuzzer::InputInfo*, bool, bool*) /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerLoop.cpp:505
    #4 0x422aee in fuzzer::Fuzzer::MutateAndTestOne() /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerLoop.cpp:746
    #5 0x42543c in fuzzer::Fuzzer::Loop(std::__Fuzzer::vector<fuzzer::SizedFile, fuzzer::fuzzer_allocator<fuzzer::SizedFile> >&) /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerLoop.cpp:883
    #6 0x418964 in fuzzer::FuzzerDriver(int*, char***, int (*)(unsigned char const*, unsigned long)) /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerDriver.cpp:906
    #7 0x408322 in main /local/scratch-2/ma843/llvm-project/compiler-rt/lib/fuzzer/FuzzerMain.cpp:20
    #8 0x7f565bdab082 in __libc_start_main /build/glibc-LcI20x/glibc-2.31/csu/../csu/libc-start.c:308:16
    #9 0x40837d in _start (/auto/homes/ma843/Fuzz-target/FuzzTarget+0x40837d)

UndefinedBehaviorSanitizer can not provide additional info.
SUMMARY: UndefinedBehaviorSanitizer: FPE /home/ma843/Fuzz-target/KTest.c:233:40 in kTest_bug
==1934521==ABORTING
MS: 2 CMP-ChangeBinInt- DE: "\x0b\x00\x00\x00"-; base unit: 8e146c3c4e33449f95a49679795f74f7ae19ecc1
0x3,0x0,0x0,0x0,0x7,0x0,0x0,0x0,
\x03\x00\x00\x00\x07\x00\x00\x00
artifact_prefix='./'; Test unit written to ./crash-58190086f63587680efac1cbd1cf51209367361d
Base64: AwAAAAcAAAA=

real	0m0.610s
user	0m0.274s
sys	0m0.032s
```

















 
