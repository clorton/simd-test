# CONSOLE APPLICATION : simdTest Project Overview

Compare iteration over individual agent objects with SIMD operation on the relevant data.

|Sample output from Intel Xeon E5-2630 @ 2.3GHz|*"brute force"*|*SIMD*|speedup|
|---|:-:|:-:|:-:|
|Total infectivity|182821.078125|182837.437500|n/a|
| Attenuate (ms)|252.352506|3.145453|80x|
| Sum (ms)|12.075272|0.220914|55x|

This is working on 2<sup>20</sup> agents/values.

The attenuate step (multiply by 0.9 ten times) is ~80x faster with SIMD.
The sum step is 50x-60x faster with SIMD (more variability in timing with the shorter duration).

The results differ because the SIMD operation uses 8 parallel accumulators which don't lose precision as quickly. The SIMD result is, technically, more correct.
