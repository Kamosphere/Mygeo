
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

class RelationBox
{
private:
	vector<float> *relationMatrix;
	Mat marks;
	int maxCounter;
	int persize;
	int Sumpoint;
public:
	RelationBox(int MaxCounter, Mat Marks,int SUM)
	{
		//向量第一位（at(0)）存储邻域个数，第二位（at(1)）存储色调（H）均值，第三位（at(2)）存储饱和度（S）均值，第四位（at(3)）存储区域内像素个数，第五位（at(4)）存储标号位，第六位（at(5)）存储周长，第七位（at(6)）存储ban，之后存储邻域们的区域标号
		//第七位ban位取值： 0（默认）--不ban，1--ban，被判定为肯定不是房屋
		maxCounter = MaxCounter;
		relationMatrix = new vector<float>[maxCounter + 1];
		marks = Marks;
		persize = 7;//这个数字就是关系矩阵中，区域号之前的各种信息的数量，现在改为7
		Sumpoint = SUM;

		//所有向量先插入七个 0，七之后存储邻域区域号
		for (int i = 0; i < maxCounter + 1; i++)
		{
			for (int j = 0; j < persize; j++)//5->7
				relationMatrix[i].push_back(float(0));
		}

		//确定邻接关系
		for (int i = 0; i < marks.rows; i++)
		{
			for (int j = 0; j < marks.cols; j++)
			{
				int index = marks.at<int>(i, j);

				std::vector<int> neigZones;
				std::vector<int>::iterator neigZonesIterator;
				int qContainer = -2;
				int numWshed = 0; //邻域中有多少个像素属于分水岭，如果不等于1不等于2，之后就不考虑这个像素了
				int container0 = 0; //实际上分割区域的标号从1开始
				int container1 = 0;

				//如果该点是分水岭
				if (marks.at<int>(i, j) == -1)
				{
					//看他的八邻域，x-j，y-i
					//左
					if (j - 1 >= 0)
					{
						// 左邻域
						//以下操作对八邻域像素重复
						qContainer = marks.at<int>(i, j - 1);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
						{
							neigZones.push_back(qContainer);
						}
						if (i - 1 >= 0)	 // 左下角
						{
							qContainer = marks.at<int>(i - 1, j - 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
							{
								neigZones.push_back(qContainer);
							}
						}
						if (i + 1 < marks.rows)	 // 左上角
						{
							qContainer = marks.at<int>(i + 1, j - 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
							{
								neigZones.push_back(qContainer);
							}
						}
					}

					//右
					if (j + 1 <  marks.cols)
					{
						// 右邻域
						qContainer = marks.at<int>(i, j + 1);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
						{
							neigZones.push_back(qContainer);
						}
						if (i - 1 >= 0)	 // 右下角
						{
							qContainer = marks.at<int>(i - 1, j + 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
							{
								neigZones.push_back(qContainer);
							}
						}
						if (i + 1 < marks.rows) // 右上角
						{
							qContainer = marks.at<int>(i + 1, j + 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
							{
								neigZones.push_back(qContainer);
							}
						}
					}

					//上下
					// 下邻域
					if (i - 1 >= 0)
					{
						qContainer = marks.at<int>(i - 1, j);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
						{
							neigZones.push_back(qContainer);
						}
					}
					// 上邻域
					if (i + 1 < marks.rows)
					{
						qContainer = marks.at<int>(i + 1, j);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //这个像素的这个邻域的容器标号值还没被存过，并且不是-1
						{
							neigZones.push_back(qContainer);
						}
					}


					if ((numWshed > 0) && (numWshed < 3)) //如果这个像素的邻域中只有一个或者两个像素属于分水岭，它两侧可能是两个分割区域
					{
						if (neigZones.size() == 2) //如果这个像素的邻域中有且仅有两个归属分割区域
						{
							container0 = neigZones.at(0);
							container1 = neigZones.at(1);
							int test0 = 0;
							int test1 = 0;

							// 互相添加邻接关系，relationship[container0]表示区域标号为container0的分割区域的邻接区域们的集合
							//新改动2017年3月30日 18:12:31 当前这个点是分水岭点，两侧是两个区域，这两个区域周长+1
							relationMatrix[container0].at(5) = relationMatrix[container0].at(5) + float(1);
							relationMatrix[container1].at(5) = relationMatrix[container0].at(5) + float(1);

							if (relationMatrix[container0].size() == persize) //这个[container0 - 1]向量还没添加过邻域信息//新改动2017年3月29日 23:34:35 5->7
							{
								relationMatrix[container0].push_back(float(container1)); //记录这个新的邻域标号
								relationMatrix[container0].at(0) = relationMatrix[container0].at(0) + float(1); //邻域个数+1
							}
							else if (relationMatrix[container0].size() > persize) //这个向量添加过邻域信息//新改动2017年3月29日 23:34:44 5->7
							{
								for (int k = persize; k < relationMatrix[container0].size(); ++k) //从第六位开始遍历//新改动2017年3月29日 23:35:28 六->八
								{
									if (relationMatrix[container0].at(k) == float(container1)) //有相同的，跳出
									{
										break;
									}
									else if (k == relationMatrix[container0].size() - 1) //到底，没相同的，插入
									{
										relationMatrix[container0].push_back(float(container1)); //记录这个新的邻域标号
										relationMatrix[container0].at(0) = relationMatrix[container0].at(0) + float(1); //邻域个数+1
										break;
									}
								}
							}

							if (relationMatrix[container1].size() == persize) //这个[container1 - 1]向量还没添加过邻域信息//新改动2017年3月29日 23:45:12 5->7
							{
								relationMatrix[container1].push_back(float(container0)); //记录这个新的邻域标号
								relationMatrix[container1].at(0) = relationMatrix[container1].at(0) + float(1); //邻域个数+1
							}
							else if (relationMatrix[container1].size() > persize) //这个向量添加过邻域信息//新改动2017年3月29日 23:46:02 5->7
							{
								for (int k = persize; k < relationMatrix[container1].size(); ++k) //从第六位开始遍历//新改动2017年3月29日 23:46:02 5->7
								{
									if (relationMatrix[container1].at(k) == float(container0)) //有相同的，跳出
									{
										break;
									}
									else if (k == relationMatrix[container1].size() - 1) //到底，没相同的，插入
									{
										relationMatrix[container1].push_back(float(container0)); //记录这个新的邻域标号
										relationMatrix[container1].at(0) = relationMatrix[container1].at(0) + float(1); //邻域个数+1
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//计算区域HS均值
	//这个方法最多只用一次
	void CountHSAverage(float **hMatrix, float **sMatrix)
	{
		for (int i = 0; i < marks.rows; i++)
		{
			for (int j = 0; j < marks.cols; j++)
			{
				int index = marks.at<int>(i, j);

				if (index > -1)
				{
					relationMatrix[index].at(1) = relationMatrix[index].at(1) + hMatrix[i][j];
					relationMatrix[index].at(2) = relationMatrix[index].at(2) + sMatrix[i][j];

					relationMatrix[index].at(3) = relationMatrix[index].at(3) + float(1);
				}
			}
		}
		//计算色调饱和度均值
		for (int i = 0; i < maxCounter + 1; ++i)
		{
			if (relationMatrix[i].size() != persize)//新改动2017年3月29日 23:46:02 5->7
			{
				relationMatrix[i].at(1) = relationMatrix[i].at(1) / relationMatrix[i].at(3);
				relationMatrix[i].at(2) = relationMatrix[i].at(2) / relationMatrix[i].at(3);
			}
		}
	}

	//返回关系矩阵relationMatrix
	vector<float> * getRelationMatrix()
	{
		return relationMatrix;
	}

	//新改动2017年3月30日 19:04:01 根据区域size设置ban
	void setBanforSize()
	{
		for (int i = 0; i < maxCounter + 1; ++i)
		{
			if (relationMatrix[i].at(3) > (Sumpoint/500))
			{
				relationMatrix[i].at(6) = 1;
			}
		}
	}

	//返回矩阵marks
	Mat getMarks()
	{
		return marks;
	}
};