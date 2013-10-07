#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
	int i=0; 
	for(i=0; i<x*y; i++)
		image[i]/=2;
	return 0;
}

int histogram_equalizer(int x, int y, unsigned char* image)
{
	/* create histogram */
	double histogram[256]={};
	int i=0;
	for(i=0; i<x*y; i++)
		histogram[image[i]]++;

	/* convert histogram to percentage (0-1) */
	for(i=0; i<256; i++)
		histogram[i]/=65536;

	/* cumulative histogram */
	for(i=0; i<256; i++)
		histogram[i]+=histogram[i-1];

	/* convert back to dynamic range 0-255 */
	for(i=0; i<256; i++)
		histogram[i]=(histogram[i]*254)+0.5;

	/* assign back using new histogram */
	for(i=0; i<x*y; i++)
		image[i]=histogram[image[i]];

	return 0;
}

int local_histogram_equalizer(int w_size, int width, int height, 
		int x, int y, unsigned char* image, unsigned char* image_r)
{
	// assume grayscale
	double histo[256]={};
	int i=0, x2=0, y2=0;
	for( i=0; i<256; i++)
		histo[i]=0;

	int pixels=0;
	for(x2= x-(w_size-1)/2; x2 <= x+(w_size-1)/2; x2++)
	{
		for(y2= y-(w_size-1)/2; y2 <= y+(w_size-1)/2; y2++)
		{
			if(x2<0 || y2<0 || x2>=width || y2>=height)
				continue;

			histo[image[x2*width+y2]]++;
			pixels++;
		}
	}

	/* convert histogram to percentage (0-1) */
	for(i=0; i<256; i++)
		histo[i]/=pixels;

	/* cumulative histogram */
	for(i=0; i<256; i++)
		histo[i]+=histo[i-1];

	/* convert back to dynamic range 0-255 */
	for(i=0; i<256; i++)
		histo[i]=histo[i]*255;

	/* assign back using new histogram */
	image_r[x*width+y]=histo[image[x*width+y]];
	return 0;

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
	histoimg=NULL;
	return 0;
}

