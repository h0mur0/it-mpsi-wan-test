import random
import string
import subprocess
import time

def generate_random_strings(n, length=50):
    return [''.join(random.choices(string.ascii_letters + string.digits, k=length)) for _ in range(n)]

'''
def write_random_strings_to_files(n, t, m, members):
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
    unselected_strings = [s for s in all_strings if s not in selected_strings]
    with open("data2.txt", 'w') as f:
        f.write('\n'.join(unselected_strings))
'''

def write_random_strings_to_files(n, t, m, members):
    # all_strings = generate_random_strings(n)
    # random.shuffle(all_strings)
    # selected_strings = set()
    data_files = {}
    for i in range(1, members):
        data_files[f"data{i}.txt"] = random.sample(range(K), m)
    data_files[f"data{members}.txt"] = random.sample(range(K), t)
    for filename, strings in data_files.items():
        with open(filename, 'w') as f:
            for data in strings:
                f.write(f"{data}\n")

def test_one_time(K, m, n):  # 修正函数名
    other = m
    write_random_strings_to_files(K, m, other, n)
    command2 = [
        "./pbc_simulator", "-m", str(n), "-t"
    ] + [f"data{i}.txt" for i in range(1, n+1)] + ["-n"] + ["5"] * n + ["-K", str(K)] + ["-net","WAN"]
    subprocess.run(command2)
    
    command2 = [
        "./pbc_simulator", "-m", str(n), "-t"
    ] + [f"data{i}.txt" for i in range(1, n+1)] + ["-n"] + ["5"] * n + ["-K", str(K)] + ["-net","LAN"]
    subprocess.run(command2)


# # 测试逻辑
# m = 2 ** 5
# for n in [100]:
#     for K in [2**7,2**8, 2**9, 2**10, 2**11, 2**12, 2**13, 2**14, 2**15, 2**16, 2**17, 2**18]:
#         test_one_time(K, m, n)  # 修正函数名
#         # time.sleep(1)


m = 2 ** 12
for n in [15]:
    for K in [2**15, 2**16, 2**17, 2**18]:
        test_one_time(K, m, n)  # 修正函数名
        # time.sleep(1)