



## Notes

C has some kind of esoteric type features, such as `uint_least16_t` which would be the smallest type that can represent a 16-bit unsigned integer.
the same goes for signed integers.

In the same vein, C also has `uint_fast_16_t` which is the fasted type that can represent a 16-bit unsigned integer, but what does fast mean? well,
sometimes choosing the exact-width type it can give performance penalties, as some architectures and assembly operations might be optimized to work
better with certain register sizes than others. So sometimes when caring about speed and not precise storage layout, it can be beneficial to store
e.g. a 16 bit integer as a 32 bit one, depending on the architecture of course.