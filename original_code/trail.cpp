/*移动端轨迹的信号分析*/
#include "gms.h"

// 计算移动距离r后的坐标(x,y)和时间hour:min:sec
void TrailGap(Trail *yd, double r, double &x, double &y, int &hour, int &min, double &sec)
{
    // 由移动距离r计算坐标
    x = yd->xs + r * (yd->xe - yd->xs) / Dist(yd->xs, yd->ys, yd->xe, yd->ye);
    y = yd->ys + r * (yd->ye - yd->ys) / Dist(yd->xs, yd->ys, yd->xe, yd->ye);
    double second = r / yd->v;
    hour = yd->hour + ((int)second / 3600);
    min = yd->min + ((int)second % 3600) / 60;
    // 处理借位减法
    if (min >= 60)
        hour++, min -= 60;
    sec = second - ((int)second / 3600) * 3600 - ((int)second % 3600 / 60) * 60;
}

// 计算某个时刻某段轨迹的位置(x,y),主要用于计算某时刻伪基站的位置
void TrailPoint(Trail *yd, double &x, double &y, int hour, int min, double sec)
{
    int dmin, dhour;
    dmin = (min - yd->min + 60) % 60;
    dhour = hour - yd->hour - 1 + (min >= yd->min ? 1 : 0);
    double r = yd->v * (dhour * 3600 + dmin * 60 + sec);
    if (r >= yd->dist) // 已经到达轨迹终点，则将坐标设置为终点
    {
        x = yd->xe;
        y = yd->ye;
    }
    else // 没到终点，根据距离计算位置
    {
        x = yd->xs + r * (yd->xe - yd->xs) / Dist(yd->xs, yd->ys, yd->xe, yd->ye);
        y = yd->ys + r * (yd->ye - yd->ys) / Dist(yd->xs, yd->ys, yd->xe, yd->ye);
    }
}

// 计算移动轨迹连接的基站序列
void Calculate(Trail *yds[], Quadtree *root, WZ *wzs[])
{
    int i = -1;
    while (yds[++i])
    {
        int pcount = -1, pid = -1, pwz = 0; // 前一个点的连接状态标志
        printf("\n第%d段轨迹,移动距离%.0lf米,开始移动时间%d:%d:%d,连接基站序列如下:\n",
               i + 1, yds[i]->dist, yds[i]->hour, yds[i]->min, 0);
        CalTrail(yds[i], root, wzs, &pcount, &pid, &pwz);
    }
    return;
}

// 计算一段移动轨迹的基站连接序列
void CalTrail(Trail *yd, Quadtree *root, WZ *wzs[], int *pcount, int *pid, int *pwz)
{
    // 标记上一个检查点的连接状态
    int i = yd->dist / 20.0, k = 0;
    int hour, min, t = 0;
    double sec, x, y;
    CalPoint(yd->xs, yd->ys, root, yd->hour, yd->min, 0, wzs, pcount, pid, pwz, &t); // 计算起点的连接情况
    while (++k <= i)
    {
        // 计算移动距离r后的坐标和时间
        TrailGap(yd, k * 20.0, x, y, hour, min, sec);
        // 分析具体点的连接情况
        CalPoint(x, y, root, hour, min, sec, wzs, pcount, pid, pwz, &t);
    }
    // 计算移动距离r后的坐标和时间
    TrailGap(yd, yd->dist, x, y, hour, min, sec);
    // 计算终点的接情况
    CalPoint(yd->xe, yd->ye, root, hour, min, sec, wzs, pcount, pid, pwz, &t);
    printf("一共切换了%d次信号\n", t);
}

