#include <tgmath.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

pixel* scalar_quantization(size_t height, size_t width, pixel matrix[static restrict height][width], uint16_t redBits, uint16_t greenBits, uint16_t blueBits) {

    //Initial check
    uint16_t check = fmax(redBits, greenBits);
    check = fmax(greenBits, blueBits);

    redBits = (255 >> redBits) + 1;
    greenBits = (255 >> greenBits) + 1;
    blueBits = (255 >> blueBits) + 1;

    if (check > 8) {
        return NULL;
    }

    pixel* result = malloc(sizeof(pixel*) * width * height);
    if (result == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            //Note: No overflow will occur here: matrix holds values <256 and 16-bit *Bits hold values < 256;
            result[i * width + j].red = ((matrix[i][j].red / redBits) * redBits + redBits / 2);
            result[i * width + j].green = ((matrix[i][j].green / greenBits) * greenBits + greenBits / 2);
            result[i * width + j].blue = ((matrix[i][j].blue / blueBits) * blueBits + blueBits / 2);
        }
    }
    return result;
}
