#include <tgmath.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

uint32_t cast_red(pixel p) {
    return p.red;
}

uint32_t cast_green(pixel p) {
    return p.green;
}

uint32_t cast_blue(pixel p) {
    return p.blue;
}

uint32_t cast_pixel(pixel p) {
    return (p.red << 16) + (p.green << 8) + (p.blue);
}

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

double mean_squere_error(size_t height, size_t width, pixel encoded[static restrict height][width],
                        pixel original[static restrict height][width], uint32_t (*cast)(pixel)) {
    double result = 0;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            double temp = 1.0 * cast(encoded[i][j]) - cast(original[i][j]);
            result += temp * temp;
        }
    }
    return result / (width * height);
}

double signal_to_noise_ratio(size_t height, size_t width, pixel original[static restrict height][width],
                            uint32_t (cast)(pixel), double mse) {

    double result = 0;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            double temp = cast(original[i][j]);
            result += temp * temp;
        }
    }
    result /= height * width;
    return result / mse;
}
 