// 计算某点的基站连接情况
void CalPoint(double x, double y, Quadtree *root, int hour, int min, double sec, WZ *wzs[], int *pcount, int *pid, int *pwz, int *t)
{
    int IFwz = JudgeWZ(wzs, x, y, hour, min, sec);
    if (IFwz) // 连接上伪基站
    {
        // 设置连接上伪基站的标志
        if (IFwz != (*pwz))
        {
            printf("%d:%d:%-2.3lf 连接伪基站%d\n", hour, min, sec, IFwz);
            (*t)++;
            *pwz = IFwz, *pcount = -1, *pid = -1;
        }
    }
    else
    {
        *pwz = IFwz; // 记录连接伪基站状态
        int count = 0;
        BasicStation *best = FindStrongest(x, y, root, &count);
        if (count && best->id != *pid)
        {
            // 能连接到基站，且连接的基站改变，则输出切换序列
            printf("%d:%d:%-2.3lf 连接基站%d,可连接%d个\n",
                   hour, min, sec, best->id, count);
            (*t)++;
            *pcount = count, *pid = best->id, *pwz = 0;
        }
        else if (!count && *pcount)
        {
            // 连接不到基站
            printf("%d:%d:%-2.3lf 不可连接\n", hour, min, sec);
            (*t)++;
            *pcount = count, *pid = -1, *pwz = 0;
        }
    }
}

// 计算移动距离r后点的状态
void CalNow(Trail *yd, double r, int *c, int *id, Quadtree *root, WZ *wzs[])
{
    int hour, min;
    double x, y, sec;
    TrailGap(yd, r, x, y, hour, min, sec);
    int IFwz = JudgeWZ(wzs, x, y, hour, min, sec);
    if (IFwz) // 连接上伪基站
        *id = IFwz, *c = -1;
    else
    {
        // 没连接到伪基站，计算连接到正常基站的状态
        int count = 0;
        BasicStation *best = FindStrongest(x, y, root, &count);
        if (count)
            *id = best->id, *c = count;
        else if (!count)
            *id = 0, *c = 0;
    }
}

// 二分法求信号边界
void DichotomyRange(Trail *yd, double *r1, double *r2, int *id2, double *r, int *c, int *id, Quadtree *root, WZ *wzs[])
{
    while (fabs(*r2 - *r1) >= 0.05) // 求基站id2范围边界
    {
        *r = (*r1 + *r2) / 2;
        CalNow(yd, *r, c, id, root, wzs);
        // 切换边界点
        if (*id != *id2)
            *r1 = *r;
        else if (*id == *id2)
            *r2 = *r;
    }
}
// 二分法求重叠区域边界
void DichotomyTwo(Trail *yd, double *r1, double *r2, double *r, int *c, int *id, Quadtree *root, WZ *wzs[])
{
    while (fabs(*r2 - *r1) >= 0.05) // 求重叠区域边界
    {
        *r = (*r1 + *r2) / 2; // 设置下一次检索点为中点
        CalNow(yd, *r, c, id, root, wzs);
        // 切换边界点
        if (*c == 1)
            *r1 = *r;
        else if (*c == 2)
            *r2 = *r;
    }
}

