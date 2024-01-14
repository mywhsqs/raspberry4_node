#coding=utf-8

import numpy as np 
import os
import math


import threading
import time
#import matplotlib.pyplot as plt
#from mpl_toolkits.mplot3d import axes3d
#import serial导入模块
#import serial.tools.list_ports
import pickle
import random

input_command = ""
isStop = True
label = -1


init_yaw = -1
yaw = -1
pitch = -1
roll = -1

mode = 1 # 0 capture samples \ 1 real time capture sample


### 20221003_zjchen_add for GPS start ###
import time
import serial
import re

utctime = ''
lat = ''
ulat = ''
lon = ''
ulon = ''
numSv = ''
msl = ''
cogt = ''
cogm = ''
sog = ''
kph = ''
gps_t = 0

### 20221210_zjchen_add for gps
f_gps = open('./config_gps', 'r')
is_gps = str(f_gps.read())
f_gps.close()
if is_gps[0:is_gps.find(";")] == "on":
    ser = serial.Serial("/dev/ttyUSB5", 9600)
else:
    ser = ""

gps_time = ''
isTime = True

isGPS = False
### 20221003_zjchen_add for GPS end ###



### 20221003_zjchen_add for GPS start ###
def Convert_to_degrees(in_data1, in_data2):
    len_data1 = len(in_data1)
    str_data2 = "%05d" % int(in_data2)
    temp_data = int(in_data1)
    symbol = 1
    if temp_data < 0:
        symbol = -1
    degree = int(temp_data / 100.0)
    str_decimal = str(in_data1[len_data1-2]) + str(in_data1[len_data1-1]) + str(str_data2)
    f_degree = int(str_decimal)/60.0/100000.0
    # print("f_degree:", f_degree)
    if symbol > 0:
        result = degree + f_degree
    else:
        result = degree - f_degree
    return result


def GPS_read():
        global utctime
        global lat
        global ulat
        global lon
        global ulon
        global numSv
        global msl
        global cogt
        global cogm
        global sog
        global kph
        global gps_t
        global gps_time
        if ser.inWaiting():
            if ser.read(1) == b'G':
                if ser.inWaiting():
                    if ser.read(1) == b'N':
                        if ser.inWaiting():
                            choice = ser.read(1)
                            if choice == b'G':
                                if ser.inWaiting():
                                    if ser.read(1) == b'G':
                                        if ser.inWaiting():
                                            if ser.read(1) == b'A':
                                                #utctime = ser.read(7)
                                                GGA = ser.read(70)
                                                GGA_g = re.findall(r"\w+(?=,)|(?<=,)\w+", str(GGA))
                                                #print("--------------------------")
                                                #print(GGA_g)
                                                # print(GGA_g)
                                                if len(GGA_g) < 13:
                                                    #print("GPS no found")
                                                    gps_t = 0
                                                    return 0
                                                elif GGA_g[2].isdigit() and GGA_g[3].isdigit() and GGA_g[5].isdigit() and GGA_g[6].isdigit():
                                                    utctime = GGA_g[0]
                                                    # lat = GGA_g[2][0]+GGA_g[2][1]+'°'+GGA_g[2][2]+GGA_g[2][3]+'.'+GGA_g[3]+'\''
                                                    lat = "%.8f" % Convert_to_degrees(str(GGA_g[2]), str(GGA_g[3]))
                                                    ulat = GGA_g[4]
                                                    # lon = GGA_g[5][0]+GGA_g[5][1]+GGA_g[5][2]+'°'+GGA_g[5][3]+GGA_g[5][4]+'.'+GGA_g[6]+'\''
                                                    lon = "%.8f" % Convert_to_degrees(str(GGA_g[5]), str(GGA_g[6]))
                                                    ulon = GGA_g[7]
                                                    numSv = GGA_g[9]
                                                    msl = GGA_g[12]+'.'+GGA_g[13]+GGA_g[14]
                                                    #print(GGA_g)
                                                    gps_t = 1
                                                    return 1
                            
                            elif choice == b'V':
                                if ser.inWaiting():
                                    if ser.read(1) == b'T':
                                        if ser.inWaiting():
                                            if ser.read(1) == b'G':
                                                if gps_t == 1:
                                                    #print("==========================")
                                                    VTG = ser.read(40)
                                                    #print(VTG)
                                                    VTG_g = re.findall(r"\w+(?=,)|(?<=,)\w+", str(VTG))
                                                    #print(VTG_g)
                                                    cogt = VTG_g[0]+'.'+VTG_g[1]+'T'
                                                    if VTG_g[3] == 'M':
                                                        cogm = '0.00'
                                                        sog = VTG_g[4]+'.'+VTG_g[5]
                                                        kph = VTG_g[7]+'.'+VTG_g[8]
                                                    elif VTG_g[3] != 'M':
                                                        cogm = VTG_g[3]+'.'+VTG_g[4]
                                                        sog = VTG_g[6]+'.'+VTG_g[7]
                                                        kph = VTG_g[9]+'.'+VTG_g[10]
                                                #print(kph)
                            
                            elif choice == b'Z':
                                if ser.inWaiting():
                                    if ser.read(1) == b'D':
                                        if ser.inWaiting():
                                            if ser.read(1) == b'A':
                                                #print("++++++++++++++++++++++++++++")
                                                ZDA = ser.read(22)
                                                #print(ZDA)
                                                ZDA_g = re.findall(r"\w+(?=,)|(?<=,)\w+", str(ZDA))
                                                #print(ZDA_g)
                                                #print(int(ZDA_g[0][0:2]))
                                                if int(ZDA_g[0][0:2]) + 8 >= 24:
                                                    gps_time = ZDA_g[4] + '-' + str(ZDA_g[3]).zfill(2) + '-' + str(int(ZDA_g[2]) + 1).zfill(2) + ' ' + str(int(ZDA_g[0][0:2]) + 8 - 24).zfill(2) + ':' + str(ZDA_g[0][2:4]).zfill(2) + ':' + str(ZDA_g[0][4:6]).zfill(2)
                                                else:
                                                    gps_time = ZDA_g[4] + '-' + str(ZDA_g[3]).zfill(2) + '-' + str(ZDA_g[2]).zfill(2) + ' ' + str(int(ZDA_g[0][0:2]) + 8).zfill(2) + ':' + str(ZDA_g[0][2:4]).zfill(2) + ':' + str(ZDA_g[0][4:6]).zfill(2)
                                                #print(gps_time)
                                                #command = 'hwclock --set --date "' + str(gps_time) + '"'
                                                #print(command)
                                                #os.system(command)
                                                #time.sleep(1)
                                                #command = "hwclock -s"
                                                #os.system(command)
                                                #time.sleep(1)
                                                #command = "hwclock"
                                                #os.system(command)
                                                #exit()



