import subprocess
import os

'''
This script will print the names of all man pages containing examples for your query.
It's basically a smarter version of: `clear && man -aK <your-query> | cat`

Note: You will get much better results if you also enter an open round bracket, i.e.: "shm_open("
'''

# get query from user
query = input("\nWhat query are you searching examples for?\n> ")
query = query.strip().lower()

# get concatenated file
with open("tmp.txt", "w") as f:
    subprocess.run(["man", "-aK", query], stdout=f)

# split into individual files
numPages = 0
with open("tmp.txt", "r") as totalFile:
    for line in totalFile:
        if "Linux Programmer's Manual" in line:
            numPages += 1
        path = "manPages" + str(numPages) + ".txt"
        with open(path, "a") as manPageFile:
            manPageFile.write(line)

# get names of man pages with example
names = []
init = 0
if not os.path.exists("manPages0.txt"):
    init = 1
for i in range(init, numPages + 1):
    path = "manPages" + str(i) + ".txt"
    with open(path, "r") as manPageFile:
        hasExamples = False
        for line in manPageFile:
            if "EXAMPLES" in line:
                hasExamples = True
        if hasExamples:
            manPageFile.seek(0)
            name = manPageFile.readline().split()[0]
            if name not in names:
                names.append(name)

# print names
print("\nResults:")
for name in names:
    num = name.split("(")[1].split(")")[0]
    name = name.split("(")[0].lower()
    print("man " + num + " " + name)

# remove files
os.remove("tmp.txt")
for file in os.listdir():
    if file.startswith("manPages"):
        os.remove(file)
