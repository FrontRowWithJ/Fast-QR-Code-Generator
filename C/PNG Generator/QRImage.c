#include "QRImage.h"

int main()
{
    QRCode qr = init(0, "HELLO WORLD", false);
    export_to_png(qr, 45, "qrImage.png");
}

void export_to_png(QRCode qr, int factor, char *filename)
{
    size_t magWidth;
    unsigned char **magQR = magnify_QR_data(qr, factor, &magWidth);
    unsigned char *buf = QR_data_to_buffer(magQR, magWidth);
    write_image(filename, magWidth, magWidth, buf, NULL);
}

unsigned char **gen_matrix_uint8(int rowLen, int columnLen)
{
    if (rowLen <= 0 || columnLen <= 0)
        return NULL;
    unsigned char **matrix = (unsigned char **)malloc(columnLen * sizeof(unsigned char *));
    for (int i = 0; i < columnLen; i++)
        matrix[i] = (unsigned char *)malloc(rowLen * sizeof(unsigned char));
    return matrix;
}
unsigned char **magnify_QR_data(QRCode qr, int factor, size_t *newWidth)
{
    *newWidth = factor * qr.QRWidth;
    unsigned char **matrix = gen_matrix_uint8(*newWidth, *newWidth);
    for (int i = 0; i < qr.QRWidth; i++)
        for (int j = 0; j < qr.QRWidth; j++)
            for (int fi = 0; fi < factor; fi++)
                for (int fj = 0; fj < factor; fj++)
                    matrix[i * factor + fi][j * factor + fj] = qr.QRData[i][j];
    return matrix;
}

unsigned char *QR_data_to_buffer(unsigned char **QRData, size_t QRWidth)
{
    unsigned char *buf = (unsigned char *)malloc(QRWidth * QRWidth * sizeof(unsigned char));
    for (int i = 0; i < QRWidth * QRWidth; i++)
        buf[i] = QRData[i / QRWidth][i % QRWidth] % 2;
    return buf;
}
int write_image(char *filename, int width, int height, unsigned char *buf, char *title)
{
    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    fp = fopen(filename, "wb");
    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    //set title
    if (title != NULL)
    {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    row = (png_bytep)malloc(3 * width * sizeof(png_byte));

    // Write image data
    int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            set_RGB(&(row[x * 3]), buf[y * width + x]);
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

finalise:
    if (fp != NULL)
        fclose(fp);
    if (info_ptr != NULL)
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL)
        free(row);

    return code;
}

//val will be either a 0 or 1 to represent black(0,0,0) or white (0,0,0)
void set_RGB(png_byte *ptr, unsigned char val)
{
    ptr[0] = ptr[1] = ptr[2] = (val ^ 1) * 255;
}