def thread_GPS():
    global isGPS

    print(threading.current_thread().name)

    if ser.isOpen():
        print("GPS Serial Opened! Baudrate=9600")
    else:
        print("GPS Serial Open Failed!")
    
    try:
        while True:
            if input_command == "q":
                isGPS = False
                break
            if GPS_read(): 
                isGPS = True   
                continue   
                print("*********************")
                #print('UTC Time:'+utctime)
                print('Latitude:'+lat+ulat)
                print('Longitude:'+lon+ulon)
                #print('Number of satellites:'+numSv)
                #print('Altitude:'+msl)
                #print('True north heading:'+cogt+'°')
                #print('Magnetic north heading:'+cogm+'°')
                #print('Ground speed:'+sog+'Kn')
                #print('Ground speed:'+kph+'Km/h')
                print("*********************")
                time.sleep(0.1)
    except KeyboardInterrupt:
        ser.close()
        isGPS = False
        print("GPS serial Close!")
    
### 20221003_zjchen_add for GPS end ###


def thread_uart():
    print(threading.current_thread().name)

    global init_yaw
    global yaw
    global pitch
    global roll

    port_list = list(serial.tools.list_ports.comports())
    print(port_list)
    if len(port_list) == 0:
       print('无可用串口')
    else:
        for i in range(0,len(port_list)):
            print(port_list[i])


    try:
        portx="/dev/ttyUSB0"
        bps=115200
        #超时设置,None：永远等待操作，0为立即返回请求结果，其他值为等待超时时间(单位为秒）
        timex=None
        ser=serial.Serial(portx,bps,timeout=timex)
        print("串口详情参数：", ser)
        '''
        #十六进制的发送
        result=ser.write(chr(0x06).encode("utf-8"))#写数据
        print("写总字节数:",result)
        '''
        #十六进制的读取
        while 1:
            data = ser.read().hex()#读一个字节
            #print(data)
            #print(int(data, 16))
            #Read a1 data
            if data == 'a1' :             
                data = 0
                data = ser.read().hex()
                #print('@@@')
                #print(data)
                #print(int(data, 16) << 4)
                data_i = int(data, 16) << 8
                #print(data_i)
                data = ser.read().hex()
                #print(data)
                #print(int(data, 16))
                #print('data : %d' % (int(data, 16) & 0x28))
                data_i = data_i + int(data, 16)
                #print(data_i)
                #print(data_i & 0x80)
                #exit()
                if data_i & 0x8000 :
                    data_i = 0 - (data_i & 0x7fff)
                else:
                    data_i = (data_i & 0x7fff)   
                #print(data_i)        
                yaw = float(data_i) / 10.0;
                #print('[aaaaaaa] yaw:%.1f\n' % yaw)


                data =0
                data = ser.read().hex()
                #print(data)
                data_i = int(data, 16) << 8
                data = ser.read().hex()
                data_i = data_i + int(data, 16)
                #print(data_i)
                if data_i & 0x8000:
                    data_i = 0 - (data_i & 0x7fff)
                else:
                    data_i = (data_i & 0x7fff)         
                pitch = float(data_i) / 10.0;
                #print('[aaaaaaa] pitch:%.1f\n' % pitch)


                data =0;
                data = ser.read().hex()
                data_i = int(data, 16) << 8
                data = ser.read().hex()
                data_i = data_i + int(data, 16)
                if data_i & 0x8000 :
                    data_i = 0 - (data_i & 0x7fff)
                else:
                    data_i = (data_i & 0x7fff)     
                roll = float(data_i) / 10.0
                #print('[aaaaaaa] roll:%.1f\n' % roll);

                #print('yaw:%.1f, pitch:%.1f, roll:%.1f\n' % (yaw, pitch, roll));

                while 1:
                    data = 0;
                    data = ser.read().hex()
                    if data == 'aa' :
                        #print("@@@@@@@@@@")
                        break

                #time.sleep(0.1)


            if data == 'a2' :
                while 1:
                    data = 0;
                    data = ser.read().hex()
                    if data == 'aa' :
                        break

            if data == 'a3' :
                while 1:
                    data = 0;
                    data = ser.read().hex()
                    if data == 'aa' :
                        break

            if input_command == "q":
                break

            if init_yaw == -1:
                init_yaw = yaw
                print('[Thread] yaw:%.1f, pitch:%.1f, roll:%.1f\n' % (yaw, pitch, roll));

        print("---------------")
        ser.close()#关闭串口

    except Exception as e:
        print("---异常---：",e)



