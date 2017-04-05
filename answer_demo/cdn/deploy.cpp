#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <queue>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include <climits>
#include <stack>
#include <ctime>

using namespace std;

struct Reader{
	char **data;
	int line;
	char *c;
	Reader(char **_data):data(_data){
		line = -1;
		c = 0;
	}
	char get_char(){
		while (!c || !(*c)){
			c = data[++line];
		}
		return *(c++);
	}
	int get_num(){
		int num = 0;
		char c = get_char();
		while (c == ' ' || c == '\n' || c == '\r'){
			c = get_char(); // jump space
		}
		// number
		while (c != ' ' && c != '\n' && c != '\r'){
			num = num * 10 + c - '0';
			c = get_char();
		}
		return num;
	}
};

struct Link{
	int start, end;
	int totalBandwidth;
	int perCost;
	// 附加信息
	double pher; // 信息素浓度
	int remainBandwidth;
	Link():pher(0.0){}
};

struct Consumer{
	int id;
	int nodeid; // 相连的网络节点ID
	int need; // 带宽需求
	// 附加信息
	int nowNeed; // 当前的带宽需求
};

typedef vector<int> Route;
typedef vector<Route> Routes;
typedef vector<int> Sequence;//一组视频节点
typedef vector<Sequence> Sequences;//一群个体

const int NODE_NUM_MAX = 1010;

int nodeNum, linkNum, consumerNum;
int serverCost;
vector<vector<int> > nodes; // 节点数, 每个节点对应了链路的ID
vector<Link> links; // 记录链路条数
vector<Consumer> consumers;
bool video_node[NODE_NUM_MAX];
int G[NODE_NUM_MAX][NODE_NUM_MAX]; // 映射到link ID
int bestScore = INT_MAX;// 越小越好
Sequence bestSeq;
Routes bestRoutes;
const int SEQ_NUM = 300;

int GetCost(Routes &routes){
	// 得到真实的代价
	// 假设routes合法
	set<int> videoNode;
	int tot_cost = 0;
	return tot_cost;
}

int GetEval(Routes &routes){
	// 得到估计的代价
	return GetCost(routes);
}

int GetLinkLoss(Link &link){
	int re = link.remainBandwidth;
	return link.perCost;
}

int GetRoute(Consumer &cs, Route &r){
	// ACO 蚁群算法 (贪心, 非最优解)
	// 返回是否找到路径
	priority_queue<pair<int, int> > q;
	// 从消费节点出发, 找视频节点
	q.push(make_pair(0, cs.nodeid));
	vector<pair<int, int> > lastBestNode(nodeNum, make_pair(INT_MAX, -1));
	vector<bool> vis(nodeNum, false);
	bool found = false;
	int cost = 0;
	int video_id = 0;
	while (!q.empty() && !found){
		const pair<int,int> &p = q.top();
		int loss = p.first;
		int id = p.second;
		vis[id] = true;
		q.pop();
		// 找一个评分最高的节点[链路]
		vector<int> &ls = nodes[id];
		for (int &t : ls){
			// t 为链路ID
			Link &li = links[t];
			if (li.remainBandwidth <= 0)continue;
			int tloss = GetLinkLoss(li);
			int tid = (li.start != id ? li.start : li.end);
			if (vis[tid])continue;
			// id -> tid
			int nloss = loss + tloss;
			if (nloss < lastBestNode[tid].first){
				lastBestNode[tid] = make_pair(nloss, id);
				if (video_node[tid]){
					found = true;
					video_id = tid;
					break;
				}
			}
			q.push(make_pair(nloss, tid)); 
		}
	}
	if (found){
		stack<int> st;
		pair<int,int> &p = lastBestNode[video_id];
		st.push(video_id);
		int t = p.second;
		while (t != -1){
			st.push(t);
			t = lastBestNode[t].second;
		}
		while (!st.empty()){
			r.push_back(st.top());st.pop();
		}
		/*
		cout << "video" << endl;
		for (int i = 0;i < NODE_NUM_MAX;++i){
			if (video_node[i])cout << i << ", ";
		}	
		cout << endl;
		cout << video_id << "===" << cs.nodeid << endl;
		for (int w : r){
			cout << w << ", ";
		}
		cout << endl;
		cout << "----" << endl;
		*/
		// fill BandWidth
		int bd = INT_MAX;
		for (int i = 0;i < int(r.size()) - 1;++i){
			// i, i+1
			Link &li = links[G[r[i]][r[i+1]]];
			//cout << "li" << r[i] << ":" << r[i+1] << "|" << li.remainBandwidth << "," << li.totalBandwidth << endl;
			bd = min(bd, li.remainBandwidth); 
			//cout << bd << "inin" << endl;
		}
		/*
		for (Link &link : links){
			cout << link.remainBandwidth << ";;;" << endl;
		}
		*/
		//cout << "fend" << endl;
		bd = min(bd, cs.nowNeed);
		//cout << "fffffffffffFF"<< bd << endl;
		for (int i = 0;i < int(r.size()) - 1;++i){
			Link &li = links[G[r[i]][r[i+1]]];
			li.remainBandwidth -= bd;
			cost += li.perCost * bd;
		}
		cs.nowNeed -= bd;

		/*
		cout << endl;
		cout << "bd: " << bd << "|cost: " << cost << "#route: " << r.size() << " need: " << cs.nowNeed << endl;
		for (int w : r){
			cout << w << ", ";
		}
		cout << endl << "===" << endl;
		cout << endl;
		*/
		return cost;
	}
	return -1;
}

