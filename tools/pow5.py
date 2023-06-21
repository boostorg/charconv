# // Copyright 2020-2023 Daniel Lemire
# // Copyright 2023 Matt Borland
# // Distributed under the Boost Software License, Version 1.0.
# // https://www.boost.org/LICENSE_1_0.txt

def format_hex(number):
    upper = number // (1 << 128)
    lower = number % (1 << 128)
    big_upper = upper // (1 << 64)
    big_lower = upper % (1 << 64)
    little_upper = lower // (1 << 64)
    little_lower = lower % (1 << 64)
    print("{"+hex(big_upper)+","+hex(big_lower)+"},{"+hex(little_upper)+","+hex(little_lower)+"},")


for q in range(-4951, 0):
    power5 = 5 ** -q
    z = 0
    while (1 << z) < power5:
        z += 1
    if q >= -27:
        b = z + 255
        c = 2 ** b // power5 + 1
        format_hex(c)
    else:
        b = 2 * z + 2 * 128
        c = 2 ** b // power5 + 1
        # truncate
        while c >= (1 << 256):
            c //= 2
        format_hex(c)


for q in range(0, 4931 + 1):
    power5 = 5 ** q
    # move the most significant bit in position
    while power5 < (1 << 255):
        power5 *= 2
    # *truncate*
    while power5 >= (1 << 256):
        power5 //= 2
    format_hex(power5)
