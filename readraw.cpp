/*** The programe reads out image data from image file and write into it ***/
// here I used gray scale image "lena.raw" as an example.
// it will read the image data to Imagedata array
// it also write data in Imagedata array to image file "lena1.raw" 

#include "stdio.h"
#include "iostream.h"
#include "stdlib.h"

// define image size 512*512
// you can change it to other size
#define Size 512

void main(void)
{
	// file pointer
	FILE *file;
	// image data array
	unsigned char Imagedata[Size][Size];

	// read image "lena.raw" into image data matrix
	if (!(file=fopen("lena.raw","rb")))
	{
		cout<<"Cannot open file!"<<endl;
		exit(1);
	}
	fread(Imagedata, sizeof(unsigned char), Size*Size, file);
	fclose(file);

	// do some image processing task...
	
	// write image data to "lena1.raw"
	if (!(file=fopen("lena1.raw","wb")))
	{
		cout<<"Cannot open file!"<<endl;
		exit(1);
	}
	fwrite(Imagedata, sizeof(unsigned char), Size*Size, file);
	fclose(file);

	exit(0);
}




	