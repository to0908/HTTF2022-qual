import os

if __name__ == '__main__':
    dir = 'scores'
    files = os.listdir(dir)
    sum = 0
    cnt = 0
    for i, file in enumerate(files):
        path = os.path.join(dir, file)
        ok = False
        with open(path) as f:
            score = f.readlines()[-1]
            score = score.lstrip("Score = ")
            if 'target' in score:
                continue 
            sum += int(score)
            print(f"{i}: {score}")
        cnt += 1

    print("sum:", sum)
    print("mean:", sum / cnt)
    print("estimated 50 case score:", sum * 50 / cnt - 4000)