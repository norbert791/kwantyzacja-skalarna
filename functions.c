#include <tgmath.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
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
 
double estimator(size_t height, size_t width, const pixel picture[restrict static height][width], uint16_t red, uint16_t green, uint16_t blue, mode optiMode) {
    pixel* temp =  scalar_quantization(height, width, picture, red, green, blue);
    if (temp == NULL) {
        return DBL_MAX;
    }
    double redE = mean_squere_error(height, width, temp, picture, cast_red);
    double greenE = mean_squere_error(height, width, temp, picture, cast_green);
    double blueE = mean_squere_error(height, width, temp, picture, cast_blue);

    if (optiMode == SNR) {
        redE = signal_to_noise_ratio(height, width, picture, cast_red, redE);
        greenE = signal_to_noise_ratio(height, width, picture, cast_green, greenE);
        blueE = signal_to_noise_ratio(height, width, picture, cast_blue, blueE);
    }

    free(temp);
    return redE + greenE + blueE;
}

optimizedParameters optimize(size_t height, size_t width, const pixel picture[restrict static height][width], int bitsPerPixel, mode optiMode) {
    uint8_t red = 9;
    uint8_t green = 9;
    uint8_t blue = 9;

    double estimated = DBL_MAX;

    size_t i,j,k;
    for (i = 0; i <= 8 && i <= bitsPerPixel; i++) {
        for (j = 0; j <= 8 && i + j <= bitsPerPixel; j++) {
            k = bitsPerPixel - i - j;
            double temp = estimator(height, width, picture, i, j, k, optiMode);
            if (estimated >= temp) {
                estimated = temp;
                red = i;
                green = j;
                blue = k;
            }    
        }
    }
    optimizedParameters result = {red, green, blue};
    return result;   
}