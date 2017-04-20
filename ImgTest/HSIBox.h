
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include<iostream>
#include <cv.h>
#include <opencv2/opencv.hpp>  
#include <highgui.h>
#include <string>
#include <algorithm>
#include "cxcore.h"
#include "cvaux.h"
#include <math.h>

using namespace std;
using namespace cv;

class HSIBox
{
private:
	//IplImage* ���͵�Դͼ��IplImage_src�����ڵõ���HSI
	IplImage* IplImage_src;
	//��ɫ����float����hMatrix[y][x]��x��Ӧwidth��y��Ӧheight
	float **hMatrix;
	float **sMatrix;
public:
	HSIBox(Mat input)
	{
		IplImage_src = &IplImage(input);

		//H��S�����ʼ��
		hMatrix = new float*[IplImage_src->height];
		for (int i = 0; i < IplImage_src->height; ++i)
		{
			hMatrix[i] = new float[IplImage_src->width];
		}
		sMatrix = new float*[IplImage_src->height];
		for (int i = 0; i < IplImage_src->height; ++i)
		{
			sMatrix[i] = new float[IplImage_src->width];
		}

		//����H����S����
		// ԭʼͼ������ָ��, HSI��������ָ��
		uchar* data;

		// rgb����
		float img_r, img_g, img_b;
		float min_rgb;  // rgb�����е���Сֵ
		// HSI����
		float fHue, fSaturation, fIntensity, fSaturation_show, fIntensity_show;

		for (int i = 0; i < IplImage_src->height; i++) //��Ӧy
		{
			for (int j = 0; j < IplImage_src->width; j++) //��Ӧx
			{
				data = cvPtr2D(IplImage_src, i, j, 0);
				img_b = *data;
				data++;
				img_g = *data;
				data++;
				img_r = *data;

				// Intensity����[0, 1]
				fIntensity = (float)((img_b + img_g + img_r) / 3) / 255;
				fIntensity_show = (float)((img_b + img_g + img_r) / 3);

				// �õ�RGB�����е���Сֵ
				float fTemp = img_r < img_g ? img_r : img_g;
				min_rgb = fTemp < img_b ? fTemp : img_b;
				// Saturation����[0, 1]
				if (img_r + img_g + img_b == 0)
				{
					fSaturation = 0;
				}
				else
				{
					fSaturation = 1 - (float)(3 * min_rgb) / (img_r + img_g + img_b);
				}

				// ����theta��
				float numerator = (img_r - img_g + img_r - img_b) / 2;
				float denominator = sqrt(
					pow((img_r - img_g), 2) + (img_r - img_b)*(img_g - img_b));

				// ����Hue����
				if (denominator != 0)
				{
					float theta = acos(numerator / denominator)  / (2*3.14);

					if (img_b <= img_g)
					{
						fHue = theta;
					}
					else
					{
						fHue = 1 - theta;
					}
				}
				else
				{
					fHue = 0;
				}
				hMatrix[i][j] = fHue;
				sMatrix[i][j] = fSaturation;
			}
		}
	}

	//����ͼ����(x,y)��HS��Ϣ
	float getH(int x, int y)
	{
		return hMatrix[y][x];
	}

	float getS(int x, int y)
	{
		return sMatrix[y][x];
	}

	//����HS����
	float **getHMatrix()
	{
		return hMatrix;
	}

	float **getSMatrix()
	{
		return sMatrix;
	}

};