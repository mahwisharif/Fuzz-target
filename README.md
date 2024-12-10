# Fuzz-target
Generation of fuzz target by prompting LLM 
<Prompt>
Your task is to write a fuzz target for the provided function signature using LLVMFuzzerTestOneInput() to work with libFuzzer.

The function under test is as follows:
'''
unsigned kTest_bug(KTest *bo) {
  unsigned i, res = 0;
  for (i = 0; i < bo->numObjects; i++)
      res += bo->objects[i].numBytes;
  return (bo->numObjects != 3) || (100 / (res - 7) != 3);
}
'''
Additional Context:

The kTest_bug function is located in the kTest.c file.
Its signature is declared in the kTest.h file, which also contains the following struct definitions:
'''
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
'''
Your goal: 
Write a fuzz target in the style of libFuzzer. The fuzz target should:

1) Be implemented in the function int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size).
2) Convert the input data into a structure or format suitable for calling kTest_bug.
3) Ensure that the generated fuzz target  builds and compiles correctly with the target code provided.

Further Guidelines:
1)Carefully analyse the code to determine the type of vulnerability it may have, and how it can be triggered best 
2)Return only the code , any comments should only be included as c-style comments.
3)Exclude extern "C" from your response.
4) The code should be self-contained and compatible with the provided function and struct definitions.



 
