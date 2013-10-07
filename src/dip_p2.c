#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define Size 256
#define SIGMA 10
#define SNP_FREQ 128

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
	  return 1;
}

int paint_histogram(int width, int height, unsigned char* image, char* filename)
{

	unsigned char *histoimg = calloc(1, width*height);
	double histo[256]={};
	int i=0, j=0;
	for(i=0; i<256; i++)
		histo[i]=0;

	for(i=0; i<width*height; i++)
		histo[image[i]]++;

	for(i=0; i<256; i++)
	{
		histo[i]/=65536;
		histo[i]*=255*40;
	}

	for(i=0; i<256; i++)
	{
		for(j=0; j<histo[i]; j++)
		{
			if(j>255)
				break;
			histoimg[i*width+j]=238;
		}
	}

	write_pgm_image(filename, width, height, histoimg);
	free(histoimg);
	return 0;
}

int add_gaussian_noise(int sigma, int width, int height, unsigned char* image)
{
	double a=0;
	int diff=0;
	int pix=0;
	int j=0, i=0;
	for(j=0; j<width*height; j++)
	{
		a=0; 
		for(i=0; i<12; i++)
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
	int *noise = calloc(sizeof(int), width*height);
	int i=0;
	for(i=0; i<width*height; i++)
		noise[i]=rand()%freq;

	for(i=0; i<width*height; i++)
	{
		if(noise[i]==0)
			image[i]=0;
		else if(noise[i]==(freq-1))
			image[i]=255;
	}
	free(noise);
	return 0;
}

int comp (const void * elem1, const void * elem2) {
    unsigned char f = *((unsigned char*)elem1);
    unsigned char s = *((unsigned char*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

int remove_snp_2d(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	unsigned char *array = calloc(1, w_size*w_size);
	int count=0;
	int z=0;
	int x=0, y=0, x2=0, y2=0;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			count=0;
			for(x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
				{
					if(x2<0 || y2<0 || x2>width || y2>height)
						array[count] = 0;
					else
						array[count]=image[x2*width+y2];

					count++;
				}
			}

			qsort(array, w_size*w_size, sizeof(*array), comp);
			image_r[x*width+y]=array[w_size*w_size/2];
		}
	}

	free(array);
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

int remove_gaussian10(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	int idx=0;
	int value=0;
	int pix=0;
	int x=0, y=0, x2=0, y2=0;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			idx=0;
			value=0;

			for(x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
				{
					pix = (int)image[x2*width+y2];
					pix *= kernel10[idx];
					value += pix;
					idx++;
				}
			}

			image_r[x*width+y] = value/10;

		}
	}

	return 0;
}
int remove_gaussian9(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	int idx=0;
	int value=0;
	int pix=0;
	int x=0, y=0, x2=0, y2=0;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			idx=0;
			value=0;

			for(x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
				{
					pix = (int)image[x2*width+y2];
					pix *= kernel9[idx];
					value += pix;
					idx++;
				}
			}

			image_r[x*width+y] = value/9;

		}
	}

	return 0;
}
int remove_gaussian16(int w_size, int width, int height, unsigned char* image, unsigned char* image_r)
{
	int idx=0;
	int value=0;
	int pix=0;
	int x=0, y=0, x2=0, y2=0;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			idx=0;
			value=0;

			for(x2=x-(w_size-1)/2; x2<=x+(w_size-1)/2; x2++)
			{
				for(y2=y-(w_size-1)/2; y2<=y+(w_size-1)/2; y2++)
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
	int x=0, y=0;
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			mse+=pow(_image[x*width+y]-image[x*width+y], 2)/(width*height);
		}
	}

	return 10*log10((255*255)/mse);
}

