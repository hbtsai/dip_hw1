/*** The programe reads out image data from image file and write into it ***/
// here I used gray scale image "lena.raw" as an example.
// it will read the image data to Imagedata array
// it also write data in Imagedata array to image file "lena1.raw" 

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <math.h>

// define image size 512*512
// you can change it to other size
#define Size 256


int write_pgm_image(char* filename, int x_dim, int y_dim, unsigned char* image)
{
	unsigned char* y = image;
	FILE* filehandle = NULL;
	filehandle = fopen(filename, "wb");
	if (filehandle) 
	{
		fprintf(filehandle, "P5\n\n%d %d 255\n", x_dim, y_dim);
		fwrite(y, 1, x_dim * y_dim, filehandle);
		fclose(filehandle);
		return 0;
	} 
	else
	{
	  return 1;
	}
}

int decrease_brightness(int x, int y, unsigned char* image)
{
	for(int i=0; i<x*y; i++)
	{
		image[i]/=2;
	}
	return 0;
}

int histogram_equalizer(int x, int y, unsigned char* image)
{
	/* create histogram */
	double histogram[256]={};
	for(int i=0; i<x*y; i++)
	{
		histogram[image[i]]++;
	}

	/* convert histogram to percentage (0-1) */
	for(int i=0; i<256; i++)
	{
		histogram[i]/=65536;
	}

	/* cumulative histogram */
	for(int i=0; i<256; i++)
	{
		histogram[i]+=histogram[i-1];
	}

	/* convert back to dynamic range 0-255 */
	for(int i=0; i<256; i++)
	{
		histogram[i]*=254;
		histogram[i]+=0.5;
	}

	/* assign back using new histogram */
	for(int i=0; i<x*y; i++)
	{
		image[i]=histogram[image[i]];
	}

	return 0;
}

int local_histogram_equalizer(int w_size, int width, int height, 
		int x, int y, unsigned char* image, unsigned char* image_r)
{
	// assume grayscale
	double histo[256]={};
	for(int i=0; i<256; i++)
	{
		histo[i]=0;
	}

	int pixels=0;
	for(int x2= x-(w_size-1)/2; x2 <= x+(w_size-1)/2; x2++)
	{
		for(int y2= y-(w_size-1)/2; y2 <= y+(w_size-1)/2; y2++)
		{
			if(x2<0 || y2<0 || x2>=width || y2>=height)
			{
				continue;
			}

			histo[image[x2*width+y2]]++;
			pixels++;
		}
	}

	/* convert histogram to percentage (0-1) */
	for(int i=0; i<256; i++)
	{
		histo[i]/=pixels;
	}

//		fprintf(stderr, " \n\n ");
	/* cumulative histogram */
	for(int i=0; i<256; i++)
	{
		histo[i]+=histo[i-1];

//		fprintf(stderr, " %f ", histo[i]);
	}

//	fprintf(stderr, "\n\n");
	/* convert back to dynamic range 0-255 */
	for(int i=0; i<256; i++)
	{
		histo[i]*=254;
		histo[i]+=0.5;
	}

	/* assign back using new histogram */
	image_r[x*width+y]=histo[image[x*width+y]];
	return 0;

}

int paint_histogram(int width, int height, unsigned char* image, char* filename)
{

	unsigned char *histoimg = new unsigned char[width*height];
	double histo[256]={};
	for(int i=0; i<256; i++)
	{
		histo[i]=0;
	}

	for(int i=0; i<width*height; i++)
	{
		histo[image[i]]++;
	}

	for(int i=0; i<256; i++)
	{
		histo[i]/=65536;
		histo[i]*=255*40;
	}

	for(int i=0; i<256; i++)
	{
		for(int j=0; j<histo[i]; j++)
		{
			histoimg[i*width+j]=255;
		}
	}

	write_pgm_image(filename, width, height, histoimg);
	delete [] histoimg;
	return 0;
}

