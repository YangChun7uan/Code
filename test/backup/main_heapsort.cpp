#include "../Headdefine.h"

void swap(vector<int> &a, int x, int y)
{
    int tmp = a[x];
    a[x] = a[y];
    a[y] = tmp;
}

//array是待调整的堆数组，i是待调整的数组元素的位置，nlength是数组的长度
//本函数功能是：根据数组array构建大根堆
void HeapAdjust(vector<int> &array, int i, int nLength)
{
    int nChild;
    int nTemp;
    for (nTemp = array[i]; 2 * i + 1 < nLength; i = nChild)
    {
        // 子结点的位置=2 *（父结点位置）+ 1
        nChild = 2 * i + 1;

        // 得到子结点中较大的结点
        if (nChild < nLength - 1 && array[nChild + 1] > array[nChild])
            ++nChild;

        // 如果较大的子结点大于父结点那么把较大的子结点往上移动，替换它的父结点
        if (nTemp < array[nChild])
        {
            array[i] = array[nChild];
            array[nChild]= nTemp;
        }
        else
        {
            // 否则退出循环
            break;
        }
    }
}

// 堆排序算法
void HeapSort(vector<int> &array)
{
    int i = array.size() / 2 ;
    cout << i << endl;
    // 调整序列的前半部分元素，调整完之后第一个元素是序列的最大的元素
    for (; i >= 0; --i)
    {
        HeapAdjust(array, i, array.size());
    }
    // 从最后一个元素开始对序列进行调整，不断的缩小调整的范围直到第一个元素
    for (int i = array.size() - 1; i > 0; --i)
    {
        // 把第一个元素和当前的最后一个元素交换，
        // 保证当前的最后一个位置的元素都是在现在的这个序列之中最大的
        swap(array, 0, i);

        // 不断缩小调整heap的范围，每一次调整完毕保证第一个元素是当前序列的最大值
        HeapAdjust(array, 0, i);
    }
}

int main(int argc, char** argv)
{
    vector<int> a;
    for (int i = 1; i < argc; i++)
    {
        a.push_back(atoi(argv[i]));
    }

    for (int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "排序前" << endl;

    HeapSort(a);

    for (int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "排序后" << endl;

    return 0;
}

