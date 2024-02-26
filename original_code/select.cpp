/*基站的连接选择模块*/
#include "gms.h"
// 基站的有效半径的平方计算
inline double Range(BasicStation *bs)
{
    bs->range = 1000000.0 * bs->type * bs->type * bs->strength;
    return (bs->range);
}

// 计算某个基站在某点的信号强度
inline double Strength(BasicStation *bs, double x, double y)
{
    if (!bs)
        exit(1);
    return ((bs->strength * 1000000.0 * bs->type * bs->type * bs->strength) / Distance(x, y, bs->x, bs->y));
}

// 计算两点间距离的平方
inline double Distance(double xs, double ys, double xe, double ye)
{
    return (pow(xs - xe, 2) + pow(ys - ye, 2));
}
// 计算两点间距离
inline double Dist(double xs, double ys, double xe, double ye)
{
    return sqrt((pow(xs - xe, 2) + pow(ys - ye, 2)));
}

// 判断定点能否连接基站
bool IfConnect(double x, double y, BasicStation *bs)
{
    if (Dist(x, y, bs->x, bs->y) < sqrt(Range(bs))) // 两点距离小于有效半径
        return true;
    else
        return false;
}

// 判断可连接基站是否已记录过
bool IfRecord(BasicStation *p[], BasicStation *bs)
{
    int i = 0;
    while (p[i])
        if (p[i++] == bs)
            return true;
    return false;
}

// 判断基站信号是否更强并修改
void IfStronger(BasicStation **best, BasicStation *bs, double x, double y)
{
    if (!(*best) || (Strength(*best, x, y) < Strength(bs, x, y)))
        *best = bs;
}

// 递归检索某个分块中能连接上定点的基站
void Connect(Quadtree *root, double x, double y, int *count, BasicStation *p[], BasicStation **best)
{
    if (!root)
        return;
    if (!root->divide && root->station) // 找到含基站的叶子结点，判断能否连接
    {
        if (IfConnect(x, y, root->station))
            if (!(*count) || !IfRecord(p, root->station))
            {
                p[*count] = root->station;
                (*count)++;
                IfStronger(best, root->station, x, y);
            }
    }
    else
    { // 递归检索孩子结点中的基站
        Connect(root->kids[0][0], x, y, count, p, best);
        Connect(root->kids[0][1], x, y, count, p, best);
        Connect(root->kids[1][0], x, y, count, p, best);
        Connect(root->kids[1][1], x, y, count, p, best);
    }
}

// 查询定点周围信号最强的基站和连接到的基站个数,不包括伪基站，伪基站另有时间函数查询
BasicStation *FindStrongest(double x, double y, Quadtree *root, int *count)
{
    Quadtree *point = FindBigLeaf(root, x, y, 4); // 存储定点所在四叉树结点
    if (!point)
        return NULL;
    double sx = point->x, sy = point->y, xt, yt;
    double w = point->w, h = point->h; // 查找范围矩形
    int depth = point->depth;
    BasicStation *p[MAX_NUM] = {0}, *best = NULL; // 连接基站，最优基站
    Connect(point, x, y, count, p, &best);        // 检索定点所在分块
    for (int i = -1; i < 2; i++)                  // 依次对八个方向进行检索
    {
        xt = sx + i * (w * 2);
        for (int j = -1; j < 2; j++)
        {
            yt = sy + j * (h * 2);
            point = FindBigLeaf(root, xt, yt, depth);
            Connect(point, x, y, count, p, &best);
        }
    }
    return best;
}

// 递归检索某个分块中距离最近的基站
void Near(Quadtree *root, double x, double y, BasicStation **near)
{
    if (!root)
        return;
    if (!root->divide && root->station)
    {
        if (!(*near) || Distance(x, y, root->station->x, root->station->y) < Distance(x, y, (*near)->x, (*near)->y))
        {
            *near = root->station;
        }
    }
    else// 递归检查结点的四个孩子
    {
        Near(root->kids[0][0], x, y, near);
        Near(root->kids[0][1], x, y, near);
        Near(root->kids[1][0], x, y, near);
        Near(root->kids[1][1], x, y, near);
    }
}

// 寻找离定点最近的基站
BasicStation *FindNearest(int x, int y, Quadtree *root)
{
    Quadtree *point = FindLeaf(root, x, y);      // 存储定点所在四叉树结点
    double sx = point->x, sy = point->y, xt, yt; // 查找范围矩形
    double w = point->w, h = point->h;
    int depth = point->depth;
    BasicStation *near = NULL; // 最近基站
    Near(point, x, y, &near);
    for (int i = -1; i < 2; i++) // 依次对八个方向进行检索
    {
        xt = sx + i * (w * 2);
        for (int j = -1; j < 2; j++)
        {
            yt = sy + j * (h * 2);
            point = FindBigLeaf(root, xt, yt, depth);
            Near(point, x, y, &near);
        }
    }
    return near;
}