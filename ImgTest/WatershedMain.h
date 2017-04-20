
/*
* 分水岭算法主函数
*/

#include<string>
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
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
	//Mat temp;
public:

	WatershedMain(Mat Q){
		sourceFile = Q.clone();
	}

//OPENCV二值化图像内孔洞填充 / 小区域去除
//CheckMode: 0代表去除黑区域，1代表去除白区域; NeihborMode：0代表4邻域，1代表8邻域;  
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
		//cout << "Neighbor mode: 8邻域." << endl;
		NeihborPos.push_back(Point2i(-1, -1));
		NeihborPos.push_back(Point2i(-1, 1));
		NeihborPos.push_back(Point2i(1, -1));
		NeihborPos.push_back(Point2i(1, 1));
	}
	else{} //cout << "Neighbor mode: 4邻域." << endl;
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

	//cout << RemoveCount << " objects removed." << endl;

	//cout << BlackCount << " objects left." << endl;
}


//去除二值图像边缘的突出部  
//uthreshold、vthreshold分别表示突出部的宽度阈值和高度阈值  
//type代表突出部的颜色，0表示黑色，1代表白色   
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


//图片边缘光滑处理  
//size表示取均值的窗口大小，threshold表示对均值图像进行二值化的阈值  
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

Vec3b RandomColor(int value)    //生成随机颜色函数</span>
{
	value = value % 255;  //生成0~255的随机数
	RNG rng;
	int aa = rng.uniform(0, value);
	int bb = rng.uniform(0, value);
	int cc = rng.uniform(0, value);
	return Vec3b(aa, bb, cc);
}

vector<pair<float, float>> seedAreaParaCal(vector<float> *BBmatrix, int areaCount) //由区域关系矩阵计算参数矩阵
{
	vector<pair<float, float>> AutoMatrix;//[0]:相似度 [1]：相对欧氏距离
	const int neinVar = 7;//向量常数位persize
	vector<float> tempNeiHueArea;//邻接区域色调分量temp
	vector<float> tempNeiSatArea;//邻接区域饱和度分量temp
	float simh = 0, sims = 0, avgh = 0, avgs = 0;//avg：平均
	vector<float> Ds;//欧氏距离，取色调分量计算

	for (int i = 0; i < areaCount; i++)
	{
		int neiArea = (int)BBmatrix[i][0];//邻接区域个数
		if (neiArea == 0 && BBmatrix[i][6] != 1){
			//cout << i << " no相似度 " << " no最大欧氏距离 " << endl;

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
				//cout << i << " " << ArealistTemp << " " << tempNeiHueArea[j] << " " << tempNeiSatArea[j] << endl;
			}
			avgh = (avgh + BBmatrix[i][1]) / (neiArea + 1);
			avgs = (avgs + BBmatrix[i][2]) / (neiArea + 1);
			//cout << i << " " << AutoMatrix[i][0] << endl;
			//计算相似度和欧氏距离
			float similar = 0, DsMaxtemp = 0;//相似度,最大欧氏距离
			for (int j = 0; j < neiArea; j++)
			{

				simh = simh + pow(tempNeiHueArea[j] - avgh, 2);
				sims = sims + pow(tempNeiSatArea[j] - avgs, 2);

				if (tempNeiHueArea[j] == 0)
					Ds[j] = 0;
				else Ds[j] = abs((tempNeiHueArea[j] - avgh) / tempNeiHueArea[j]);
				//cout << i << " " << Ds[j] << endl;

				if (Ds[j]>DsMaxtemp)
				{
					DsMaxtemp = Ds[j];
				}
			}
			simh = sqrt(simh / neiArea + 1);
			sims = sqrt(sims / neiArea + 1);
			similar = 0.8*simh + 0.2*sims;
			//cout << i << " 相似度 " << similar << " 最大欧氏距离 " << DsMaxtemp << endl;
			AutoMatrix.push_back(pair<float, float>(similar, DsMaxtemp));
		}

	}
	return AutoMatrix;
}
double similarOtsu(vector<pair<float, float>> AutoMatrix){//大津思想求阈值
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
		//cout << a << " " << pixelPro[a] << " " << pixelCount[a] << endl;
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
			//cout << deltaMax << " is zuida" << endl;
			threshold = j;
			//cout << threshold << " is zuida" << endl;
		}
	}
	return threshold;
}



