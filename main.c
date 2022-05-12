#include <tga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#include "functions.h"

typedef enum mode {
    SNR,
    MSE
} mode;

typedef struct{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} optimizedParameters;

double estimator(size_t height, size_t width, const pixel picture[restrict static height][width], uint16_t red, uint16_t green, uint16_t blue, mode optiMode) {
    pixel* temp =  scalar_quantization(height, width, picture, red, green, blue);
    if (temp == NULL) {
        return DBL_MAX;
    }
    double redE = mean_squere_error(height, width, temp, picture, cast_red);
    double greenE = mean_squere_error(height, width, temp, picture, cast_green);
    double blueE = mean_squere_error(height, width, temp, picture, cast_blue);

    if (optiMode == SNR) {
        redE = signal_to_noise_ratio(height, width, picture, cast_red, red);
        greenE = signal_to_noise_ratio(height, width, picture, cast_green, green);
        blueE = signal_to_noise_ratio(height, width, picture, cast_blue, blue);
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
            for (k = 0; k <= 8 && i + j + k <= bitsPerPixel; k++) {
                double temp = estimator(height, width, picture, i, j, k, optiMode);
                if (estimated >= temp) {
                    estimated = temp;
                    red = i;
                    green = j;
                    blue = k;
                }
            }
        }
    }
    optimizedParameters result = {red, green, blue};
    return result;
}

int main(int argc, char** argv) {

    TGA *tga;
    TGAData data;
    size_t width = 0;
    size_t height = 0;
    
    mode optimizationMode; 

    //Error handling
    if(argc < 4) {
        perror("Four parameters are required: input name, output name, bits per pixel, mode specifier\n");
        return EXIT_FAILURE;
    }

    int bitsPerPixel = strtol(argv[3], NULL, 10);
    if (errno != 0) {
        perror("Conversion error: 3rd argument must be a number between 0 and 24\n");
        return EXIT_FAILURE;
    }
    else if (bitsPerPixel > 24 || bitsPerPixel < 0) {
        perror("Conversion error: 3rd argument must be a number between 0 and 24\n");
        return EXIT_FAILURE;
    }
    if (strcmp(argv[4], "SNR") == 0) {
        optimizationMode = SNR;
    }
    else if (strcmp(argv[4], "MSE") == 0) {
        optimizationMode = MSE;
    }
    else {
        perror("Unknown specifier, The only viable specifiers are: SNR, MSE\n");
        return EXIT_FAILURE;
    }
    //End of error handling


    //Otwieramy plik tga
    tga = TGAOpen(argv[1], "r");
    if (!tga || tga->last != TGA_OK) {
        perror("opening error\n");
        return EXIT_FAILURE;
    }
    
    //Wczytujemy dane
    data.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_RGB;
	if(TGAReadImage(tga, & data) != TGA_OK) {
	    perror("parsing error\n");
        return EXIT_FAILURE;
	}
    
    width = tga->hdr.width;
    height = tga->hdr.height;
    
    if (tga->hdr.depth != 24 && tga->hdr.map_t != 0 && tga->hdr.alpha > 0) {
        perror("The image does not meets requierd conditions:\n"
                "- there must not be a color map\n"
                "- alpha must be 0 bytes long\n"
                "- depth must be 24 bits (8 per color)\n");
                return EXIT_FAILURE;
    }
    
    //Generujemy pixele z danych tga
    pixel array[height][width];

    size_t counter = 0;

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            array[i][j].red = (uint8_t) data.img_data[counter++];
            array[i][j].green = (uint8_t) data.img_data[counter++];
            array[i][j].blue = (uint8_t) data.img_data[counter++];
        }
    }

    //Otwieramy plik wyjscia
    TGA* output = TGAOpen(argv[2], "w");
    if (!output || output->last != TGA_OK) {
        TGAClose(tga);
        perror("opening error\n");
        return EXIT_FAILURE;
    }

    //Kopiujemy header dla outputu
    memcpy(&(output->hdr), &(tga->hdr), sizeof(tga->hdr));   
    
    counter = 0;

    optimizedParameters parameters = optimize(height, width, array, bitsPerPixel, optimizationMode);

    pixel* result = scalar_quantization(height, width, array, parameters.red, parameters.green, parameters.blue);
    
    if (result == NULL) {
        TGAClose(tga);
        perror("Quantization error\n");
        return EXIT_FAILURE;
    }

    printf("Scalar quantization with R = %d G = %d B = %d bits used per pixel\n",
            (int) parameters.red, (int) parameters.green, (int) parameters.blue);

    double mse[4];
    mse[0] = mean_squere_error(height, width, result, array, cast_pixel);
    mse[1] = mean_squere_error(height, width, result, array, cast_red);
    mse[2] = mean_squere_error(height, width, result, array, cast_green);
    mse[3] = mean_squere_error(height, width, result, array, cast_blue);

    printf("mse = %lf \nmse(r) = %lf \nmse(g) = %lf\nmse(b) = %lf\n", mse[0], mse[1], mse[2], mse[3]);

    double stn[4];
    stn[0] = signal_to_noise_ratio(height, width, array, cast_pixel, mse[0]);
    stn[1] = signal_to_noise_ratio(height, width, array, cast_red, mse[1]);
    stn[2] = signal_to_noise_ratio(height, width, array, cast_green, mse[2]);
    stn[3] = signal_to_noise_ratio(height, width, array, cast_blue, mse[3]);

    printf("SNR = %lf \nSNR(r) = %lf \nSNR(g) = %lf\nSNR(b) = %lf\n", stn[0], stn[1], stn[2], stn[3]);

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            data.img_data[counter++] = result[i * width + j].red;
            data.img_data[counter++] = result[i * width + j].green;
            data.img_data[counter++] = result[i * width + j].blue;
        }
    }
    TGAWriteImage(output, &data);
    free(result);
    free(data.img_data);
    free(data.cmap);
    free(data.img_id);
    TGAClose(tga);
    TGAClose(output);
}
