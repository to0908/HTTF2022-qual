import os

if __name__ == '__main__':
    dir = 'scores'
    files = os.listdir(dir)
    sum = 0
    cnt = 0
    for i, file in enumerate(files):
        cnt += 1
        path = os.path.join(dir, file)
        with open(path) as f:
            score = f.readlines()[-1]
            score = score.lstrip("Score = ")
            sum += int(score)
            print(f"{i}: {score}")
    print("sum:", sum)
    print("mean:", sum / cnt)
    print("estimated 50 case score:", sum * 50 / cnt)