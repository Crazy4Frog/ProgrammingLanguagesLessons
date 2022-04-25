#include <iostream>
#include <vector>
#include <map>
using namespace std;
pair<vector<int>,map<int,int>> fill_vector(vector<int> data, map<int, int>dataCounter, int count, int counter) {
    int elem;
    cin >> elem;
    data.push_back(elem);
    map<int,int>::iterator it = dataCounter.find(elem);
    (it->first == elem) ? ++it->second : dataCounter[elem] = 1;
    return counter++ == count ? make_pair(data, dataCounter): fill_vector(data, dataCounter, count, counter);
}

void print_vector(vector<int> data, int count, int counter){
    cout << data[counter - 1] << " ";
    return counter++ == count ? void() : print_vector(data, count, counter);
}

void print_map(map<int,int>data,map<int,int>::iterator it){
    cout << "Key = " << it->first << ", value = " << it->second << '\n';
    return it == data.end() ? void() : print_map(data, ++it);
}

int main() {
    int count, counter = 1;
    cin >> count;
    vector<int> data;
    map <int,int> dataCounter;
    pair<vector<int>, map<int,int>>container = fill_vector(data,dataCounter, count, counter);
    data = container.first;
    dataCounter = container.second;
    print_vector(data, count,  counter);
    cout << endl;
    print_map(dataCounter, dataCounter.begin());

    std::cout << "\nHello, World!" << std::endl;
    return 0;
}
