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
#define SIGMA 10
#define SNP_FREQ 256

#define max(a, b) (((a)>(b))?(a):(b))
#define min(a, b) (((a)<(b))?(a):(b))
#define max3(a, b, c) ( max(a, max(b, c))) 
#define min3(a, b, c) ( min(a, min(b, c)))
#define MAXMIN(a, b, c, d, e) max3(min3(a, b, c), min3(b, c, d), min3(c, d, e))
#define MINMAX(a, b, c, d, e) min3(max3(a, b, c), max3(b, c, d), max3(c, d, e))

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

int add_gaussian_noise(int sigma, int width, int height, unsigned char* image)
{
	double a=0;
	int diff=0;
	int pix=0;
	for(int j=0; j<width*height; j++)
	{
		a=0; 
		for(int i=0; i<12; i++)
			a += rand();

		diff = (int)(sigma*(double)((double)(a/RAND_MAX)-6));
		pix = (int)image[j];
		pix+=diff;

		if(pix<0)
			pix=0;
		else if(pix>255)
			pix=255;

		image[j]=pix;

	}
	return 0;
}

int add_snp_noise(int freq, int width, int height, unsigned char* image)
{
	int *noise = new int[width*height];

	for(int i=0; i<width*height; i++)
	{
		noise[i]=rand()%freq;
	}

	for(int i=0; i<width*height; i++)
	{
		if(noise[i]==0)
			image[i]=0;
		else if(noise[i]==(freq-1))
			image[i]=255;
	}
	delete [] noise;
	return 0;
}

int comp (const void * elem1, const void * elem2) {
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

int remove_snp_2d(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	int *array = new int[w_size*w_size];
	int count=0;
	int median;
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			count=0;
			for(int x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(int y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
				{
					/*
					if(x2<0)
						array[count]=image[-x2*width+y];
					else if (x2>width)
						array[count]=image[(2*width-x2)*width+y];
					else
						array[count]=image
						*/
					if(x2<0 || y2<0 || x2>width || y2>height)
						array[count] = 0;
					else
						array[count]=image[x2*width+y2];

					count++;
				}
			}

			qsort(array, sizeof(array), sizeof(*array), comp);
			image_r[x*width+y]=array[w_size*w_size/2];
		}
	}

	delete [] array;
	return 0;
}

static const int kernel9[9]={ 1, 1, 1,
							  1, 1, 1,
							  1, 1, 1 };

static const int kernel10[9]={ 1, 1, 1,
							 1, 2, 1,
							 1, 1, 1 };

static const int kernel16[9]={ 1, 2, 1,
							 2, 4, 2,
							 1, 2, 1 };

int remove_gaussian(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	int idx=0;
	int value=0;
	int pix=0;
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			idx=0;
			value=0;

			for(int x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(int y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
				{
					pix = (int)image[x2*width+y2];
					pix *= kernel16[idx];
					value += pix;
					idx++;
				}
			}

			image_r[x*width+y] = value/16;

		}
	}

	return 0;
}

float psnr(int width, int height, unsigned char* _image, unsigned char* image)
{
	float mse = 0;
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			mse+=pow(_image[x*width+y]-image[x*width+y], 2)/(width*height);
		}
	}
	return 10*log10((255*255)/mse);
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
	write_pgm_image("sample2.pgm", Size, Size, Imagedata);
	paint_histogram(Size, Size, Imagedata, "histogram_sample2.pgm");

	unsigned char imageN[Size*Size]={};
	memcpy(imageN, Imagedata, sizeof(Imagedata));
	add_gaussian_noise(SIGMA, Size, Size, imageN);
	write_pgm_image("solve_p2_ng.pgm", Size, Size, imageN);
	paint_histogram(Size, Size, imageN, "histogram_ng.pgm");

	unsigned char imageP[Size*Size]={};
	memcpy(imageP, Imagedata, sizeof(Imagedata));
	add_snp_noise(SNP_FREQ, Size, Size, imageP);
	write_pgm_image("solve_p2_np.pgm", Size, Size, imageP);
	paint_histogram(Size, Size, imageP, "histogram_np.pgm");

	unsigned char imageB[Size*Size]={};
	memcpy(imageB, imageN, sizeof(imageN));
	add_snp_noise(SNP_FREQ, Size, Size, imageB);
	write_pgm_image("solve_p2_nb.pgm", Size, Size, imageB);
	paint_histogram(Size, Size, imageB, "histogram_nb.pgm");
	
	unsigned char imageRP[Size*Size]={};
	remove_snp_2d(3, Size, Size, imageP, imageRP);
	write_pgm_image("solve_p2_rp.pgm", Size, Size, imageRP);

	unsigned char imageRG[Size*Size]={};
	remove_gaussian(3, Size, Size, imageN, imageRG);
	write_pgm_image("solve_p2_rg.pgm", Size, Size, imageRG);

	unsigned char imageRB[Size*Size]={};
	unsigned char imageRB2[Size*Size]={};
	remove_snp_2d(3, Size, Size, imageB, imageRB);
	remove_gaussian(3, Size, Size, imageRB, imageRB2);
	write_pgm_image("solve_p2_rb.pgm", Size, Size, imageRB2);


	float psnr_g = psnr(Size, Size, Imagedata, imageN);
	float psnr_rg = psnr(Size, Size, Imagedata, imageRG);
	float psnr_p = psnr(Size, Size, Imagedata, imageP);
	float psnr_rp = psnr(Size, Size, Imagedata, imageRP);
	float psnr_b = psnr(Size, Size, Imagedata, imageB);
	float psnr_rb = psnr(Size, Size, Imagedata, imageRB2);

	fprintf(stderr, "psnr of gaussian noise = %f\n", psnr_g);
	fprintf(stderr, "psnr of repaired with low-pass filter = %f\n", psnr_rg);
	fprintf(stderr, "psnr of impulse noise = %f\n", psnr_p);
	fprintf(stderr, "psnr of repaired with median filter = %f\n", psnr_rp);
	fprintf(stderr, "psnr of mixed noise = %f\n", psnr_b);
	fprintf(stderr, "psnr of repaired with median and low-pass = %f\n", psnr_rb);

	exit(0);
	return 0;
}


