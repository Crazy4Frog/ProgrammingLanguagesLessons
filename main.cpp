#include <iostream>
#include <vector>
#include <map>
using namespace std;

int inputInt(){
    int elem;
    cin >> elem;
    return elem;
}

pair<vector<int>,map<int,int>> fill_vector(vector<int> data, map<int, int>dict, int count) {
    int elem = inputInt();
    data.push_back(elem);
    dict[elem]++;
    return count > 1 ? fill_vector(data, dict, count-1) : make_pair(data, dict);
}

void print_vector(vector<int> data, int count){
    int size = data.size();
    cout << data[size - count] << " ";
    return count > 1 ? print_vector(data, count-1) : void();
}

void print_histogram(map<int,int>&data, map<int,int>::iterator it){
    cout << it->first << ", count: " << it->second << endl;
    return ++it != data.end() ?  print_histogram(data, it) : void();
}

int main() {
    vector<int> data = {};
    map<int,int> histogram = {};
    histogram = fill_vector(data, histogram, inputInt()).second;
    print_histogram(histogram, histogram.begin());
    return 0;

}
