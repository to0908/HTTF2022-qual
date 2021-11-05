# HTTF2022-qual

## 考察
- skillの初期値を問題文通りのランダムで初期化するか、適当な値もしくは0で初期化するか

- AHC003の上位解法が部分的に使えそう？  
    - 1st eivourさん 
    https://qiita.com/contramundum/items/b945400b81536df42d1a
    - 10th ymatsuxさん  
    https://ymatsux-cp-ja.blogspot.com/2021/06/atcoder-heuristic-contest-003-10.html

- パラメータ復元
    - スキル: 20人 * K = 400個
    - s の生成に用いる式の分子 randdouble(20, 60)  
    -> 401個？


- 標準正規分布の絶対値から生成される  
    -> https://qiita.com/toku_san/items/22bdce9b9efb92798c97  
    -> 平均 sqrt(2/pi), 分散 1 - sqrt(2/pi)  
    -> mean: 0.80, var: 0.20 (ほんまか？)


- もし正確に全員のスキルが分かっているとき、どのようにタスクを割り振るのが最適か
    - dousiyo~
    - なるべく全員が同時に動けるように. 序盤、中盤で手の空いている人がいるのは無駄