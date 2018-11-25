#include<iostream>
#include<vector>
#include<random>
#include<windows.h>
#include<algorithm>
#include<cmath>
#include <omp.h>
using namespace std;

namespace Nerve
{
	//��Ԫ�ṹ 
	struct Neuron
	{
		double net = 0.0, out = 0.0, p = 0.0;							//��Ԫ�������� 
		vector<double> w;												//��Ԫͻ�����ԣ�����Ԫ���ϲ���Ԫ��֮���Ȩֵ 
	};
	
	//��ṹ
	struct Layer
	{
		vector<Neuron> neurons;
		Layer(int n)													//����һ��Ԫ����Ϊn�Ĳ� 
		{
			for(int i=0;i<n;i++)
				neurons.push_back(Neuron());
		}
		 
		Layer(int n,const Layer &pre)									//����һ����Ԫ�����Զ�������һ�㣨��ʼ��Ȩֵ��ƫ�ã� 
		{
			for(int i=0;i<n;i++)						
				neurons.push_back(Neuron());
				
			default_random_engine e(GetTickCount());/*��ʼ���������*/
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
	
	//������ 
	class Nerve_net
	{
		//==========================================================//==========================================================

	public :
		Nerve_net(int n,int m,const vector<int> &mid)					//����һ��n����Ԫ�����m����Ԫ������������,midΪ���м�����Ԫ����
		{
 			layers.push_back(Layer(n));

			//�����м��
			int _max = max(n, m);//������
			for (auto i = 0; i < mid.size(); i++)
			{
				layers.push_back(Layer(mid[i], layers.back()));
				_max = max(mid[i], _max);
			}
			d_H[0].resize(_max);
			d_H[1].resize(_max);
			d_Wjk.resize(_max, vector<double>(_max));
			layers.push_back(Layer(m, layers.back()));
		}

		bool Input(const vector<double> inp)							//������������Ϣ.
		{
			if (inp.size() != layers[0].neurons.size())return -1;

			for (int i = 0; i < layers[0].neurons.size(); i++)
				layers[0].neurons[i].out = inp[i];
			return 0;
		}

		bool Set_Desired_output(const vector<double> &val)				//�����������
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

		void Figue()													//����������
		{
			for (int i = 1; i < layers.size(); i++)
			{
#pragma omp parallel for//һ������
				for (int j = 0; j < layers[i].neurons.size(); j++)
				{
					f(layers[i - 1], layers[i].neurons[j]);
				}
			}
		}

		vector<double> Output()											//�����������Ϣ
		{
			vector<double> out;
			for (int i = 0; i < layers.back().neurons.size(); i++)
				out.push_back(layers.back().neurons[i].out);
			return out;
		}

		void Learn()													//���򴫲���(BP)
		{
			//���d_H
			fill(d_H[0].begin(), d_H[0].end(), 0.0);
			fill(d_H[1].begin(), d_H[1].end(), 0.0);
			//���ڱ����H����w�����ݽṹ��������ѡ���ظ�����������ʡʱ�䣬����Ҫ���������

			for (int i = 0; i < layers.back().neurons.size(); i++)//����������Ħ�H
			{
				d_H[bjH][i] = Figue_d_output_layer(i);
			}
			for (int L = layers.size() - 1; L > 0; L--)//ѭ���㣬L�㼴��ǰ�����
			{
#pragma omp parallel for//һ������
				for (int k = 0; k < layers[L - 1].neurons.size(); k++)//ѭ��L-1ÿһ����Ԫ
				{
					for (int j = 0; j < layers[L].neurons.size(); j++)//ѭ��Lÿһ����Ԫ
					{
						d_Wjk[j][k] = Figue_d_w(L, j, k);

						d_H[bjH ^ 1][k] += Figue_d_H(L, j, k);
					}

				}
				for (int j = 0; j < layers[L].neurons.size(); j++)//ѭ��Lÿһ����Ԫ
				{
					for (int k = 0; k < layers[L - 1].neurons.size(); k++)//ѭ��L-1ÿһ����Ԫ
					{
						//����L���W
						layers[L].neurons[j].w[k] -= learning_rate * d_Wjk[j][k];
					}
					//����L���P(�ɹ�ʽ����֪��Pj=��Hj�����ԾͿ��Ժܺ�͵��)
					layers[L].neurons[j].p -= learning_rate * d_H[bjH][j];
				}
				bjH ^= 1;
			}
		}
		//========================================================Math=========================================================

		double sigmoid(double x) 
		{ 
			return 1 / (1 + exp(-x)); 
		}
		//double sigmoid_d(double) {return sigmoid(x)(1-sigmoid(x))};

		double C()														//���ۺ���~
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
		double Figue_d_output_layer(int n)								//�����������Ԫ��net���ֶԹ��ۺ�����ƫ��,����H_last
		{
			return	2 * (layers.back().neurons[n].out - desired_ouput[n]) *
				(1 - layers.back().neurons[n].out)*layers.back().neurons[n].out;
		}
		double Figue_d_w(int L, int j, int k)							//����L���(j-k)��Ȩֵ��ƫ������Wjk��
		{
			return d_H[bjH][j] * layers[L-1].neurons[k].out;
		}
		double Figue_d_H(int L,int j,int k)								//����L����Ԫj��L-1����Ԫk��ƫ��ֵ��Hk�Ĺ���
		{
			return d_H[bjH][j] * layers[L].neurons[j].w[k] *
				(1 - layers[L - 1].neurons[k].out)*layers[L - 1].neurons[k].out;
		}
	private:
		vector<Layer>			layers;
		vector<double>			desired_ouput;
		double					learning_rate = 0.5;

		vector<double>			d_H[2];	//Learn�����д����ͽ�������ƫ����H
		short					bjH = 0;//��ǵ�ǰd_Hʹ�õ�����һ��
		vector<vector<double>>	d_Wjk;	//learn���̴���������Ȩֵƫ��
	};
}
using namespace Nerve;

int main()
{
	Nerve_net net(2, 2, vector<int>{5,4});
	
	int o = 10000;
	while (o--)
	{
		net.Input(vector<double>{0,1});
		net.Set_Desired_output(vector<double>{1,0});
		net.Figue();
		net.Learn();

		net.Input(vector<double>{1, 0});
		net.Set_Desired_output(vector<double>{0, 1});
		net.Figue();
		net.Learn();
	}
	//�̻���ʶ��01
	net.Input(vector<double>{0,1});
	net.Figue();
	vector<double> oo = net.Output();
	for (int i = 0; i < oo.size(); i++)
		cout << oo[i] << " ";
}
