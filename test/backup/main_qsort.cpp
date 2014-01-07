#include "../Headdefine.h"

void swap(vector<int> &a, int x, int y)
{
    int tmp = a[x];
    a[x] = a[y];
    a[y] = tmp;
}

int findmid(vector<int> &a, int begin, int end, int key_pos)
{
    cout << "findmid begin:" << begin << " end:" << end << endl;

    if( end <= begin )
        return begin;

    int key = a[key_pos];

    int posd = end;
    for(; posd > begin; posd--)
    {
        if(key > a[posd])
        {
            a[key_pos] = a[posd]; //两个位置交换
            a[posd] = key;        //两个位置交换
            break;
        }
    }
    for(int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "从后往前比较结果" << endl;

    int key_pos1 = posd;
    if( (end - begin) == 1)
        return key_pos1;
    if(key_pos1 == begin)
        return key_pos1;

    int posa = key_pos + 1;
    for(; posa < key_pos1; posa++)
    {
        if(key < a[posa])
        {
            a[key_pos1] = a[posa];  //两个位置交换
            a[posa] = key;          //两个位置交换
            break;
        }
    }

    int key_pos2 = posa;

    for(int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "从前往后比较结果" << endl;

    return findmid(a, key_pos2, key_pos1-1, key_pos2);
}

int qsort(vector<int> &a, int begin, int end)
{
    cout << "qsort begin:" << begin << " end:" << end << endl;
    if(end <= begin)
        return 0;

    int mid = findmid(a, begin, end, begin);
    cout << "findmid:" << mid << endl;


    qsort(a, begin, mid -1);
    qsort(a, mid +1 , end);

    return 0;
}
/////////////////////////////



void qs(vector<int> &a, int begin, int end)
{
    int compare = a[begin], left = begin, right = end;

    if (left > right)
        return;
    while (left < right)
    {
        while ((left < right) && a[right] >= compare)
            right--;
        swap(a, left, right);

        while ((left < right) && (a[left] < compare))
            left++;
        swap(a, left, right);
    }
    a[right] = a[left];

    qs(a, begin, right - 1);
    qs(a, right + 1, end);
}

int main(int argc, char** argv)
{
    vector<int>  a;
    for(int i = 1; i < argc; i++)
    {
        a.push_back( atoi(argv[i]) );
    }

    for(int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "排序前" << endl;

    qs(a, 0, a.size()-1);

    for(int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "排序后" << endl;

    return 0;
}


