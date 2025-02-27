# test tcmalloc

This repository is the minimal reproducer of a deadlock which happens at the [Async profiler startup](https://github.com/async-profiler) when using [tcmalloc](https://github.com/gperftools/gperftools) on `x86_64` systems due to its internal use of `libunwind`.
See the https://github.com/async-profiler/async-profiler/issues/1151 for more details.

This reproducer has the same behaviour regarding the locks:

- 1 thread performs `malloc` (this is any Java thread, for instance the bytecode compilation)
- 1 thread performs `dl_iterate_phdr` and then `malloc` (doing the same as the async profiler thread)

The workarounds of this issue:

- use `libtcmalloc_minimal`, without the memory profiler, instead of `libtcmalloc`
- use another `TCMALLOC_STACKTRACE_METHOD` value than the default `libunwind` (`libgcc` or `generic_fp` both seem to work)

## Run the reproducer

The reproducer runs 3 tests:

- 1 test with the default system allocator (no `LD_PRELOAD`)
- 1 test with `LD_PRELOAD=libtcmalloc_minimal.so`, the test should succeed
- 1 test with `LD_PRELOAD=libtcmalloc.so`, the test with the default stacktrace method `libunwind` should fail, and succeed when using alternative methods

### Ubuntu

To run on `x86_64`:

```bash
apt install -y build-essential libtcmalloc-minimal4 libgoogle-perftools4

./run.sh

# Workaround with libtcmalloc
./run.sh --stacktrace libgcc
```

### Docker

This will build an `x86_64` docker image and run the test:

```bash
./run_docker.sh

# Workaround with libtcmalloc
./run_docker.sh --stacktrace libgcc
```

## Examples of outputs

### Stuck output using `libunwind` in tcmalloc

The last run with `LD_PRELOAD=libtcmalloc.so` never returns

```bash
$ ./run_docker.sh
+ g++ -std=c++11 -Wall -Wextra -pedantic -g -o /tmp/test-tcmalloc main.cpp
+ echo ## Run without LD_PRELOAD
## Run without LD_PRELOAD
+ /tmp/test-tcmalloc 314572800
Allocating in each thread 314572800 bytes = 300MB by chunks of 30B
Starting: thread
Starting: dl_iterate_phdr
Done: dl_iterate_phdr
Done: thread
## Run with LD_PRELOAD=libtcmalloc_minimal.so
+ echo ## Run with LD_PRELOAD=libtcmalloc_minimal.so
+ LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 /tmp/test-tcmalloc 314572800
Allocating in each thread 314572800 bytes = 300MB by chunks of 30B
Starting: dl_iterate_phdr
Starting: thread
Done: thread
Done: dl_iterate_phdr
## Run with LD_PRELOAD=libtcmalloc.so
+ echo ## Run with LD_PRELOAD=libtcmalloc.so
+ LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc.so.4 /tmp/test-tcmalloc 314572800
Chosen stacktrace method is libunwind
Supported methods:
* libgcc
* generic_fp
* generic_fp_unsafe
* libunwind
* x86
```

### Workaround using `libgcc` in tcmalloc

```bash
$ ./run_docker.sh --stacktrace libgcc
+ g++ -std=c++11 -Wall -Wextra -pedantic -g -o /tmp/test-tcmalloc main.cpp
+ echo ## Run without LD_PRELOAD
+ /tmp/test-tcmalloc 314572800
## Run without LD_PRELOAD
Allocating in each thread 314572800 bytes = 300MB by chunks of 30B
Starting: thread
Starting: dl_iterate_phdr
Done: dl_iterate_phdr
Done: thread
+ echo ## Run with LD_PRELOAD=libtcmalloc_minimal.so
## Run with LD_PRELOAD=libtcmalloc_minimal.so
+ LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 /tmp/test-tcmalloc 314572800
Allocating in each thread 314572800 bytes = 300MB by chunks of 30B
Starting: dl_iterate_phdr
Starting: thread
Done: thread
Done: dl_iterate_phdr
## Run with LD_PRELOAD=libtcmalloc.so
+ echo ## Run with LD_PRELOAD=libtcmalloc.so
+ LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc.so.4 /tmp/test-tcmalloc 314572800
Chosen stacktrace method is libgcc
Supported methods:
* libgcc
* generic_fp
* generic_fp_unsafe
* libunwind
* x86

Allocating in each thread 314572800 bytes = 300MB by chunks of 30B
Starting: dl_iterate_phdr
Starting: thread
Done: thread
Done: dl_iterate_phdr
```