void addMarkToSubTempArea(int jointedAreaNum, int targetAreaSeedNum, vector<float> *BBmatrix){
	const int neinVar = 7;//向量常数位persize
	if (BBmatrix[jointedAreaNum][0] >= 0 && BBmatrix[targetAreaSeedNum][0] >= 0 && BBmatrix[jointedAreaNum][6] != 1 && BBmatrix[targetAreaSeedNum][6] != 1){//约定不被合并过的区域一定为正的
		vector<float> jointArea;
		vector<float> targetArea;
		//cout << jointedAreaNum << " " << BBmatrix[jointedAreaNum][4] << "combined with" << targetAreaSeedNum << " " << BBmatrix[targetAreaSeedNum][4] << endl;
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


//向量第一位（at(0)）存储邻域个数，第二位（at(1)）存储色调（H）均值，第三位（at(2)）存储饱和度（S）均值，第四位（at(3)）存储区域内像素个数，第五位（at(4)）存储标号位，第六位（at(5)）存储周长，第七位（at(6)）存储ban，之后存储邻域们的区域标号
//第七位ban位取值： 0（默认）--不ban，1--ban，被判定为肯定不是房屋
vector<int> regionalGrowth(vector<float> *BBmatrix, int areaCount, int realAreaCount, int SUM){
	//矩阵，总区域数，有效区域数，总图像像素数,屏蔽区域
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
		//cout << seedArea.at(i) << " is seed" << endl;
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
							//cout << markedAreaVari[0] << "独一区域 " << endl;
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
							//cout << bestAreaAdd << "最佳区域 " << endl;
							addMarkToSubTempArea(subTempArea, seedArea[bestAreaAdd - 1], BBmatrix);//种子区域的序号；被添加的区域序号；添加入的种子区域序号，区域关系矩阵
						}
					}
					neiArea = (int)BBmatrix[seedTemp][0];//重新获得邻接区域个数
					//cout << neiArea << "重新获取 " << endl;
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
					//cout << " complete joint " << endl;
					addMarkToSubTempArea( subTempArea, seedTemp, BBmatrix);//种子区域的序号；被添加的区域序号；添加入的种子区域序号，区域关系矩阵
				}
			}
			neiArea = (int)BBmatrix[seedTemp][0];//重新获得邻接区域个数
		}
	}

	for (int i = 0; i<areaCount; i++){
		if (BBmatrix[i][0]>0 && BBmatrix[i][3]<150){//大于零保证其没有被合并，小于SUM/150（需定义或传参）保证足够小
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
				//cout << " small area " << endl;
				addMarkToSubTempArea( i, bestAreaAdd, BBmatrix);
			}
		}
	}

	/*输出合并后信息
	for (int i = 0; i < areaCount; i++)
	{
		cout << "container " << i  << " : ";
		for (int j = 0; j < BBmatrix[i].size(); j++)
			cout << BBmatrix[i].at(j) << ' ';
		cout << endl;
		//cout << AutoMatrix[i][0] << " " << "is para" << endl;
	}
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
	cout << seedCount-1 << "个种子" << endl;
	for (int i = 0; i < areaCount; i++)
	{
		if (BBmatrix[i][6]==1){
			cout << "ban container " << i << " : ";
			//jj++;
			for (int j = 0; j < BBmatrix[i].size(); j++)
				cout << BBmatrix[i].at(j) << ' ';
			cout << endl;
		}
	}*/
	return seedArea;
}

int mainMethod()
{
	//clock_t start_time = clock();
	if (sourceFile.empty())
	{
		cout << "Can't read image" << endl;
		return -1;
	}
	imwrite("imgTemp.jpg", sourceFile);
	string MSimg = "aftermeanshift.jpg";

	/*输出内容到文件
	fstream fs;
	fs.open("xxx.txt", ios_base::out | ios_base::trunc);
	cout.rdbuf(fs.rdbuf());*/
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

	if (mainMethod2() == 0){
		processView::viewer++;
		return 0;
	}
	else return -1;
}
	
