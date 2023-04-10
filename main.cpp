#include <iostream>
#include <vector>
#include <map>
using namespace std;

class Node{
  public:
    int weight;
    int nodeId;
    int lineId;
    Node* next;
    // Iscolito:希望实现一个查找可用管道开销在O(1)的数据结构,觉得还是位运算比较合适
    // 低64bit存0-39号管道占用情况,高64位bit存剩下管道的占用情况
    unsigned long long highMap;
    unsigned long long lowMap;
    int pipeNum;
    // Iscolito:节点id,边权重,边id,id按照输入进行编号,pipes存储了管道的使用情况,未使用是-1
    Node(int nodeId, int weight,int lineId,int pipeNum) {
        this->nodeId = nodeId;
        this->weight = weight;
        this->lineId = lineId;
        this->pipeNum=pipeNum;
        if(pipeNum<=40){
            this->lowMap=((long)1<<pipeNum)-1;
        }
        else{
            this->lowMap=((long)1<<40)-1;
            this->highMap=((long)1<<pipeNum)-1;
        }
    }
    Node(int nodeId, int weight,int lineId,int pipeNum,Node* next) {
        this->nodeId = nodeId;
        this->weight = weight;
        this->lineId = lineId;
        this->next=next;
        this->pipeNum=pipeNum;
        if(pipeNum<=40){
            this->lowMap=((long)1<<pipeNum)-1;
        }
        else{
            this->lowMap=((long)1<<40)-1;
            this->highMap=((long)1<<pipeNum)-1;
        }
    }
};

class dijNode{
    public:
        int nodeId;
        vector<int> route;
        int length;
        dijNode(int length,vector<int> route,int node){
            this->length=length;
            this->route=route;
            this->nodeId=node;
        }
};

class Task {
  public:
    int start;
    int end;
    unsigned long long highPipe;
    unsigned long long lowPipe;
    Task(int start, int end,int pipeNum) {
        this->start = start;
        this->end = end;
        // Iscolito:此处将任务的管道可选项都初始化为1
        if(pipeNum<=40){
            this->lowPipe=((long)1<<pipeNum)-1;
        }
        else{
            this->lowPipe=((long)1<<40)-1;
            this->highPipe=((long)1<<pipeNum)-1;
        }
    }
};

class HeadNode {
  public:
    map<int,Node*> NodeList;
  private:
    // Iscolito:头节点类,维护一个最小长度哈希表，减少增加边时的查找开销
    int nodeId;
    map<int,int> minWeight;

  public:
    HeadNode(int nodeId) { this->nodeId = nodeId; }
    void addNode(int nodeId,int weight,int lineId,int pipeNum){
        if(this->NodeList.find(nodeId)==this->NodeList.end()){
            this->NodeList[nodeId]=new Node(nodeId, weight,lineId,pipeNum);
        }
        else{
            Node* ptr=this->NodeList[nodeId];
            while(ptr->next!=nullptr&&ptr->next->weight<weight){
                ptr=ptr->next;
            }
            ptr->next=new Node(nodeId, weight,lineId,pipeNum,ptr->next);
        }
    }
    int getMinWeight(int nodeId){
        return this->NodeList[nodeId]->weight;
    }
    vector<int> getNodeIdList(){
        vector<int> res;
        for (map<int,Node*>::iterator it=this->NodeList.begin();it!=this->NodeList.end();it++) {
            res.push_back(it->first);
        }
        return res;
    }
    vector<Node*> getNodeList(){
        vector<Node*> res;
        for (map<int,Node*>::iterator it=this->NodeList.begin();it!=this->NodeList.end();it++) {
            res.push_back(it->second);
        }
        return res;
    }
};

