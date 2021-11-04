#include<bits/stdc++.h> 
using namespace std;
#define all(x) (x).begin(),(x).end()
template<typename T1,typename T2> bool chmin(T1 &a,T2 b){if(a<=b)return 0; a=b; return 1;}
template<typename T1,typename T2> bool chmax(T1 &a,T2 b){if(a>=b)return 0; a=b; return 1;}
template< typename T > ostream &operator<<(ostream &os, const vector< T > &v) {
    for(int i = 0; i < (int) v.size(); i++) os << v[i] << (i + 1 != (int) v.size() ? " " : "");
    return os;
}

const int N = 1000, M = 20;
int K, R;
vector<vector<int>> d(N);
vector<vector<int>> v(N), rev(N);

bool dayEnd(vector<int> &w, vector<int> &cnt, queue<int> &que){
    int n;
    cin>>n;
    if(n == -1) return true;
    for(int i=0;i<n;i++){
        int a; cin>>a; a--;
        // cnt[w[a]]--;
        int task = w[a];
        for(auto i:v[task]){
            cnt[i]--;
            if(cnt[i] == 0) que.push(i);
        }
        w[a] = -1;
    }
    return false;
}

void solve(){
    queue<int> q;
    vector<int> cnt(N);
    for(auto v1:v){
        for(auto i:v1){
            cnt[i]++;
        }
    }
    for(int i=0;i<N;i++){
        if(cnt[i] == 0) q.push(i);
    }
    vector<int> working(M, -1);
    while(true){
        vector<int> ans;
        int sz = 0;
        for(int i=0;i<M;i++){
            if(q.empty()) break;
            if(working[i] != -1) continue;
            ans.emplace_back(i + 1);
            ans.emplace_back(q.front() + 1);
            working[i] = q.front();
            q.pop();
            sz++;
        }
        cout << sz << " ";
        cout << ans << endl;
        if(dayEnd(working, cnt, q)) return;
    }
}

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
        rev[b].emplace_back(a);
    }

    solve();
}