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
struct Train
{
	int train_times;	//训练次数

	int exam_times;		//测试次数 

	int exam_A;			//正确样本数
	int exam_W;			//错误样本数 
	
	Nerve_net &nerve;	//训练神经网络 
	char Nerve_ID[16];	//神经网络ID 
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
		ifstream in(file,ios::binary);
		Data data;
		vector<double> vec(N*N,0.0);
		vector<double> outp(10,0.0);
		while(1)
		{
			in.read((char*)&data,sizeof(data));
			if(in.gcount()!=0)
			{
				for(int i=0;i<N*N;i++)
					vec[i]=data.point[i];
				outp[data.ans]=1.0;
				nerve.Input(vec);
				nerve.Set_Desired_output(outp);
				nerve.Figue();
				nerve.Train();
				
				train_times++;
			}
			else break;
		}
	}
	void exam(char *file)
	{
		ifstream in(file,ios::binary);
		Data data;
		vector<double> vec(N*N,0.0);
		vector<double> ans(10,0.0);
		while(1)
		{
			in.read((char*)&data,sizeof(data));
			if(in.gcount()!=0)
			{
				for(int i=0;i<N*N;i++)
					vec[i]=data.point[i];
				nerve.Input(vec);
				nerve.Figue();
				ans=nerve.Output();
				double maxx=0;
				int k=0;
				for(int i=0;i<10;i++)
				if(maxx<ans[i])
					maxx=ans[i],k=i;
				if(k==data.ans)exam_A++;
				else exam_W++;
			}
			else break;
		}
	}
	void output_statistic(ostream &out)
	{
	}
};
int main()
{
	
    return 0;
}
