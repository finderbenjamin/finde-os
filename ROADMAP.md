# PRINT
print("Text")
print("Ohne Umbruch", end=" ")
print("A", "B", "C", sep="|")
print("Zeile1\nZeile2")
print("A\tB\tC")

# VARIABLEN / TYPEN
x = 5          # int
y = 2.5        # float
s = "Hi"       # str
print(type(x))

# CASTING
n = int("123")
f = float("2.5")
t = str(99)

# IMPORTS
import math
from math import pi
from random import randint
from time import sleep
from os import system
from getpass import getpass

# INPUT (IMMER str!)
age = int(input("Age: "))
dist = float(input("km: "))

# RUNDEN
print(round(3.14159, 2))
print(math.ceil(2.1), math.floor(2.9))

# IF
if x >= 20:
    ...
elif x >= 6:
    ...
else:
    ...

# FOR / RANGE
for i in range(1, 6):       # 1..5
    print(i)

for i in range(10, 0, -1):  # rückwärts
    print(i)

# BREAK / CONTINUE
for i in range(10):
    if i == 3:
        continue
    if i == 7:
        break

# ZAHLENFORMAT
print(str(7).zfill(4))      # "0007"
print(f"{7:04d}")           # "0007"
print(format(7, "04d"))     # "0007"

# WHILE
i = 1
while i <= 50:
    i += 1

# INPUT CHECK
txt = input("Zahl: ")
if txt.isdigit():
    n = int(txt)