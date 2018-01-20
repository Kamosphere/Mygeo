#include<string>
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<fstream>
#include<thread>

#include"HSIBox.h"
#include"RelationBox.h"

using namespace std;
using namespace cv;

class processView{
public:
	static int viewer;
};
int processView::viewer = 0;

inline bool sw_ordering(const std::pair<double, double> a, const std::pair<double, double> b)//对数据对的第二项比大小
{
	return a.second < b.second;
}

class WatershedMain{
private:
	Mat sourceFile;
public:
	WatershedMain(Mat Q){
		sourceFile = Q.clone();
	}

/*
OPENCV二值化图像内孔洞填充 / 小区域去除
输入：
Mat& Src：源文件
Mat& Dst：目的文件
int AreaLimit：区域大小限制
int CheckMode：0代表去除黑区域，1代表去除白区域
int NeihborMode：0代表4邻域，1代表8邻域
输出：
Mat& Dst：目的文件
*/
void RemoveSmallRegion(Mat& Src, Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;       //记录除去的个数
	int BlackCount = 0;		//记录黑色区域数，针对本项目来说
	//记录每个像素点检验状态的标签，0代表未检查，1代表正在检查,2代表检查不合格（需要反转颜色），3代表检查合格或不需检查  
	Mat Pointlabel = Mat::zeros(Src.size(), CV_8UC1);

	if (CheckMode == 1)
	{
		//cout << "Mode: 去除小区域. ";
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] < 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}
	else
	{
		//cout << "Mode: 去除孔洞. ";
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] > 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}

	vector<Point2i> NeihborPos;  //记录邻域点位置  
	NeihborPos.push_back(Point2i(-1, 0));
	NeihborPos.push_back(Point2i(1, 0));
	NeihborPos.push_back(Point2i(0, -1));
	NeihborPos.push_back(Point2i(0, 1));
	if (NeihborMode == 1)
	{
		NeihborPos.push_back(Point2i(-1, -1));
		NeihborPos.push_back(Point2i(-1, 1));
		NeihborPos.push_back(Point2i(1, -1));
		NeihborPos.push_back(Point2i(1, 1));
	}
	else{} 
	int NeihborCount = 4 + 4 * NeihborMode;
	int CurrX = 0, CurrY = 0;
	//开始检测  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 0)
			{
				//********开始该点处的检查**********  
				vector<Point2i> GrowBuffer;                                      //堆栈，用于存储生长点  
				GrowBuffer.push_back(Point2i(j, i));
				Pointlabel.at<uchar>(i, j) = 1;
				int CheckResult = 0;                                               //用于判断结果（是否超出大小），0为未超出，1为超出  

				for (int z = 0; z<GrowBuffer.size(); z++)
				{

					for (int q = 0; q<NeihborCount; q++)                                      //检查四个邻域点  
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows)  //防止越界  
						{
							if (Pointlabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(Point2i(CurrX, CurrY));  //邻域点加入buffer  
								Pointlabel.at<uchar>(CurrY, CurrX) = 1;           //更新邻域点的检查标签，避免重复检查  
							}
						}
					}

				}
				if (GrowBuffer.size() > AreaLimit){ CheckResult = 2; BlackCount++; }                 //判断结果（是否超出限定的大小），1为未超出，2为超出  
				else { CheckResult = 1;   RemoveCount++; }
				for (int z = 0; z<GrowBuffer.size(); z++)                         //更新Label记录  
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					Pointlabel.at<uchar>(CurrY, CurrX) += CheckResult;
				}
				//********结束该点处的检查**********  
			}
		}
	}

	CheckMode = 255 * (1 - CheckMode);
	//开始反转面积过小的区域  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iData = Src.ptr<uchar>(i);
		uchar* iDstData = Dst.ptr<uchar>(i);
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 2)
			{
				iDstData[j] = CheckMode;
			}
			else if (iLabel[j] == 3)
			{
				iDstData[j] = iData[j];
			}
		}
	}

	cout << RemoveCount << " regions moved" << endl;
	cout << BlackCount << " regions saved" << endl;
}

/*
去除二值图像边缘的突出部
输入：
Mat& Src：源文件
Mat& Dst：目的文件
int uthreshold：突出部的宽度阈值
int vthreshold：突出部的高度阈值
int type：突出部的颜色，0表示黑色，1代表白色
输出：
Mat& Dst：目的文件
*/
void delete_jut(Mat& src, Mat& dst, int uthreshold, int vthreshold, int type)
{
	int threshold;
	src.copyTo(dst);
	int height = dst.rows;
	int width = dst.cols;
	int k;  //用于循环计数传递到外部  
	for (int i = 0; i < height - 1; i++)
	{
		uchar* p = dst.ptr<uchar>(i);
		for (int j = 0; j < width - 1; j++)
		{
			if (type == 0)
			{
				//行消除  
				if (p[j] == 255 && p[j + 1] == 0)
				{
					if (j + uthreshold >= width)
					{
						for (int k = j + 1; k < width; k++)
							p[k] = 255;
					}
					else
					{
						for (k = j + 2; k <= j + uthreshold; k++)
						{
							if (p[k] == 255) break;
						}
						if (p[k] == 255)
						{
							for (int h = j + 1; h < k; h++)
								p[h] = 255;
						}
					}
				}
				//列消除  
				if (p[j] == 255 && p[j + width] == 0)
				{
					if (i + vthreshold >= height)
					{
						for (k = j + width; k < j + (height - i)*width; k += width)
							p[k] = 255;
					}
					else
					{
						for (k = j + 2 * width; k <= j + vthreshold*width; k += width)
						{
							if (p[k] == 255) break;
						}
						if (p[k] == 255)
						{
							for (int h = j + width; h < k; h += width)
								p[h] = 255;
						}
					}
				}
			}
			else
			{
				//行消除  
				if (p[j] == 0 && p[j + 1] == 255)
				{
					if (j + uthreshold >= width)
					{
						for (int k = j + 1; k < width; k++)
							p[k] = 0;
					}
					else
					{
						for (k = j + 2; k <= j + uthreshold; k++)
						{
							if (p[k] == 0) break;
						}
						if (p[k] == 0)
						{
							for (int h = j + 1; h < k; h++)
								p[h] = 0;
						}
					}
				}
				//列消除  
				if (p[j] == 0 && p[j + width] == 255)
				{
					if (i + vthreshold >= height)
					{
						for (k = j + width; k < j + (height - i)*width; k += width)
							p[k] = 0;
					}
					else
					{
						for (k = j + 2 * width; k <= j + vthreshold*width; k += width)
						{
							if (p[k] == 0) break;
						}
						if (p[k] == 0)
						{
							for (int h = j + width; h < k; h += width)
								p[h] = 0;
						}
					}
				}
			}
		}
	}
}

