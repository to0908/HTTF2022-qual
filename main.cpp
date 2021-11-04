#include<bits/stdc++.h> 
using namespace std;
#define all(x) (x).begin(),(x).end()
template<typename T1,typename T2> bool chmin(T1 &a,T2 b){if(a<=b)return 0; a=b; return 1;}
template<typename T1,typename T2> bool chmax(T1 &a,T2 b){if(a>=b)return 0; a=b; return 1;}

const int N = 1000, M = 20;
int K, R;
vector<vector<int>> d(N);
vector<vector<int>> v(N);

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(0);

    cin>>K>>K>>K>>R;
    for(int i=0;i<N;i++){
        d[i].resize(K);
        for(int j=0;j<K;j++) cin>>d[i][j];
    }
    for(int i=0;i<R;i++){
        int a,b;
        cin>>a>>b;
        a--,b--;
        v[a].emplace_back(b);
    }

    while(true){
        cout << 0 << endl;
        int n;
        cin>>n;
        if(n == -1) return 0;
        int a;
        for(int i=0;i<n;i++) cin>>a;
    }
}