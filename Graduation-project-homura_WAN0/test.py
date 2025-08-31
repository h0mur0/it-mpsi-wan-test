import random
import string
import subprocess
import time

def generate_random_strings(n, length=50):
    """生成n个随机字符串，每个字符串长度为length"""
    return [''.join(random.choices(string.ascii_letters + string.digits, k=length)) for _ in range(n)]

def write_random_strings_to_files(n, t, m, members):
    """
    创建n个随机字符串，分布写入data1.txt ~ data5.txt，确保全覆盖。
    data1.txt写入t个字符串，data2.txt ~ data5.txt分别写入m个字符串。
    多余未分配的字符串写入data2.txt
    """
    if (members - 1) * m < n:
        raise ValueError("t和m的设置无法覆盖n个字符串，请调整参数。")

    # 生成n个随机字符串
    all_strings = generate_random_strings(n)
    random.shuffle(all_strings)

    selected_strings = set()

    data_files = {}
    for i in range(1, members):
        data_files[f"data{i}.txt"] = random.sample(all_strings, m)
    data_files[f"data{members}.txt"] = random.sample(all_strings, t)
    for strings in data_files.values():
        selected_strings.update(strings)

    for filename, strings in data_files.items():
        with open(filename, 'w') as f:
            f.write('\n'.join(strings))

    # 把未被选中的字符串写入 data2.txt 的末尾
    unselected_strings = [s for s in all_strings if s not in selected_strings]
    with open("data2.txt", 'a') as f:  # 追加写入 data2.txt
        f.write('\n'.join(unselected_strings))

def test_one_time(K,m,n):
    other = (K - m) // (n - 1) * 2
    write_random_strings_to_files(K, m, other, n)
    command = ["main.exe", "-m", str(n), "-t"] +[f"data{i}.txt" for i in range(1,n+1)] + ["-n"] + ["11"] * n
    subprocess.run(command)

# test1
K = 2 ** 20
m = 2 ** 15
for n in [3, 4 ,5 ,6 , 7, 8, 9, 10]:
    test_one_time(K,m,n)
    time.sleep(1)

# test2
K = 2 ** 20
n = 5
for m in [2 ** 15, 2 ** 16, 2 ** 17, 2 ** 18]:
    test_one_time(K,m,n)
    time.sleep(1)

# test3
m = 2 ** 15
n = 5
for K in [2 ** 17, 2 ** 18, 2 ** 19, 2 ** 20]:
    test_one_time(K,m,n)
    time.sleep(1)