/*
图片边缘光滑处理
输入：
Mat& Src：源文件
Mat& Dst：目的文件
Size size：取均值的窗口大小
int threshold：对均值图像进行二值化的阈值
输出：
Mat& Dst：目的文件
*/
void imageblur(Mat& src, Mat& dst, Size size, int threshold)
{
	int height = src.rows;
	int width = src.cols;
	blur(src, dst, size);
	for (int i = 0; i < height; i++)
	{
		uchar* p = dst.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			if (p[j] < threshold)
				p[j] = 0;
			else p[j] = 255;
		}
	}
}

/*
生成随机颜色函数
输入：
int value：任意随机数
输出：
Vec3b：某个RGB颜色
*/
Vec3b RandomColor(int value)    //
{
	value = value % 255;  //生成0~255的随机数
	RNG rng;
	int aa = rng.uniform(0, value);
	int bb = rng.uniform(0, value);
	int cc = rng.uniform(0, value);
	return Vec3b(aa, bb, cc);
}

/*
由区域关系矩阵计算参数矩阵
输入：
vector<float> *BBmatrix：区域关系矩阵
int areaCount：区域总数
输出：
vector<pair<float, float>> AutoMatrix：参数矩阵，[0]:相似度 [1]：相对欧氏距离
*/
vector<pair<float, float>> seedAreaParaCal(vector<float> *BBmatrix, int areaCount) 
{
	vector<pair<float, float>> AutoMatrix;
	const int neinVar = 7;//向量常数位persize
	vector<float> tempNeiHueArea;//邻接区域色调分量temp
	vector<float> tempNeiSatArea;//邻接区域饱和度分量temp
	float simh = 0, sims = 0, avgh = 0, avgs = 0;//avg：平均
	vector<float> Ds;//欧氏距离，取色调分量计算

	for (int i = 0; i < areaCount; i++)
	{
		int neiArea = (int)BBmatrix[i][0];//邻接区域个数
		if (neiArea == 0 && BBmatrix[i][6] != 1){
			cout << i << " no similarity " << " no maxinum Ds " << endl;
		}
		else{
			tempNeiHueArea.resize(neiArea);
			tempNeiSatArea.resize(neiArea);
			Ds.resize(neiArea);
			//统计每个区域的邻接区域的色调均值和饱和度均值，求所有区域的均值
			for (int j = 0; j < neiArea; j++)
			{
				int ArealistTemp = (int)BBmatrix[i][neinVar+j];//邻接区域号
				tempNeiHueArea[j] = BBmatrix[ArealistTemp][1];
				avgh = avgh + tempNeiHueArea[j];
				tempNeiSatArea[j] = BBmatrix[ArealistTemp][2];
				avgs = avgs + tempNeiSatArea[j];
			}
			avgh = (avgh + BBmatrix[i][1]) / (neiArea + 1);
			avgs = (avgs + BBmatrix[i][2]) / (neiArea + 1);
			//计算相似度和欧氏距离
			float similar = 0, DsMaxtemp = 0;//相似度,最大欧氏距离
			for (int j = 0; j < neiArea; j++)
			{

				simh = simh + pow(tempNeiHueArea[j] - avgh, 2);
				sims = sims + pow(tempNeiSatArea[j] - avgs, 2);

				if (tempNeiHueArea[j] == 0)
					Ds[j] = 0;
				else Ds[j] = abs((tempNeiHueArea[j] - avgh) / tempNeiHueArea[j]);

				if (Ds[j]>DsMaxtemp)
				{
					DsMaxtemp = Ds[j];
				}
			}
			simh = sqrt(simh / neiArea + 1);
			sims = sqrt(sims / neiArea + 1);
			similar = 0.8*simh + 0.2*sims;
			cout << i << " similarity  " << similar << " maximum Ds " << DsMaxtemp << endl;
			AutoMatrix.push_back(pair<float, float>(similar, DsMaxtemp));
		}
	}
	return AutoMatrix;
}

