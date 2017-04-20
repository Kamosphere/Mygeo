
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
		//������һλ��at(0)���洢����������ڶ�λ��at(1)���洢ɫ����H����ֵ������λ��at(2)���洢���Ͷȣ�S����ֵ������λ��at(3)���洢���������ظ���������λ��at(4)���洢���λ������λ��at(5)���洢�ܳ�������λ��at(6)���洢ban��֮��洢�����ǵ�������
		//����λbanλȡֵ�� 0��Ĭ�ϣ�--��ban��1--ban�����ж�Ϊ�϶����Ƿ���
		maxCounter = MaxCounter;
		relationMatrix = new vector<float>[maxCounter + 1];
		marks = Marks;
		persize = 7;//������־��ǹ�ϵ�����У������֮ǰ�ĸ�����Ϣ�����������ڸ�Ϊ7
		Sumpoint = SUM;

		//���������Ȳ����߸� 0����֮��洢���������
		for (int i = 0; i < maxCounter + 1; i++)
		{
			for (int j = 0; j < persize; j++)//5->7
				relationMatrix[i].push_back(float(0));
		}

		//ȷ���ڽӹ�ϵ
		for (int i = 0; i < marks.rows; i++)
		{
			for (int j = 0; j < marks.cols; j++)
			{
				int index = marks.at<int>(i, j);

				std::vector<int> neigZones;
				std::vector<int>::iterator neigZonesIterator;
				int qContainer = -2;
				int numWshed = 0; //�������ж��ٸ��������ڷ�ˮ�룬���������1������2��֮��Ͳ��������������
				int container0 = 0; //ʵ���Ϸָ�����ı�Ŵ�1��ʼ
				int container1 = 0;

				//����õ��Ƿ�ˮ��
				if (marks.at<int>(i, j) == -1)
				{
					//�����İ�����x-j��y-i
					//��
					if (j - 1 >= 0)
					{
						// ������
						//���²����԰����������ظ�
						qContainer = marks.at<int>(i, j - 1);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
						{
							neigZones.push_back(qContainer);
						}
						if (i - 1 >= 0)	 // ���½�
						{
							qContainer = marks.at<int>(i - 1, j - 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
							{
								neigZones.push_back(qContainer);
							}
						}
						if (i + 1 < marks.rows)	 // ���Ͻ�
						{
							qContainer = marks.at<int>(i + 1, j - 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
							{
								neigZones.push_back(qContainer);
							}
						}
					}

					//��
					if (j + 1 <  marks.cols)
					{
						// ������
						qContainer = marks.at<int>(i, j + 1);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
						{
							neigZones.push_back(qContainer);
						}
						if (i - 1 >= 0)	 // ���½�
						{
							qContainer = marks.at<int>(i - 1, j + 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
							{
								neigZones.push_back(qContainer);
							}
						}
						if (i + 1 < marks.rows) // ���Ͻ�
						{
							qContainer = marks.at<int>(i + 1, j + 1);
							if (qContainer == -1)
							{
								numWshed++;
							}
							neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
							if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
							{
								neigZones.push_back(qContainer);
							}
						}
					}

					//����
					// ������
					if (i - 1 >= 0)
					{
						qContainer = marks.at<int>(i - 1, j);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
						{
							neigZones.push_back(qContainer);
						}
					}
					// ������
					if (i + 1 < marks.rows)
					{
						qContainer = marks.at<int>(i + 1, j);
						if (qContainer == -1)
						{
							numWshed++;
						}
						neigZonesIterator = find(neigZones.begin(), neigZones.end(), qContainer);
						if ((neigZonesIterator == neigZones.end()) && (qContainer != -1)) //������ص����������������ֵ��û����������Ҳ���-1
						{
							neigZones.push_back(qContainer);
						}
					}


					if ((numWshed > 0) && (numWshed < 3)) //���������ص�������ֻ��һ�����������������ڷ�ˮ�룬����������������ָ�����
					{
						if (neigZones.size() == 2) //���������ص����������ҽ������������ָ�����
						{
							container0 = neigZones.at(0);
							container1 = neigZones.at(1);
							int test0 = 0;
							int test1 = 0;

							// ��������ڽӹ�ϵ��relationship[container0]��ʾ������Ϊcontainer0�ķָ�������ڽ������ǵļ���
							//�¸Ķ�2017��3��30�� 18:12:31 ��ǰ������Ƿ�ˮ��㣬�������������������������ܳ�+1
							relationMatrix[container0].at(5) = relationMatrix[container0].at(5) + float(1);
							relationMatrix[container1].at(5) = relationMatrix[container0].at(5) + float(1);

							if (relationMatrix[container0].size() == persize) //���[container0 - 1]������û��ӹ�������Ϣ//�¸Ķ�2017��3��29�� 23:34:35 5->7
							{
								relationMatrix[container0].push_back(float(container1)); //��¼����µ�������
								relationMatrix[container0].at(0) = relationMatrix[container0].at(0) + float(1); //�������+1
							}
							else if (relationMatrix[container0].size() > persize) //���������ӹ�������Ϣ//�¸Ķ�2017��3��29�� 23:34:44 5->7
							{
								for (int k = persize; k < relationMatrix[container0].size(); ++k) //�ӵ���λ��ʼ����//�¸Ķ�2017��3��29�� 23:35:28 ��->��
								{
									if (relationMatrix[container0].at(k) == float(container1)) //����ͬ�ģ�����
									{
										break;
									}
									else if (k == relationMatrix[container0].size() - 1) //���ף�û��ͬ�ģ�����
									{
										relationMatrix[container0].push_back(float(container1)); //��¼����µ�������
										relationMatrix[container0].at(0) = relationMatrix[container0].at(0) + float(1); //�������+1
										break;
									}
								}
							}

							if (relationMatrix[container1].size() == persize) //���[container1 - 1]������û��ӹ�������Ϣ//�¸Ķ�2017��3��29�� 23:45:12 5->7
							{
								relationMatrix[container1].push_back(float(container0)); //��¼����µ�������
								relationMatrix[container1].at(0) = relationMatrix[container1].at(0) + float(1); //�������+1
							}
							else if (relationMatrix[container1].size() > persize) //���������ӹ�������Ϣ//�¸Ķ�2017��3��29�� 23:46:02 5->7
							{
								for (int k = persize; k < relationMatrix[container1].size(); ++k) //�ӵ���λ��ʼ����//�¸Ķ�2017��3��29�� 23:46:02 5->7
								{
									if (relationMatrix[container1].at(k) == float(container0)) //����ͬ�ģ�����
									{
										break;
									}
									else if (k == relationMatrix[container1].size() - 1) //���ף�û��ͬ�ģ�����
									{
										relationMatrix[container1].push_back(float(container0)); //��¼����µ�������
										relationMatrix[container1].at(0) = relationMatrix[container1].at(0) + float(1); //�������+1
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

	//��������HS��ֵ
	//����������ֻ��һ��
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
		//����ɫ�����ͶȾ�ֵ
		for (int i = 0; i < maxCounter + 1; ++i)
		{
			if (relationMatrix[i].size() != persize)//�¸Ķ�2017��3��29�� 23:46:02 5->7
			{
				relationMatrix[i].at(1) = relationMatrix[i].at(1) / relationMatrix[i].at(3);
				relationMatrix[i].at(2) = relationMatrix[i].at(2) / relationMatrix[i].at(3);
			}
		}
	}

	//���ع�ϵ����relationMatrix
	vector<float> * getRelationMatrix()
	{
		return relationMatrix;
	}

	//�¸Ķ�2017��3��30�� 19:04:01 ��������size����ban
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

	//���ؾ���marks
	Mat getMarks()
	{
		return marks;
	}
};