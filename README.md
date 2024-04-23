# bigint classic

An attempt to implement classic bigint library.
## Features
* get, set hex
* get, set limb
* bitwise not, xor, or, and
* bitwise shift left, shift right
* addition
* subtraction
* multiplication (long and karatsuba)
* long division
* comparison
* montgomery reduce and multiplication
* randomized tests in python with ctypes and legacy fun colored specific in main.c (not enabled by default)
* support uint64_t, uint32_t, uint16_t, uint8_t as limbs

## Instruction
To test that everything is ok
```bash
./build.sh && python test.py
```

## Planned:
* montgomery exponentiation
* [Montgomery reduction with even modulus](https://cetinkayakoc.net/docs/j34.pdf)

## Experience
In some ways, it was quite a challenging project for me, but gdb and patience helped a lot. Of course, there is always room for improvement: you can formally verify, separate the algorithms from the interface, look at optimizations in libdivide, make negative numbers, fast multiplication via Fourier transform, and much more, but it requires even more patience. If you have the courage to go here, you will see that even well-known professors admit that they do not understand the long division algorithm...

If you look at the calendar, I spent a lot of time in general, but on the code itself, I'd estimate that it was somewhere around 30 hours, where most of the time was spent on bug fixing (and maybe there is still more).
