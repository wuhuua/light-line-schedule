#include <iostream>
#include <vector>
using namespace std;

class Node {
  public:
    int weight;
    int nodeId;
    Node(int nodeId, int weight) {
        this->nodeId = nodeId;
        this->weight = weight;
    }
};

class Task {
  public:
    int start;
    int end;
    Task(int start, int end) {
        this->start = start;
        this->end = end;
    }
};

class HeadNode {
  public:
    vector<Node*> NodeList;

  private:
    int nodeId;

  public:
    HeadNode(int nodeId) { this->nodeId = nodeId; }
};

void testData(vector<HeadNode*> HeadNodeList, vector<Task*> TaskList) {
    for (int i = 0; i < HeadNodeList.size(); i++) {
        cout << "-----HeadNode " << i << "--------" << endl;
        for (int j = 0; j < HeadNodeList[i]->NodeList.size(); j++) {
            cout << HeadNodeList[i]->NodeList[j]->nodeId << ":"
                 << HeadNodeList[i]->NodeList[j]->weight << ' ';;
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
    vector<HeadNode*> HeadNodeList;
    vector<Task*> TaskList;
    for (int i = 0; i < nodeNum; i++) {
        HeadNodeList.push_back(new HeadNode(i));
    }
    int headID = 0, nodeID = 0, weight = 0;
    for (int i = 0; i < lineNum; i++) {
        cin >> headID >> nodeID >> weight;
        HeadNodeList[headID]->NodeList.push_back(new Node(nodeID, weight));
        HeadNodeList[nodeID]->NodeList.push_back(new Node(headID, weight));
    }
    int start = 0, end = 0;
    for (int i = 0; i < taskNum; i++) {
        cin >> start >> end;
        TaskList.push_back(new Task(start, end));
    }
    testData(HeadNodeList, TaskList);
}
