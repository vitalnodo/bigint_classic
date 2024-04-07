# bigint classic
An attempt to implement classic bigint library.
Currently supported operations:
* set hex
* get hex
* bitwise not, xor, or, and
* bitwise shift left, shift right
* addition
* subtraction
* multiplication (long and karatsuba)
* long division
* comparison
* montgomery reduce and multiplication

There are also fun colored tests if any operation is poorly implemented.

Planned:
* montgomery exponentiation
* [Montgomery reduction with even modulus](https://cetinkayakoc.net/docs/j34.pdf)