/*
大津思想求阈值
输入：
vector<pair<float, float>> AutoMatrix：参数矩阵，[0]:相似度 [1]：相对欧氏距离
输出：
double threshold：阈值
*/
double similarOtsu(vector<pair<float, float>> AutoMatrix){
	int areaCount = AutoMatrix.size();
	vector<float> pixelPro(areaCount, -1);
	vector<int> pixelCount(areaCount, -1);
	vector<bool> pixelFlag(areaCount, false);
	float j = 0, i = 0, threshold = 0;

	//计算每个相似度占数据的比例
	for (int a = 0; a < areaCount; a++)
	{
		if (pixelFlag[a])
			break;
		else
		{
			for (int b = 0; b<areaCount; b++)
			{
				if (AutoMatrix[a].first == AutoMatrix[b].first)
				{
					pixelFlag[b] = true;
					pixelCount[a]++;
				}
			}
			pixelPro[a] = ((double)pixelCount[a] + 1) / areaCount;
		}
	}

	//遍历相似度,寻找合适的threshold
	float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax = 0;
	for (int b = 0; b < areaCount; b++)
	{
		if (pixelCount[b] == -1)
			break;
		j = AutoMatrix[b].first;
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0;
		for (int c = 0; c < areaCount; c++)
		{
			if (pixelCount[c] == -1)
				break;
			i = AutoMatrix[c].first;
			if (j <= i)
			{
				w0 += pixelPro[c];
				u0tmp += i * pixelPro[c];
			}
			else
			{
				w1 += pixelPro[c];
				u1tmp += i * pixelPro[c];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		deltaTmp = (float)(w0 *w1* pow((u0 - u1), 2));
		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = j;
		}
	}
	return threshold;
}

/*
对区域进行合并
输入：
vector<float> *BBmatrix：区域关系矩阵
int jointedAreaNum：被合并的区域标号
int targetAreaSeedNum：合并主体的区域标号
输出：
vector<float> *BBmatrix：区域关系矩阵
*/
void addMarkToSubTempArea(int jointedAreaNum, int targetAreaSeedNum, vector<float> *BBmatrix){
	const int neinVar = 7;//向量常数位persize
	if (BBmatrix[jointedAreaNum][0] >= 0 && BBmatrix[targetAreaSeedNum][0] >= 0 && BBmatrix[jointedAreaNum][6] != 1 && BBmatrix[targetAreaSeedNum][6] != 1){//约定不被合并过的区域一定为正的
		vector<float> jointArea;
		vector<float> targetArea;
		//取得两个区域的邻接区域列表 以用来去重
		for (int i = 0; i<BBmatrix[jointedAreaNum][0]; i++){
			if (BBmatrix[jointedAreaNum][i + neinVar] != targetAreaSeedNum)
				jointArea.push_back(BBmatrix[jointedAreaNum][i + neinVar]);
		}
		for (int i = 0; i<BBmatrix[targetAreaSeedNum][0]; i++){
			if (BBmatrix[targetAreaSeedNum][i + neinVar] != jointedAreaNum)
				targetArea.push_back(BBmatrix[targetAreaSeedNum][i + neinVar]);
		}
		//保序去重复
		BBmatrix[targetAreaSeedNum].insert(BBmatrix[targetAreaSeedNum].end(), jointArea.begin(), jointArea.end());
		vector<float>::iterator it, it1;
		for (it = ++BBmatrix[targetAreaSeedNum].begin() + neinVar; it != BBmatrix[targetAreaSeedNum].end();)
		{
			it1 = find(BBmatrix[targetAreaSeedNum].begin() + neinVar, it, *it);
			if (it1 != it)
				it = BBmatrix[targetAreaSeedNum].erase(it);
			else
				it++;
		}
		//计算参数
		//第一位（at(0)）存储邻域个数，第二位（at(1)）存储色调（H）均值，第三位（at(2)）存储饱和度（S）均值，第四位（at(3)）存储区域内像素个数，第五位（at(4)）存储标号位
		float newAreaPara[5];
		newAreaPara[0] = BBmatrix[targetAreaSeedNum].size() - neinVar;
		newAreaPara[3] = BBmatrix[targetAreaSeedNum][3] + BBmatrix[jointedAreaNum][3];
		newAreaPara[1] = (BBmatrix[targetAreaSeedNum][1] * BBmatrix[targetAreaSeedNum][3] + BBmatrix[jointedAreaNum][1] * BBmatrix[jointedAreaNum][3]) / (newAreaPara[3]);
		newAreaPara[2] = (BBmatrix[targetAreaSeedNum][2] * BBmatrix[targetAreaSeedNum][3] + BBmatrix[jointedAreaNum][2] * BBmatrix[jointedAreaNum][3]) / (newAreaPara[3]);
		if (BBmatrix[targetAreaSeedNum][4] == 0){
			newAreaPara[4] = BBmatrix[jointedAreaNum][4];
		}
		else {
			newAreaPara[4] = BBmatrix[targetAreaSeedNum][4];
		}

		BBmatrix[jointedAreaNum][4] = newAreaPara[4];
		BBmatrix[jointedAreaNum][0] = BBmatrix[jointedAreaNum][0] * (-1);

		BBmatrix[targetAreaSeedNum][0] = newAreaPara[0];
		BBmatrix[targetAreaSeedNum][1] = newAreaPara[1];
		BBmatrix[targetAreaSeedNum][2] = newAreaPara[2];
		BBmatrix[targetAreaSeedNum][3] = newAreaPara[3];
		BBmatrix[targetAreaSeedNum][4] = newAreaPara[4];
	}
}

/*
对区域进行合并
输入：
vector<float> *BBmatrix：区域关系矩阵，其中：
第一位（at(0)）存储邻域个数，第二位（at(1)）存储色调（H）均值，
第三位（at(2)）存储饱和度（S）均值，第四位（at(3)）存储区域内像素个数，
第五位（at(4)）存储标号位，第六位（at(5)）存储周长，
第七位（at(6)）存储ban，0（默认）--不ban、1--ban、被判定为肯定不是房屋，之后存储邻域们的区域标号
int areaCount：所有区域数
int realAreaCount：实际区域数（对角黑块和ban区域去除）
int sumPixelSize：图像总像素数（暂时未用）
输出：
vector<int> seedArea：区域合并后的矩阵
*/
vector<int> regionalGrowth(vector<float> *BBmatrix, int areaCount, int realAreaCount, int sumPixelSize){
	const int neinVar = 7;//向量常数位persize
	vector<pair<float, float>> AutoMatrix = seedAreaParaCal(BBmatrix, areaCount);
	float threshold = similarOtsu(AutoMatrix);
	int seedCount = 1;
	vector<int> seedArea;//用来对原始种子区域做索引
	for (int i = 0; i<AutoMatrix.size(); i++){
		if (AutoMatrix[i].first<threshold&&AutoMatrix[i].second<0.1&&BBmatrix[i][6]!=1){//相似度小于阈值，欧氏距离小于阈值，不在屏蔽列表
			seedArea.push_back(i);//数组位
			BBmatrix[i][4] = seedCount;//区域标号
			//cout << seedCount << endl;
			seedCount++;
		}
	}
	for (int i = 0; i < seedArea.size(); i++){
		cout << seedArea.at(i) << " is seed" << endl;
	}

	//区域生长
	for (int i = 0; i<seedArea.size(); i++){
		int seedTemp = seedArea[i];
		int neiArea = (int)BBmatrix[seedTemp][0];//种子区域seedtemp的邻接区域个数
		//cout << i << "号种子区域 " << endl;
		for (int j = 0; j<neiArea; j++){
			//cout << j << "号邻接区域 " << endl;
			int subTempArea = (int)BBmatrix[seedTemp][neinVar + j];//seedtemp中的邻接区域之一subTempArea
			int ifSeedArea = 0;
			for (int k = 0; k<seedArea.size(); k++){
				if (subTempArea == seedArea[i])
					ifSeedArea = 1;
			}
			if (ifSeedArea == 0 && BBmatrix[seedTemp].at(6) != 1 && BBmatrix[subTempArea].at(6) != 1){
				int subNeiArea = (int)BBmatrix[subTempArea][0];//区域subTempArea的邻接区域个数
				vector<pair<float, float>> markedAreaPair;
				if (subNeiArea > 0 && neiArea > 0){//约定不被合并过的区域一定为正的
					for (int k = 0; k<subNeiArea; k++){
						int sub2 = (int)BBmatrix[subTempArea][neinVar + k];
						if (BBmatrix[sub2][4] != 0 && BBmatrix[sub2][6] != 1){
							float addin1 = BBmatrix[subTempArea][neinVar+ k];
							float addin2 = BBmatrix[sub2][4];
							//cout << k << "对序号 " << BBmatrix[sub2][4] << endl;
							markedAreaPair.push_back(pair<float, float>(addin1, addin2));//subTempArea的已标记的邻接区域
						}
					}
					sort(markedAreaPair.begin(), markedAreaPair.end(), sw_ordering);
					vector<int> markedAreaVari;
					int sentinal = markedAreaPair[0].second;
					markedAreaVari.push_back(sentinal);
					for (int k = 1; k<markedAreaPair.size(); k++){
						if (sentinal != markedAreaPair[k].second&&markedAreaPair[k].second != 0){
							sentinal = markedAreaPair[k].second;
							markedAreaVari.push_back(sentinal);
						}
					}
					if (markedAreaVari.size() == 1){//标记过的区域 标记全相同
						int labelNum = (int)seedArea[markedAreaVari[0] - 1];
						if (abs(BBmatrix[labelNum][1] - BBmatrix[subTempArea][1])<0.02){
							//cout << markedAreaVari[0] << "unique region " << endl;
							addMarkToSubTempArea( subTempArea, seedArea[markedAreaVari[0] - 1], BBmatrix);//一个将subTempArea添加到seedArea[i]的函数
						}

					}
					else if (markedAreaVari.size()>1){//标记过的区域 标记不同
						int bestAreaAdd = 0;
						float minAreaHue = 65535;
						for (int k = 0; k<markedAreaVari.size(); k++){
							int labelNum = (int)seedArea[markedAreaVari[k] - 1];
							if (abs(BBmatrix[labelNum][1] - BBmatrix[subTempArea][1])<minAreaHue){
								minAreaHue = abs(BBmatrix[labelNum][1] - BBmatrix[subTempArea][1]);
								bestAreaAdd = markedAreaVari[k];
							}
						}
						if (minAreaHue < 0.02){
							addMarkToSubTempArea(subTempArea, seedArea[bestAreaAdd - 1], BBmatrix);//种子区域的序号；被添加的区域序号；添加入的种子区域序号，区域关系矩阵
						}
					}
					neiArea = (int)BBmatrix[seedTemp][0];//重新获得邻接区域个数
				}
			}
		}
	}
	
	//区域生长2
	for (int i = 0; i < seedArea.size(); i++){
		int seedTemp = seedArea[i];
		for (int j = 0; j<areaCount; j++){
			if (BBmatrix[j][0]>0 && BBmatrix[j][4] == 0)//约定不被合并过的区域一定为正的,优先合并不属于种子区域的区域
			{
				if (abs(BBmatrix[seedTemp][1] - BBmatrix[j][1]) < 0.02&& BBmatrix[seedTemp][6] != 1 && BBmatrix[j][6] != 1){
					addMarkToSubTempArea(j, seedTemp, BBmatrix);
				}
			}
		}
	}
	
	//考虑对合并进去的区域，置邻接区域数量为负，以屏蔽检索
	//区域合并
	for (int i = 0; i<seedArea.size(); i++){
		int seedTemp = seedArea[i];
		int neiArea = (int)BBmatrix[seedTemp][0];//种子区域seedtemp的邻接区域个数
		for (int j = 0; j<neiArea; j++){
			int subTempArea = (int)BBmatrix[seedTemp][neinVar + j];//seedtemp中的邻接区域之一subTempArea
			if (BBmatrix[subTempArea][0] > 0 && neiArea > 0 && BBmatrix[subTempArea][6] != 1){//约定不被合并过的区域一定为正的
				if (abs(BBmatrix[seedTemp][1] - BBmatrix[subTempArea][1])<0.04&&abs(BBmatrix[seedTemp][2] - BBmatrix[subTempArea][2])<0.05){
					addMarkToSubTempArea( subTempArea, seedTemp, BBmatrix);//种子区域的序号；被添加的区域序号；添加入的种子区域序号，区域关系矩阵
				}
			}
			neiArea = (int)BBmatrix[seedTemp][0];//重新获得邻接区域个数
		}
	}

	for (int i = 0; i<areaCount; i++){
		if (BBmatrix[i][0]>0 && BBmatrix[i][3]<150){//大于零保证其没有被合并，小于sumPixelSize/150（需定义或传参）保证足够小
			int neiArea = (int)BBmatrix[i][0];
			int minHue = 65535;
			int bestAreaAdd = -1;
			for (int j = 0; j<neiArea; j++){
				int subTempArea = (int)BBmatrix[i][neinVar + j];
				if (BBmatrix[subTempArea][0] > 0 && neiArea > 0 && BBmatrix[subTempArea][6] != 1){//约定不被合并过的区域一定为正的
					if (abs(BBmatrix[i][1] - BBmatrix[subTempArea][1])<minHue){
						if (BBmatrix[i][4] != 0 && BBmatrix[subTempArea][4] != 0){}//不同时为种子区域（不然上一步为何不合并）
						else
						{
							minHue = abs(BBmatrix[i][1] - BBmatrix[subTempArea][1]);
							bestAreaAdd = subTempArea;
						}
					}
				}
			}
			if (bestAreaAdd != -1){
				addMarkToSubTempArea( i, bestAreaAdd, BBmatrix);
			}
		}
	}

	//输出合并后信息
	int ii=0,jj = 0;
	for (int i = 0; i < areaCount; i++)
	{
		if (BBmatrix[i][4]>0&&BBmatrix[i][3]>100){
			cout << "special container " << i << " : ";
			ii++;
			for (int j = 0; j < BBmatrix[i].size(); j++)
				cout << BBmatrix[i].at(j) << ' ';
			cout << endl;
		}
	}
	cout << ii << "'s special " << threshold << endl;
	for (int i = 0; i < areaCount; i++)
	{
		if (BBmatrix[i][0]>0 && BBmatrix[i][4]>0){
			cout << "core container " << i  << " : ";
			jj++;
			for (int j = 0; j < BBmatrix[i].size(); j++)
				cout << BBmatrix[i].at(j) << ' ';
			cout << endl;
		}
	}
	cout << jj << "'s core " << threshold << endl;
	cout << seedCount-1 << " seeds" << endl;
	for (int i = 0; i < areaCount; i++)
	{
		if (BBmatrix[i][6]==1){
			cout << "ban container " << i << " : ";
			//jj++;
			for (int j = 0; j < BBmatrix[i].size(); j++)
				cout << BBmatrix[i].at(j) << ' ';
			cout << endl;
		}
	}
	return seedArea;
}

/*
得到sobel轮廓，并对该轮廓二值化、去除小斑块
输入：
Mat Src：meanshift处理后的一个Mat
输出：
写入磁盘BSobel0503_Dwhite_35.jpg
return：
marks：二值化sobel轮廓，并且已经去除了像素小于35的轮廓，内容等同BSobel0503_Dwhite_35.jpg
*/
Mat Sobel_Contour(Mat Src)
{
	Mat dst_x, dst_y;
	Mat imageGray;

	Sobel(Src, dst_x, Src.depth(), 1, 0); //X方向梯度
	Sobel(Src, dst_y, Src.depth(), 0, 1); //Y方向梯度

	convertScaleAbs(dst_x, dst_x);
	convertScaleAbs(dst_y, dst_y);

	addWeighted(dst_x, 0.5, dst_y, 0.5, 0, imageGray);//合并梯度(近似)

	Mat marks(Src.size(), 0);   //Opencv分水岭第二个矩阵参数
	for (int i = 0; i < imageGray.rows; i++)
	{
		for (int j = 0; j < imageGray.cols; j++)
		{
			uchar index = imageGray.at<uchar>(i, j);
			if ((int)index > 25)
			{
				marks.at<uchar>(i, j) = 255;
			}

		}
	}
	//此处调用去除小斑块填充空洞函数，去黑阀值0，去白阀值35
	RemoveSmallRegion(marks, marks, 0, 0, 1);//去除黑
	RemoveSmallRegion(marks, marks, 35, 1, 0);//去除白

	imwrite("BSobel0503_Dwhite_35.jpg", marks);
	return marks;
}

/*
实现jpg与png叠加的函数
输入：
Mat& Src：源文件
Mat& Dst：目的文件
double scale：尺度
输出：
Mat& Dst：目的文件
*/
//
int cvAdd4cMat_q(cv::Mat &Dst, cv::Mat &Src, double scale)
{
	if (Dst.channels() != 3 || Src.channels() != 4)
	{
		return true;
	}
	if (scale < 0.01)
		return false;
	std::vector<cv::Mat>scr_channels;
	std::vector<cv::Mat>dstt_channels;
	split(Src, scr_channels);
	split(Dst, dstt_channels);
	CV_Assert(scr_channels.size() == 4 && dstt_channels.size() == 3);

	if (scale < 1)
	{
		scr_channels[3] *= scale;
		scale = 1;
	}
	for (int i = 0; i < 3; i++)
	{
		dstt_channels[i] = dstt_channels[i].mul(255.0 / scale - scr_channels[3], scale / 255.0);
		dstt_channels[i] += scr_channels[i].mul(scr_channels[3], scale / 255.0);
	}
	merge(dstt_channels, Dst);
	return true;
}

/*
按hu的原理5*5拟合边缘
输入：
Mat smooth：之前步骤中对提取结果进行平滑，去除阀值为3000的黑色斑块，填充阀值为50的白色空洞的Mat
输出：
写入磁盘Hu _smooth_5.jpg.jpg
return：
Operating_smooth：5*5边缘拟合后的Mat，内容等同Hu _smooth_5.jpg.jpg
*/
Mat Hu_Smooth(Mat smooth)
{
	Mat Operating_smooth;
	convertScaleAbs(smooth, Operating_smooth);

	uchar smooth_p = 0;
	uchar operating_p = 0;

	for (int i = 0; i < smooth.rows; i++)
	{
		for (int j = 0; j < smooth.cols; j++)
		{
			smooth_p = smooth.at<uchar>(i, j);
			//找边界点，并记录边界点邻域黑白比
			int white = 0;
			int black = 0;
			//左左
			if (j - 2 >= 0)
			{
				operating_p = smooth.at<uchar>(i, j - 2);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//左左下
				if (i - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 1, j - 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//左下左下
				if (i - 2 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 2, j - 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//左左上
				if (i + 1 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 1, j - 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//左上左上
				if (i + 2 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 2, j - 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//右右
			if (j + 2 < smooth.cols)
			{
				operating_p = smooth.at<uchar>(i, j + 2);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//右右下
				if (i - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 1, j + 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//右下右下
				if (i - 2 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 2, j + 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//右右上
				if (i + 1 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 1, j + 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//右上右上
				if (i + 2 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 2, j + 2);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//下下
			if (i - 2 >= 0)
			{
				operating_p = smooth.at<uchar>(i - 2, j);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//下左下
				if (j - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 2, j - 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//下右下
				if (j + 1 < smooth.cols)
				{
					operating_p = smooth.at<uchar>(i - 2, j + 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//上上
			if (i + 2 < smooth.rows)
			{
				operating_p = smooth.at<uchar>(i + 2, j);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//上左上
				if (j - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i + 2, j - 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//上右上
				if (j + 1 < smooth.cols)
				{
					operating_p = smooth.at<uchar>(i + 2, j + 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//左
			if (j - 1 >= 0)
			{
				operating_p = smooth.at<uchar>(i, j - 1);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//左下
				if (i - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 1, j - 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//左上
				if (i + 1 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 1, j - 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//右
			if (j + 1 < smooth.cols)
			{
				operating_p = smooth.at<uchar>(i, j + 1);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
				//右下
				if (i - 1 >= 0)
				{
					operating_p = smooth.at<uchar>(i - 1, j + 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
				//右上
				if (i + 1 < smooth.rows)
				{
					operating_p = smooth.at<uchar>(i + 1, j + 1);
					if ((int)operating_p == 255)
						white++;
					if ((int)operating_p == 0)
						black++;
				}
			}
			//下
			if (i - 1 >= 0)
			{
				operating_p = smooth.at<uchar>(i - 1, j);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
			}
			//上
			if (i + 1 < smooth.rows)
			{
				operating_p = smooth.at<uchar>(i + 1, j);
				if ((int)operating_p == 255)
					white++;
				if ((int)operating_p == 0)
					black++;
			}
			//比较黑白数，确定边界点，如果一个边界点八邻域内白大于等于5，该边界点涂白；如果白小于3，整个八邻域涂黑
			if (black != 0 && white != 0)
			{
				if (white >= 15)
				{
					Operating_smooth.at<uchar>(i, j) = 255;
				}
				if (white < 13)
				{
					Operating_smooth.at<uchar>(i, j - 1) = 0;
					Operating_smooth.at<uchar>(i - 1, j - 1) = 0;
					Operating_smooth.at<uchar>(i + 1, j - 1) = 0;
					Operating_smooth.at<uchar>(i, j + 1) = 0;
					Operating_smooth.at<uchar>(i - 1, j + 1) = 0;
					Operating_smooth.at<uchar>(i + 1, j + 1) = 0;
					Operating_smooth.at<uchar>(i - 1, j) = 0;
					Operating_smooth.at<uchar>(i + 1, j) = 0;

					operating_p = smooth.at<uchar>(i, j - 2);
					operating_p = smooth.at<uchar>(i - 1, j - 2);
					operating_p = smooth.at<uchar>(i - 2, j - 2);
					operating_p = smooth.at<uchar>(i + 1, j - 2);
					operating_p = smooth.at<uchar>(i + 2, j - 2);

					operating_p = smooth.at<uchar>(i, j + 2);
					operating_p = smooth.at<uchar>(i - 1, j + 2);
					operating_p = smooth.at<uchar>(i - 2, j + 2);
					operating_p = smooth.at<uchar>(i + 1, j + 2);
					operating_p = smooth.at<uchar>(i + 2, j + 2);

					operating_p = smooth.at<uchar>(i - 2, j);
					operating_p = smooth.at<uchar>(i - 2, j - 1);
					operating_p = smooth.at<uchar>(i - 2, j + 1);

					operating_p = smooth.at<uchar>(i + 2, j);
					operating_p = smooth.at<uchar>(i + 2, j - 1);
					operating_p = smooth.at<uchar>(i + 2, j + 1);
				}
			}
		}
	}
	imwrite("Hu _smooth_5.jpg", Operating_smooth);
	return Operating_smooth;
}

/*
把进行了一系列平滑、去除、拟合后的提取结果与sobel轮廓叠加，之后再与原图叠加，作为输出结果
输入：
Mat top：之前步骤中Sobel_Contour函数的返回值
Mat bp：之前步骤中Hu_Smooth函数的返回值
Mat or：系统输入的jpg原图
输出：
写入磁盘mg_sobel0505_Dwhite_35_transparent.png 是提取结果为红色，轮廓为白色，其他为透明的png图片
mg_sobel0505_Dwhite_35_Hu_final.png 是上图和原图叠加后的最终结果图
return：
0
*/
Mat Merge_Contour_Extract(Mat top, Mat bp, Mat or)
{
	Mat real_out_8UC4(bp.size(), CV_8UC4);

	uchar top_p;
	uchar bp_p;
	//BGR
	Vec4b v4_transparent(255, 255, 255, 0);
	Vec4b v4_white(255, 255, 255, 255);
	Vec4b v4_Red(0, 0, 255, 180);

	for (int i = 0; i < bp.rows; i++)
	{
		for (int j = 0; j < bp.cols; j++)
		{
			top_p = top.at<uchar>(i, j);
			bp_p = bp.at<uchar>(i, j);
			if ((int)bp_p == 255)//背景图里是白色
			{
				real_out_8UC4.at<Vec4b>(i, j) = v4_transparent;
			}
			if ((int)bp_p == 0)//背景图里是黑色
			{
				real_out_8UC4.at<Vec4b>(i, j) = v4_Red;
			}
			if ((int)top_p == 255 && (int)bp_p == 0)//轮廓图是白色，背景图是黑色
			{
				real_out_8UC4.at<Vec4b>(i, j) = v4_transparent;
				if (j + 1 < bp.cols)//右
				{
					real_out_8UC4.at<Vec4b>(i, j + 1) = v4_transparent;
					if (i - 1 >= 0)//右下
					{
						real_out_8UC4.at<Vec4b>(i + 1, j + 1) = v4_transparent;
					}
				}
				if (i - 1 >= 0)//下
				{
					real_out_8UC4.at<Vec4b>(i, j + 1) = v4_transparent;
				}
			}
		}
	}
	imwrite("mg_sobel0505_Dwhite_35_transparent.png", real_out_8UC4);
	Mat finaler = or.clone();
	cvAdd4cMat_q(finaler, real_out_8UC4, 1.0);
	imwrite("mg_sobel0505_Dwhite_35_Hu_final.png", finaler);
	return finaler;
}

/*
把分水岭轮廓(黑色线条)和最终结果叠加，用来判断总体精度
输入：
Mat top：执行watershed()之后的Mat marks，注意该Mat单通道
Mat bp：叠加了透明png和原图的最终结果图jpg，注意该Mat三通道
输出：
写入磁盘for_count_0506.jpg 是分水岭轮廓和最终结果叠加图
return：
三通道Mat for_count
*/
Mat Merge_Water_Final(Mat top,Mat bp)
{
	uchar top_p;
	Vec3b top_v;
	uchar bp_p;

	Mat for_count;

	Vec3b v3_black(0, 0, 0);
	Vec3b v3_white(255, 255, 255);

	convertScaleAbs(bp, for_count);

	for (int i = 0; i < bp.rows; i++)
	{
		for (int j = 0; j < bp.cols; j++)
		{
			top_p = top.at<uchar>(i, j);
			if ((int)top_p == 0)
			{
				for_count.at<Vec3b>(i, j) = v3_black;
			}
		}
	}
	imwrite("for_count_0506.jpg", for_count);
	return for_count;
}

/*
主方法入口
*/
int mainMethod(){
	int step1 = mainMeanshift();
	int step2 = mainAreaFind();
	int step3 = mainFinalProcess();
	return (step1 + step2 + step3);
}

/*
完成Meanshift
*/
int mainMeanshift()
{
	if (sourceFile.empty())
	{
		cout << "Can't read image" << endl;
		return -1;
	}
	imwrite("imgTemp.jpg", sourceFile);
	string MSimg = "aftermeanshift.jpg";

	//输出内容到文件
	fstream fs;
	fs.open("log.txt", ios_base::out | ios_base::trunc);
	cout.rdbuf(fs.rdbuf());
	IplImage* src0;  //原图像
	IplImage* dst0;  //meanshift后图像
	int spatialRad = 25, colorRad = 20, maxPryLevel = 2;

	src0 = cvLoadImage("imgTemp.jpg");;//读入
	CvSize size;
	size.width = src0->width;
	size.height = src0->height;
	dst0 = cvCreateImage(size, src0->depth, 3);  //设置目标图像尺寸

	cvPyrMeanShiftFiltering(src0, dst0, spatialRad, colorRad, maxPryLevel);
	cvSaveImage(MSimg.c_str(), dst0);
	fs.close();

	return 0;
}

/*
完成分水岭+种子生长
*/
int mainAreaFind(){
	/*opencv watershed*/
	processView::viewer++;
	fstream fs;
	fs.open("log.txt", ios_base::out | ios_base::app);
	cout.rdbuf(fs.rdbuf());

	string MSimg = "aftermeanshift.jpg";
	Mat image = imread(MSimg);    //载入RGB彩色图像

	//灰度化，滤波，Canny边缘检测
	Mat imageGray;
	cvtColor(image, imageGray, CV_RGB2GRAY);//灰度转换
	Canny(imageGray, imageGray, 80, 150);

	//查找轮廓
	vector<vector<Point>> contours(50000);
	vector<Vec4i> hierarchy(50000);
	findContours(imageGray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point());
	Mat imageContours = Mat::zeros(image.size(), CV_8UC1);  //轮廓	
	Mat marks(image.size(), CV_32S);   //Opencv分水岭第二个矩阵参数
	marks = Scalar::all(0);
	//cout << hierarchy[1][0] << endl;
	int compCount = 0;
	for (int index = 0; index >= 0; index = hierarchy[index][0], compCount++)
	{
		//对marks进行标记，对不同区域的轮廓进行编号，相当于设置注水点，有多少轮廓，就有多少注水点
		drawContours(marks, contours, index, Scalar::all(compCount + 1), 1, 8, hierarchy);
		drawContours(imageContours, contours, index, Scalar(255), 1, 8, hierarchy);
	}

	//矩阵marks
	Mat marksShows;
	convertScaleAbs(marks, marksShows);
	watershed(image, marks);

	//分水岭算法之后的矩阵marks
	Mat afterWatershed;
	convertScaleAbs(marks, afterWatershed);
	imwrite("afterWatershed.jpg", afterWatershed);
	//存放marks里数字种类
	//区域号从0开始
	int maxCounter = 0;
	std::vector<int> counters;
	std::vector<int>::iterator countersIterator;

	//对每一个区域进行颜色填充
	Mat PerspectiveImage = Mat::zeros(image.size(), CV_8UC3);
	int count = 0;
	for (int i = 0; i < marks.rows; i++)
	{
		for (int j = 0; j<marks.cols; j++)
		{
			int index = marks.at<int>(i, j);
			if (index > maxCounter)
			{
				maxCounter = index;
			}

			if (marks.at<int>(i, j) == -1)
			{
				PerspectiveImage.at<Vec3b>(i, j) = Vec3b(255, 255, 255);

			}
			else
			{
				PerspectiveImage.at<Vec3b>(i, j) = RandomColor(index);
				countersIterator = find(counters.begin(), counters.end(), index);
				if (countersIterator == counters.end())
				{
					counters.push_back(index);
				}
			}
		}
	}
	imwrite("AfterColorFill.jpg", PerspectiveImage);

	//计算HSI,可与上一步并行处理
	int sumPixelSize = afterWatershed.rows*afterWatershed.cols;
	HSIBox hsiBox = HSIBox(image);

	vector<float> *relationMatrix;

	RelationBox relationBox = RelationBox(maxCounter, marks, sumPixelSize);
	relationBox.CountHSAverage(hsiBox.getHMatrix(), hsiBox.getSMatrix());
	relationBox.setBanforSize();

	relationMatrix = relationBox.getRelationMatrix();

	//测试关系矩阵内容
	for (int i = 0; i < maxCounter + 1; i++)
	{
	cout << "container " << i << " : ";
	for (int j = 0; j < relationMatrix[i].size(); j++)
	cout << relationMatrix[i].at(j) << ' ';
	cout << endl;
	}

	//去掉四角黑块
	vector<int> banlist;
	int banmarkleftup = marks.at<int>(1, 1);
	int banmarkleftdown = marks.at<int>(1, marks.cols - 2);
	int banmarkrightup = marks.at<int>(marks.rows - 2, 1);
	int banmarkrightdown = marks.at<int>(marks.rows - 2, marks.cols - 2);
	banlist.push_back(banmarkleftup);
	banlist.push_back(banmarkleftdown);
	banlist.push_back(banmarkrightup);
	banlist.push_back(banmarkrightdown);
	for (int i = 0; i < banlist.size(); i++){
		relationMatrix[banlist[i]][6] = 1;
	}
	vector<int> seedAreaMap;
	seedAreaMap = regionalGrowth(relationMatrix, maxCounter + 1, counters.size(), sumPixelSize);

	//对生长区域进行颜色填充
	Mat regionalGrowthImage = Mat::zeros(image.size(), CV_8UC3);
	for (int i = 0; i < marks.rows; i++)
	{
		for (int j = 0; j<marks.cols; j++)
		{
			int index = marks.at<int>(i, j);
			if (index > maxCounter)
			{
				maxCounter = index;
			}
			int relationmark = marks.at<int>(i, j);
			if (relationmark == -1 || relationMatrix[relationmark].at(4) == 0 || relationMatrix[relationmark].at(6) == 1)
			{
				regionalGrowthImage.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
			}
			else
			{
				regionalGrowthImage.at<Vec3b>(i, j) = Vec3b(0, 0,0);
				countersIterator = find(counters.begin(), counters.end(), index);
				if (countersIterator == counters.end())
				{
					counters.push_back(index);
				}
			}
		}
	}
	char* imagePath = "regionalGrowthImage1.jpg";//输入文件
	char* OutPath = "out_smooth_out.jpg";//输出文件

	imwrite(imagePath, regionalGrowthImage);
	fs.close();

	return 0;
}

/*
完成平滑操作
*/
int mainFinalProcess(){
	processView::viewer++;
	fstream fs;
	fs.open("log.txt", ios_base::out | ios_base::app);
	cout.rdbuf(fs.rdbuf());

	string MSimg = "aftermeanshift.jpg";
	Mat image = imread(MSimg);
	char* imagePath = "regionalGrowthImage1.jpg";//输入文件
	char* OutPath = "out_smooth_out.jpg";//输出文件
	Mat Src = imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Dst = Mat::zeros(Src.size(), CV_8UC1);

	//二值化处理  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iData = Src.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iData[j] == 0 || iData[j] == 255) continue;
			else if (iData[j] < 10)
			{
				iData[j] = 0;
				//cout<<'#';  
			}
			else if (iData[j] > 10)
			{
				iData[j] = 255;
				//cout<<'!';  
			}
		}
	}

	cout << "Image Binary processed." << endl;

	RemoveSmallRegion(Src, Dst, 3000, 0, 1);//去除黑
	RemoveSmallRegion(Dst, Dst, 50, 1, 0);//去除白
	cout << "Done!" << endl;

	imwrite(OutPath, Dst);

	//开始去除突出&平滑

	char* imagePath_2 = "out_smooth_out.jpg";
	char* OutPath_3 = "final.jpg";//这个图是最终结果

	Size sizen = Size(3, 3);//平滑窗口

	Mat Src_2 = imread(imagePath_2, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Dst_1 = Mat::zeros(Src_2.size(), CV_8UC1);
	Mat Dst_2 = Mat::zeros(Src_2.size(), CV_8UC1);
	Mat Dst_3 = Mat::zeros(Src_2.size(), CV_8UC1);

	delete_jut(Src_2, Dst_1, 5, 5, 0);
	imageblur(Dst_1, Dst_2, sizen, 10);
	RemoveSmallRegion(Dst_2, Dst_3, 1000, 0, 1);//再次去除黑色小区域
	imwrite(OutPath_3, Dst_3);

	Mat finalResult;
	Mat huImg = Hu_Smooth(Dst_3);
	Mat G_After_Meanshift = imread("aftermeanshift.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	Sobel_Contour(G_After_Meanshift);
	Mat sobelResult = imread("BSobel0503_Dwhite_35.jpg", 0);
	
	Mat Original = imread("imgTemp.jpg");
	finalResult = Merge_Contour_Extract(sobelResult, huImg, Original);

	imwrite("wshed.jpg", finalResult);
	Mat top = imread("afterWatershed.jpg", 0);
	Mat currency=Merge_Water_Final(top, finalResult);

	fs.close();
	processView::viewer++;
	return 0;
}

};


