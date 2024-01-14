#coding=utf-8


import os
import time


os.system("bash ./data_copy.sh")
time.sleep(60)

os.system("python3 MinIO.py")
time.sleep(5)



