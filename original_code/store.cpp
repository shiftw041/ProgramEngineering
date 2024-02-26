/*系统信息存储模块*/
#include "gms.h"

// 循环读入基站文件
void ReadJZData(char *filenames[], int &fnum, int *xmin, int *ymin, int *xmax, int *ymax)
{
    // 循环读入基站文件
    while (fnum < MAX_NUM)
    {
        char filename[MAX_NUM];
        printf("请输入基站文件名(输入-1停止):\n");
        scanf("%s", filename);
        if (strcmp(filename, "-1") == 0)
        {
            break;
        }
        AnalyseJZData(filename, xmin, ymin, xmax, ymax);
        // 将输入的文件名复制到文件名数组中
        filenames[fnum] = (char *)malloc(strlen(filename) + 1);
        strcpy(filenames[fnum], filename);
        fnum++;
    }
}

// 分析基站数据，选取合适的四叉树根结点和长度
void AnalyseJZData(char *filename, int *xmin, int *ymin, int *xmax, int *ymax)
{

    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("打开基站文件失败！\n");
        exit(1);
    }
    printf("成功打开基站文件！\n");
    char type[10];
    fscanf(fp, "%2s", type); // 读取 "JZ" 类型信息
    if (strcmp(type, "JZ") != 0)
    {
        printf("无效基站文件!\n");
        exit(1);
    }
    int x, y, id;
    double i;
    // 循环读入基站文件中的每一行，找出区域横纵坐标的最大最小值
    while (fscanf(fp, "%d,%d,%9s %lf,%d", &x, &y, type, &i, &id) == 5)
    {
        if (x < *xmin)
            *xmin = x;
        else if (x > *xmax)
            *xmax = x;
        if (y < *ymin)
            *ymin = y;
        else if (y > *ymax)
            *ymax = y;
    }
}

// 读取存储文本文件中的位置信息并存储在轨迹数组中
void ReadYDData(Trail *yds[])
{
    char filename[MAX_NUM];
    printf("请输入轨迹文件名:\n");
    scanf("%s", filename);
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("打开轨迹文件失败！\n");
        exit(1);
    }
    printf("成功打开轨迹文件！\n");
    char type[10];
    fscanf(fp, "%2s", type); // 读取 "YD" 类型信息
    if (strcmp(type, "YD") != 0)
    {
        printf("无效轨迹文件！\n");
        exit(1);
    }
    int xs, ys, xe, ye, k = 0, hour, min;
    double v;
    // 循环读入移动终端文件的每一行并存储每一段轨迹到轨迹数组中
    while (fscanf(fp, "%d,%d,%d,%d,%lf,%d,%d", &xs, &ys, &xe, &ye, &v, &hour, &min) == 7)
    {
        Trail *yd = (Trail *)malloc(sizeof(Trail));
        *yd = {xs, ys, xe, ye, v / 3.6, hour, min, Dist(xs, ys, xe, ye)};
        yds[k] = yd;
        k++;
    }
    printf("完成%d条移动轨迹的存储\n\n", k);
}

// 读取存储伪基站并存储在伪基站数组中
void ReadWZData(WZ *wzs[])
{
    char filename[MAX_NUM];
    printf("请输入伪基站文件名:\n");
    scanf("%s", filename);
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("打开伪基站文件失败！\n");
        exit(1);
    }
    printf("成功打开伪基站文件！\n");
    char type[10];
    fscanf(fp, "%2s", type); // 读取 "WZ" 类型信息
    if (strcmp(type, "WZ") != 0)
    {
        printf("无效伪基站文件\n");
        exit(1);
    }
    int xs, ys, xe, ye, k = 0, hour, min, id;
    double v;
    // 循环读入每一个伪基站，
    while (fscanf(fp, "%d,%d,%d,%d,%lf,%d,%d,%d", &xs, &ys, &xe, &ye, &v, &hour, &min, &id) == 8)
    {
        WZ *wz = (WZ *)malloc(sizeof(WZ));
        *wz = {{xs, ys, xe, ye, v / 3.6, hour, min, Dist(xs, ys, xe, ye)}, id};
        wzs[k] = wz;
        k++;
    }
    printf("完成%d个伪基站的读入与存储\n\n", k);
}

