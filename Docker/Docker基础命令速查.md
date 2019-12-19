# Docker基础命令速查

# 目录
* [镜像操作](#镜像操作)
  * [获取镜像](#获取镜像)
  * [查看镜像](#查看镜像)
  * [添加标签](#添加标签)
  * [删除镜像](#删除镜像)
  * [搜索镜像](#搜索镜像)
* [容器操作](#容器操作)
  * [创建容器](#创建容器)
  * [启动容器](#启动容器)
  * [创建并启动容器](#创建并启动容器)
  * [执行命令](#执行命令)
  * [停止容器](#停止容器)
  * [删除容器](#删除容器)


# 镜像操作
镜像是一个只读的文件，它保存了容器所需要的操作系统和应用程序等，是容器创建的模板。

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

## 查看镜像
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


# 容器操作

## 查看容器
```
docker container ps
```
* 只显示正在运行的容器
* 添加`-a`选项查看所有容器

示例 : 
```
$ docker ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
```

## 创建容器
```
docker container create -it <NAME:TAG>
```

示例 :  
```
$ docker container create -it ubuntu:latest
af8f4f922dafee22c8fe6cd2aelld16e25087d61flblfa55b36e94db7ef45178
$ docker ps -a
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
$ docker ps
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
af8f4f922daf ubuntu:latest "/bin/bash" 1 minutes ago  Up 3 seconds ago
```

## 创建并启动容器
```
docker container run -itd <NAME:TAG>
```
* 可以使用`-p <port-host>:<port-container>`进行端口映射，这样可以通过网络访问容器中的服务

示例 :  
```
$ docker container run -itd ubuntu
c0d418eabe5ee2039d3722f4d4454d89fabc0a2bdce226585b78ad8c5164ab36
$ docker ps
CONTAINER ID IMAGE         COMMAND     CREATED         STATUS  
c0d418eabe5e ubuntu:latest "/bin/bash" 3 seconds ago   Up 3 seconds ago
af8f4f922daf ubuntu:latest "/bin/bash" 15 minutes ago  Up 3 seconds ago
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
docker container pause <CONTAINER-ID>
```

示例 :  
```
$ docker container pause af8f4f922daf
$ docker ps -a
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
$ docker ps -a
CONTAINER ID IMAGE         COMMAND     CREATED        STATUS  
```
