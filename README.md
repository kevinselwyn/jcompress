#jcompress

Utility for observing the data lost in JPEG compression

##Description

This utility will take plaintext, convert it into a JPEG, compress it, decompress it, and then convert it back into plaintext.

See [In Depth](#in-depth) below for more details.

##Installation

```bash
make && sudo make install
```

##Usage

```bash
Usage: jcompress [--text,-t] <text outfile>
                 [--jpeg,-j] <jpeg outfile>
                 <infile> (<quality, 1-100>)
```

Example:

```bash
jcompress test/sonnet.txt
```

The default quality is 100.

##In Depth

Here's a bit of a breakdown of the process:

The size of the image is always square. If a file is 663 bytes, you take the square root of it and round up (excess bytes will be shaved off at the end):

```
√663 = 25.748786379167466

Width = 26
Height = 26
```

Each byte corresponds to the red, green, and blue value of one pixel. The letter `A` would correspond to `0x65` and a pixel with the color `#656565`. The resulting image will have pixels that range from white to black and all the greys inbetween.

Once our image has been assembled (the extra blank bytes/pixels are `0x00` or `black`), it is compressed with the provided JPEG quality (default 100) using the libjpeg C library.

It is immediately decompressed and rendered back into plaintext. Because of the space-saving, lossy algorithm that JPEG uses, contents may have shifted during compression and/or decompression.

So when we get back to the pixel that should represent our `A`, the color of the pixel could have been changed slightly to be `#666666`. In which case, the resulting plaintext would be `0x66` or a `B`.

It is interesting to note just how quickly plaintext (and images) can get mangled, even at the highest qualities, by JPEG compression and decompression.

#Disclaimer

This is pretty obviously inspired by Tom Scott's [Romeo](http://www.tomscott.com/romeo/) project. Tom imported text files as RAW in Adobe Photoshop, saved as JPEG, reopened, and then saved again as RAW. I had managed to achieve this workflow with some Photoshop scripting, but was ultimately unsatisfied with the slow speed and lack of portability (Hello, Linux users without Photoshop).