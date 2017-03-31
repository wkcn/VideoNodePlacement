#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

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
};

struct Consumer{
	int id;
	int nodeid; // 相连的网络节点ID
	int need; // 带宽需求
};

vector<vector<int> > nodes; // 节点数, 每个节点对应了链路的ID
vector<Link> links; // 记录链路条数
vector<Consumer> consumers;

void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename){
	Reader reader(topo);
	int nodeNum, linkNum, consumerNum;
	nodeNum = reader.get_num(); 
	linkNum = reader.get_num(); 
	consumerNum = reader.get_num(); 
	cout << nodeNum << "|" << linkNum << "|" << consumerNum << endl;
	int serverCost = reader.get_num(); // 单个视频节点花费
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
		nodes[link.start].push_back(link.end);
		nodes[link.end].push_back(link.start);
	}
	for (int i = 0;i < consumerNum;++i){
		Consumer e;
		e.id = reader.get_num();
		e.nodeid = reader.get_num();
		e.need = reader.get_num();
		consumers[i] = e;
	}
	// GO
	// output
	string res;
    char * topo_file = (char *)res.c_str();
    write_result(topo_file, filename);
}