int UpdateRoutes(Sequence &seq, Routes &rs){
	// 使用蚁群算法，计算出较短路径
	// 出发点seq
	//int consumerNum = consumers.size();
	int cost = 0;
	rs.clear();
	for (Link &link : links){
		link.remainBandwidth = link.totalBandwidth;
	}
	for (Consumer &cs : consumers){
		cs.nowNeed = cs.need;
	}
	memset(video_node, 0, sizeof(video_node));
	for (int &i : seq)video_node[i] = true;

	// VideoNodeCost
	cost += seq.size() * serverCost; 

	//const int ANT_NUM = 100;
	Route route;
	for (size_t i = 0;i < consumers.size();++i){
		Consumer &cs = consumers[i];
		if (cs.nowNeed > 0){
			// Find route
			Route r;
			int co = GetRoute(cs, r);
			if (co == -1){
				// NA
				return -1;
			}
			rs.push_back(r);
			cost += co;
		}
	}
	/*
	for (int a = 0;a < ANT_NUM;++a){
		// 从consumers出发
		//rs.push_back(route);
	}*/
	return cost;
}

void random_seqs(Sequences &seqs){
	seqs.resize(SEQ_NUM);
	for (Sequence &seq : seqs){
		int p = 0;
		while (rand() % nodeNum > p){
			seq.push_back(rand() % nodeNum);
			p += rand() % nodeNum;
		}
	}
}
void select(Sequences &seqs){
	Sequences rank(SEQ_NUM);
	vector<int> scores(SEQ_NUM);
	int k = 0;
	int tot_score = 0;
	for (int i = 0;i < SEQ_NUM;++i){
		Routes rs;
		Sequence &seq = seqs[i];
		int score = UpdateRoutes(seq, rs); // score越小越好
		if (score != -1){
			// 只保留能生成完整路径的视频节点序列
			rank[k] = seq;
			tot_score += score;
			scores[k] = tot_score;
			++k;
			if (score < bestScore){
				bestScore = score;
				bestSeq = seq;
				bestRoutes = rs;
				cout << "BestScore: " << bestScore << endl;
				cout << "video Node: " << endl;
				for (int u:bestSeq){
					cout << u << ", ";
				}
				cout << endl << endl;
				cout << "best routes" << endl;
				for (Route &r : bestRoutes){
					for (int w : r){
						cout << w << " <- ";
					}
					cout << endl;
				}
				cout << endl << endl;
			}
		}
	}
	rank.resize(k);
	scores.resize(k);
	// 使用轮盘赌
	for (int i = 0;i < SEQ_NUM;++i){
		int s = rand() % tot_score;
		int j = 0;
		while (s > scores[j])++j;
		//  注意, 这里是反过来的, 因为score越小越好
		seqs[i] = rank[k - 1 - j];
	}
}
void crossover(Sequences &seqs){
	// vector<int>
	int len=seqs.size();
	for(int i=0;i<len;++i)
	{
		if (rand() % 100 > 50){
			int randindex=rand()%len;
			int randindex1=rand()%len;
			int onelen=seqs[randindex].size();
			int twolen=seqs[randindex1].size();
			int rand1=rand()%onelen;
			int rand2=rand()%twolen;
			int temp=seqs[randindex][rand1];
			seqs[randindex][rand1]=seqs[randindex1][rand2];
			seqs[randindex1][rand2]=temp;
		}
	}
}
void mutate(Sequences &seqs){
	int len=seqs.size();
	for(int i=0;i<len;++i){
		int add=rand() % 100;
		if (add > 80 && seqs[i].size() > 1){
			int sublen=seqs[i].size();
			int randindex=rand()%sublen;
			seqs[i].erase(seqs[i].begin()+randindex);
		}else if(add > 95){
			seqs[i].push_back(rand()%nodeNum);
		}
	}
}

void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename){
	Reader reader(topo);
	nodeNum = reader.get_num(); 
	linkNum = reader.get_num(); 
	consumerNum = reader.get_num(); 
	cout << "nodeNum | linkNum | consumerNum" << endl;
	cout << nodeNum << "|" << linkNum << "|" << consumerNum << endl;
	serverCost = reader.get_num(); // 单个视频节点花费
	cout << "ServerCost: " << serverCost << endl;
	nodes.resize(nodeNum);
	links.resize(linkNum);
	consumers.resize(consumerNum);
	for (int i = 0;i < linkNum;++i){
		Link link;
		link.start = reader.get_num();
		link.end = reader.get_num();
		link.totalBandwidth = reader.get_num();
		link.perCost = reader.get_num();
		links[i] = link; 
		// 加入链路ID
		nodes[link.start].push_back(i);
		nodes[link.end].push_back(i);
		G[link.start][link.end] = i;
		G[link.end][link.start] = i;
	}
	for (int i = 0;i < consumerNum;++i){
		Consumer e;
		e.id = reader.get_num();
		e.nodeid = reader.get_num();
		e.need = reader.get_num();
		consumers[i] = e;
	}
	// GO
	// 使用遗传算法生成一个视频节点ID序列Sequence
	// rs为输出结果
	// 调用int UpdateRoutes(Sequence &seq, Routes &rs){
	// 若函数返回-1, 则不存在这样的路径
	// 若函数返回正数, 代表实际的代价
	Sequences seqs;
	random_seqs(seqs);
	int st_time = time(0);
	const int TIME_LIMIT = 90 - 20; // max 90s 
	do{
		//cout << "select" << endl;
		select(seqs); // 选择
		// 若种群无变化, 退出循环
		//cout << "crossover" << endl;
		crossover(seqs); // 交叉
		//cout << "mutate" << endl;
		mutate(seqs); // 变异
		//cout << time(0) - st_time << endl;
	}while(time(0) - st_time <= TIME_LIMIT);
	// output
	// 输出最短路
	string res;
    char * topo_file = (char *)res.c_str();
    write_result(topo_file, filename);
}
