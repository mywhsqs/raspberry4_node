#coding=utf-8


import os
import time

f = open('./pow_iq_data', 'r')
pre_str = str(f.read())
f.close()


while 1:
    try:
        time.sleep(60)
        f = open('./pow_iq_data', 'r')
        now_str = str(f.read())
        f.close()
        print("{}   {}".format(pre_str, now_str))
        if pre_str != now_str:
            pre_str = now_str
        else:
            #print("reboot")
            os.system("reboot")
    except:
        print("error")
        os.system("reboot")



