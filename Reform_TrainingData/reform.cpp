#include<iostream>
#include<fstream>
#include<vector>
#include<random>
#include<windows.h>
using namespace std;
/*
�������ݽṹ
*/
struct Data
{
	char point[28 * 28];
	char ans;
};

/*
������Builder
�����ݼ������ݣ���������ݶ��½��ķ��������������
������������ĳ�ָ�ʽ���뵽.train�ļ�����
*/
class Builder
{
public:
	Builder(char *file, char *file2):in(file, ios::binary),ans_in(file2, ios::binary)
	{
		int		a = 0;
		char	p[28 * 28], ans;
		in.read((char*)&a, 4);		ans_in.read((char*)&a, 4);
		in.read((char*)&number, 4);	ans_in.read((char*)&a, 4);
		in.read((char*)&a, 4);
		in.read((char*)&a, 4);
		trans(number);

		for (int i = 0; i < number; i++)
		{
			in.read((char*)&p, 28 * 28);
			ans_in.read((char*)&ans, 1);

			data[ans].push_back(Data());
			memcpy(data[ans].back().point, p, 28 * 28);
			data[ans].back().ans = ans;
		}
		in.close();

	}
	void wan()
	{
		default_random_engine e(GetTickCount());/*��ʼ���������*/

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < data[i].size(); j++)
			{
				uniform_int_distribution<int> u(0, data[i].size()-1);
				swap(data[i][j], data[i][u(e)]);
			}
		}
	}
	void Export(char *k)
	{
		if (out.is_open())out.close();
		out.open(k, ios::binary);

		bool end = false;
		int count = 0;
		int lis[10] = { 0,1,2,3,4,5,6,7,8,9 };
		while (!end)
		{
			for (int i = 0; i<10; i++)//���1-9�����˳�� 
				swap(lis[i], lis[rand() % 10]);
			end = true;
			for (int i = 0; i < 10; i++)
			{
				if (count<data[lis[i]].size())
					out.write((char*)&data[lis[i]][count], sizeof(Data)), end = false;
			}
			count++;
		}
		out.close();
	}
private:
	ifstream	in;
	ifstream	ans_in;
	ofstream	out;

	int			number;
	vector<Data>data[10];

	void trans(int &n)
	{
		int a = n;
		((char*)&n)[0] = ((char*)&a)[3];
		((char*)&n)[1] = ((char*)&a)[2];
		((char*)&n)[2] = ((char*)&a)[1];
		((char*)&n)[3] = ((char*)&a)[0];
	}
};
int main()
{
    Builder r("t10k-images.idx3-ubyte","t10k-labels.idx1-ubyte");
    r.wan();
    r.Export("export.exam");
    return 0;
}