// 计算某段轨迹连接上第一个基站的时刻和时间段
void CalFirst(Trail *yd, Quadtree *root, WZ *wzs[])
{
    double r0 = 0, r1 = 0, r2 = yd->dist, r = yd->dist / 2, dr = 1.0, sec, x, y;
    int c1, c, c2, id1, id, id2; // 上一点，下一点，当前点状态标记
    int hour, min;
    CalNow(yd, 0, &c1, &id1, root, wzs);
    if (c1) // 一开始就进入基站范围
    {
        printf("%d:%d:0一开始就进入首个基站%d范围\n", yd->hour, yd->min, id1);
        // 二分法逼近脱离基站范围的边界
        DichotomyRange(yd, &r2, &r1, &id1, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开首个基站%d范围\n", hour, min, sec, id1);
        printf("时间段长度%.3lf\n\n", r / yd->v);
    }
    else
    {
        r2 = dr;
        id2 = 0;
        while (!id2) // 查找进入基站范围的时刻
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        // 二分法逼近无信号到有信号的边界
        DichotomyRange(yd, &r1, &r2, &id2, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        r0 = r;
        printf("%d:%d:%lf进入首个基站%d范围\n", hour, min, sec, id2);
        r1 = r2, id1 = id2;
        r2 = yd->dist;
        // 二分法逼近脱离基站范围的边界
        DichotomyRange(yd, &r2, &r1, &id1, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开首个基站%d范围\n", hour, min, sec, id2);
        printf("时间段长度%.3lf\n\n", (r - r0) / yd->v);
    }
    return;
}

// 计算某段轨迹通过信号重叠区的时间长度
void CalOverlap(Trail *yd, Quadtree *root, WZ *wzs[])
{
    double r0 = 0, r1 = 0, r2 = yd->dist, r = yd->dist / 2, dr = 1.0, sec, x, y;
    int c1, c, c2, id1, id, id2; // 上一点，下一点，当前点状态标记
    int hour, min;
    CalNow(yd, 0, &c1, &id1, root, wzs);
    if (c1 == 2) // 一开始就进入重叠区
    {
        printf("%d:%d:0一开始就进入重叠范围\n", yd->hour, yd->min);
        r2 = 0, id2 == 2;
        while (c2 == 2) // 找脱离重叠区域的点
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        // 二分法逼近脱离重叠范围的边界
        DichotomyTwo(yd, &r2, &r1, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开重叠范围\n", hour, min, sec);
        printf("基站%d转到%d的时间段长度%.3lf\n\n", id1, id2, r / yd->v);
    }
    else
    {
        r2 = dr;
        c2 = 1;
        while (c2 != 2) // 找重叠区域
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        DichotomyTwo(yd, &r1, &r2, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        r0 = r;
        printf("%d:%d:%lf进入重叠区范围\n", hour, min, sec);
        id1 = id; // 此时id存储的是第一个基站编号
        r1 = r2;
        while (c2 == 2) // 寻找脱离重叠区的点
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        // 二分法逼近脱离重叠区的边界
        DichotomyTwo(yd, &r2, &r1, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开重叠区范围\n", hour, min, sec);
        printf("基站%d转到%d的时间段长度%.3lf\n\n", id1, id2, (r - r0) / yd->v);
    }
    return;
}

// 计算某段轨迹连接到伪基站的时间段
void CalWZ(Trail *yd, Quadtree *root, WZ *wzs[])
{
    double r0 = 0, r1 = 0, r2 = yd->dist, r = yd->dist / 2, dr = 1.0, sec, x, y;
    int c1, c, c2, id1, id, id2; // 上一点，下一点，当前点状态标记
    int hour, min;
    CalNow(yd, 0, &c1, &id1, root, wzs);
    if (c1 == -1 && id1)
    {
        printf("%d:%d:0一开始就进入伪基站%d范围\n", yd->hour, yd->min, id1);
        r2 = dr, c2 = -1;
        while (c2 == -1) // 查找断开伪基站时刻
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        // 二分法逼近断开伪基站的边界
        DichotomyRange(yd, &r1, &r2, &id2, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开伪基站%d范围\n", hour, min, sec, id1);
        printf("时间段长度%.3lf\n\n", r / yd->v);
    }
    else
    {
        r2 = dr;
        c2 = 0;
        while (c2 != -1) // 查找连接到伪基站时刻
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        DichotomyRange(yd, &r1, &r2, &id2, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        r0 = r; // 连接伪基站首时刻
        printf("%d:%d:%lf进入伪基站%d范围\n", hour, min, sec, id2);
        r1 = r2, id1 = id2;
        while (c2 == -1) // 查找断开伪基站的点
        {
            CalNow(yd, r2, &c2, &id2, root, wzs);
            r2 += dr;
        }
        // 二分法逼近边界
        DichotomyRange(yd, &r2, &r1, &id1, &r, &c, &id, root, wzs);
        TrailGap(yd, r, x, y, hour, min, sec);
        printf("%d:%d:%lf离开伪基站%d范围\n", hour, min, sec, id1);
        printf("时间段长度%.3lf\n\n", (r - r0) / yd->v);
    }
    return;
}

// 计算定点某个时刻是否连接到伪基站，返回编号
int JudgeWZ(WZ *wzs[], double x, double y, int hour, int min, double sec)
{
    int i = -1;
    double nx, ny; // 记录伪基站当前坐标
    while (wzs[++i])
    {
        TrailPoint(&(wzs[i]->trail), nx, ny, hour, min, sec);
        if (Dist(nx, ny, x, y) <= 40) // 连接上某个伪基站，返回编号
            return wzs[i]->id;
    }
    return 0;
}