int mainMethod2(){
	/*opencv watershed*/
	string MSimg = "aftermeanshift.jpg";
	Mat image = imread(MSimg);    //载入RGB彩色图像

	//灰度化，滤波，Canny边缘检测
	Mat imageGray;
	cvtColor(image, imageGray, CV_RGB2GRAY);//灰度转换
	//imshow("Gray Image", imageGray);
	//imwrite("GrayImage.jpg", imageGray);
	Canny(imageGray, imageGray, 80, 150);
	//imshow("Canny Image", imageGray);
	//imwrite("CannyImage.jpg", imageGray);
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

	//我们来看一下传入的矩阵marks里是什么东西
	Mat marksShows;
	convertScaleAbs(marks, marksShows);
	//imshow("marksShow", marksShows);
	//imshow("轮廓", imageContours);
	watershed(image, marks);

	//我们再来看一下分水岭算法之后的矩阵marks里是什么东西
	Mat afterWatershed;
	convertScaleAbs(marks, afterWatershed);
	//imshow("After Watershed", afterWatershed);
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
	//cout << "counters.size() " << counters.size() << endl; 
	//cout << "maxCounter " << maxCounter << endl;
	//imshow("After ColorFill", PerspectiveImage);
	imwrite("AfterColorFill.jpg", PerspectiveImage);

	//计算HSI,可与上一步并行处理
	int SUM = afterWatershed.rows*afterWatershed.cols;
	HSIBox hsiBox = HSIBox(image);
	//判断邻接关系_2017年2月19日 22:50:54_李润泽
	//向量第一位（at(0)）存储邻域个数，第二位（at(1)）存储色调（H）均值，第三位（at(2)）存储饱和度（S）均值，第四位（at(3)）存储区域内像素个数，第五位（at(4)）存储标号位，之后存储邻域们的区域标号
	vector<float> *relationMatrix;

	RelationBox relationBox = RelationBox(maxCounter, marks, SUM);
	relationBox.CountHSAverage(hsiBox.getHMatrix(), hsiBox.getSMatrix());
	relationBox.setBanforSize();

	relationMatrix = relationBox.getRelationMatrix();

	/*测试关系矩阵内容

	for (int i = 0; i < maxCounter + 1; i++)
	{
	cout << "container " << i << " : ";
	for (int j = 0; j < relationMatrix[i].size(); j++)
	cout << relationMatrix[i].at(j) << ' ';
	cout << endl;
	}*/

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
	//cout << "banlist: " << banmarkleftup << " " << banmarkleftdown << " " << banmarkrightup << " " << banmarkrightdown << " " << endl;

	vector<int> seedAreaMap;
	seedAreaMap = regionalGrowth(relationMatrix, maxCounter + 1, counters.size(), SUM);

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
				regionalGrowthImage.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
				countersIterator = find(counters.begin(), counters.end(), index);
				if (countersIterator == counters.end())
				{
					counters.push_back(index);
				}
			}
		}
	}
	char* imagePath = "regionalGrowthImage1.jpg";//输入文件
	char* OutPath = "out_smooth_out_4_2_1000_50.jpg";//输出文件

	imwrite(imagePath, regionalGrowthImage);
	if (mainMethod3() == 0){
		processView::viewer++;
		return 0;
	}
	else return -1;
}

int mainMethod3(){
	string MSimg = "aftermeanshift.jpg";
	Mat image = imread(MSimg);
	char* imagePath = "regionalGrowthImage1.jpg";//输入文件
	char* OutPath = "out_smooth_out_4_2_1000_50.jpg";//输出文件
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

	//cout << "Image Binary processed." << endl;

	RemoveSmallRegion(Src, Dst, 3000, 0, 1);//去除黑
	RemoveSmallRegion(Dst, Dst, 50, 1, 0);//去除白
	//cout << "Done!" << endl;

	imwrite(OutPath, Dst);

	//开始去除突出&平滑

	char* imagePath_2 = "out_smooth_out_4_2_1000_50.jpg";
	//char* OutPath_1 = "smooth_out_4_1.jpg";//这个图是去除边缘突出后的结果
	//char* OutPath_2 = "smooth_out_4_2.jpg";//这个图是上图基础上，平滑的结果
	char* OutPath_3 = "final.jpg";//这个图是最终结果，为了方便，上面的观察过程用的中间产生结果可以省略

	Size sizen = Size(3, 3);//平滑窗口

	Mat Src_2 = imread(imagePath_2, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Dst_1 = Mat::zeros(Src_2.size(), CV_8UC1);
	Mat Dst_2 = Mat::zeros(Src_2.size(), CV_8UC1);
	Mat Dst_3 = Mat::zeros(Src_2.size(), CV_8UC1);

	delete_jut(Src_2, Dst_1, 5, 5, 0);

	//imwrite(OutPath_1, Dst_1);

	imageblur(Dst_1, Dst_2, sizen, 10);

	//imwrite(OutPath_2, Dst_2);

	RemoveSmallRegion(Dst_2, Dst_3, 1000, 0, 1);//再次去除黑色小区域

	imwrite(OutPath_3, Dst_3);

	//cout << "Done!" << endl;

	//step 2
	//imwrite("regionalGrowthImage1.jpg", regionalGrowthImage);
	//imwrite("regionalGrowthImage2.jpg", regionalGrowthImage2);
	//分割并填充颜色的结果跟原始图像融合

	Mat wshed;
	//addWeighted(DstB, 0.5, DstA, 0.5, 0, wshedMiddle);
//	imwrite("wshedmiddle.jpg", DstB);
	Mat wshedz = imread("final.jpg");
	addWeighted(image, 0.4, wshedz, 0.6, 0, wshed);
	//imshow("AddWeighted Image", wshed);
	imwrite("wshed.jpg", wshed);
	//temp = wshed;
	//fs.close();

	processView::viewer++;
	return 0;
	
}

};


