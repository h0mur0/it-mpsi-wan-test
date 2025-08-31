import time
def read_file_lines(filename):
    with open(filename, 'r') as file:
        return {line.rstrip('\n') for line in file}

# 替换为你的文件名列表
files = ['data1.txt', 'data2.txt', 'data3.txt', 'data4.txt', 'data5.txt']

# 读取每个文件的行集合
line_sets = [read_file_lines(f) for f in files]

# 计算所有集合的交集
common_lines = set.intersection(*line_sets)

# 输出共同行（按字母顺序排序）
#for line in sorted(common_lines):
    # print(line)
t = time.localtime()
with open("test.txt","a") as f:
    f.write("{}-{}-{} {:02d}:{:02d}:{:02d}".format(t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec))
    f.write("\t")
    f.write(str(len(common_lines)))
    f.write('\n')