#include<iostream>
#include<fstream>
#include<vector>
#include<random>
#include<windows.h>
#include<algorithm>
#include<cmath>
#include <omp.h>
#include <thread>
using namespace std;

namespace Nerve
{
	//神经元结构 
	struct Neuron
	{
		double net = 0.0, out = 0.0, p = 0.0;							//神经元基本属性 
		vector<double> w;												//神经元突触属性，该神经元与上层神经元的之间的权值 
	};
	
	//层结构
	struct Layer
	{
		vector<Neuron> neurons;
		Layer(int n)													//创建一神经元数量为n的层 
		{
			for(int i=0;i<n;i++)
				neurons.push_back(Neuron());
		}
		 
		Layer(int n,const Layer &pre)									//创建一层神经元，并自动连接上一层（初始化权值、偏置） 
		{
			for(int i=0;i<n;i++)						
				neurons.push_back(Neuron());
				
			default_random_engine e(GetTickCount());/*初始化随机引擎*/
			uniform_real_distribution<double> u(0.0,1.0);

			int m = pre.neurons.size();
			for (int i = 0; i < neurons.size(); i++)
			{
				for (int j = 0; j < m; j++)
				{
					neurons[i].w.push_back(u(e));
					neurons[i].p = u(e);
				}
			}
		}
	}; 
	
	//神经网络 
	class Nerve_net
	{
		//==========================================================//==========================================================

	public :
		Nerve_net(int n,int m,const vector<int> &mid)					//创建一个n个神经元输入层m个神经元输出层的神经网络,mid为各中间层的神经元个数
		{
 			layers.push_back(Layer(n));

			//创建中间层
			int _max = max(n, m);//求最厚层
			for (auto i = 0; i < mid.size(); i++)
			{
				layers.push_back(Layer(mid[i], layers.back()));
				_max = max(mid[i], _max);
			}
			d_H[0].resize(_max);
			d_H[1].resize(_max);
			layers.push_back(Layer(m, layers.back()));
			for (int i = 0; i < layers.size(); i++)
				d_Wjk.push_back(vector<vector<double>>(_max, vector<double>(_max, 0.0)));
			d_P.resize(layers.size(), vector<double>(_max, 0.0));
		}

		//为了方便测试，我增加了保存神经网络到文件的操作
		Nerve_net(char *file)
		{
			int n, m;
			ifstream in(file, ios::binary);
			in.read((char*)&n, 4);		//1~4 byte 网络隐藏层数

			in.read((char*)&m, 4);		//5~8 byte 输入层神经元数目
			int _max = m;				//求最厚层
			layers.push_back(Layer(m));

			for (int i = 0; i < n; i++)	//9~(n*4-1) byte 每层神经元数目
			{
				in.read((char*)&m, 4);
				layers.push_back(Layer(m, layers.back()));
				_max = max(m, _max);
			}
			in.read((char*)&m, 4);		//xxx byte 输出层神经元数目
			layers.push_back(Layer(m, layers.back()));
			_max = max(m, _max);

			d_H[0].resize(_max);
			d_H[1].resize(_max);
			for (int i = 0; i < layers.size(); i++)
				d_Wjk.push_back(vector<vector<double>>(_max, vector<double>(_max, 0.0)));
			d_P.resize(layers.size(), vector<double>(_max, 0.0));
			
			double data;
			for (int i = 1; i < layers.size(); i++)
			{
				for (int j = 0; j < layers[i].neurons.size(); j++)
				{
					in.read((char*)&data, sizeof(double));
					layers[i].neurons[j].p = data;
					for (int k = 0; k < layers[i - 1].neurons.size(); k++)
					{
						in.read((char*)&data, sizeof(double));
						int u = in.gcount();
						layers[i].neurons[j].w[k] = data;
					}
				}
			}
			in.read((char*)&learning_rate, sizeof(double));
			in.close();
		}
		void SaveTofile(char *file)
		{
			ofstream out(file, ios::binary);						//保存结构信息
			int n = layers.size() - 2;
			int a = 0;
			out.write((char*)&n, 4);
			for (int i = 0; i < layers.size(); i++)
			{
				a = layers[i].neurons.size();
				out.write((char*)&a, 4);
			}

			for (int i = 1; i < layers.size(); i++)	//保存偏置、边权
			{
				for (int j = 0; j < layers[i].neurons.size(); j++)
				{
					out.write((char*)&layers[i].neurons[j].p, sizeof(double));
					for (int k = 0; k < layers[i - 1].neurons.size(); k++)
					{
						out.write((char*)&layers[i].neurons[j].w[k], sizeof(double));
					}
				}
			}
			out.write((char*)&learning_rate, sizeof(double));
			out.close();
		}

