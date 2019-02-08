#include <png.h>
#include "../QR Code Generator/QRCode.h"

void export_to_png(QRCode qr, int factor, char *filename);
unsigned char **magnify_QR_data(QRCode qr, int factor, size_t *newWidth);
unsigned char **gen_matrix_uint8(int rowLen, int columnLen);
unsigned char *QR_data_to_buffer(unsigned char **QRData, size_t QRWidth);
int write_image(char *filename, int width, int height, unsigned char *buf, char *title);
void set_RGB(png_byte *ptr, unsigned char val);