// 建立指定大小和中心位置的空四叉树
Quadtree *CreateQuadtree(int x, int y, double w, double h)
{
    Quadtree *quadtree = (Quadtree *)calloc(1, sizeof(Quadtree));
    quadtree->x = x;
    quadtree->y = y;
    quadtree->divide = 0;
    quadtree->w = w;
    quadtree->h = h;
    quadtree->depth = 1;
    printf("根结点坐标：(%d %d), 矩形半边长：%.0lf %.0lf\n\n", x, y, w, h);
    return quadtree;
}

// 创建指定大小和位置的四叉树结点，并存入给定基站
void InitNode(Quadtree *node, int X, int Y, double w, double h, int i, BasicStation *bs)
{
    node->kids[X][Y] = (Quadtree *)calloc(1, sizeof(Quadtree));
    node->kids[X][Y]->x = node->x + 2 * X * w - w;
    node->kids[X][Y]->y = node->y + 2 * Y * h - h;
    node->kids[X][Y]->w = w;
    node->kids[X][Y]->h = h;
    node->kids[X][Y]->depth = i + 1;
    node->kids[X][Y]->parent = node;
    node->kids[X][Y]->station = bs;
}

// 循环存储每个基站文件中的基站
void StoreJZ(char *filenames[], int fnum, Quadtree *root)
{
    for (int i = 0; i < fnum; i++)
    {
        StoreJZData(filenames[i], root);
    }
}

// 读取文本文件中的基站信息，并将其存入四叉树
void StoreJZData(char *filename, Quadtree *root)
{
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("打开基站文件失败！\n");
        exit(1);
    }
    printf("成功打开基站文件！\n");
    char type[10];
    fscanf(fp, "%2s", type); // 读取 "JZ" 类型信息
    if (strcmp(type, "JZ") != 0)
    {
        printf("无效基站文件!\n");
        exit(1);
    } // 读取文件中的每一行，并将其存储在四叉树中
    int x, y, id, k = 0;
    double i;
    // 循环读入每一个基站并插入四叉树中
    while (fscanf(fp, "%d,%d,%9s %lf,%d", &x, &y, type, &i, &id) == 5)
    {
        BasicStation *bs = (BasicStation *)malloc(sizeof(BasicStation));
        *bs = {x, y, 1.0, i, id, 0};
        if (strcmp(type, "城区") == 0)
            bs->type = 0.3;
        else if (strcmp(type, "高速") == 0)
            bs->type = 5.0;
        InsertStation(bs, root);
        k++;
    }
    printf("完成%d个基站的读入与存储\n\n", k);
    return;
}

// 迭代法将单个基站插入到四叉树中
Quadtree *InsertStation(BasicStation *bs, Quadtree *root)
{
    if (!root)
        return NULL;
    int i = 0, X = 0, Y = 0;
    double w = root->w, h = root->h;
    Quadtree *node = root;
    while (i++ < 16) // 设置四叉树的最大层数,在四叉树中找到结点的合适插入位置
    {
        if (!node->divide && !node->station) // 叶子没存有基站且没分裂,只有第一个插入的点会有这种情况
        {
            node->station = bs; // 将基站存入该叶子结点
            printf("000第%d个基站在第%d层(%d,%d)叶子坐标:%d,%d半边长:%.3lf %.3lf\n", bs->id, i, bs->x, bs->y, node->x, node->y, node->w, node->h);
            break;
        }
        else
        {
            w /= 2, h /= 2;
            if (node->station && !node->divide) // 叶子有基站但没有分裂
            {
                node->divide = 1;
                // 分裂结点，将原有基站存储到对应子结点,并初始化子结点
                X = (node->station->x >= node->x) ? 1 : 0;
                Y = (node->station->y >= node->y) ? 1 : 0;
                InitNode(node, X, Y, w, h, i, node->station);
                node->station = NULL; // 把该结点的基站指针清空
            }
            // 没有存储有基站，但是已经有向下分裂的结点，向下寻找基站位置
            X = (bs->x >= node->x) ? 1 : 0;
            Y = (bs->y >= node->y) ? 1 : 0;
            if (node->kids[X][Y]) // 新基站和旧基站仍在同一片叶子，继续向下循环
                node = node->kids[X][Y];
            else // 基站不和旧基站同结点，可放入空叶子中
            {
                InitNode(node, X, Y, w, h, i, bs);
                // printf("111第%d个基站在第%d层,%d,%d,坐标:%d,%d长度:%lf %lf\n", bs->id, node->depth, bs->x, bs->y, node->x, node->y, node->w, node->h);
                break;
            }
        }
    }
    return node;
}