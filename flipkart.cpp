#include <bits/stdc++.h>
using namespace std;

int main(){
    int n=3, m=6;
    string p="abcabc";
    string q="cbaacb";
    string r="ababca";

    int freqP[26]={0};
    int freqQ[26]={0};
    int freqR[26]={0};

    int maxP=0, maxQ=0, maxR=0;

    for(int i=0; i<p.size(); i++){
        freqP[p[i]-'a']++;
    }
    for(int i=0; i<q.size(); i++){
        freqQ[q[i]-'a']++;
    }
    for(int i=0; i<r.size(); i++){
        freqR[r[i]-'a']++;
    }

    for(int i=0; i<26; i++){
        maxP=max(maxP,freqP[i]);
    }
    for(int i=0; i<26; i++){
        maxQ=max(maxQ,freqQ[i]);
    }
    for(int i=0; i<26; i++){
        maxR=max(maxR,freqR[i]);
    }

    maxP=min(m,maxP+n); // bob
    maxQ=min(m,maxQ+n); // ben
    maxR=min(m,maxR+n); // mike

    if(maxP > maxQ){
        if(maxP > maxR){
            cout << "Bob" << endl;
        }else if(maxP < maxR){
            cout << "Mike" << endl;
        }else{
            cout << "Draw" << endl;
        }
    }else if(maxP < maxQ){
        if(maxQ > maxR){
            cout << "Ben" << endl;
        }else if(maxQ < maxR){
            cout << "Mike" << endl;
        }else{
            cout << "Draw" << endl;
        }
    }else{
        if(maxP < maxR){
            cout << "Mike" << endl;
        }else{
            cout << "Draw" << endl;
        }
    }
}