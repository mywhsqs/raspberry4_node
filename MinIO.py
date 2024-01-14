from minio import Minio
import io
import os
import shutil


client = Minio(
    "10.33.55.7:30008",
    access_key="esdp",
    secret_key="mn72qQXP*h",
    secure=False,
)

bucket_name = "rk3568node501"

# 检查存储桶是否存在，不存在则创建
found = client.bucket_exists(bucket_name)
if not found:
    client.make_bucket(bucket_name)


def is_empty_folder(path):
    with os.scandir(path) as entries:
        for entry in entries:
            if entry.is_file():
                return False
    return True


def upload_to_minio(local_path, minio_path=""):
    file_list = []
    folder_list = []
    for root, dirs, files in os.walk(local_path):
        for file in files:
            file_path = os.path.join(root, file)
            file_list.append(file_path)

        for directory in dirs:
            dir_path = os.path.join(root, directory)
            folder_list.append(dir_path)

    # 按照文件创建时间降序排序文件列表
    file_list.sort(key=lambda x: os.path.getctime(x), reverse=True)

    for file_path in file_list:
        object_name = os.path.relpath(file_path, local_path).replace("\\", "/")
        try:
            with open(file_path, 'rb') as file_data:
                file_stat = os.stat(file_path)
                client.put_object(
                    bucket_name,
                    minio_path + "/" + object_name,
                    file_data,
                    file_stat.st_size
                )
            print(f"{minio_path}/{object_name} 上传成功！")
        except Exception as e:
            print(f"{minio_path}/{object_name} 上传失败：{e}")
            # 记录上传失败日志
            with open("upload_failed.log", "a") as log_file:
                log_file.write(f"{minio_path}/{object_name} 上传失败：{e}\n")
        else:
            # 上传成功后删除本地文件
            os.remove(file_path)

    # 按照名称排序文件夹列表
    folder_list.sort()

    for dir_path in folder_list:
        object_name = os.path.relpath(dir_path, local_path).replace("\\", "/")

        # 检查文件夹是否为空
        if is_empty_folder(dir_path):
            continue

        try:
            client.put_object(bucket_name, minio_path + "/" + object_name + "/", io.BytesIO(b""), 0)
            print(f"{minio_path}/{object_name} 文件夹创建成功！")
        except Exception as e:
            print(f"{minio_path}/{object_name} 文件夹创建失败：{e}")
            # 记录文件夹创建失败日志
            with open("folder_creation_failed.log", "a") as log_file:
                log_file.write(f"{minio_path}/{object_name} 文件夹创建失败：{e}\n")
        upload_to_minio(dir_path, minio_path)


def upload_and_delete(local_path, minio_path=""):
    # 上传到MinIO服务器
    upload_to_minio(local_path, minio_path)

    # 删除本地文件夹及其内容
    shutil.rmtree(local_path)


# 使用示例
local_path = "/mnt"
upload_and_delete(local_path)