		bool Input(const vector<double> inp)							//向网络输入信息.
		{
			if (inp.size() != layers[0].neurons.size())return -1;

			for (int i = 0; i < layers[0].neurons.size(); i++)
				layers[0].neurons[i].out = inp[i];
			return 0;
		}

		bool Set_Desired_output(const vector<double> &val)				//设置期望输出
		{
			if (val.size() != layers.back().neurons.size())return false;
			for (int i = 0; i < layers.back().neurons.size(); i++)
			{
				if (desired_ouput.size() <= i)
					desired_ouput.push_back(val[i]);
				else
					desired_ouput[i] = val[i];
			}
		}

		void Figue()													//逐层正向计算
		{
			for (int i = 1; i < layers.size(); i++)
			{
#pragma omp parallel for//一步加速
				for (int j = 0; j < layers[i].neurons.size(); j++)
				{
					f(layers[i - 1], layers[i].neurons[j]);
				}
			}
		}

		vector<double> Output() const									//返回输出层信息
		{
			vector<double> out;
			for (int i = 0; i < layers.back().neurons.size(); i++)
				out.push_back(layers.back().neurons[i].out);
			return out;
		}

		void Began_training()
		{
			if (if_training)Send_error("is training");
			samples = 0;
			for (int i = 1; i < layers.size(); i++)
				for (int j = 0; j < layers[i].neurons.size(); j++)
					for (int k = 0; k < layers[i - 1].neurons.size(); k++)
						d_Wjk[i][j][k] = 0.0;

			for (int i = 1; i < layers.size(); i++)
				for (int j = 0; j < layers[i].neurons.size(); j++)
					d_P[i][j] = 0.0;
			if_training = true;
		}
		void Learn()													//反向传播！(BP)
		{
			if (!if_training)Send_error("is no training");
			//对于保存δH、δw的数据结构，这里我选择重复利用容器节省时间，所以要分情况讨论

			for (int i = 0; i < layers.back().neurons.size(); i++)//先求出输出层的δH
			{
				d_H[bjH][i] = Figue_d_output_layer(i);
			}
			for (int L = layers.size() - 1; L > 0; L--)//循环层，L层即当前处理层
			{
				//清空d_H
				fill(d_H[bjH ^ 1].begin(), d_H[bjH ^ 1].end(), 0.0);
#pragma omp parallel for//一步加速
				for (int j = 0; j < layers[L].neurons.size(); j++)//循环L每一个神经元
				{
					for (int k = 0; k < layers[L - 1].neurons.size(); k++)//循环L-1每一个神经元
					{
						d_Wjk[L][j][k] += Figue_d_w(L, j, k);//累加当前样本的δ权值

						d_H[bjH ^ 1][k] += Figue_d_H(L, j, k);
					}
					d_P[L][j] += d_H[bjH][j];//累加当前样本的δ偏置
				}
				
				bjH ^= 1;
			}
			samples++;//处理样本数+1
		}

		void End_training()
		{
			if (!if_training)Send_error("is no training");
			if_training = false;
			for (int L = layers.size() - 1; L > 0; L--)//循环层，L层即当前处理层
			{
				for (int j = 0; j < layers[L].neurons.size(); j++)//循环L每一个神经元
				{
					for (int k = 0; k < layers[L - 1].neurons.size(); k++)//循环L-1每一个神经元
					{
						//更新L层的W
						layers[L].neurons[j].w[k] -= learning_rate * d_Wjk[L][j][k] / (double)samples;
					}
					//更新L层的P(由公式可推知δPj=δHj，所以就可以很好偷懒)
					layers[L].neurons[j].p -= learning_rate * d_P[L][j] / (double)samples;
				}
			}
		}
		//========================================================Math=========================================================

