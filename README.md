# Open Badge Project

This is the code that drives the badge designs found in the [badge-hardware](https://github.com/OpenBadgeProject/badge-hardware) repo.

## How to program the chip

In order to program the ATTINY85 you will need a programmer. There is a lovely page that explains how to use an arduino as your [programmer](https://www.instructables.com/How-to-Program-an-Attiny85-From-an-Arduino-Uno/)

The reset pin on this chip is special. We are purposely not using that pin (so we really only have 5 IO pins now) because if you use the reset pin, you now require a high voltage programmer. Since we want people to program this thing using only an arduino, we'll keep the reset pin free.

## How to develop for the badge
See [DEVELOPING.md](DEVELOPING.md)
