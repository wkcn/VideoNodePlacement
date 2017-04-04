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
typedef vector<int> Sequence;
typedef vector<Sequence> Sequences;

const int NODE_NUM_MAX = 1010;

int nodeNum, linkNum, consumerNum;
int serverCost;
vector<vector<int> > nodes; // 节点数, 每个节点对应了链路的ID
vector<Link> links; // 记录链路条数
vector<Consumer> consumers;
bool video_node[NODE_NUM_MAX];
Link* G[NODE_NUM_MAX][NODE_NUM_MAX];

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
	if (re < 0)return INT_MAX;
	return link.perCost;
}

int GetRoute(Consumer &cs, Route &r){
	// ACO 蚁群算法 (贪心, 非最优解)
	// 返回是否找到路径
	priority_queue<pair<int, int> > q;
	q.push(make_pair(0, cs.nodeid));
	vector<pair<int, int> > lastBestNode(nodeNum, make_pair(INT_MAX, -1));
	vector<bool> vis(nodeNum, false);
	bool found = false;
	int cost = 0;
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
			int tloss = GetLinkLoss(li);
			if (tloss == INT_MAX)continue;
			int tid = (li.start != id ? li.start : li.end);
			int nloss = loss + tloss;
			if (nloss < lastBestNode[tid].first){
				lastBestNode[tid] = make_pair(nloss, tid);
				if (tid == cs.nodeid){
					found = true;
					break;
				}
			}
			if (!vis[tid])
				q.push(make_pair(nloss, tid)); 
		}
	}
	if (found){
		stack<int> st;
		pair<int,int> &p = lastBestNode[cs.nodeid];
		int t = p.second;
		while (t != -1){
			st.push(t);
			t = lastBestNode[t].second;
		}
		while (!st.empty()){
			r.push_back(st.top());st.pop();
		}
		r.push_back(cs.nodeid);
		// fill BandWidth
		int bd = INT_MAX;
		for (size_t i = 0;i < r.size() - 1;++i){
			// i, i+1
			Link &li = *G[r[i]][r[i+1]];
			bd = min(bd, li.remainBandwidth); 
		}
		bd = min(bd, cs.nowNeed);
		for (size_t i = 0;i < r.size() - 1;++i){
			Link &li = *G[r[i]][r[i+1]];
			li.remainBandwidth -= bd;
			cost += li.perCost * bd;
		}
		cs.nowNeed -= bd;
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
}
void select(Sequences &seqs){
}
void crossover(Sequences &seqs){
}
void mutate(Sequences &seqs){
}

void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename){
	Reader reader(topo);
	nodeNum = reader.get_num(); 
	linkNum = reader.get_num(); 
	consumerNum = reader.get_num(); 
	cout << nodeNum << "|" << linkNum << "|" << consumerNum << endl;
	serverCost = reader.get_num(); // 单个视频节点花费
	cout << "ServerCost: " << serverCost << endl;
	nodes.resize(nodeNum);
	links.resize(linkNum);
	consumers.resize(consumerNum);
	memset(G, 0, sizeof(G));
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
		G[link.start][link.end] = &link;
		G[link.end][link.start] = &link;
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
	const int SEQ_NUM = 100;
	Sequences seqs;
	random_seqs(seqs);
	int st_time = time(0);
	const int TIME_LIMIT = 90 - 20; // max 90s 
	do{
		select(seqs); // 选择
		// 若种群无变化, 退出循环
		crossover(seqs); // 交叉
		mutate(seqs); // 变异
	}while(time(0) - st_time <= TIME_LIMIT);
	// output
	// 输出最短路
	string res;
    char * topo_file = (char *)res.c_str();
    write_result(topo_file, filename);
}
