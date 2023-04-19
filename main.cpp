#include <iostream>
#include <vector>
#include <map>
#include <queue>
using namespace std;

void printRoute(vector<int> route);

// 日志类,记录每次路径规划的日志
class LogList{
    public:
        int pipeNum;
        vector<int>nodeList;
        vector<int>LineIdList;
        vector<int>setList;
    // 标准任务规划打印
    void printLog(){
        cout<<this->pipeNum<<' '<<this->LineIdList.size()<<' '<<this->setList.size()<<' ';
        for(int i=0;i<LineIdList.size();i++){
            cout<<LineIdList[i]<<' ';
        }
        for(int i=0;i<setList.size();i++){
            cout<<setList[i]<<' ';
        }
        cout<<endl;
    }
    void printLogIndetail(){
        cout<<"-------------------------------"<<endl;
        cout<<"打印管道"<<endl;
        cout<<pipeNum<<endl;
        cout<<"打印节点"<<endl;
        for(int i=0;i<nodeList.size();i++){
            cout<<nodeList[i]<<' ';
        }
        cout<<endl;
        cout<<"打印路径"<<endl;
        for(int i=0;i<LineIdList.size();i++){
            cout<<LineIdList[i]<<' ';
        }
        cout<<endl;
        cout<<"-------------------------------"<<endl;
    }
};


class Node{
  public:
    int nodeId;
    // Iscolito:值得注意的一点是,按照管道进行分类,边作为管道参数的情况会导致权值难以确定,因此暂且考虑以最小权值作为边权值近似处理
    priority_queue<pair<int,int>, vector<pair<int, int> >, greater<pair<int, int> > > parallelPath; //Iscolito:存放weight和lineId
    int pipeNum;//节点管道数
    vector<int> pipes;
    // Iscolito:节点id,边权重,边id,id按照输入进行编号,pipes存储了管道的使用情况,未使用时等于管道数
    Node(int nodeId,int pipeNum) {
        this->nodeId = nodeId;
        this->parallelPath=priority_queue<pair<int,int>, vector<pair<int, int> >, greater<pair<int, int> > >();
        this->pipeNum=pipeNum;
        this->pipes=vector<int>(pipeNum);
        for(int i=0;i<pipeNum;i++){
            pipes[i]=0;
        }
    }
    void addParallelPath(int weight,int lineId){
        this->parallelPath.push(pair<int,int>(weight,lineId));
        for(int i=0;i<pipeNum;i++){
            pipes[i]++;
        }
    }
    pair<unsigned long long,unsigned long long> getBinaryPipes(){
        pair<unsigned long long,unsigned long long> pipeBinary;
        for(int i=0;i<pipeNum;i++){
            if(i<40){
                if(this->pipes[i]!=0){
                    pipeBinary.first+=(unsigned long long)1<<i;
                }
            }
            else{
                if(this->pipes[i]!=0){
                    pipeBinary.second+=(unsigned long long)1<<(i-40);
                }
            }
        }
        return pipeBinary;
    }
    // 迭代优先队列到vector中,方便之后使用
    vector<int> getLineList(){
        vector<int> lines;
        priority_queue<pair<int,int>, vector<pair<int, int> >, greater<pair<int, int> > > temp=this->parallelPath;
        while(temp.size()!=0){
            lines.push_back(temp.top().second);
            temp.pop();
        }
        return lines;
    }
};

class dijNode{
    public:
        int nodeId;
        vector<int> route;
        int length;
        pair<unsigned long long,unsigned long long> pipes;
        // Iscolito:dijstra算法的预热节点
        dijNode(int length,vector<int> route,int node,int pipeNum){
            this->length=length;
            this->route=route;
            this->nodeId=node;
            if(pipeNum<=40){
                pipes.first=(unsigned long long)(1<<pipeNum)-1;
                pipes.second=0;
            }
            else{
                pipes.first=(unsigned long long)(1<<pipeNum)-1;
                pipes.second=(unsigned long long)(1<<(pipeNum-40))-1;
            }
        }
        dijNode(int length,vector<int> route,int node,pair<unsigned long long,unsigned long long> pipes){
            this->length=length;
            this->route=route;
            this->nodeId=node;
            this->pipes=pipes;
        }
        
};