def thread_send_signal():
    global isStop
    global label

    path_signal = ['/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_ask', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_fsk', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_msk', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_ofdm', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_bpsk', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_dpsk', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_16qam', '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/735M_GNU_64qam']

    while 1:
        if input_command == "q":
            break
        if isStop == False :
            label = random.randint(0, 7)
            #label = threading.current_thread().name
            command = "hackrf_transfer -t " + str(path_signal[int(label)]) + " -f 735000000 -x 47 -a 1 -p 1 -s 8000000 -b 8000000 -d 0000000000000000874461dc218a6657"
            print('[Thread]: Send signal thread')

            print("-------------------------- Signal:" + str(label) + " Start  --------------------------")
            print(command)
            os.system(command)
            print("-------------------------- Signal:" + str(label) + " End  --------------------------")
            isStop = True

# 为线程定义一个函数
def thread_input_command():
    print(threading.current_thread().name)

    np.set_printoptions(threshold=np.inf)

    ### 20220515_zjchen_add for set freq for file
    #start_freq = 1000000  #732MHz
    #stop_freq = 8000000 #739MHz
    #step_freq = 6000000    #6MHz
    f_freq = open('./config_freq', 'r')
    is_freq = str(f_freq.read())
    f_freq.close()
    strlist = is_freq.split(",")
    start_freq = float(strlist[0]) #732MHz
    stop_freq = float(strlist[1]) #739MHz
    step_freq = float(strlist[2]) #6MHz
    #print(start_freq)
    #print(stop_freq)
    #print(step_freq)
    #exit()
    

    #folder = 0
    #f = open('./config_count', 'w')
    #f.write(str(folder))
    #f.close()
    '''
    f = open('./config_count', 'r')
    folder = int(f.read())
    f.close()
    '''
    folder = 1
    f = open('./config_count', 'w')
    f.write(str(folder))
    f.close()
                    
    '''
    f = open('./config_freq', 'r')
    start_freq = float(f.read())
    f.close()
    '''

    Ew_array = []
    Cp_array = []
    Max_Ew_array = []
    isSignal = True
    Tw = 80406.93

    x_Ew = []
    y_Cp = []
    z_max_cp = []

    #path = '/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20201027-lab_EQ_EM_test'
    path = '/home/pi/chattochat'
	
    ### 20220514_zjchen_add for backup for file
    f_backup = open('./config_backup', 'r')
    is_backup = str(f_backup.read())
    f_backup.close()
    ### 20221210_zjchen_modify for backup for file iq or energy
    if is_backup[0:3] == "yes":

        folder_name = time.strftime("%Y%m%d_%H%M%S", time.localtime())# + "_A"
        os.system("mkdir ./iq/" + folder_name)
        #all energy data
        os.system("touch ./iq/" + folder_name + "/energy")
    
    x_max = 1000
    y_max = 1000
    z_max = 100

    label_score_array = np.zeros((1, 8)) 
    #######################################
    ###label_score_array = [[1057., 806., 1156., 731., 573., 1181., 585., 1140.]]
    #data_array_p = np.zeros((x_max, y_max, z_max))
    #data_array_p_g = np.zeros((2, x_max, y_max))
    data_array_p_g = np.zeros((x_max, y_max))
    data_array_p = np.zeros((x_max, y_max))
    #data_array_P = pickle.load(open('/media/zjchen/5b8cbd15-7a4b-45cd-bfde-b139ad4b725b/zjchen/data2/20190826-lab_signals_test/array_1_1000', 'rb'))
    #data_array_P = data_array_P * 1000
    result_mc_list = []
    #result_mc_list = pickle.load(open('/home/zjchen/data2/result_mc', 'rb'))

    data_score_display = 0 
    #######################################
    ###data_score_display = result_mc_list[9239] * int(folder - 1)

    g_array = np.zeros((x_max, y_max))
    xpos = []
    ypos = []
    zpos = []

    distance = []
    array_pwr = []
    array_time = []
    '''
    print('[MC] Geting Data (=_=).o0O')
    data_array_0 = pickle.load(open(path + '/array_0_0', 'rb'))
    print('[MC] Geted Data (^_=).o0O')
    '''

    learn_per = np.ones((1, 8)) 
    '''
    learn_per[0][3] = learn_per[0][3] + 0.1
    learn_per[0][4] = learn_per[0][4] + 0.11
    learn_per[0][5] = learn_per[0][5] + 0.1
    learn_per[0][6] = learn_per[0][6] + 0.15
    learn_per[0][7] = learn_per[0][7] + 0.09
    '''
    learn_per[0][2] = learn_per[0][2] + 0.05
    learn_per[0][6] = learn_per[0][6] + 0.05
    learn_per[0][7] = learn_per[0][7] + 0.05
    error_label = np.zeros((8, 8)) 
    #######################################
    '''
    error_label = [[0., 0., 0., 0., 2., 0., 0., 60.],
 [0., 0.,309., 0., 0., 0., 0., 0.],
 [0., 0., 0., 0., 0., 0., 0., 0.],
 [0., 0., 0., 0., 227., 88., 99., 32.],
 [0., 0., 0., 218., 0., 21., 339., 2.],
 [0., 0., 0., 7., 0., 0., 0., 0.],
 [0., 0., 0., 125., 152., 269., 0., 0.],
 [0., 0., 0., 23., 1., 0., 41., 0.]]
    '''
    global isStop
    global label

    global init_yaw
    global yaw
    global pitch
    global roll

    #######################################
    '''
    print(result_mc_list[9239])
    print(int(folder - 1))
    print(data_score_display)
    print(error_label)
    print(label_score_array[0])
    label_score_array[0][0] = label_score_array[0][0] + 1
    print(label_score_array[0])
    exit()
    '''


    ax=[]   #保存图1数据
    ay=[]
    bx=[]   #保存图2数据
    by=[]
    num=0   #计数
    if mode == 1:
        ### draw IQ distance start ###
        '''
        #plt.ion()
        plt.figure("Distance")
        plt.grid(True)
        #plt.show()
        '''
        ### draw IQ distance start ###

        #fig = plt.figure("3D")
        #ax = fig.gca(projection='3d')

        '''
        plt.suptitle()
        agraphic=plt.subplot(2,1,1)
        agraphic.set_title('子图表标题1')      #添加子标题
        agraphic.set_xlabel('x轴',fontsize=10)   #添加轴标签
        agraphic.set_ylabel('y轴', fontsize=20)
        bgraghic=plt.subplot(2, 1, 2)
        bgraghic.set_title('子图表标题2')
        '''
        '''
        plt.ion()
        plt.grid(True)
        plt.rcParams['figure.figsize'] = (6, 6)      
        plt.rcParams['font.sans-serif']=['SimHei']  
        plt.rcParams['axes.unicode_minus'] = False
        plt.rcParams['lines.linewidth'] = 0.5  
        print('[FP] Draw Figure')
        '''

    circle = 1
    pre_max_pwr = 0

    ### 20220515_zjchen_add for init
    max_pwr = 0
    distance_tmp = 0
    now_time = time.time()

    min_iq_com = 50000
    max_iq_com = 20
  
    ### 20220523_zjchen_add for get hackrf Serial number
    serial_num = ""
    command = "hackrf_info > hackrf_file"
    os.system(command)
    f_hackrf = open('./hackrf_file', 'r')
    is_hackrf = str(f_hackrf.read())
    f_hackrf.close()
    #print(is_hackrf)
    #print(is_hackrf.find("Serial number: "))
    #print(is_hackrf[is_hackrf.find("Serial number: ") + len("Serial number: ") : is_hackrf.find("Serial number: ") + len("Serial number: ") + 32])
    serial_num = is_hackrf[is_hackrf.find("Serial number: ") + len("Serial number: ") : is_hackrf.find("Serial number: ") + len("Serial number: ") + 32]
    print("Serial number: " + serial_num + "//")
    #exit()

    
    ### 20221210_zjchen_add for backup for file
    f_backup = open('./config_backup', 'r')
    is_backup = str(f_backup.read())
    f_backup.close()

    ### 20221210_zjchen_add Save configuration data 
    str_start = str(start_freq)
    str_stop = str(start_freq + step_freq * int((stop_freq - start_freq) / step_freq))
    f_config = open("./iq/" + folder_name + '/config_data', 'w+')
    config_data = ""
    tmp_data = "Hackrf Serial Number : " + str(serial_num) + "\n"
    config_data = config_data + tmp_data
    tmp_data = "Start Freq : " + str_start + "Hz \n"
    config_data = config_data + tmp_data
    tmp_data = "Stop Freq : " + str_stop + "Hz \n"
    config_data = config_data + tmp_data
    tmp_data = "Steep : " + str(step_freq) + "Hz \n"
    soapy_B = 100000
    config_data = config_data + tmp_data
    tmp_data = "RBW : " + str(soapy_B) + "Hz \n"
    soapy_r = 6000000
    config_data = config_data + tmp_data
    tmp_data = "Sample Rate : " + str(soapy_r) + "Hz \n"
    config_data = config_data + tmp_data
    tmp_data = "Circle : " + str(circle) + "\n"
    config_data = config_data + tmp_data
    tmp_data = "Start Time : " + folder_name + "\n"
    config_data = config_data + tmp_data
    f_config.write(str(config_data))
    f_config.flush()
    #f_config.close()

    ####################### while start #######################
    while 1:
        
        ####################### for start #######################
        for windows in range(1, int(math.ceil((stop_freq - start_freq) / step_freq))):

            '''
            while 1 :
                if isStop :
                    label = -1
                    isStop = False
                    time.sleep(5)
                    break
                else:
                    continue
            '''


            str_start = str(start_freq + step_freq * (windows - 1))
            str_stop = str(start_freq + step_freq * windows)
            #TEST
            print("")
            print("------------------- Circle:" + str(circle) + ", Freq:" + str_start + "Hz, Count:" + str(folder) + " Sart -------------------")
            print("[main] Commond Windows:{}, start_freq:{}, stop_freq:{}".format(str(windows), str_start, str_stop))
            
            ### 20221210_zjchen_modify for save iq data
            if is_backup[0:is_backup.find(";")] == "yes_iq":
                command = "hackrf_transfer -r data.iq -f " + str_start + ":" + str_stop + " -g 16 -l 32 -a 1 -s 1750000 -b 1750000 -n 3500000"
                os.system(command)
            else:
                time.sleep(3)

            command = "soapy_power -r " + str(soapy_r) + " -B " + str(soapy_B) + " -O scan_freq -D constant -F rtl_power_fftw -n 60 -d serial=" + serial_num + " -f " + str_start + ":" + str_stop + "-"
            #command = "soapy_power -r 6000000 -B 100000 -O scan_freq -D constant -F rtl_power_fftw -n 60 -d serial=0000000000000000866863dc395732cf -f " + str_start + ":" + str_stop + "-"

            print(command)
            os.system(command)
            print('[main] soapy_power scripts Done')

            ### max power
            max_pwr = pickle.load(open(path + '/max_pwr', 'rb'))
            if max_pwr == pre_max_pwr:
                print("reboot max_pwr:%s\n" % (max_pwr))
                #exit()
                #continue
                #os.system("reboot")
            pre_max_pwr = max_pwr
            print('[main] power:%s' % (max_pwr))
            array_pwr.append(float(max_pwr))
            #print('pwr:%s, yaw:%.1f, pitch:%.1f, roll:%.1f\n' % (max_pwr, yaw, pitch, roll));

            now_time = time.time()
            tmp_time = time.strftime("%Y%m%d_%H%M%S", time.localtime(now_time))
            array_time.append(now_time)
			
            ### 20220514_zjchen_add for backup for file
            #f_backup = open('./config_backup', 'r')
            #is_backup = str(f_backup.read())
            #f_backup.close()

            ### 20221210_zjchen_modify for backup for file iq
            #if is_backup[0:is_backup.find(";")] == "yes_iq":
            if is_backup[0:3] == "yes":
                #all energy data
                f_scan_freq = open('./scan_freq', 'r')
                data_scan_freq = str(f_scan_freq.read())
                #print(data_scan_freq)
                f_all_energy = open("./iq/" + folder_name + "/energy", 'a')
                f_all_energy.write(str(tmp_time) + "/")
                
                ### 20221003_zjchen_add for GPS start ###
                ### 20221210_zjchen_add for gps
                f_gps = open('./config_gps', 'r')
                is_gps = str(f_gps.read())
                f_gps.close()
                if is_gps[0:is_gps.find(";")] == "on":
                    global isGPS
                    #global utctime
                    global gps_time
                    global isTime
                    if isGPS is True:
                        f_all_energy.write(str(lat+ulat) + "/" + str(lon+ulon) + "/" + "\n")
                        print('[main] GPS Lat:%s, Lon:%s         ^o^' % (str(lat+ulat), str(lon+ulon)))
                        #print('[main] GPS Lat:%s, Lon:%s' % (str(utctime)))
                        #print(utctime)
                        #print(str(int(utctime) + 80000))
                        #print(tmp_time)
                    else:
                        print('[main] !!! No GPS signal !!!         -_-')
                        f_all_energy.write("\n")
                    print('[main] GPS time:%s' % (str(gps_time)))
                    if isTime is True:
                        command = 'hwclock --set --date "' + str(gps_time) + '"'
                        print("[main]" + command)
                        os.system(command)                    
                        time.sleep(1)
                        command = "hwclock -s"
                        print("[main]" + command)
                        os.system(command)
                        print("[main] update hwclock time success          $_$")
                        isTime = False
                else:
                    print('[main] GPS off         -_-')
                    f_all_energy.write("\n")
                ### 20221003_zjchen_add for GPS end ###
                
                f_all_energy.write(data_scan_freq)
                f_all_energy.close()
                f_scan_freq.close()


            ### 20221014_zjchen_modify remove save distance
            '''
            ### xyz : Get IQ 
            xpos = pickle.load(open(path + '/xpos', 'rb'))
            ypos = pickle.load(open(path + '/ypos', 'rb'))
            zpos = pickle.load(open(path + '/zpos', 'rb'))
            '''
            ### 20221210_zjchen_modify for save iq distance with no iq data
            if is_backup[0:is_backup.find(";")] == "yes_en":
                samples_array = pickle.load(open('samples', 'rb'))
                point_map = {}
                xpos = []
                ypos = []
                zpos = []

                distance = []
                for item in samples_array:
                    s = str(item)
                    if s in point_map.keys():
                        point_map[s] = point_map[s] + 1
                    else:
                        point_map[s] = 1

                array_component = [];    

                for key in point_map.keys():
                    x_max = 100000
                    y_max = 100000
                    z_max = 50000
                    x_index = int(np.around(complex(key).real, 5) * 100000)
                    y_index = int(np.around(complex(key).imag, 5) * 100000)
                    z_index = int(point_map[key] / 1)
                    xpos.append(x_index)
                    ypos.append(y_index)
                    zpos.append(z_index)
                    array_component.append(point_map[key])
            # iq distance end


            #print('##################')
            #data_score = []
            
            ##############################
            #print('Total Learn per: 8')
            #print(learn_per)


            #print(data_array.shape)
            
            ################ iq distance start ##############
            ### 20221210_zjchen_modify for save iq distance with no iq data
            if is_backup[0:is_backup.find(";")] == "yes_en":
                #print(xpos)
                #print(ypos)
                #print(zpos)
                array_xpos = np.array(xpos)
                array_ypos = np.array(ypos)
                array_zpos = np.array(zpos)
                #print(np.sqrt(array_xpos^2 + array_ypos^2 + array_zpos^2))
                #print((array_xpos^2 + array_ypos^2 + array_zpos^2))
                distance_tmp = np.sum(np.sqrt(abs(array_xpos)^2 + abs(array_ypos)^2 + abs(array_zpos)^2))
                new_xpos = np.sqrt(abs(array_xpos)^2 + abs(array_ypos)^2)
                new_ypos = array_zpos          
                '''
                f = open(path + '/trainss_data_dis', 'r') 
                distance_tmp = f.read()
                f.close()
                '''
                distance.append(float(distance_tmp))
                print('[main] FP distance:%f' % distance_tmp)
            else:
                print('[main] FP distance: N/A')
            # iq distance end
            ################ iq distance end ##############
            
            ### 20220514_zjchen_add for backup for file
            #f_backup = open('./config_backup', 'r')
            #is_backup = str(f_backup.read())
            #f_backup.close()

            ### 20221210_zjchen_modify for backup for file iq
            if is_backup[0:is_backup.find(";")] == "yes_iq":
                #print(now_time)
                os.system("cp " + path + "/samples " + path + "/iq/" + folder_name + "/samples_" + tmp_time)
                os.system("cp " + path + "/data.iq " + path + "/iq/" + folder_name + "/iq_" + tmp_time)

            elif is_backup[0:is_backup.find(";")] == "yes_en":
                ### 20221014_zjchen_modify remove save distance
                ### 20221227_zjchen_modify for save iq xpos,ypos,zpos in xyzpos
                xyzpos = []
                xyzpos.append(xpos)
                xyzpos.append(ypos)
                xyzpos.append(zpos)
                pickle.dump(xyzpos, open('./xyzpos', 'wb'))
                os.system("cp " + path + "/xyzpos " + path + "/iq/" + folder_name + "/xyzpos_" + tmp_time)
                '''
                os.system("cp " + path + "/xpos " + path + "/iq/" + folder_name + "/xpos_" + tmp_time)
                os.system("cp " + path + "/ypos " + path + "/iq/" + folder_name + "/ypos_" + tmp_time)
                os.system("cp " + path + "/zpos " + path + "/iq/" + folder_name + "/zpos_" + tmp_time)
                '''
                pickle.dump(distance, open("./iq/" + folder_name + '/distance_' + folder_name, 'wb'))

            ### 20221210_zjchen_add for save data every time
            if is_backup[0:3] == "yes":
                pickle.dump(array_pwr, open("./iq/" + folder_name + '/array_pwr_' + folder_name, 'wb'))
                pickle.dump(array_time, open("./iq/" + folder_name + '/array_time_' + folder_name, 'wb'))

                ### 20221210_zjchen_add for config
                #f_config = open("./iq/" + folder_name + '/config_data', 'a')
                #config_data = ""
                tmp_data = "Time " + str(num) + " : " + tmp_time + "\n"
                config_data = tmp_data
                #print(config_data)
                f_config.write(str(config_data))
                f_config.flush()
                #f_config.close()


            if len(xpos) > max_iq_com:
                max_iq_com = len(xpos)
            elif len(xpos) < min_iq_com:
                min_iq_com = len(xpos) 
            ######################## 

            '''
            #################################################
            ### Y : Get Cp 
            f = open('/home/zjchen/data2/trainss_data2', 'r') 
            data2 = f.read()
            f.close()
            Cp_array = np.append(Cp_array, float(data2))
            ##
            ### X : Get Ew 
            f = open('/home/zjchen/data2/trainss_data', 'r')
            data = f.read()
            f.close()
            if len(data) != 0:
                Ew_array = np.append(Ew_array, float(data))
            ##
            
            ### Z : Get Test 
            f = open('/home/zjchen/data2/trainss_data3', 'r')
            data_max_ew = f.read()
            f.close()
            Max_Ew_array = np.append(Max_Ew_array, float(data_max_ew))
            ##


            z_max_cp.append(float(data_max_ew))
            y_Cp.append(float(data2))
            x_Ew.append(float(data))


            ### Z : Get Test 
            ### 5 Ew
            print('[Samples]: Locations low Ew:%f' % (np.mean(Max_Ew_array)))
            max_ew_data_file = open('/home/zjchen/data2/trainss_data_test', 'a')
            max_ew_data_file.write(data_max_ew + " ")
            max_ew_data_file.close()
            '''

            '''
            ### Max Ew
            print('[Samples]: Locations Max Ew:%f' % (np.mean(Max_Ew_array)))
            max_ew_data_file = open('/home/zjchen/data2/trainss_data_max_ew_locations', 'a')
            max_ew_data_file.write(data_max_ew + " ")
            max_ew_data_file.close()

            '''

            #print('[Samples]: data %d' % int(data))
 
            ### calculate Pd
            #Ew_array = np.array(Ew)
            #print('[Samples]: Num:%d , Good:%d , Pd:%.2f%%' % (len(Ew_array), np.sum(Ew_array > Tw), (np.sum(Ew_array > Tw) / len(Ew_array)) * 100))
            #time.sleep(1)
            ###
           
            ### save data
            #print(Ew_array)
            #print('[Samples]: average Cp:%d , Ew:%d ' % (np.mean(Cp_array), np.mean(Ew_array)))
            '''
            if isSignal:
                print('[Samples]: Total:%d , Good:%d , Pd(signal):%.2f%% !!!' % (len(Ew_array), np.sum(Ew_array < Tw), (np.sum(Ew_array < Tw) / len(Ew_array)) * 100))
            else:
                print('[Samples]: Total:%d , Good:%d , Pd(noise):%.2f%% @@@' % (len(Ew_array), np.sum(Ew_array > Tw), (np.sum(Ew_array > Tw) / len(Ew_array)) * 100))
            '''
            '''
            #################################################
            ### X : Get Ew 
            ew_data_file = open('/home/zjchen/data2/trainss_data_ew', 'a')
            ew_data_file.write(data + " ")
            ew_data_file.close()
            ### Y : Get Cp 
            cp_data_file = open('/home/zjchen/data2/trainss_data_cp', 'a')
            cp_data_file.write(data2 + " ")
            cp_data_file.close()
            ###
            '''

            print("[main] time:" + tmp_time);
            print("------------------- Circle:" + str(circle) + ", Freq:" + str_start + "Hz End  --------------------------")
            print("")
            print("IQ Com length : " + str(len(xpos)))
            print(str(min_iq_com) + " < iq_com < " + str(max_iq_com))

            if mode == 1:
                ### draw IQ distance start ###
                '''
                #print(np.around(max_pwr, 2))
                #print(np.around(distance_tmp, 2))
                #print(new_xpos)
                #print(new_ypos)
                #plt.clf()
                #plt.plot(new_xpos, new_ypos, 'b.')
                #plt.plot(1, 2, 'b.')
                x = [np.around(max_pwr, 2)]
                y = [np.around(distance_tmp, 2)]
                plt.plot(x, y, 'b.')
                plt.pause(0.01)
                '''
                ### draw IQ distance end ###

                #ax.plot(xpos, ypos, zpos, 'b.')
                #ax.set_xlabel('X')
                #ax.set_ylabel('Y')
                #ax.set_zlabel('Z')
                '''
                plt.clf() 
                plt.suptitle("title",fontsize=20) 
                g1=np.random.random() 
                print('#####################')
                print(np.around(max_pwr, 2))
                print(np.around(distance_tmp, 2))
                print(num)
                print(g1)
                #ax.append(np.around(max_pwr, 2))   
                #ay.append(np.around(distance_tmp, 2))    
                ax.append(num)   
                ay.append(np.around(distance_tmp, 2)) 
                agraphic=plt.subplot(2,1,1)
                agraphic.set_title('distance') 
                agraphic.set_xlabel('x',fontsize=10)  
                agraphic.set_ylabel('y', fontsize=20)
                plt.plot(ax,ay,'b.') 

                bx.append(num)
                by.append(np.around(max_pwr, 2))
                bgraghic=plt.subplot(2, 1, 2)
                bgraghic.set_title('energy')
                bgraghic.plot(bx,by,'r.')
                '''
                num = num + 1

                #plt.pause(0.4)

            if input_command == "q":
                print("[Main] input_command:%s" % input_command)
                #plt.plot(x_Ew, y_Cp, 'b.')
                '''
                ax.plot(x_Ew, y_Cp, z_max_cp, 'b.')
                ax.set_xlabel('X(Cir in Sum)')
                ax.set_ylabel('Y(Cp)')
                ax.set_zlabel('Z(Cir in Num)')
                '''
                #plt.show()
                break
        ####################### for end #######################
       
        time.sleep(0.5)

        folder = folder + 1
        f = open('./config_count', 'w')
        f.write(str(folder))
        f.close()
        '''
        if folder > 19:
            exit()
        '''
        f = open('./pow_iq_data', 'w')
        #print(max_pwr)
        f.write(str(max_pwr) + "," + str(distance_tmp))
        f.close()

        save_time = time.strftime("%H%M%S", time.localtime(now_time))
        print(int(save_time))
        ####################### if start #######################
        #if folder > 1000 or input_command == "q":
        if (int(save_time) > 0 and int(save_time) < 10) or (int(save_time) > 20000 and int(save_time) < 20010) or (int(save_time) > 40000 and int(save_time) < 40010) or (int(save_time) > 60000 and int(save_time) < 60010) or (int(save_time) > 80000 and int(save_time) < 80010) or (int(save_time) > 100000 and int(save_time) < 100010) or (int(save_time) > 120000 and int(save_time) < 120010) or (int(save_time) > 140000 and int(save_time) < 140010) or (int(save_time) > 160000 and int(save_time) < 160010) or (int(save_time) > 180000 and int(save_time) < 180010) or (int(save_time) > 200000 and int(save_time) < 200010) or (int(save_time) > 220000 and int(save_time) < 220010) or input_command == "q":
        #if input_command == "q":
            if mode == 0:
                
                #fig = plt.figure()
                #ax = fig.gca(projection='3d')
                #plt.plot(x_Ew, y_Cp, 'b.')
                '''
                ax.plot(x_Ew, y_Cp, z_max_cp, 'b.')
                ax.set_xlabel('X(Cir in Sum)')
                ax.set_ylabel('Y(Cp)')
                ax.set_zlabel('Z(Cir in Num)')
                '''
                '''
                ax.plot(xpos, ypos, zpos, 'b.')
                ax.set_xlabel('X')
                ax.set_ylabel('Y')
                ax.set_zlabel('Z')
                '''
                #result_mc_list = pickle.load(open(path + '/result_mc', 'rb'))
                #plt.figure()
                #print(result_mc_list)
                #plt.plot(result_mc_list)
                '''
                ################ distance start ##############
                plt.figure()
                plt.plot(distance)

                plt.figure()
                plt.plot(array_pwr)
                print('[main] averge power:%f' % np.mean(array_pwr))

                plt.figure()
                plt.plot(new_xpos, new_ypos, 'b.')
                '''

                ################ distance end ##############


                print('[MC] Count:%d' % int(folder - 1))
                print('[MC] Saving Data (=_=).o0O')
                '''
                ### XYZ 
                data_array = data_array / (folder)
                #data_array = (data_array + data_array_0) / 2
                pickle.dump(data_array, open(path + '/array_0', 'wb'), protocol = 4)
                '''
                ### XY 
                #data_array_p = (data_array_p) / (folder - 1)
                #print(data_array_p)
                #print(len(np.argwhere(data_array_p > 0)))
                #print((np.sum(data_array_p)))
                #print(sorted(data_array_p.reshape(1, x_max * y_max)))
                #data_array_p_g = np.array(np.gradient(data_array_p))
                #pickle.dump(data_array_p_g, open(path + '/data/array_g/7_1d_1x1000_2', 'wb'), protocol = 4)
                #pickle.dump(g_array, open(path + '/array_3_2', 'wb'), protocol = 4)
                print('[MC] Saved Data (^_^).o0O')
                #plt.show()
                break

            elif mode == 1:
                ### 20220514_zjchen_add for backup for file
                '''
                f_backup = open('./config_backup', 'r')
                is_backup = str(f_backup.read())
                f_backup.close()
                '''
                ### 20221210_zjchen_modify for save config
                if is_backup[0:3] == "yes":
                    '''
                    #pickle.dump(array_pwr, open(path + '/data/array_pwr_' + tmp_time + ':' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    #pickle.dump(distance, open(path + '/data/distance_' + tmp_time + ':' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    #pickle.dump(array_time, open(path + '/data/array_time_' + tmp_time + ':' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    pickle.dump(array_pwr, open("./iq/" + folder_name + '/array_pwr_' + tmp_time + '-' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    pickle.dump(array_time, open("./iq/" + folder_name + '/array_time_' + tmp_time + '-' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    
                    # iq distance start
                    if is_backup[0:is_backup.find(";")] == "yes_en":
                        pickle.dump(distance, open("./iq/" + folder_name + '/distance_' + tmp_time + '-' + time.strftime("%Y%m%d_%H%M%S", time.localtime()), 'wb'))
                    # iq distance end
                    '''
                    array_pwr = []
                    distance = []
                    array_time = []

                    ### 20221210_zjchen_modify Save configuration data 
                    #f = open("./iq/" + folder_name + '/config_data', 'w')
                    #config_data = ""
                    #tmp_data = "Start Time : " + folder_name + "\n"
                    #config_data = config_data + tmp_data
                    tmp_data = "End Time : " + tmp_time + "\n"
                    config_data = tmp_data
                    f_config.write(str(config_data))
                    f_config.flush()
                    f_config.close()
                
                print('[FP] Done')
                #plt.show()
                
                ########## circle start ##########
                if input_command != "q":
                    print("reboot")
                    #continue
                    exit()
                    #os.system("reboot") # 6 hour reboot
                    '''
                    folder = 1
                    f = open('./config_count', 'w')
                    f.write(str(folder))
                    f.close()
                    folder_name = time.strftime("%Y%m%d_%H%M%S", time.localtime())
                    os.system("mkdir ./iq/" + folder_name)
                    circle = circle + 1
                
                    continue
                    '''
                ########## circle end ##########
                print('[FP] Done ! ' + "Circle:" + str(circle))
                break

        ####################### if end #######################


    print("[Thread]: " + str(start_freq) + "-" + str(stop_freq) + " All Done")
    #plt.show()
    ####################### while end #######################


