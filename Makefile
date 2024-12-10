CC = clang
CFLAGS= -g -fsanitize=fuzzer

fuzz_harness: 
	$(CC) $(CFLAGS)  -o FuzzTarget FuzzTarget.c KTest.c
clean:
	rm -f FuzzTarget

