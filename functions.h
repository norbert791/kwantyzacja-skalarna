#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <inttypes.h>

typedef enum mode {
    SNR,
    MSE
} mode;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel, optimizedParameters;

pixel* scalar_quantization(size_t height, size_t width,
                            pixel matrix[static restrict height][width], uint16_t redBits,
                            uint16_t greenBits, uint16_t blueBits);

double mean_squere_error(size_t height, size_t width, pixel encoded[static restrict height][width],
                            pixel original[static restrict height][width],  uint32_t (*cast)(pixel));

double signal_to_noise_ratio(size_t height, size_t width, pixel original[static restrict height][width],
                            uint32_t (*cast)(pixel), double mse);

uint32_t cast_red(pixel p);
uint32_t cast_green(pixel p);
uint32_t cast_blue(pixel p);
uint32_t cast_pixel(pixel p);

double estimator(size_t height, size_t width, const pixel picture[restrict static height][width], uint16_t red, uint16_t green, uint16_t blue, mode optiMode);
optimizedParameters optimize(size_t height, size_t width, const pixel picture[restrict static height][width], int bitsPerPixel, mode optiMode);

#endif
