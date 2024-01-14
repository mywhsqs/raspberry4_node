# Version 2改动部分：以hackrf序列号后六位命名存储文件夹，区分各个hackrf存储的数据
#!/bin/bash

# 设置要同步的目标文件夹
src_path="/home/pi/chattochat/iq"

# 设置 备份的文件夹地址
dst_path="/mnt"

# 改动部分：以hackrf序列号后六位命名存储文件夹————2022-11-06
r=$(hackrf_info | grep "Serial")
r=${r: -6}
dst_path=$dst_path/$r

# 设置子文件夹多余 n 个执行同步操作
folder_run=5

# 设置子文件夹小于 m 个停止同步操作
folder_keep=3

# 判断目标文件夹里子文件夹数量
folders_left() {

    num=$(ls -l ${src_path} | grep ^d | wc -l )

    echo
    echo ">>> $num folders left..."
    echo

}

# 判断备份的文件夹地址 是否存在，不存在则新建
if [ ! -d "$src_path" ];then
  echo ">>> ERROR! source path of directories not exist!"
  exit 2

elif [ ! -d "$dst_path" ];then
  mkdir -p $dst_path
fi

# 进入目标文件夹路径
cd $src_path


# 开始执行循环子文件夹同步的操作，输出子文件夹总数
folders_num=$(ls -l ${src_path} | grep ^d | wc -l )
echo ">>> Synchronize folders number are: $folders_num"

# 子文件夹大于n个执行操作
if [[ $folders_num -gt $folder_run ]];then

  while true; do
    folders_left
    # 设置保留的文件夹数量
    folders_num=$(ls -l ${src_path} | grep ^d | wc -l )
    if [[ $folders_num -gt $folder_keep ]];then
      # 开始同步过程
      # head -n 1指的是一个一个复制子文件夹
      folder=$(ls -l | grep ^d | awk '{print $9}' | head -n 1)
      echo "Current copying directory: $folder"
      rsync --remove-source-files -avh $src_path/$folder $dst_path
       # 子文件夹同步完成后进行删除，并告知当前子文件夹个数
      rm -rf $src_path/$folder
      folders_left

    else
      # 剩余文件夹不足m个，退出脚本
      n=$(ls -l ${src_path} | grep ^d | wc -l )
      echo
      echo "number of sub folders are: $n, quit!"
      echo "Works Done!"
      exit 3

    fi

  done

# 子文件夹数小于5个则不执行复制功能，退出脚本
else
  n=$(ls -l ${src_path} | grep ^d | wc -l )
  echo
  echo "number of sub folders are: $n, quit!"
  echo "Folders less than $folder_run cannot be copied "
  exit 5

fi

