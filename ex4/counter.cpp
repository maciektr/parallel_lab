#include<bits/stdc++.h>
using namespace std;

int main(int argc, char** argv){
    std::ifstream file(argv[1]);
    unordered_map<string, int> counter;
    if ( file.is_open() ) {    
        string token; 
        while ( file.good() ) {
            file >> token;
            auto insert_result = counter.insert({token, 1});
            if (!insert_result.second) 
                insert_result.first->second = (insert_result.first->second)+1; 
        }         
    }
}
