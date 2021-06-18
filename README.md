# thread-per-core-bench

This benchmark shows that using a thread-per-code pinned to the core offers the best performance.

Mainly due to better data locality and thread affinity.  

Requirements:
* pthreads
* benchmark



Some numbers:

```
Running /mnt/c/Development/multi-thread/cmake-build-debug/multi_thread
Run on (12 X 2592.01 MHz CPU s)
CPU Caches:
L1 Data 32 KiB (x6)
L1 Instruction 32 KiB (x6)
L2 Unified 256 KiB (x6)
L3 Unified 12288 KiB (x1)
Load Average: 0.12, 0.12, 0.06
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
bench_singleT/real_time               285 ms          285 ms            2
bench_multiT_false/real_time          596 ms        0.260 ms            1
bench_multiT_padded/real_time        86.6 ms        0.248 ms            8
bench_multiT_subopt/real_time         471 ms        0.307 ms            2
bench_multiT_optimal/real_time        177 ms        0.268 ms            4
bench_multiT_percore/real_time       84.5 ms        0.328 ms            9

Process finished with exit code 0
```
