#include <tga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

/**
 * @brief Used for updating best solution
 * 
 */
#define compareAsign(A, B, C, D) if ((A) < (B)) {B = A; D = C;}

int main(int argc, char** argv) {

    TGA *tga;
    TGAData data;
    size_t width = 0;
    size_t height = 0;
    
    if(argc < 2) {
        perror("Another parameter requierd: pls input filename as programm parameter\n");
        return EXIT_FAILURE;
    }

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
    
    pixel array[height][width];

    size_t counter = 0;

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            array[i][j].red = (uint8_t) data.img_data[counter++];
            array[i][j].green = (uint8_t) data.img_data[counter++];
            array[i][j].blue = (uint8_t) data.img_data[counter++];
        }
    }

    TGA* output = TGAOpen("dump.tga", "w");
    if (!output || output->last != TGA_OK) {
        TGAClose(tga);
        perror("opening error\n");
        return EXIT_FAILURE;
    }
    memcpy(&(output->hdr), &(tga->hdr), sizeof(tga->hdr));   
    
    counter = 0;

    pixel* result = scalar_quantization(height, width, array, 8, 8, 0);
    if (result == NULL) {
        TGAClose(tga);
        perror("Quantization error\n");
        return EXIT_FAILURE;
    }

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