// Iscolito:图类,继承于vector,存放头节点
class NodeMap:public vector<HeadNode*>{
    public:
        int lineCount;
        int pipeNum;
        NodeMap(int lineCount,int pipeNum){
            this->lineCount=lineCount;
            this->pipeNum=pipeNum;
        }
        // Iscolito:只能在已有边的两个节点添加新的边,此处没有设置检查,用逻辑保证
        void addLine(vector<HeadNode*> HeadNodeList,int nodeId1,int nodeId2){
            if(nodeId1<nodeId2){
                (*this)[nodeId1]->addNode(nodeId2,(*this)[nodeId1]->getMinWeight(nodeId2),lineCount,this->pipeNum);
            }
            else{
                (*this)[nodeId2]->addNode(nodeId1,(*this)[nodeId2]->getMinWeight(nodeId1),lineCount,this->pipeNum);
            }
            lineCount++;
        }
        // Iscolito:基本dijstra算法,有路走路,没路增边
        pair<vector<int>,int> FindWayByDijstra(int start,int end){
            // Iscolito:为保证搜索速度在O(1),用一定的空间换取时,nodemap存储vecotor中每个下标对应的实际节点id
            map<int,int> nodemap;
            vector<dijNode*> nodelist;
            // Iscolito:预热dijstra临近点集
            for (map<int,Node*>::iterator it=(*this)[start]->NodeList.begin();it!=(*this)[start]->NodeList.end();it++){
                nodelist.push_back(new dijNode(it->second->weight,vector<int>(),it->second->nodeId));
                nodemap[it->first]=nodelist.size()-1;
                nodelist[nodelist.size()-1]->route.push_back(it->first);
            }
            // Iscolito:此处调用各种类型较多, 但是本质上是利用栈的特性进行的扩点/更新点处理
            // Iscolito:从栈底开始遍历全栈,由于nodemap的扩增,实际上一次for循环即可更新所有点
            for(int i=0;i<nodelist.size();i++){
                HeadNode* kidList=(*this)[nodelist[i]->nodeId];
                for(map<int,Node*>::iterator it=kidList->NodeList.begin();it!=kidList->NodeList.end();it++){
                    if(nodemap.find(it->first)==nodemap.end()){
                        nodelist.push_back(new dijNode(nodelist[i]->length+it->second->weight,nodelist[i]->route,it->first));
                        nodelist[nodelist.size()-1]->route.push_back(it->first);
                        nodemap[it->first]=nodelist.size()-1;
                    }
                    else{
                        if(nodelist[i]->length+it->second->weight<nodelist[nodemap[it->second->nodeId]]->length){
                            nodelist[nodemap[it->first]]->length=nodelist[i]->length+it->second->weight;
                            nodelist[nodemap[it->first]]->route=nodelist[i]->route;
                            nodelist[nodemap[it->first]]->route.push_back(it->first);
                        }
                    }
                }
            }
            // Iscolito:查找成功返回路径和长度
            if(nodemap.find(end)!=nodemap.end()){
                return pair<vector<int>,int>(nodelist[nodemap[end]]->route,nodelist[nodemap[end]]->length);
            }
            // Iscolito:查找失败返回全部可达节点和失败标记-1
            else{
                vector<int> nodes;
                for(map<int,int>::iterator it=nodemap.begin();it!=nodemap.end();it++){
                    nodes.push_back(it->first);
                }
                return pair<vector<int>,int>(nodes,-1);
            }
        }
};

// 测试数据录入结果
void testData(vector<HeadNode*> HeadNodeList, vector<Task*> TaskList) {
    for (int i = 0; i < HeadNodeList.size(); i++) {
        cout << "-----HeadNode " << i << "--------" << endl;
        for (map<int,Node*>::iterator it=HeadNodeList[i]->NodeList.begin();it!=HeadNodeList[i]->NodeList.end();it++) {
            cout <<"nodeId:"<<it->second->nodeId << ";"
                 <<"Weight:"<<it->second->weight << ";"
                 <<"minWeight:"<<HeadNodeList[i]->getMinWeight(it->second->nodeId)<<' ';
        }
        cout << endl;

    }
    cout << endl;
    cout << "-----Tasks------" << endl;
    for (int i = 0; i < TaskList.size(); i++) {
        cout << "Task" << i << " start:" << TaskList[i]->start
             << " end:" << TaskList[i]->end << endl;
    }
}



int main() {
    int nodeNum = 0, lineNum = 0, taskNum = 0, pipeNum = 0, maxDistance = 0;
    cin >> nodeNum >> lineNum >> taskNum >> pipeNum >> maxDistance;
    NodeMap HeadNodeList(lineNum,pipeNum);
    vector<Task*> TaskList;
    for (int i = 0; i < nodeNum; i++) {
        HeadNodeList.push_back(new HeadNode(i));
    }
    int headID = 0, nodeID = 0, weight = 0;
    for (int i = 0; i < lineNum; i++) {
        cin >> headID >> nodeID >> weight;
        // TuNNan:矩阵在存储边时以序号小的为headID
        if(headID < nodeID){
        HeadNodeList[headID]->addNode(nodeID,weight,i,pipeNum);
        }
        else{
        HeadNodeList[nodeID]->addNode(headID,weight,i,pipeNum);
        }
    }
    int start = 0, end = 0;
    for (int i = 0; i < taskNum; i++) {
        cin >> start >> end;
        TaskList.push_back(new Task(start, end,pipeNum));
    }
    testData(HeadNodeList, TaskList);
    pair<vector<int>,int> result=HeadNodeList.FindWayByDijstra(0,6);
    for(int i=0;i<result.first.size();i++){
        cout<<result.first[i];
    }
    cout<<endl;
    cout<<result.second;
}
