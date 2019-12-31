# Docker基础命令速查

# 目录
* [镜像](#镜像)
  * [获取镜像列表](#获取镜像列表)
  * [查看镜像列表](#查看镜像列表)
  * [查看镜像详情](#查看镜像详情)
  * [添加标签](#添加标签)
  * [删除镜像](#删除镜像)
  * [搜索镜像](#搜索镜像)
* [容器](#容器)
  * [创建容器](#创建容器)
  * [启动容器](#启动容器)
  * [执行命令](#执行命令)
  * [停止容器](#停止容器)
  * [删除容器](#删除容器)
  * [run命令](#run命令)
    * [新建一个容器并启动](#新建一个容器并启动)
    * [设置环境变量](#设置环境变量)
    * [端口映射](#端口映射)
  * [重启容器](#重启容器)
* [数据卷](#数据卷)
  * [查看数据卷列表](#查看数据卷列表)
  * [查看数据卷详情](#查看数据卷详情)
  * [新建数据卷](#新建数据卷)
  * [挂载数据卷](#挂载数据卷)
  * [删除数据卷](#删除数据卷)


# 镜像
镜像是一个只读的文件，它保存了创建容器所需要的应用程序，是容器的模板。

## 获取镜像
```
docker image pull <NAME[:TAG]> 
```
* `TAG`是镜像的标签，默认为`latest`，即最新的镜像。

示例 :  
```
$ docker pull ubuntu
Using default tag: latest
latest: Pulling from library/ubuntU
Digest: sha256:e27e9d7f7f28d67aa9e2d7540bdc2b33254b452ee8e60f388875e5b7d9b2b696 
Status: Downloaded newer image for ubuntu:latest
```

## 查看镜像列表
```
docker image ls
```

示例 :  
```
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
ubuntu              18.04               452a96d8lc30        5 days ago          79.6MB
ubuntu              latest              452a96d8lc30        5 days ago          79.6MB
```
* 相同的`IMAGE ID`表示他们是同一个镜像的不同标签，它们会共用同一个文件。

## 查看镜像详情
```
docker image inspect <NAME>
```

## 添加标签
```
docker image tag <NAME:TAG> <NEW-NAME:NEW-TAG>
```

示例 :  
```
$ docker image tag ubuntu:18.04 myubuntu:mytag
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
ubuntu              18.04               452a96d8lc30        5 days ago          79.6MB
ubuntu              latest              452a96d8lc30        5 days ago          79.6MB
myubuntu            mytag               452a96d8lc30        5 days ago          79.6MB
```


## 删除镜像
```
docker image rm <NAME:TAG>|<IMAGE-ID>
```
* 使用`NAME:TAG`进行删除时，如果该镜像在本地只有这一个标签，则会删除镜像文件，否则仅仅删除标签。
* 使用`IMAGE ID`进行删除时，如果该镜像在本地有不只一个标签，则无法删除。
* 正在运行的容器所使用的镜像不能删除。

示例 :  
```
$ docker image rm myubuntu:mytag
$ docker image ls
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
ubuntu              18.04               452a96d8lc30        5 days ago          79.6MB
ubuntu              latest              452a96d8lc30        5 days ago          79.6MB
```


## 搜索镜像
```
docker search <KEYWORD>
```

示例 :  
```
$ docker search mysql
NAME                    DESCRIPTION                                      STARS   OFFICIAL     AUTOMATED
mysql                   MySQL is a widely used, open-source relation…    8944    [OK]         
mariadb                 MariaDB is a community-developed fork of MyS…    3151    [OK]         
mysql/mysql-server      Optimized MySQL Server Docker images. Create…    662                  [OK]
```

docker的镜像搜索功能好像没有查标签的功能，可以用下面的Python脚本来搜标签 :

```Python
#! /usr/bin/env python3
# Python >= 3.6 for f-string
import json
import os
import ssl
import sys
from urllib import request

ssl._create_default_https_context = ssl._create_unverified_context

if len(sys.argv) not in (2, 3):
    print(f"Usage : {os.path.basename(__file__)} <repo> [tag]")
    print(f"        {os.path.basename(__file__)} nginx")
    print(f"        {os.path.basename(__file__)} mysql 5.6")
    quit(1)

url = f"https://registry.hub.docker.com/v1/repositories/{sys.argv[1]}/tags"
try:
    res = request.urlopen(url, timeout=2)
    tags = json.loads(res.read().decode("utf-8"))

    tags = [
        tag["name"] for tag in tags if len(sys.argv) == 2 or sys.argv[2] in tag["name"]
    ]
    for tag in tags:
        print(f"{sys.argv[1]}:{tag}")
    print()
    print(f"total {len(tags)} tags")
except Exception as e:
    print(e)
```


# 容器
容器是基于镜像之上添加可写层创建的轻量级虚拟环境。

## 查看容器列表
```
docker container ps
```
* 只显示正在运行的容器
* 添加`-a`选项查看所有容器

示例 : 
```
$ docker container ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
```

## 查看容器详情
```
docker container inspect <NAME>
```

## 创建容器
```
docker container create -it <NAME:TAG>
```
>```
> -i: --interactive   Keep STDIN open even if not attached(即使没有连接，也要保持STDIN打开)  
> -t: --tty           Allocate a pseudo-TTY(分配一个伪终端)
>```
* 可以使用`--name`选项给容器设置名字

示例 :  
```
$ docker container create -it ubuntu:latest
af8f4f922dafee22c8fe6cd2aelld16e25087d61flblfa55b36e94db7ef45178
$ docker container ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
af8f4f922daf ubuntu:latest "/bin/bash" 17 seconds ago Created 
```

## 启动容器
```
docker container start <CONTAINER-ID>
```

示例 :  
```
$ docker container start af8f4f922daf
$ docker container ps 
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
af8f4f922daf ubuntu:latest "/bin/bash" 1 minutes ago  Up 3 seconds ago
```

## 执行命令
```
docker container exec -it <CONTAINER-ID> <COMMAND>
```
* `COMMAND`参数设为`/bin/bash`即可启动容器中的shell与容器进行交互

示例 :  
```
$ docker container exec -it af8f4f922daf /bin/bash
root@af8f4f922daf:/# 
```

## 停止容器
```
docker container stop <CONTAINER-ID>
```

示例 :  
```
$ docker container stop af8f4f922daf
$ docker container ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
af8f4f922daf ubuntu:latest "/bin/bash" 5 minutes ago Created 
```

## 删除容器
```
docker container rm <CONTAINER-ID>
```

示例 : 
```
$ docker container rm af8f4f922daf
$ docker container ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
```


## run命令
`docker container run`命令是最常用的命令，设计者**似乎**把所有功能都放到了这个命令里了。

### 新建一个容器并启动
```
docker container run -it <NAME:TAG> <COMMAND>
```

示例 :  
```
$ docker container run -it busybox
/ # ls
bin   dev   etc   home  proc  root  sys   tmp   usr   var
/ # pwd
/
/ # who
/ # whoami
root
/ # uname -a
Linux 27a94c7b04ed 4.15.0-45-generic #48-Ubuntu SMP Tue Jan 29 16:28:13 UTC 2019 x86_64 GNU/Linux
/ # exit
$ docker container ps
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
```

如果要让容器在后台运行，可以添加`-d`（守护进程Daemon）选项 :  
```
$ docker container run -itd  ubuntu
51d7a00d45f124304ea0fa7f57ff86397e8590e46bf6166f4e946bb8d2e12913
$ docker container ps
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
51d7a00d45f1 ubuntu:latest "/bin/bash" 5 minutes ago Created 
```
* 可以使用[`docker container exec -it <CONTAINER-ID> /bin/sh`](#执行命令)来进入容器的shell

### 设置环境变量
在启动容器时可以使用`-e NAME=VALUE`的方式设置环境变量，从而对容器中的程序进行配置。

```
$ docker run -e ABC=123456 -it busybox
/ # echo $ABC
123456
/ # 
```

### 端口映射
在使用容器时，很多情况是运行一个服务程序（例如MySQL）来为宿主机提供服务，这时就需要进行端口映射，让宿主机能够访问到该服务。

端口映射的方法是在`run`命令中使用`-p <host-port:container-port>`选项 :  

```
$ docker run -p 2333:3306 -e MYSQL_ROOT_PASSWORD=123456 -itd mysql
$ mysql -u root -h 127.0.0.1 -P 2333 -p
Enter password: 
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 8
Server version: 8.0.18 MySQL Community Server - GPL

Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> 

```
* 这里通过`-e MYSQL_ROOT_PASSWORD=123456`选项将MySQL的root密码设置为了123456

## 重启容器
```
docker restart demo
```
使用`run`命令，每次都会新建一个容器，如果要再启动一个容器的话，可以使用`restart`命令。

```
$ docker run --name=demo -e ABC=123456 -itd busybox
f52705f4cc936809336c7c5014ea2ab0e24c3776e5ad0142a7eeb6d66e34580f
$ docker stop demo
demo
$ docker restart demo
demo
$ docker exec -it demo /bin/sh
/ # echo $ABC
123456
/ # 
```

# 数据卷
数据卷是一个特殊的目录，通过数据卷可以在容器与宿主机之间、容器与容器之间共享文件数据等。

## 查看数据卷列表
```
docker volume ls
```

## 新建数据卷
```
docker volume create <NAME>
```

示例 :  
```
$ docker volume ls
DRIVER              VOLUME NAME
local               87f5bfa7d7fc4642e47412841fd7786f9bebd9f2f0475ae934a70aa5527069a7
local               328cead32e015e490c883a4992a385eb28ffeeadd1b439ec0ab705a3d91c76d8
local               48701fb5554a82a38d5846d06ef7569c88242af8122a770a253d4fb2ad060984
local               97192203dc66c6ae1c74df7d89b4983145105f0d17f608dc328fb336e95c9753
local               bcfd9fe80ac9bacecbd61377aeb07b8488000e3edfd731adaabb007fdd57166b
local               c6a58a9596b3667b5c09f80c51557123da00280f10f5cd06b6ac90ea8fa4b613
local               d0f177b24933e26b1877c88f224198b6ee30f16114db2b8b306b07f9af4b2094
local               da228d169e0bc123e5f548a723ba4e112b5b556f06539354e6b2e9ad97882570
local               demo
local               eb5b46f0064b1d667093c98a8ec350f83684be40edf3c4ae4e1f8821b3abde1e
```

## 查看数据卷详情
```
docker volume create <NAME>
```

示例 :  

```
$ docker volume inspect demo
[
    {
        "CreatedAt": "2019-12-31T16:32:46+08:00",
        "Driver": "local",
        "Labels": {},
        "Mountpoint": "/var/lib/docker/volumes/demo/_data",
        "Name": "demo",
        "Options": {},
        "Scope": "local"
    }
]
```
* 其中`"Mountpoint": "/var/lib/docker/volumes/demo/_data"`是数据卷在本地的路径

## 挂载数据卷
挂载数据卷需要在`run`命令中使用`-v`选项
```
docker container run -v <VOLUME-NAME>:<CONTAINER-PATH>
```
* 将名为`VOLUME-NAME`的数据卷挂载到容器中的`CONTAINER-PATH`目录下
* 如果该数据卷不存在，则会自动创建

示例 :  
```
$ docker run -v demo:/tmp/demo -it busybox
/ # ls /tmp
demo
```

## 删除数据卷
```
docker volume rm <NAME>
```

示例 :  
```
docker volume rm demo
```
* 需要注意的是，删除数据卷之前必须删除相关的容器，即使该容器没有在运行（我没有找到卸载数据卷的方法）