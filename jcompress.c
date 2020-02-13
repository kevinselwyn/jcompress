#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jpeglib.h>
#include <unistd.h>

#define TMP "tmp.jpg"

char *exec = NULL;

int usage() {
	int indent = (int)strlen(exec) + 8;

	printf("Usage: %s [--text,-t] <text outfile>\n", exec);
	printf("%*s[--jpeg,-j] <jpeg outfile>\n", indent, "");
	printf("%*s<infile> (<quality, 1-100>)\n", indent, "");

	return 1;
}

int main(int argc, char *argv[]) {
	int rc = 0, i = 0, l = 0, x = 0, args = 0;
	int level = 100, width = 0, height = 0, filler = 0, row_stride = 0, color = 0, counter = 0;
	int out_text = 0, out_jpeg = 0;
	size_t insize = 0;
	char *inname = NULL, *jpegname = NULL, *textname = NULL;
	unsigned char *indata = NULL, *outdata = NULL;
	FILE *infile = NULL, *jpegfile = NULL, *textfile = NULL;
	struct jpeg_compress_struct cinfo_compress;
	struct jpeg_decompress_struct cinfo_decompress;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	JSAMPARRAY pJpegBuffer;

	exec = argv[0];

	if (argc < 2) {
		rc = usage();
		goto cleanup;
	}

	for (i = 1, l = argc; i < l; i++) {
		if ((strcmp(argv[i], "--text") == 0 || strcmp(argv[i], "-t") == 0) && i + 1 < argc) {
			textname = argv[++i];
			args = i;
			out_text = 1;
			continue;
		}

		if ((strcmp(argv[i], "--jpeg") == 0 || strcmp(argv[i], "-j") == 0) && i + 1 < argc) {
			jpegname = argv[++i];
			args = i;
			out_jpeg = 1;
			continue;
		}
	}

	if (args + 1 >= argc) {
		rc = usage();
		goto cleanup;
	}

	inname = argv[++args];

	if (args + 1 < argc) {
		level = atoi(argv[++args]);
	}

	infile = fopen(inname, "rb");

	if (!infile) {
		printf("Could not open %s\n", inname);

		rc = 1;
		goto cleanup;
	}

	fseek(infile, 0, SEEK_END);
	insize = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	if (!insize) {
		printf("%s is empty\n", inname);

		rc = 1;
		goto cleanup;
	}

	width = ceil(sqrt(insize));
	height = width;
	filler = (width * height) - insize;

	indata = malloc(sizeof(char) * (insize + filler) + 1);
	outdata = malloc(sizeof(char) * (insize + filler) + 1);

	if (!indata || !outdata) {
		printf("Memory error\n");

		rc = 1;
		goto cleanup;
	}

	if (fread(indata, 1, insize, infile) != insize) {
		printf("Could not read %s\n", inname);

		rc = 1;
		goto cleanup;
	}

	for (i = 0, l = filler; i < l; i++) {
		indata[insize + i] = '\0';
	}

	if (!out_jpeg) {
		jpegname = TMP;
	}

	jpegfile = fopen(jpegname, "w+");

	if (!jpegfile) {
		printf("Could not open %s\n", jpegname);

		rc = 1;
		goto cleanup;
	}

	cinfo_compress.err = jpeg_std_error(&jerr);
	cinfo_decompress.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo_compress);
	jpeg_stdio_dest(&cinfo_compress, jpegfile);

	cinfo_compress.image_width = width;
	cinfo_compress.image_height = height;
	cinfo_compress.input_components = 1;
	cinfo_compress.in_color_space = JCS_GRAYSCALE;
	jpeg_set_defaults(&cinfo_compress);
	jpeg_set_quality(&cinfo_compress, level, TRUE);
	jpeg_start_compress(&cinfo_compress, TRUE);

	row_stride = cinfo_compress.input_components * cinfo_compress.image_width;

	while (cinfo_compress.next_scanline < cinfo_compress.image_height) {
		row_pointer[0] = (JSAMPROW)&indata[cinfo_compress.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo_compress, (JSAMPARRAY)&row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo_compress);
	jpeg_destroy_compress(&cinfo_compress);

	fseek(jpegfile, 0, SEEK_SET);

	jpeg_create_decompress(&cinfo_decompress);
	jpeg_stdio_src(&cinfo_decompress, jpegfile);

	jpeg_read_header(&cinfo_decompress, TRUE);
	jpeg_start_decompress(&cinfo_decompress);

	width = cinfo_decompress.output_width;
	height = cinfo_decompress.output_height;
	row_stride = width * cinfo_decompress.output_components;

	pJpegBuffer = (*cinfo_decompress.mem->alloc_sarray)((j_common_ptr) &cinfo_decompress, JPOOL_IMAGE, row_stride, 1);

	counter = 0;
	while (cinfo_decompress.output_scanline < cinfo_decompress.output_height) {
		jpeg_read_scanlines(&cinfo_decompress, pJpegBuffer, 1);

		for (x = 0; x < width; x++) {
			color = pJpegBuffer[0][cinfo_decompress.output_components * x];

			outdata[counter++] = color;
		}
	}

	if (!out_text) {
		printf("%s\n", outdata);
		goto cleanup;
	}

	textfile = fopen(textname, "w+");

	if (!textfile) {
		printf("Could not open %s\n", textname);

		rc = 1;
		goto cleanup;
	}

	if (fwrite(outdata, 1, insize, textfile) != insize) {
		printf("Could not write %s\n", textname);

		rc = 1;
		goto cleanup;
	}

cleanup:
	if (indata) {
		free(indata);
	}

	if (outdata) {
		free(outdata);
	}

	if (infile) {
		fclose(infile);
	}

	if (jpegfile) {
		if (!out_jpeg) {
			unlink(jpegname);
		}

		fclose(jpegfile);
	}

	if (textfile) {
		fclose(textfile);
	}

	return rc;
}
