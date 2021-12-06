#include "bitmap.h"

int BYTES_PER_PIXEL = 4;
int BITMAP_FILE_HEADER_SIZE = 14;
int BITMAP_INFO_HEADER_SIZE = 40;
#pragma acc routine seq
unsigned int color_to_int(color_t color) {
	
	// A B G R
	
	return (((unsigned int)floor(color.a * 255.0f) & 0xFF) << 24) | 
		   (((unsigned int)floor(color.b * 255.0f) & 0xFF) << 16) | 
		   (((unsigned int)floor(color.g * 255.0f) & 0xFF) << 8) | 
		   (((unsigned int)floor(color.r * 255.0f) & 0xFF) << 0);
	
}


unsigned char* createBitmapFileHeader(int height, int stride) {
	
	int fileSize = BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE + (stride * height);
	
	static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };
	
	fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE);
	
	return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width) {
	
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(BITMAP_INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}

void save_bmp(unsigned char * data, int width, int height, char * title) {
    
    
	
	int widthBytes = width * BYTES_PER_PIXEL;
	
	unsigned char padding[4] = {0, 0, 0, 0};
	int paddingSize = (4 - (widthBytes) % 4) % 4;
	
	int stride = (widthBytes) + paddingSize;
	
	FILE * imageFile = fopen(title, "wb");
	
	unsigned char * fileHeader = createBitmapFileHeader(height, stride);
	fwrite(fileHeader, 1, BITMAP_FILE_HEADER_SIZE, imageFile);
	
	unsigned char * infoHeader = createBitmapInfoHeader(height, width);
	fwrite(infoHeader, 1, BITMAP_INFO_HEADER_SIZE, imageFile);
	
	for(int i = 0; i < height; i++) {
		fwrite(data + (i * widthBytes), BYTES_PER_PIXEL, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}
	
	fclose(imageFile);
}
