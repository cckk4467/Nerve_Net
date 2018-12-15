#include<iostream>
#include<fstream>
#include<vector>
#include"..//Nerve//Nerve.cpp"
#include<windows.h>
using namespace std;
/*
单体数据结构
*/
#define N 28
class Train
{
	int train_times = 0;	//训练次数

	int exam_times = 0;		//测试次数 

	int exam_A = 0;			//正确样本数
	int exam_W = 0;			//错误样本数 
	
	Nerve_net &nerve;	//训练神经网络 

	struct Data
	{
		char point[N * N];
		char ans;
	};
	
public:
	Train(Nerve_net &i):nerve(i)
	{
	}
	void train(char *file)
	{
		ifstream in(file, ios::binary);
		Data data;
		vector<double> vec(N*N, 0.0);
		vector<double> outp(10, 0.0);
		bool ok = false;
		while (!ok)
		{
			nerve.Began_training();
			for (int k = 1; k <= 6000; k++)
			{
				fill(outp.begin(), outp.end(), 0.0);
				in.read((char*)&data, sizeof(data));
				if (in.gcount() != 0)
				{
					for (int i = 0; i < N*N; i++)
						vec[i] = (unsigned char)data.point[i] / (double)255;
					outp[data.ans] = 1.0;
					nerve.Input(vec);
					nerve.Set_Desired_output(outp);
					nerve.Figue();
					cout << nerve.C() << endl;
					nerve.Learn();

					train_times++;
				}
				else ok = true;
			}
			nerve.End_training();
		}
		in.close();
	}
	void exam(char *file)
	{
		ifstream in(file, ios::binary);
		Data data;
		vector<double> vec(N*N, 0.0);
		vector<double> ans(10, 0.0);
		while(1)
		{
			in.read((char*)&data,sizeof(data));
			if(in.gcount()!=0)
			{
				for(int i=0;i<N*N;i++)
					vec[i] = (unsigned char)data.point[i] / (double)255;
				nerve.Input(vec);
				nerve.Figue();
				ans = nerve.Output();
				char k=0;
				for(char i=1;i<10;i++)
				if(ans[k]<ans[i])
					k=i;
				if(k==data.ans)exam_A++;
				else exam_W++;

				exam_times++;
			}
			else break;
		}
		in.close();
	}
	void output_statistic(ostream &out)
	{
		out << "**********************************************************" << endl;
		out << "训练次数:" << train_times << endl;
		out << "测试次数:" << exam_times << endl;
		out << "正确样本数:" << exam_A << endl;
		out << "错误样本数:" << exam_W << endl;
		out << "正确率:" << (double)exam_A / exam_times * 100 << "%" << endl;
		out << "**********************************************************" << endl;
	}
};
int main()
{
	Nerve_net nerve(N*N, 10, vector<int>{16,16});
	Train train(nerve);
	train.train("export.train");
	train.exam("export.exam");
	train.output_statistic(cout);

	ofstream out("statistic.txt");
	train.output_statistic(cout);
	train.output_statistic(out);
	out.close();
    return 0;
}
