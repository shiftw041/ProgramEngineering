/*基站信息查询模块*/
#include "gms.h"

// 递归查找某点所处四叉树子树
Quadtree *
FindLeaf(Quadtree *root, double x, double y)
{
    if (!root)
        exit(1);
    if ((x > root->x + root->w + 1) || (x < root->x - root->w - 1) || (y > root->y + root->h + 1) || (y < root->y - root->h - 1))
        return NULL;
    if (!root->kids[(x >= root->x) ? 1 : 0][(y >= root->y) ? 1 : 0])
        // 找到一个包含有基站的叶子,但包含的基站可能不止一个，因为它的非空孩子可能有很多基站
        // 也可能该叶子正好只存了一个基站且没有分裂
        return root;
    else
        return FindLeaf(root->kids[(x >= root->x) ? 1 : 0][(y >= root->y) ? 1 : 0], x, y);
}

// 递归查找规定深度以下的某点所处四叉树子树
Quadtree *FindBigLeaf(Quadtree *root, double x, double y, int depth)
{
    if (!root)
        exit(1);
    if ((x > root->x + root->w + 1) || (x < root->x - root->w - 1) || (y > root->y + root->h + 1) || (y < root->y - root->h - 1))
        return NULL; // 查询点不在四叉树范围内
    if (root->depth == depth || !root->kids[(x >= root->x) ? 1 : 0][(y >= root->y) ? 1 : 0])
        // 找到包含目标点的结点，该结点要么为深度小于depth的叶子结点，要么为深度等于depth的非叶子结点
        return root;
    else
        return FindBigLeaf(root->kids[(x >= root->x) ? 1 : 0][(y >= root->y) ? 1 : 0], x, y, depth);
}

// 递归输出某个子树所含的所有基站
Quadtree *OutputLeaf(Quadtree *root)
{
    if (!root)
        return NULL;
    if (root->station) // 检测点为存有基站的叶子，直接输出
        printf("(%d,%d),%d\n", root->station->x, root->station->y, root->station->id);
    else // 检测点还有四个非全空子树，继续向下检索
    {
        if (root->kids[0][0])
            OutputLeaf(root->kids[0][0]);
        if (root->kids[0][1])
            OutputLeaf(root->kids[0][1]);
        if (root->kids[1][0])
            OutputLeaf(root->kids[1][0]);
        if (root->kids[1][1])
            OutputLeaf(root->kids[1][1]);
    }
    return root;
}

// 递归查询四角基站序列
Quadtree *FindCornerStations(Quadtree *root, int X, int Y)
{
    if (!root->kids[X][Y]) // 已递归到角落
    {
        if (root->station) // 该结点是一个叶子，输出它包含的基站
            OutputLeaf(root);
        // 该结点是分支结点，继续向下检索并输出它的四个子树
        else if (root->kids[0][0])
            OutputLeaf(root->kids[0][0]);
        else if (root->kids[0][1])
            OutputLeaf(root->kids[0][1]);
        else if (root->kids[1][0])
            OutputLeaf(root->kids[1][0]);
        else if (root->kids[1][1])
            OutputLeaf(root->kids[1][1]);
        return root;
    }
    else // 未搜索到角落，继续向下一层寻找
        return FindCornerStations(root->kids[X][Y], X, Y);
}

// 将方向字符串转换为位置查询四角基站
Quadtree *FindCorner(Quadtree *root)
{
    string direction = "方向";
    printf("\n请输入想要查找的角落块方向:\n");
    scanf("%s", &direction[0]);
    if (direction == "西南")
        return FindCornerStations(root, 0, 0);
    else if (direction == "西北")
        return FindCornerStations(root, 0, 1);
    else if (direction == "东南")
        return FindCornerStations(root, 1, 0);
    else if (direction == "东北")
        return FindCornerStations(root, 1, 1);
    else
        printf("输入方向不规范\n");
    return NULL;
}

// 递归寻找同样大小的侧块，输出该区域的所有基站
Quadtree *OutputSide(Quadtree *root, int depth, int x, int y)
{
    if (!root)
        return NULL;
    if ((x > root->x + root->w + 1) || (x < root->x - root->w - 1) || (y > root->y + root->h + 1) || (y < root->y - root->h - 1))
        return NULL; // 查询点不在四叉树范围内
    if (!root->kids[(x > root->x) ? 1 : 0][(y > root->y) ? 1 : 0] || root->depth == depth)
        return OutputLeaf(root); // 找到包含目标点的结点，该结点要么为深度小于depth的叶子结点，要么为深度等于depth的非叶子结点
    else                         // 没找到叶子或查找深度不够
        return OutputSide(root->kids[(x > root->x) ? 1 : 0][(y > root->y) ? 1 : 0], depth, x, y);
}

// 寻找分块某一侧同大小分块并输出区域中所有的基站
Quadtree *FindSideStations(Quadtree *root, Quadtree *node, int x, int y)
{
    if (!node)
        return NULL;
    int xt = node->x + x * node->w, yt = node->y + y * node->h; // 待查找结点中心坐标
    return OutputSide(root, node->depth, xt, yt);
}

// 将方向转换为点查询某侧基站
Quadtree *FindSide(Quadtree *root, Quadtree *node)
{
    string side = "方向";
    printf("\n请输入要查找的侧块方向:\n");
    scanf("%s", &side[0]);
    if (side == "西南")
        return FindSideStations(root, node, -2, -2);
    else if (side == "西边")
        return FindSideStations(root, node, -2, 0);
    else if (side == "西北")
        return FindSideStations(root, node, -2, 2);
    else if (side == "北边")
        return FindSideStations(root, node, 0, 2);
    else if (side == "东北")
        return FindSideStations(root, node, 2, 2);
    else if (side == "东边")
        return FindSideStations(root, node, 2, 0);
    else if (side == "东南")
        return FindSideStations(root, node, 2, -2);
    else if (side == "南边")
        return FindSideStations(root, node, 0, -2);
    else
        printf("输入方向不规范\n");
    return NULL;
}
