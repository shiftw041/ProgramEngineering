/*
 * @Description:本程序为GSM蜂窝移动网络通信模拟程序,分为main, store, search, select, trail几个部分
 * @Author: 网安2104邬雪菲
 * @Date: 2023-2-20 19:34:20
 * @LastEditTime: 2023-2-28 10:09:57
 */
#ifndef GMS_H
#define GMS_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>
using namespace std;
#define MAX_NUM 100

typedef struct Trail
{
    int xs, ys;    // 起点坐标
    int xe, ye;    // 终点坐标
    double v;      // 速度
    int hour, min; // 开始时间
    double dist;   // 移动距离
} Trail;

// 基站结构
typedef struct BasicStation
{
    int x, y;        // 基站坐标
    double type;     // 基站类型
    double strength; // 基站信号基准强度
    int id;          // 基站编号
    double range;    // 信号有效半径的平方
} BasicStation;

// 伪基站结构
typedef struct WZ
{
    Trail trail; // 伪基站的移动轨迹
    int id;      // 伪基站编号
} WZ;

// 四叉树结点结构
typedef struct Quadtree
{
    int x, y;              // 中心坐标
    double w, h;           // 结点矩形半边长
    Quadtree *kids[2][2];  // 四个子树
    int depth;             // 结点深度
    int divide;            // 结点是否分裂
    BasicStation *station; // 结点所存储的基站，每个结点最多一个
    Quadtree *parent;      // 结点的父结点
} Quadtree;

// 系统信息存储模块
void ReadJZData(char *filenames[], int &, int *, int *, int *, int *);    // 循环读入基站文件
void AnalyseJZData(char *filename, int *, int *, int *, int *);           // 分析基站数据，选取合适的四叉树根结点和长度
void StoreJZ(char *filenames[], int fnum, Quadtree *root);                // 循环存储每个基站文件中的基站
void StoreJZData(char *filename, Quadtree *quadtree);                     // 读取文本文件中的基站信息，并将其存储在四叉树中
void ReadYDData(Trail *yds[]);                                            // 读取存储文本文件中的位置信息并存储在轨迹数组中
void ReadWZData(WZ *wzs[]);                                               // 读取存储伪基站并存储在伪基站数组中
Quadtree *CreateQuadtree(int x, int y, double w, double h);               // 建立指定大小和中心位置的空四叉树
void InitNode(Quadtree *, int, int, double, double, int, BasicStation *); // 创建指定大小和位置的四叉树结点，并存入给定基站
Quadtree *InsertStation(BasicStation *bs, Quadtree *root);                // 迭代法将单个基站插入到四叉树中
// 基站信息查询模块
Quadtree *FindLeaf(Quadtree *root, double x, double y);                   // 递归查找某点所处四叉树子树
Quadtree *FindBigLeaf(Quadtree *root, double x, double y, int depth);     // 递归查找规定深度以下的某点所处四叉树子树
Quadtree *OutputLeaf(Quadtree *root);                                     // 递归输出某个子树所含的所有基站
Quadtree *FindCornerstations(Quadtree *root, int x, int y);               // 递归查询四角基站序列
Quadtree *FindCorner(Quadtree *root);                                     // 将方向字符串转换为位置查询四角基站
Quadtree *FindSide(Quadtree *root, Quadtree *node);                       // 将方向字符串转换为点查询某侧基站
Quadtree *OutputSide(Quadtree *root, int depth, int x, int y);            // 递归寻找同样大小的侧块，输出该区域的所有基站
Quadtree *FindSideStations(Quadtree *root, Quadtree *node, int x, int y); // 寻找分块某一侧同大小分块并输出区域中所有的基站
// 基站的连接选择模块
inline double Range(BasicStation *bs);                                              // 计算基站有效半径平方
inline double Strength(BasicStation *bs, double x, double y);                       // 计算某个基站在某点的信号强度
inline double Distance(double xs, double ys, double xe, double ye);                 // 计算两点间距离的平方
inline double Dist(double xs, double ys, double xe, double ye);                     // 计算两点间距离
bool IfConnect(double x, double y, BasicStation *bs);                               // 判断定点能否连接基站
bool IfRecord(BasicStation *p[], BasicStation *bs);                                 // 判断可连接基站是否已记录过
void IfStronger(BasicStation *, BasicStation *, double, double);                    // 判断基站信号是否更强并修改
void Connect(Quadtree *, double, double, int *, BasicStation *p[], BasicStation *); // 递归检索某个分块中能连接上定点的基站
BasicStation *FindStrongest(double x, double y, Quadtree *root, int *count);        // 查询定点周围信号最强的基站，不包括伪基站
void Near(Quadtree *root, double x, double y, BasicStation *near);                  // 递归检索某个分块中距离最近的基站
BasicStation *FindNearest(double x, double y, Quadtree *root);                      // 寻找最近的基站

// 移动端轨迹的信号分析
void Calculate(Trail *yds[], Quadtree *root, WZ *wzs[]);                                            // 计算移动轨迹连接的基站序列
void CalTrail(Trail *yd, Quadtree *root, WZ *wzs[], int *pcount, int *pid, int *pwz);               // 计算一段移动轨迹的基站连接序列
void CalPoint(double, double, Quadtree *, int, int, double, WZ *wzs[], int *, int *, int *, int *); // 计算某点的基站连接情况
void CalFirst(Trail *yd, Quadtree *root, WZ *wzs[]);                                                // 计算某段轨迹连接上第一个基站的时刻和时间段
void CalOverlap(Trail *yd, Quadtree *root);                                                         // 计算某段轨迹通过信号重叠区的时间长度
void CalWZ(Trail *yd, Quadtree *root, WZ *wzs[]);                                                   // 计算某段轨迹连接到伪基站的时间段

void TrailGap(Trail *yd, double r, double *x, double *y, int *hour, int *min, double *sec); // 计算移动距离r后的坐标(x,y)和时间hour:min:sec
void TrailPoint(Trail *yd, double &x, double &y, int hour, int min, double sec);            // 计算某个时刻某段轨迹的位置(x,y),主要用于计算某时刻伪基站的位置
void CalNow(Trail *yd, double r, int *c, int *id, Quadtree *root, WZ *wzs[]);               // 计算移动距离r后点的状态
int JudgeWZ(WZ *wzs[], double x, double y, int hour, int min, double sec);                  // 计算定点某个时刻是否连接到伪基站，返回编号

void DichotomyRange(Trail *yd, double *r1, double *r2, int *id2, double *r, int *c, int *id, Quadtree *root, WZ *wzs[]); // 二分法求某基站信号边界
void DichotomyTwo(Trail *yd, double *r1, double *r2, double *r, int *c, int *id, Quadtree *root, WZ *wzs[]);             // 二分法求重叠区域边界
#endif