if __name__ == '__main__':
    
    print("##########################################################################")
    print("Function: Scan frequency. Judge signal or noise. Save IQ samples")

    '''
    while 1:
        print("[Main] Please check direction :")
        input_command = input()
        if input_command == "e":
            mode = 0
            print("[Main] ||| Capture samples ||| Start...")
            break
        elif input_command == "r":
            mode = 1
            print("[Main] ||| Real time capture samples ||| Start...")
            break
        print("[Main] Input error !")
    '''
    S1 = threading.Thread(target = thread_input_command, name = "[Thread]: Enter scan thread")
    S1.start()

    #S2 = threading.Thread(target = thread_send_signal, name = "[Thread]: Sned signal thread")
    #S2.start()

    #S3 = threading.Thread(target = thread_uart, name = "[Thread]: Uart thread")
    #S3.start()

    ### 20221210_zjchen_add for gps
    f_gps = open('./config_gps', 'r')
    is_gps = str(f_gps.read())
    f_gps.close()
    if is_gps[0:is_gps.find(";")] == "on":
        S4 = threading.Thread(target = thread_GPS, name = "[Thread]: GPS thread")
        S4.start()

    #print(time.strftime("%Y%m%d-%H%M%S", time.localtime()))
    #exit()


    while 1:
        input_command = input()
        if input_command == "q":
            print("[Main] Quiting...")
            exit()







