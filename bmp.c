#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<stdint.h>

FILE *fp1 = NULL;
FILE *fp2 = NULL;

typedef struct {             // Total: 54 bytes
  uint16_t  type;             // Magic identifier: 0x4d42
  uint32_t  size;             // File size in bytes
  uint16_t  reserved1;        // Not used
  uint16_t  reserved2;        // Not used
  uint32_t  offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  uint32_t  dib_header_size;  // DIB Header size in bytes (40 bytes)
  int32_t   width_px;         // Width of the image
  int32_t   height_px;        // Height of image
  uint16_t  num_planes;       // Number of color planes
  uint16_t  bits_per_pixel;   // Bits per pixel
  uint32_t  compression;      // Compression type
  uint32_t  image_size_bytes; // Image size in bytes
  int32_t   x_resolution_ppm; // Pixels per meter
  int32_t   y_resolution_ppm; // Pixels per meter
  uint32_t  num_colors;       // Number of colors  
  uint32_t  important_colors; // Important colors 
} BMPHeader;

typedef struct {
    BMPHeader header;
    unsigned char* data; 
} BMPImage;



unsigned char reverse(unsigned char in)
{
  unsigned char out;
  out = 0;
#if 1
  if (in & 0x01) out |= 0x80;
  if (in & 0x02) out |= 0x40;
  if (in & 0x04) out |= 0x20;
  if (in & 0x08) out |= 0x10;
  if (in & 0x10) out |= 0x08;
  if (in & 0x20) out |= 0x04;
  if (in & 0x40) out |= 0x02;
  if (in & 0x80) out |= 0x01;
#endif
  return(out);
}



unsigned char* ReadBMP(char* filename)
{
	int i;
	FILE* f = fopen(filename, "r");

	if(f == NULL) {
		printf("Argument Exception");
		exit(-1);
	}

	fseek(f, 0L, SEEK_END);
	int sz = ftell(f);

	fseek(f, 0L, SEEK_SET);
	unsigned char icon[sz];


	FILE* fw= fopen("robin_marble.bmp", "w");
	if(fw == NULL) {
		printf("Argument Exception");
		exit(-1);
	} 

	fread(icon, sz, 1, f); 
	fseek(f, 0L, SEEK_SET);
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	printf("\n");
	for(int i = 0; i < 54 ; i++){
	    printf(" 0x%x ",info[i]);
	}
	printf("\n");

	fwrite(info,sizeof(unsigned char), 54,fw);

	// extract image height and width from header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int bits_per_pixel = info[28];
	int num_planes = info[26];
	printf("\n  Name: %s",filename );
	printf("\n Width:%d ", width);
	printf( "\n Height: %d ", height);
	printf( "\n num_planes: %d ", num_planes);
	printf( "\n bits_per_pixel: %d ", bits_per_pixel);

	int palette_size = 0;
	switch(bits_per_pixel)
	{
		case 1: palette_size = 2 * 4; break;
		case 4: palette_size = 16 * 4; break;
		case 8: palette_size = 256 * 4; break;
		default:break;
	}
	printf("\n bpp = %d ,palette_size = %d",bits_per_pixel, palette_size);


	int palette_offset =  54;
	int bits_offset = palette_offset + palette_size;

	unsigned char *palette = &icon[palette_offset];
	unsigned char *bits = &icon[bits_offset];


	fread(palette, sizeof(unsigned char), palette_size, f); // read the 54-byte header
	fwrite(palette,sizeof(unsigned char), palette_size,fw);

	int row_padded = (width*3 + 3) & (~3);

	printf("\n row_padded = %d ",row_padded );

	row_padded = ((width * bits_per_pixel + 31) / 32) * 4; //(width*3 + 3) & (~3);


	printf("\n row_padded = %d ",row_padded );



	int imagesize = height * row_padded ;

	unsigned char *invert = malloc(imagesize*(sizeof(unsigned char)));

	printf("\n imagesize = %d",imagesize);
	int width_in_bytes = ((width * bits_per_pixel + 31) / 32) * 4;
	printf("\n width_in_bytes = %d\n",width_in_bytes);

	int gap =  0;

	if(bits_per_pixel== 4){
	gap = width_in_bytes - width / 2;
	}

	if(bits_per_pixel== 24 ){
	gap = 3;
	}

	printf("\n gap  = %d\n", gap);

	for(int i = 0; i < height; i++)
	{
		int offset = row_padded*i;

		for(int x = 0 ; x < row_padded; x++){
			int src = offset + x;
			int dst = offset + width_in_bytes - x - 1 - gap ;
			
			if(dst < 0 || dst >= imagesize)
			    continue;

			invert[dst] = bits[src];
			#if 1
			//2 4-bit pixels are packed in to one byte, swap the pixels:
			unsigned char p = invert[dst];
			if(bits_per_pixel == 4) {
				invert[dst] = ((p & 0x0F) << 4) | ((p & 0xF0) >> 4);
			} 
			if(bits_per_pixel == 24) {
				//invert[dst] = reverse(p);
			}
			#endif	
		}
		if(bits_per_pixel == 24) {
			//now change the oder of the RGB component 
			for(int j = 0; j < width*3; j += 3)
			{
			    int src = offset + j;
			    // Convert (B, G, R) to (R, G, B)
			    unsigned char tmp = invert[src];
			    invert[src] = invert[src+2];
			    invert[src+2] = tmp;

			}
		}
		unsigned char *mydata = invert+offset;
		fwrite(invert+offset, sizeof(unsigned char), row_padded, fw);

	}

	//free(data);
	fclose(f);
	fclose(fw);
	return bits;
}

int main(){
	ReadBMP("MARBLES.BMP");
	//ReadBMP("flag_b24.bmp");
	//ReadBMP("bmp.bmp");
	

	return 0;

}
