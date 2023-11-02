#include <stdio.h>
#include <stdlib.h>

#include "profiler_inline.cpp"

f32 SomeWork() {
    f32 sum = 0.0f;
    for (u32 i = 0; i < 1000000; ++i) {
        f32 random = (f32)rand() / (f32)RAND_MAX;
        sum += random;
    }
    return sum;
}

void TestTwoBlocksLoops() {
    printf("Test Two Blocks Loops:\n");
    srand(1234);

    BeginProfiling();

    for (u32 i = 0; i < 5; ++i) {
        PROFILE_BLOCK("should be 25% total, 25% exclusive");
        SomeWork();
    }

    for (u32 i = 0; i < 15; ++i) {
        PROFILE_BLOCK("should be 75% total, 75% exclusive");
        SomeWork();
    }

    FinishProfiling();

    PrintProfilingResults();

    printf("\n");
}

void TestRecursiveFunc(u32 counter) {
    PROFILE_BLOCK("should be 80% total, 80% exclusive");
    SomeWork();

    if (counter != 0) {
        TestRecursiveFunc(--counter);
    }
}

void TestRecursive() {
    printf("Test Recursive:\n");

    BeginProfiling();
    {
        PROFILE_BLOCK("should be 100% total, 20% exclusive");
        SomeWork();

        TestRecursiveFunc(3);
    }
    FinishProfiling();
    PrintProfilingResults();

    printf("\n");
}

void RecursiveIndirectFuncA(u32 counter);
void RecursiveIndirectFuncB(u32 counter) {
    PROFILE_BLOCK("should be 67% total, 33% exclusive");
    SomeWork();

    if (counter != 0) {
        RecursiveIndirectFuncA(--counter);
    }
}

void RecursiveIndirectFuncA(u32 counter) {
    PROFILE_BLOCK("should be 83% total, 50% exclusive");
    SomeWork();

    if (counter != 0) {
        RecursiveIndirectFuncB(--counter);
    }
}

void TestRecursiveIndirect() {
    printf("Test Recursive Indirect:\n");

    BeginProfiling();
    {
        PROFILE_BLOCK("should be 100% total, 17% exclusive");
        SomeWork();

        RecursiveIndirectFuncA(4);
    }
    FinishProfiling();
    PrintProfilingResults();

    printf("\n");
}

void TestSimpleNested() {
    printf("Test Simple Nested:\n");

    BeginProfiling();
    {
        PROFILE_BLOCK("should be 100% total, 50% exclusive");
        SomeWork();
        {
            PROFILE_BLOCK("should be 50% total, 50% exclusive");
            SomeWork();
        }
    }
    FinishProfiling();
    PrintProfilingResults();

    printf("\n");
}

void TestDoubleNested() {
    printf("Test Double Nested:\n");

    BeginProfiling();
    {
        PROFILE_BLOCK("should be 100% total, 33% exclusive");
        SomeWork();
        {
            PROFILE_BLOCK("should be 66% total, 33% exclusive");
            SomeWork();

            {
                PROFILE_BLOCK("should be 33% total, 33% exclusive");
                SomeWork();
            }
        }
    }
    FinishProfiling();
    PrintProfilingResults();

    printf("\n");
}

void TestComplexNesting() {
    printf("Test Complex Nesting:\n");

    BeginProfiling();
    {
        PROFILE_BLOCK("should be 0% total, 0% exclusive");

        srand(1234);
    }

    for (u32 i = 0; i < 5; ++i) {
        PROFILE_BLOCK("should be 50% total, 50% exclusive");
        f32 sum = SomeWork();
    }

    {
        PROFILE_BLOCK("should be 50% total, 10% exclusive");
        f32 sum = SomeWork();
        {
            {
                PROFILE_BLOCK("should be 20% total, 10% exclusive");
                f32 sum = SomeWork();
                {
                    PROFILE_BLOCK(
                        "should be 10% total, 10% exclusive");
                    f32 sum = SomeWork();
                }
            }

            {
                PROFILE_BLOCK(
                    "should be 20% total, 10% exclusive");
                sum = SomeWork();
                {
                    PROFILE_BLOCK(
                        "should be 10% total, 10% exclusive");
                    f32 sum = SomeWork();
                }
            }
        }
    }
    FinishProfiling();
    PrintProfilingResults();

    printf("\n");
}

int main(int argc, char** argv) {
    TestSimpleNested();
    TestTwoBlocksLoops();
    TestDoubleNested();
    TestComplexNesting();
    TestRecursive();
    TestRecursiveIndirect();

    return 0;
}
