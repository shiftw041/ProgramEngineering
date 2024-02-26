/*GSM蜂窝移动通信模拟主程序main.cpp*/
#include "gms.h"
#include "store.cpp"  // 系统信息存储模块
#include "search.cpp" // 基站信息查询模块
#include "select.cpp" // 基站选择模块
#include "trail.cpp"  // 轨迹分析模块
int main()
{
    // 读取基站信息并存储在四叉树中
    int xmin = 0, ymin = 0, xmax = 0, ymax = 0;
    double w = 0, h = 0;
    char *filenames[MAX_NUM] = {0}; // 存储基站文件名的字符数组
    // 循环读入基站文件名
    for (int i = 0; i < MAX_NUM; i++)
    {
        filenames[i] = (char *)malloc(MAX_NUM);
    }
    int fnum = 0;                                            // 输入的文件数
    ReadJZData(filenames, fnum, &xmin, &ymin, &xmax, &ymax); // 读入基站文件并分析
    // 创建空四叉树并插入基站
    Quadtree *root = CreateQuadtree((xmax + xmin) / 2, (ymax + ymin) / 2, (xmax - xmin + 2) / 2, (ymax - ymin + 2) / 2);
    StoreJZ(filenames, fnum, root);
    // 读取并存储伪基站和移动端轨迹信息
    WZ *wzs[MAX_NUM] = {0};    // 伪基站数组
    Trail *yds[MAX_NUM] = {0}; // 轨迹数组
    ReadWZData(wzs);           // 读取伪基站信息
    ReadYDData(yds);           // 读取移动轨迹信息
    // 基站信息查找
    Quadtree *nw = FindCorner(root);      // 查找西北角
    Quadtree *se = FindCorner(root);      // 查找东南角
    FindSide(root, nw);                   // 查找西北角的侧块
    FindSide(root, nw);                   // 查找西北角的侧块
    Quadtree *se_nw = FindSide(root, se); // 查找东南角的侧块
    FindSide(root, se_nw);                // 查找东南角的侧块的侧块
    //  基站选择
    int count; // 可连接基站数量
    int x, y;  // 待检测点坐标
    while (1)  // 循环分析定点信号连接情况
    {
        printf("\n请输入要搜索信号的点(输入-1 -1停止):\n");
        scanf("%d %d", &x, &y);
        if (x == -1 && y == -1)
            break;
        count = 0;
        // 查找最优基站
        BasicStation *best = FindStrongest(x, y, root, &count);
        printf("有%d个可连接的基站:\n", count);
        if (!best)
        {
            printf("查找距离最近的基站\n");
            best = FindNearest(x, y, root);
            printf("id=%d, (%d, %d), i=%lf,d=%.0lf\n", best->id, best->x, best->y, Strength(best, x, y), sqrt(Distance(best->x, best->y, x, y)));
        }
        else if (count >= 1)
            printf("信号最强的基站:\nid=%d, (%d, %d), i=%lf,d=%.0lf\n", best->id, best->x, best->y, Strength(best, x, y), sqrt(Distance(best->x, best->y, x, y)));
    }
    // 分析轨迹
    printf("\n接下来开始分析移动轨迹(输入1回车开始)\n");
    int i;
    scanf("%d", &i);
    if (i == 1) // 输入1回车开始分析完整的移动轨迹
        Calculate(yds, root, wzs);
    printf("\n接下来检查单段移动轨迹:\n");
    // 分析轨迹进入首个基站范围的时间
    CalFirst(yds[0], root, wzs);
    // 分析轨迹进入首个重叠范围的时间
    CalOverlap(yds[2], root, wzs);
    CalOverlap(yds[5], root, wzs);
    // 分析轨迹连接上伪基站的时间
    CalWZ(yds[11], root, wzs);
    CalWZ(yds[8], root, wzs);

    system("pause");
    return 0;
}