int log_transform(int c, int width, int height, unsigned char* image)
{
	int tmp=0;
	for(int i=0; i<width*height; i++)
	{
		tmp=c*log(1+image[i]);
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int inverse_log_transform(int c, int width, int height, unsigned char* image)
{
	int tmp=0;
	for(int i=0; i<width*height; i++)
	{
		tmp=c/(log(1+image[i]));
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int power_law_transform(int c, double gamma, int width, int height, unsigned char* image)
{
	int tmp=0;
	for(int i=0; i<width*height; i++)
	{
		tmp=c*(double)pow(image[i], gamma);
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int main(int argc, char** argv)
{
	// file pointer
	FILE *file = NULL;
	// image data array
	unsigned char Imagedata[Size*Size] = {};

	char fname[1024]={};
	if(argv[1] != NULL && strlen(argv[1])>0)
	{
		strcpy(fname, argv[1]);
	}
	else
	{
		fprintf(stderr, "please specify filename of raw input image.\n");
		exit(-1);
	}

	// read image "lena.raw" into image data matrix
	if (!(file=fopen(fname,"rb")))
	{
		cout<<"Cannot open file!"<<endl;
		exit(1);
	}
	fread(Imagedata, sizeof(unsigned char), Size*Size, file);
	fclose(file);

	/* save the original image for comparision */
	write_pgm_image("sample1.pgm", Size, Size, Imagedata);

	unsigned char imageD[Size*Size]={};
	/* copy image data */
	memcpy(imageD, Imagedata, sizeof(Imagedata));

	/* decrease brightness (D) */ 
	decrease_brightness(Size, Size, imageD);
	write_pgm_image("solved_a.pgm", Size, Size, imageD);

	/*
	int his[Size]={};
	for(int i=0; i<Size*Size; i++)
	{
		his[Imagedata[i]]++;
	}
	for(int i=0; i<256; i++)
	{
		fprintf(stderr, " %d ", his[i]);
	}
	*/

	unsigned char imageH[Size*Size]={};
	/* copy image data */
	memcpy(imageH, imageD, sizeof(imageD));

	/* histogram_equalizer (H) */
	histogram_equalizer(Size, Size, imageH);
//	int his[Size]={};
	/*
	for(int i=0; i<256; i++)
	{
		his[i]=0;
	}
	for(int i=0; i<Size*Size; i++)
	{
		his[imageH[i]]++;
	}
	for(int i=0; i<256; i++)
	{
		fprintf(stderr, " %d ", his[i]);
	}
	*/
	write_pgm_image("solved_b.pgm", Size, Size, imageH);


	/* here loop create local histogram equalizer with different window size */
	/* change w_size to whatever 0< number <256 */
	/*
	int w_size=50;
	unsigned char imageL[Size*Size]={};
	char file_name[1024]={};

	for( int w_size=10; w_size<250; w_size+=10)
	{
		memset(imageL, 0, sizeof(imageL));
		for(int i=0; i<Size; i++)
		{
			for(int j=0; j<Size; j++)
			{
				// just ignore the boarder & corner
				local_histogram_equalizer(w_size, Size, Size, i, j, imageD, imageL);
			}
		}
		sprintf(file_name, "solved_c_%d.pgm", w_size);
		write_pgm_image(file_name, Size, Size, imageL);
	}
	*/

	int w_size=10;
	unsigned char imageL[Size*Size]={};
	for(int i=0; i<Size; i++)
	{
		for(int j=0; j<Size; j++)
		{
			// just ignore the boarder & corner
			local_histogram_equalizer(w_size, Size, Size, i, j, imageD, imageL);
		}
	}
	write_pgm_image("solved_c.pgm", Size, Size, imageL);


	paint_histogram(Size, Size, Imagedata, "histogram_I.pgm");
	paint_histogram(Size, Size, imageD, "histogram_D.pgm");
	paint_histogram(Size, Size, imageH, "histogram_H.pgm");
	paint_histogram(Size, Size, imageL, "histogram_L.pgm");

	unsigned char imageLog[Size*Size] = {};
	unsigned char imageILog[Size*Size] = {};
	unsigned char imagePlaw[Size*Size] = {};

	memcpy(imageLog, imageD, sizeof(imageD));
	memcpy(imageILog, imageD, sizeof(imageD));
	memcpy(imagePlaw, imageD, sizeof(imageD));

	/* log_c is the constant for log transform */
	const int log_c = 50;
	log_transform(log_c, Size, Size, imageLog);
	write_pgm_image("solved_f1.pgm", Size, Size, imageLog);
	paint_histogram(Size, Size, imageLog, "histogram_log.pgm");

	const int log_d = 250;
	inverse_log_transform(log_d, Size, Size, imageILog);
	write_pgm_image("solved_f2.pgm", Size, Size, imageILog);
	paint_histogram(Size, Size, imageILog, "histogram_ilog.pgm");

	const int pow = 10;
	const double gamma = 0.7;
	power_law_transform(pow, gamma, Size, Size, imagePlaw);
	paint_histogram(Size, Size, imagePlaw, "histogram_plaw.pgm");
	write_pgm_image("solved_f3.pgm", Size, Size, imagePlaw);



	/* copy image data */
	//memcpy(imageL, imageD, sizeof(imageD));

	// do some image processing task...
	
	// write image data to "lena1.raw"
	/*
	if (!(file=fopen("lena1.raw","wb")))
	{
		cout<<"Cannot open file!"<<endl;
		exit(1);
	}
	fwrite(Imagedata, sizeof(unsigned char), Size*Size, file);
	fclose(file);
	*/

	exit(0);
	return 0;
}




	
