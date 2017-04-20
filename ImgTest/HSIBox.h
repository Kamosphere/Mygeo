
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
	//IplImage* 类型的源图像IplImage_src，用于得到其HSI
	IplImage* IplImage_src;
	//存色调的float矩阵，hMatrix[y][x]，x对应width，y对应height
	float **hMatrix;
	float **sMatrix;
public:
	HSIBox(Mat input)
	{
		IplImage_src = &IplImage(input);

		//H和S矩阵初始化
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

		//计算H矩阵S矩阵
		// 原始图像数据指针, HSI矩阵数据指针
		uchar* data;

		// rgb分量
		float img_r, img_g, img_b;
		float min_rgb;  // rgb分量中的最小值
		// HSI分量
		float fHue, fSaturation, fIntensity, fSaturation_show, fIntensity_show;

		for (int i = 0; i < IplImage_src->height; i++) //对应y
		{
			for (int j = 0; j < IplImage_src->width; j++) //对应x
			{
				data = cvPtr2D(IplImage_src, i, j, 0);
				img_b = *data;
				data++;
				img_g = *data;
				data++;
				img_r = *data;

				// Intensity分量[0, 1]
				fIntensity = (float)((img_b + img_g + img_r) / 3) / 255;
				fIntensity_show = (float)((img_b + img_g + img_r) / 3);

				// 得到RGB分量中的最小值
				float fTemp = img_r < img_g ? img_r : img_g;
				min_rgb = fTemp < img_b ? fTemp : img_b;
				// Saturation分量[0, 1]
				if (img_r + img_g + img_b == 0)
				{
					fSaturation = 0;
				}
				else
				{
					fSaturation = 1 - (float)(3 * min_rgb) / (img_r + img_g + img_b);
				}

				// 计算theta角
				float numerator = (img_r - img_g + img_r - img_b) / 2;
				float denominator = sqrt(
					pow((img_r - img_g), 2) + (img_r - img_b)*(img_g - img_b));

				// 计算Hue分量
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

	//返回图像中(x,y)处HS信息
	float getH(int x, int y)
	{
		return hMatrix[y][x];
	}

	float getS(int x, int y)
	{
		return sMatrix[y][x];
	}

	//返回HS矩阵
	float **getHMatrix()
	{
		return hMatrix;
	}

	float **getSMatrix()
	{
		return sMatrix;
	}

};