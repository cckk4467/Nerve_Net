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
	Builder(char *file, char *file2):in(file),ans_in(file2)
	{
		int		a = 0;
		char	p[28 * 28], ans;
		in.read((char*)&a, 4);		ans_in.read((char*)&a, 4);
		in.read((char*)&number, 4);	ans_in.read((char*)&a, 4);
		in.read((char*)&a, 4);		ans_in.read((char*)&a, 4);
		in.read((char*)&a, 4);		ans_in.read((char*)&a, 4);
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
	void export(char *k)
	{
		if (out.is_open())out.close();
		out.open(k);

		bool end = false;
		int count = 0;
		while (!end)
		{
			for (int i = 0; i < 10; i++)
			{
				if(count<data[i].size())
					out.write((char*)&data[i][count], sizeof(Data));
			}
		}
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