int log_transform(int c, int width, int height, unsigned char* image)
{
	int tmp=0;
	int i=0;
	for(i=0; i<width*height; i++)
	{
		tmp=c*log2(1+image[i]);
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int inverse_log_transform(int c, int width, int height, unsigned char* image)
{
	int i=0, tmp=0;
	for(i=0; i<width*height; i++)
	{
		tmp=c/(log2(1+image[i]));
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int power_law_transform(double c, double gamma, int width, int height, unsigned char* image)
{
	int i=0, tmp=0;
	for(i=0; i<width*height; i++)
	{
		tmp=c*(double)pow(image[i], gamma);
		if(tmp>255) tmp=255;
		image[i]=tmp;
	}
	return 0;
}

int otsu_method_2(int width, int height, unsigned char* image )
{
	double histogram[256]={};
	double prob[256]={};
	double omega[256]={};
	double myu[256]={}, myu0=0;
	double sigma[256]={};
	int sum=0;
	double max_sigma;
	int threshold, threshold2;
	int i=0;
	for(i=0; i<256; i++) 
	{
		histogram[i]=0;
		omega[i]=0;
		myu[i]=0;
	}

	for(i=0; i<width*height; i++)
	{
		histogram[image[i]]++;
		sum+=image[i];
	}

	/* convert histogram to percentage (0-1) */
	for(i=0; i<256; i++)
		prob[i]=histogram[i]/65536;

	omega[0]=prob[0];
	myu0=sum/width*height;

	for(i=1; i<256; i++)
	{
		omega[i]=omega[i-1]+prob[i];
		myu[i]=(myu[i-1]+i*prob[i]);
	}

	for(i=0; i<255; i++)	
	{
		if(omega[i]!= 0 && omega[i]!=1)
			sigma[i]=pow(myu[255]*omega[i]-myu[i], 2)/(omega[i]*(1-omega[i]));
		else
			sigma[i]=0;

		if(sigma[i] > max_sigma )
		{
			max_sigma = sigma[i];
			threshold2=threshold;
			threshold=i;
		}
	}

	fprintf(stderr, "    INFO: threshold=%d\n", threshold);
	fprintf(stderr, "    INFO: threshold2=%d\n", threshold2);

	return (threshold+threshold2)/2;

}


int otsu_method(int width, int height, unsigned char* image )
{
	double histogram[256]={};
	float sum=0, sumb=0;
	float wb=0, wf=0;
	int i=0, threshold=0, threshold2=0;
	float sigma=0, max_sigma=0;

	for(i=0; i<width*height; i++)
	{
		histogram[image[i]]++;
		sum+=image[i];
	}

	float mb=0, mf=0;

	for(i=0; i<width; i++)
	{
		wb += histogram[i];
		if(wb==0)
			continue;

		wf=width*height-wb;
		if(wf==0)
			break;

		sumb+= (float)i*histogram[i];
		mb=sumb/wb;
		mf=(sum-sumb)/wf;

		sigma = wb*wf*pow(mb-mf, 2);
		if(max_sigma < sigma)
		{
			max_sigma = sigma;
			threshold2=threshold;
			threshold=i;
		}
	}
	return (threshold+threshold2)/2;
}


int convert_to_black_n_white(int threshold, int width, int height, unsigned char* image)
{
	int i=0;
	for(i=0; i<width*height; i++)
	{
		if(image[i]>=threshold)
			image[i]=255;
		else
			image[i]=0;
	}
	return 0;
}


int main(int argc, char** argv)
{
	FILE *file = NULL;
	// image data array
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
	write_pgm_image("dip_hw1_p1_I.pgm", Size, Size, Imagedata);

	unsigned char imageD[Size*Size]={};
	/* copy image data */
	memcpy(imageD, Imagedata, sizeof(Imagedata));

	/* decrease brightness (D) */ 
	decrease_brightness(Size, Size, imageD);
	write_pgm_image("dip_hw1_p1_D.pgm", Size, Size, imageD);

	unsigned char imageH[Size*Size]={};
	/* copy image data */
	memcpy(imageH, imageD, sizeof(imageD));

	/* histogram_equalizer (H) */
	histogram_equalizer(Size, Size, imageH);
	write_pgm_image("dip_hw1_p1_H.pgm", Size, Size, imageH);


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

	int i=0, j=0, w_size=10;
	unsigned char imageL[Size*Size]={};
	for(i=0; i<Size; i++)
		for(j=0; j<Size; j++)
			local_histogram_equalizer(w_size, Size, Size, i, j, imageD, imageL);

	write_pgm_image("dip_hw1_p1_L_10.pgm", Size, Size, imageL);
	paint_histogram(Size, Size, imageL,    "dip_hw1_p1_histogram_L_10.pgm");

	memset(imageL, 0, Size*Size);
	for(i=0; i<Size; i++)
		for(j=0; j<Size; j++)
			local_histogram_equalizer(30, Size, Size, i, j, imageD, imageL);
	write_pgm_image("dip_hw1_p1_L_30.pgm", Size, Size, imageL);
	paint_histogram(Size, Size, imageL,    "dip_hw1_p1_histogram_L_30.pgm");

	memset(imageL, 0, Size*Size);
	for(i=0; i<Size; i++)
		for(j=0; j<Size; j++)
			local_histogram_equalizer(120, Size, Size, i, j, imageD, imageL);
	write_pgm_image("dip_hw1_p1_L_120.pgm", Size, Size, imageL);
	paint_histogram(Size, Size, imageL,    "dip_hw1_p1_histogram_L_120.pgm");

	memset(imageL, 0, Size*Size);
	for(i=0; i<Size; i++)
		for(j=0; j<Size; j++)
			local_histogram_equalizer(180, Size, Size, i, j, imageD, imageL);
	write_pgm_image("dip_hw1_p1_L_180.pgm", Size, Size, imageL);
	paint_histogram(Size, Size, imageL,    "dip_hw1_p1_histogram_L_180.pgm");


	paint_histogram(Size, Size, Imagedata, "dip_hw1_p1_histogram_I.pgm");
	paint_histogram(Size, Size, imageD,    "dip_hw1_p1_histogram_D.pgm");
	paint_histogram(Size, Size, imageH,    "dip_hw1_p1_histogram_H.pgm");

	unsigned char imageLog[Size*Size] = {};
	unsigned char imageILog[Size*Size] = {};
	unsigned char imagePlaw[Size*Size] = {};

	memcpy(imageLog, imageD, sizeof(imageD));
	memcpy(imageILog, imageD, sizeof(imageD));
	memcpy(imagePlaw, imageD, sizeof(imageD));

	/* log_c is the constant for log transform */
	const int log_c = 35;
	log_transform(log_c, Size, Size, imageLog);
	write_pgm_image("dip_hw1_p1_D_log.pgm", Size, Size, imageLog);
	paint_histogram(Size, Size, imageLog, "dip_hw1_p1_histogram_D_log.pgm");

	const int log_d = 240;
	inverse_log_transform(log_d, Size, Size, imageILog);
	write_pgm_image("dip_hw1_p1_D_ilog.pgm", Size, Size, imageILog);
	paint_histogram(Size, Size, imageILog, "dip_hw1_p1_histogram_D_ilog.pgm");

	double pow = 30;
	double gamma = 0.5;
	power_law_transform(pow, gamma, Size, Size, imagePlaw);
	write_pgm_image("dip_hw1_p1_D_pow.pgm", Size, Size, imagePlaw);
	paint_histogram(Size, Size, imagePlaw, "dip_hw1_p1_histogram_D_pow.pgm");

	pow = 1;
	gamma = 1.5;
	memcpy(imagePlaw, imageD, sizeof(imageD));
	power_law_transform(pow, gamma, Size, Size, imagePlaw);
	write_pgm_image("dip_hw1_p1_D_pow1.pgm", Size, Size, imagePlaw);
	paint_histogram(Size, Size, imagePlaw, "dip_hw1_p1_histogram_D_pow1.pgm");

	unsigned char imageOtsu[Size*Size]={};
	int thre = otsu_method(Size, Size, Imagedata);
	fprintf(stderr, " thresold = %d \n", thre);
	memcpy(imageOtsu, Imagedata, sizeof(imageOtsu));	
	convert_to_black_n_white(thre, Size, Size, imageOtsu);
	write_pgm_image("dip_hw1_p1_I_otsu.pgm", Size, Size, imageOtsu);

	memcpy(imageOtsu, Imagedata, sizeof(imageOtsu));	
	convert_to_black_n_white(64, Size, Size, imageOtsu);
	write_pgm_image("dip_hw1_p1_I_64.pgm", Size, Size, imageOtsu);

	memcpy(imageOtsu, Imagedata, sizeof(imageOtsu));	
	convert_to_black_n_white(128, Size, Size, imageOtsu);
	write_pgm_image("dip_hw1_p1_I_128.pgm", Size, Size, imageOtsu);

	memcpy(imageOtsu, Imagedata, sizeof(imageOtsu));	
	convert_to_black_n_white(192, Size, Size, imageOtsu);
	write_pgm_image("dip_hw1_p1_I_192.pgm", Size, Size, imageOtsu);

	exit(0);
	return 0;
}




	
