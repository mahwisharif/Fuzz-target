#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "KTest.h"
#define MAX_NUM_OBJECTS 10         /* Limit the number of objects to prevent excessive allocation */
#define MAX_NUM_BYTES_PER_OBJECT 1024 /* Limit the number of bytes per object */


int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < sizeof(unsigned)) {
        // Not enough data to parse numObjects
        return 0;
    }

    // Extract numObjects from the input data
    unsigned numObjects = *(const unsigned *)Data;
    Data += sizeof(unsigned);
    Size -= sizeof(unsigned);
    /* Clamp numObjects to a safe limit */
    if (numObjects > MAX_NUM_OBJECTS) {
        numObjects = MAX_NUM_OBJECTS;
    }

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
                /* Clamp numBytes to a safe limit */
        if (numBytes > MAX_NUM_BYTES_PER_OBJECT) {
            numBytes = MAX_NUM_BYTES_PER_OBJECT;
        }

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