int main(int argc, char** argv)
{
	FILE *file = NULL;
	unsigned char Imagedata[Size*Size] = {};

	char fname[1024]={};
	if(argv[1] != NULL && strlen(argv[1])>0)
		strcpy(fname, argv[1]);
	else
	{
		fprintf(stderr, "please specify filename of raw input image.\n");
		exit(-1);
	}

	if (!(file=fopen(fname,"rb")))
	{
		fprintf(stderr, "Cannot open file!\n");
		exit(1);
	}
	fread(Imagedata, sizeof(unsigned char), Size*Size, file);
	fclose(file);

	/* save the original image for comparision */
	write_pgm_image("dip_hw1_p2_sample.pgm", Size, Size, Imagedata);
//	paint_histogram(Size, Size, Imagedata, "dip_hw1_p2_histogram_sample.pgm");

	unsigned char imageN[Size*Size]={};
	unsigned char imageB[Size*Size]={};
	unsigned char imageP[Size*Size]={};

	unsigned char imageRP[Size*Size]={};
	unsigned char imageRG[Size*Size]={};
	unsigned char imageRB[Size*Size]={};
	unsigned char imageRB2[Size*Size]={};
	float psnr_g  = 0.0; 
	float psnr_rg = 0.0; 
	float psnr_p  = 0.0; 
	float psnr_rp = 0.0; 
	float psnr_b  = 0.0; 
	float psnr_rb = 0.0; 



	memcpy(imageN, Imagedata, sizeof(Imagedata));
	add_gaussian_noise(SIGMA, Size, Size, imageN);
	write_pgm_image("dip_hw1_p2_ng.pgm", Size, Size, imageN);

	psnr_g = psnr(Size, Size, Imagedata, imageN);
	fprintf(stderr, "PSNR of Gaussian(sigma=10) = %f\n", psnr_g);

	memset(imageRG, 0, sizeof(imageRG));
	remove_gaussian16(3, Size, Size, imageN, imageRG);
	write_pgm_image("dip_hw1_p2_rg.pgm", Size, Size, imageRG);

	psnr_rg = psnr(Size, Size, Imagedata, imageRG);
	fprintf(stderr, "PSNR of Gaussian(sigma=10) repair with kernel16 = %f\n", psnr_rg);

	memset(imageRG, 0, sizeof(imageRG));
	remove_gaussian10(3, Size, Size, imageN, imageRG);
	write_pgm_image("dip_hw1_p2_rg10.pgm", Size, Size, imageRG);

	psnr_rg = psnr(Size, Size, Imagedata, imageRG);
	fprintf(stderr, "PSNR of Gaussian(sigma=10) repair with kernel10 = %f\n", psnr_rg);

	memset(imageRG, 0, sizeof(imageRG));
	remove_gaussian9(3, Size, Size, imageN, imageRG);
	write_pgm_image("dip_hw1_p2_rg9.pgm", Size, Size, imageRG);

	psnr_rg = psnr(Size, Size, Imagedata, imageRG);
	fprintf(stderr, "PSNR of Gaussian(sigma=10) repair with kernel9 = %f\n", psnr_rg);


	/* sigma=10, freq=128 */
	memcpy(imageB, imageN, sizeof(imageN));
	add_snp_noise(SNP_FREQ, Size, Size, imageB);
	write_pgm_image("dip_hw1_p2_nb.pgm", Size, Size, imageB);

	/* repair sigma=10, freq=128 */
	memset(imageRB, 0, sizeof(imageRB));
	memset(imageRB2, 0, sizeof(imageRB2));
	remove_snp_2d(3, Size, Size, imageB, imageRB);
	remove_gaussian16(3, Size, Size, imageRB, imageRB2);
	write_pgm_image("dip_hw1_p2_rb.pgm", Size, Size, imageRB2);

	/* ----- repair finished */
	
	psnr_b = psnr(Size, Size, Imagedata, imageB);
	fprintf(stderr, "add Gaussian and SNP = %f\n", psnr_b);
	psnr_rb = psnr(Size, Size, Imagedata, imageRB2);
	fprintf(stderr, "    repair  = %f\n", psnr_rb);


	memcpy(imageN, Imagedata, sizeof(Imagedata));
	add_gaussian_noise(20, Size, Size, imageN);
	write_pgm_image("dip_hw1_p2_ng_20.pgm", Size, Size, imageN);
//	paint_histogram(Size, Size, imageN, "dip_hw1_p2_histogram_ng.pgm");

	memset(imageRG, 0, sizeof(imageRG));
	remove_gaussian16(3, Size, Size, imageN, imageRG);
	write_pgm_image("dip_hw1_p2_rg_20.pgm", Size, Size, imageRG);
	

	/* sigma=20, freq=50 */
	memcpy(imageB, imageN, sizeof(imageN));
	add_snp_noise(50, Size, Size, imageB);
	write_pgm_image("dip_hw1_p2_nb_2050.pgm", Size, Size, imageB);

	/* repair sigma=20, freq=50 */
	memset(imageRB, 0, sizeof(imageRB));
	memset(imageRB2, 0, sizeof(imageRB2));
	remove_snp_2d(3, Size, Size, imageB, imageRB);
	remove_gaussian16(3, Size, Size, imageRB, imageRB2);
	write_pgm_image("dip_hw1_p2_rb_2050.pgm", Size, Size, imageRB2);

	/* ---- repair finished */

	psnr_b = psnr(Size, Size, Imagedata, imageB);
	fprintf(stderr, "add Gaussian and SNP = %f\n", psnr_b);
	psnr_rb = psnr(Size, Size, Imagedata, imageRB2);
	fprintf(stderr, "    repair  = %f\n", psnr_rb);


	memcpy(imageN, Imagedata, sizeof(Imagedata));
	add_gaussian_noise(50, Size, Size, imageN);
	write_pgm_image("dip_hw1_p2_ng_50.pgm", Size, Size, imageN);


	memset(imageRG, 0, sizeof(imageRG));
	remove_gaussian16(3, Size, Size, imageN, imageRG);
	write_pgm_image("dip_hw1_p2_rg_50.pgm", Size, Size, imageRG);
	

	/* sigma=50, freq=10 */
	memcpy(imageB, imageN, sizeof(imageN));
	add_snp_noise(10, Size, Size, imageB);
	write_pgm_image("dip_hw1_p2_nb_5010.pgm", Size, Size, imageB);

	/* repair sigma=50, freq=10 */
	memset(imageRB, 0, sizeof(imageRB));
	memset(imageRB2, 0, sizeof(imageRB2));
	remove_snp_2d(3, Size, Size, imageB, imageRB);
	remove_gaussian16(3, Size, Size, imageRB, imageRB2);
	write_pgm_image("dip_hw1_p2_rb_5010.pgm", Size, Size, imageRB2);

	/* ---- repair finished */

	psnr_b = psnr(Size, Size, Imagedata, imageB);
	fprintf(stderr, "add Gaussian and SNP = %f\n", psnr_b);
	psnr_rb = psnr(Size, Size, Imagedata, imageRB2);
	fprintf(stderr, "    repair  = %f\n", psnr_rb);




	memcpy(imageP, Imagedata, sizeof(Imagedata));
	add_snp_noise(SNP_FREQ, Size, Size, imageP);
	write_pgm_image("dip_hw1_p2_np.pgm", Size, Size, imageP);

	psnr_p = psnr(Size, Size, Imagedata, imageP);
	fprintf(stderr, "PSNR of adding SNP (freq=128) = %f\n", psnr_p);

	memset(imageRP, 0, sizeof(imageRP));
	remove_snp_2d(3, Size, Size, imageP, imageRP);
	write_pgm_image("dip_hw1_p2_rp.pgm", Size, Size, imageRP);

	psnr_rp = psnr(Size, Size, Imagedata, imageRP);
	fprintf(stderr, "SNP removal with median filter (w_size=3) = %f\n", psnr_rp);

	memset(imageRP, 0, sizeof(imageRP));
	remove_snp_2d(5, Size, Size, imageP, imageRP);
	write_pgm_image("dip_hw1_p2_rp5.pgm", Size, Size, imageRP);

	psnr_rp = psnr(Size, Size, Imagedata, imageRP);
	fprintf(stderr, "SNP removal with median filter (w_size=5) = %f\n", psnr_rp);

	memset(imageRP, 0, sizeof(imageRP));
	remove_snp_2d(9, Size, Size, imageP, imageRP);
	write_pgm_image("dip_hw1_p2_rp9.pgm", Size, Size, imageRP);

	psnr_rp = psnr(Size, Size, Imagedata, imageRP);
	fprintf(stderr, "SNP removal with median filter (w_size=9) = %f\n", psnr_rp);


	memcpy(imageP, Imagedata, sizeof(Imagedata));
	add_snp_noise(50, Size, Size, imageP);
	write_pgm_image("dip_hw1_p2_np_50.pgm", Size, Size, imageP);

	memset(imageRP, 0, sizeof(imageRP));
	remove_snp_2d(3, Size, Size, imageP, imageRP);
	write_pgm_image("dip_hw1_p2_rp_50.pgm", Size, Size, imageRP);

	memcpy(imageP, Imagedata, sizeof(Imagedata));
	add_snp_noise(10, Size, Size, imageP);
	write_pgm_image("dip_hw1_p2_np_10.pgm", Size, Size, imageP);

	memset(imageRP, 0, sizeof(imageRP));
	remove_snp_2d(3, Size, Size, imageP, imageRP);
	write_pgm_image("dip_hw1_p2_rp_10.pgm", Size, Size, imageRP);


	/*
	psnr(Size, Size, Imagedata, imageN);
	psnr(Size, Size, Imagedata, imageRG);
	psnr(Size, Size, Imagedata, imageP);
	psnr(Size, Size, Imagedata, imageRP);
	psnr(Size, Size, Imagedata, imageB);
	psnr(Size, Size, Imagedata, imageRB2);

	fprintf(stderr, "PSNR of gaussian noise = %f\n", psnr_g);
	fprintf(stderr, "PSNR of repaired with low-pass filter = %f\n", psnr_rg);
	fprintf(stderr, "PSNR of impulse noise = %f\n", psnr_p);
	fprintf(stderr, "PSNR of repaired with median filter = %f\n", psnr_rp);
	fprintf(stderr, "PSNR of mixed noise = %f\n", psnr_b);
	fprintf(stderr, "PSNR of repaired with median and low-pass = %f\n", psnr_rb);
	*/

	exit(0);
	return 0;
}


