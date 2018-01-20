# Mygeo
A project to simply extract the house information from GIS 

## 简介
大创时所写的一个项目，主要目的是从遥感影像中提取房屋信息。

## 原理
对读入的影像取前三个波段组合成RGB颜色格式的图像

对图像进行meanshift以去除噪点

对图像使用Opencv自带的分水岭算法以分割区域（使用Canny算子）

计算图像的HSI信息

由图像分割结果结合HSI信息获得关系矩阵

对关系矩阵使用区域种子生长算法获得房屋信息区域

对房屋信息区域进行平滑操作，并叠加到原有图像上

## 结语
感谢项目代码的主要贡献者LRZ，感谢项目的负责人ZX