		double sigmoid(double x) 
		{ 
			return 1 / (1 + exp(-x)); 
		}
		//double sigmoid_d(double) {return sigmoid(x)(1-sigmoid(x))};

		double C() const													//咕价函数~
		{
			int last = layers.size()-1;

			double cc = 0.0;
			for (int i = 0; i < layers[last].neurons.size(); i++)
			{
				cc += pow(layers[last].neurons[i].out - desired_ouput[i], 2);
			}
			return cc;
		}

	private:
		void f(const Layer &L, Neuron &N)
		{
			N.net = 0.0;
			for (int i = 0; i < L.neurons.size(); i++)
				N.net += L.neurons[i].out*N.w[i];
			N.net += N.p;
			N.out = sigmoid(N.net);
		}
		double Figue_d_output_layer(int n)								//计算输出层神经元的net部分对估价函数的偏导,即δH_last
		{
			return	2 * (layers.back().neurons[n].out - desired_ouput[n]) *
				(1 - layers.back().neurons[n].out)*layers.back().neurons[n].out;
		}
		double Figue_d_w(int L, int j, int k)							//计算L层的(j-k)边权值的偏导（δWjk）
		{
			return d_H[bjH][j] * layers[L-1].neurons[k].out;
		}
		double Figue_d_H(int L,int j,int k)								//计算L层神经元j对L-1层神经元k的偏导值δHk的贡献
		{
			return d_H[bjH][j] * layers[L].neurons[j].w[k] *
				(1 - layers[L - 1].neurons[k].out)*layers[L - 1].neurons[k].out;
		}
		void Send_error(char *info) 
		{
			MessageBox(NULL, info, "class Nerve_net", MB_OK);
		}
	private:
		vector<Layer>			layers;
		vector<double>			desired_ouput;
		double					learning_rate = 0.5;
		int						samples;//一次训练的样本数
		bool					if_training;//是否正在训练中

		vector<double>			d_H[2];	//Train过程中处理层和将处理层的偏导δH
		short					bjH = 0;//标记当前d_H使用的是哪一个
		vector<vector<vector<double>>>	d_Wjk;	//learn过程所有权值偏导
		vector<vector<double>>			d_P;	//learn过程所有偏置偏导
	};
}
using namespace Nerve;

//int main()//a simple example
//{
//	Nerve_net net(4, 3, vector<int>{3,4});
//	//Nerve_net net("a.nerve");//从文件读入
//	int o = 60000;
//	while (o--)
//	{
//		net.Began_training();								//① 
//
//		net.Input(vector<double>{1, 1, 0, 0});				//②
//		net.Set_Desired_output(vector<double>{1, 0, 0});	//③
//		net.Figue();										//④
//		net.Learn();										//⑤
//
//		net.Input(vector<double>{0, 1, 1, 0});
//		net.Set_Desired_output(vector<double>{0, 1, 0});
//		net.Figue();
//		net.Learn();
//
//		net.Input(vector<double>{0, 0, 1, 1});
//		net.Set_Desired_output(vector<double>{0, 0, 1});
//		net.Figue();
//		net.Learn();
//
//		net.End_training();									//⑥
//
//	}
//	//教会它识别01
//	net.Input(vector<double>{1, 1, 0, 0.0});
//	net.Figue();
//	vector<double> oo = net.Output();
//	for (int i = 0; i < oo.size(); i++)
//		std::cout << oo[i] << " ";
//
//	std::cout << endl;
//
//	net.Input(vector<double>{0.0,0.00,1.0,0.41});
//	net.Figue();
//	vector<double> ooo = net.Output();
//	for (int i = 0; i < ooo.size(); i++)
//		std::cout << ooo[i] << " ";
//
//	//net.SaveTofile("a.nerve");//保存到文件
//}
