## Inverse Square Root Test

A basic and not entirely apples-to-apples benchmark because I was curious about how a modern C compiler would handle both the Quake 3's fast inverse square root approximation and the plain inverse square root (**1.0f / sqrtf(x)**) as well as how would hand written Assembly code compare to either of them.

It's better to use higher iteration counts (over 1 million) for more reliable results - but on modern CPUs they may differ from run to run.

### Usage
```
isrt ITERATIONS SEED
```

It will generate a seed on its own if no arguments or only the iteration count are provided
