#include "../Headdefine.h"

void swap(vector<int> &a, int x, int y)
{
    int tmp = a[x];
    a[x] = a[y];
    a[y] = tmp;
}

//array�Ǵ������Ķ����飬i�Ǵ�����������Ԫ�ص�λ�ã�nlength������ĳ���
//�����������ǣ���������array���������
void HeapAdjust(vector<int> &array, int i, int nLength)
{
    int nChild;
    int nTemp;
    for (nTemp = array[i]; 2 * i + 1 < nLength; i = nChild)
    {
        // �ӽ���λ��=2 *�������λ�ã�+ 1
        nChild = 2 * i + 1;

        // �õ��ӽ���нϴ�Ľ��
        if (nChild < nLength - 1 && array[nChild + 1] > array[nChild])
            ++nChild;

        // ����ϴ���ӽ����ڸ������ô�ѽϴ���ӽ�������ƶ����滻���ĸ����
        if (nTemp < array[nChild])
        {
            array[i] = array[nChild];
            array[nChild]= nTemp;
        }
        else
        {
            // �����˳�ѭ��
            break;
        }
    }
}

// �������㷨
void HeapSort(vector<int> &array)
{
    int i = array.size() / 2 ;
    cout << i << endl;
    // �������е�ǰ�벿��Ԫ�أ�������֮���һ��Ԫ�������е�����Ԫ��
    for (; i >= 0; --i)
    {
        HeapAdjust(array, i, array.size());
    }
    // �����һ��Ԫ�ؿ�ʼ�����н��е��������ϵ���С�����ķ�Χֱ����һ��Ԫ��
    for (int i = array.size() - 1; i > 0; --i)
    {
        // �ѵ�һ��Ԫ�غ͵�ǰ�����һ��Ԫ�ؽ�����
        // ��֤��ǰ�����һ��λ�õ�Ԫ�ض��������ڵ��������֮������
        swap(array, 0, i);

        // ������С����heap�ķ�Χ��ÿһ�ε�����ϱ�֤��һ��Ԫ���ǵ�ǰ���е����ֵ
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
    cout << "����ǰ" << endl;

    HeapSort(a);

    for (int i = 0; i < a.size(); i++)
    {
        cout << a[i] << "_";
    }
    cout << "�����" << endl;

    return 0;
}