class Task {
  public:
    int start;
    int end;
    pair<unsigned long long,unsigned long long> pipes;
    Task(int start, int end,int pipeNum) {
        this->start = start;
        this->end = end;
        // Iscolito:此处将任务的管道可选项都初始化为1
        if(pipeNum<=40){
            pipes.first=(unsigned long long)(1<<pipeNum)-1;
            pipes.second=0;
        }
        else{
            pipes.first=(unsigned long long)(1<<pipeNum)-1;
            pipes.second=(unsigned long long)(1<<(pipeNum-40))-1;
        }
    }
};
// Iscolito:头节点类,维护一个最小长度哈希表，减少增加边时的查找开销
class HeadNode {
  public:
    map<int,Node*> NodeList;

    int nodeId;

  public:
    HeadNode(int nodeId) { this->nodeId = nodeId; }
    void addNode(int nodeId,int weight,int lineId,int pipeNum){
        if(NodeList.find(nodeId)==NodeList.end()){
            NodeList[nodeId]=new Node(nodeId,pipeNum);
        }
        NodeList[nodeId]->addParallelPath(weight, lineId);
    }
    int getMinWeight(int nodeId){
        return this->NodeList[nodeId]->parallelPath.top().first;
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
        int maxDistance;
        vector<LogList> logs;
        vector<pair<int,int> > newLines;
        // 边的权重
        map<int,int> lineWeights;
        NodeMap(int lineCount,int pipeNum,int maxDistance){
            this->lineCount=lineCount;
            this->pipeNum=pipeNum;
            this->maxDistance=maxDistance;
            this->logs=vector<LogList>();
        }
        // Iscolito:只能在已有边的两个节点添加新的边,此处没有设置检查,用逻辑保证
        void addLine(int nodeId1,int nodeId2){
            (*this)[nodeId1]->addNode(nodeId2,(*this)[nodeId1]->getMinWeight(nodeId2),lineCount,this->pipeNum);
            (*this)[nodeId2]->addNode(nodeId1,(*this)[nodeId2]->getMinWeight(nodeId1),lineCount,this->pipeNum);
            // 增边的时候统一使用最小权值
            lineWeights[lineCount]=(*this)[nodeId1]->getMinWeight(nodeId2);
            lineCount++;
            this->newLines.push_back(pair<int,int>(nodeId1,nodeId2));
        }
        // Iscolito:基本dijstra算法,有路走路,没路增边
        pair<vector<int>,int> FindWayByDijstra(int start,int end){
            // Iscolito:为保证搜索速度在O(1),用一定的空间换取时间,nodemap存储vecotor中每个下标对应的实际节点id
            map<int,int> nodemap;
            vector<dijNode*> nodelist;
            // Iscolito:预热dijstra临近点集
            for (map<int,Node*>::iterator it=(*this)[start]->NodeList.begin();it!=(*this)[start]->NodeList.end();it++){
                //判断start到it有无可用通道
                pair<unsigned long long,unsigned long long> pipes=it->second->getBinaryPipes();
                if(pipes.first|pipes.second){
                    nodelist.push_back(new dijNode(it->second->parallelPath.top().first,vector<int>(),it->second->nodeId,this->pipeNum));
                    nodemap[it->first]=nodelist.size()-1;
                    nodelist[nodelist.size()-1]->route.push_back(it->first);
                }
            }
            // Iscolito:此处调用各种类型较多, 但是本质上是利用栈的特性进行的扩点/更新点处理
            // Iscolito:从栈底开始遍历全栈,由于nodemap的扩增,实际上一次for循环即可更新所有点
            for(int i=0;i<nodelist.size();i++){
                HeadNode* kidList=(*this)[nodelist[i]->nodeId];
                for(map<int,Node*>::iterator it=kidList->NodeList.begin();it!=kidList->NodeList.end();it++){
                    pair<unsigned long long,unsigned long long> pipes=it->second->getBinaryPipes();
                    pair<unsigned long long,unsigned long long> dijPipes=nodelist[i]->pipes;
                    dijPipes.first&=pipes.first;
                    dijPipes.second&=pipes.second;
                    if(dijPipes.first|dijPipes.second){
                        if(nodemap.find(it->first)==nodemap.end()){
                            nodelist.push_back(new dijNode(nodelist[i]->length+it->second->parallelPath.top().first,nodelist[i]->route,it->first,dijPipes));
                            nodelist[nodelist.size()-1]->route.push_back(it->first);
                            nodemap[it->first]=nodelist.size()-1;
                        }
                        else{
                            if(nodelist[i]->length+it->second->parallelPath.top().first<nodelist[nodemap[it->second->nodeId]]->length){
                                nodelist[nodemap[it->first]]->length=nodelist[i]->length+it->second->parallelPath.top().first;
                                nodelist[nodemap[it->first]]->route=nodelist[i]->route;
                                nodelist[nodemap[it->first]]->route.push_back(it->first);
                                nodelist[nodemap[it->first]]->pipes=dijPipes;
                            }
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
        // 获得可用的管道,如果不成功返回-1
        int getPipe(Task task,vector<int> route){
            HeadNode* node=(*this)[task.start];
            pair<unsigned long long,unsigned long long> pipes=task.pipes;
            pair<unsigned long long,unsigned long long> nodePipes;
            // 每次都将所遍历到的节点作为当前头节点,再推出下一个
            for(int i=0;i<route.size();i++){
                nodePipes=node->NodeList[route[i]]->getBinaryPipes();
                pipes.first&=nodePipes.first;
                pipes.second&=nodePipes.second;
                node=(*this)[route[i]];
            }
            for(int i=0;i<40;i++){
                if(pipes.first%2==1){
                    return i;
                }
                pipes.first=pipes.first>>1;
            }
            for(int i=0;i<40;i++){
                if(pipes.second%2==1){
                    return i;
                }
                pipes.second=pipes.second>>1;
            }
            return -1;
        }
        // 刷新管道的边数,需要回删
        void refreshRoute(int start,int pipeNo,vector<int>route){
            HeadNode* node=(*this)[start];
            int backNode=0;
            for(int i=0;i<route.size();i++){
                node->NodeList[route[i]]->pipes[pipeNo]--;
                backNode=node->nodeId;
                node=(*this)[route[i]];
                node->NodeList[backNode]->pipes[pipeNo]--;
            }
        }
        // 本算法目前未经过验证,可能会导致程序异常退出
        vector<int> bfsAddLine(vector<int> accessNodes,int end){
            // 队列里面存储当前扩边路径和节点
            queue<int>nodeQueue;
            map<int,vector<int> > accessMap;
            // 预热BFS现有节点
            for(int i=0;i<accessNodes.size();i++){
                nodeQueue.push(accessNodes[i]);
                accessMap[accessNodes[i]]=vector<int>();
                accessMap[accessNodes[i]].push_back(accessNodes[i]);
            }
            // 增边时则不考虑管道的通达性
            while(nodeQueue.size()!=0&&accessMap.find(end)==accessMap.end()){
                int extendNode=nodeQueue.front();
                nodeQueue.pop();
                for(map<int,Node*>::iterator it=(*this)[extendNode]->NodeList.begin();it!=(*this)[extendNode]->NodeList.end();it++){
                    if(accessMap.find(it->first)==accessMap.end()){
                        vector<int> route=accessMap[extendNode];
                        route.push_back(it->first);
                        if(it->first==end){
                            return route;
                        }
                        nodeQueue.push(it->first);
                        accessMap[it->first]=route;
                    }
                }
            }
            // 没有点和目标节点相连则返回空vector(通常不会)
            return vector<int>();
        }
        // 贪心增边法,搜索当前可达节点到目标节点路径最短的一个
        void addNewLine(vector<int> accessNodes,int end){
            for(int i=0;i<accessNodes.size();i++){
                // 直接可达的话增加这条边
                if((*this)[accessNodes[i]]->NodeList.find(end)!=(*this)[accessNodes[i]]->NodeList.end()){
                    addLine(accessNodes[i],end);
                    return;
                }
            }
            // 不直接可达的话bfs广搜
            vector<int> route=bfsAddLine(accessNodes,end);
            if(route.size()!=0){
                for(int i=0;i<route.size()-1;i++){
                    addLine(route[i],route[i+1]);
                }
                return;
            }
        }
        // 打印单条路径日志,更新路径的管道使用情况
        vector<int> printRouteLogs(int pipeNo,int start,vector<int> route,map<int,map<int,bool> > &linePipes){
            vector<int> routeLog;
            HeadNode* node=(*this)[start];
            vector<int> lines=node->NodeList[route[0]]->getLineList();
            for(int i=0;i<lines.size();i++){
                if(linePipes[lines[i]][pipeNo]){
                    routeLog.push_back(lines[i]);
                    linePipes[lines[i]][pipeNo]=false;
                    break;
                }
            }
            for(int i=0;i<route.size()-1;i++){
                node=(*this)[route[i]];
                lines=node->NodeList[route[i+1]]->getLineList();
                for(int j=0;j<lines.size();j++){
                    if(linePipes[lines[j]][pipeNo]){
                        routeLog.push_back(lines[j]);
                        linePipes[lines[j]][pipeNo]=false;
                        break;
                    }
                }
            }
            return routeLog;
        }
        // 打印放大器日志
        vector<int> printBooster(int start,vector<int> routeLog,vector<int> route){
            vector<int> setLog=vector<int>();
            int distance=0;
            distance+=this->lineWeights[route[0]];
            for(int i=0;i<routeLog.size()-1;i++){
                if(distance+this->lineWeights[route[i+1]]>this->maxDistance){
                    setLog.push_back(route[i]);
                    distance=0;
                }
                distance+=this->lineWeights[route[i+1]];
            }
            return setLog;
        }
        // 打印结果的函数
        void printLogs(vector<pair<int,vector<int> > > results,vector<Task*> TaskList){
            // 打印增加的边
            cout<<this->newLines.size()<<endl;
            for(int i=0;i<this->newLines.size();i++){
                cout<<this->newLines[i].first<<' '<<this->newLines[i].second<<endl;
            }
            // 预热边的管道使用情况
            map<int,map<int,bool> > linePipes=map<int,map<int,bool> >();
            for(int i=0;i<this->lineCount;i++){
                linePipes[i]=map<int,bool>();
                for(int j=0;j<this->pipeNum;j++){
                    linePipes[i][j]=true;
                }
            }
            for(int i=0;i<results.size();i++){
                LogList log=LogList();
                log.LineIdList=printRouteLogs(results[i].first, TaskList[i]->start, results[i].second,linePipes);
                log.nodeList=results[i].second;
                log.pipeNum=results[i].first;
                log.setList=printBooster(TaskList[i]->start,log.LineIdList,results[i].second);
                log.printLog();
                //log.printLogIndetail();
                this->logs.push_back(log);
            }
        }

        // 生成一个分配结果
        vector<pair<int,vector<int> > > generate(vector<Task*> TaskList){
            int pipeNo=0;
            pair<vector<int>,int> route;
            vector<pair<int,vector<int> > > results;
            for(int i=0;i<TaskList.size();i++){
                route=FindWayByDijstra(TaskList[i]->start,TaskList[i]->end);
                //printRoute(route.first);
                if(route.second!=-1){
                    // 如果找到了路径,那么getPipe一定会找到一个可用的管道
                    pipeNo=getPipe(*TaskList[i],route.first);
                }
                else{
                    // 如果没找到路径,那么返回了所有的可达节点
                    addNewLine(route.first,TaskList[i]->end);
                    route=FindWayByDijstra(TaskList[i]->start,TaskList[i]->end);
                    pipeNo=getPipe(*TaskList[i],route.first); 
                }
                refreshRoute(TaskList[i]->start,pipeNo,route.first);
                results.push_back(pair<int,vector<int> >(pipeNo,route.first));
            }
            return results;
        }
};

// 测试数据录入结果
void testData(vector<HeadNode*> HeadNodeList, vector<Task*> TaskList) {
    for (int i = 0; i < HeadNodeList.size(); i++) {
        cout << "-----HeadNode " << i << "--------" << endl;
        for (map<int,Node*>::iterator it=HeadNodeList[i]->NodeList.begin();it!=HeadNodeList[i]->NodeList.end();it++) {
            cout <<"nodeId:"<<it->second->nodeId << ";"
                 <<"Weight:"<<it->second->parallelPath.top().first << ";"
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

// 打印路径
void printRoute(vector<int> route){
    for(int i=0;i<route.size();i++){
        cout<<route[i]<<' ';
    }
    cout<<endl;
}



int main() {
    int nodeNum = 0, lineNum = 0, taskNum = 0, pipeNum = 0, maxDistance = 0;
    cin >> nodeNum >> lineNum >> taskNum >> pipeNum >> maxDistance;
    NodeMap HeadNodeList(lineNum,pipeNum,maxDistance);
    vector<Task*> TaskList;
    for (int i = 0; i < nodeNum; i++) {
        HeadNodeList.push_back(new HeadNode(i));
    }
    int headID = 0, nodeID = 0, weight = 0;
    for (int i = 0; i < lineNum; i++) {
        cin >> headID >> nodeID >> weight;
        HeadNodeList[headID]->addNode(nodeID,weight,i,pipeNum);
        HeadNodeList[nodeID]->addNode(headID,weight,i,pipeNum);
        HeadNodeList.lineWeights[i]=weight;
    }
    int start = 0, end = 0;
    for (int i = 0; i < taskNum; i++) {
        cin >> start >> end;
        TaskList.push_back(new Task(start, end,pipeNum));
    }
    vector<pair<int,vector<int> > > result=HeadNodeList.generate(TaskList);
    cout<<endl;
    HeadNodeList.printLogs(result,TaskList);

    return 0;
}
