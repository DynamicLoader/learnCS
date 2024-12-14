# GCScholarInsight

## Authors

- [@Alexlin (Hongjie Lin)](https://github.com/AlexLin625)
- [@DynamicLoader (Yi Lu)](https://github.com/DynamicLoader)
- [@hohq (White Bill)](https://github.com/hohq)
- [@perfsakuya (Hibiki)](https://github.com/perfsakuya)

## 构建说明（使用WSL2）

### 前端

要构建 GCScholarWeb 前端，请按照以下步骤操作：

1. 进入到项目目录

   ```bash
   cd GCScholarWeb
   ```

2. 安装 pnpm 以及依赖

   ```bash
   pnpm add -D sass
   ```

3. 完成后，可以运行前端

   ```bash
   pnpm dev
   ```

4. 前端将部署在 localhost:5173 上

### 后端

要构建 GCScholarInsight 后端，请按照以下步骤操作：

1. 进入到项目目录

   ```bash
   cd GCScholarInsight
   git submodule init
   git submodule update
   cp /path/to/dblp.xml ./xml
   ```

2. 使用 Ninja 构建 C++ 部分

   ```bash
   cmake -S ./ -B ./ -G Ninja
   ninja
   ```

3. 完成后，可以运行 Python 后端

   ```bash
   python3 -m venv .venv
   source .venv/bin/activate
   pip install -r requirements.txt
   # Just install any package if there are still errors
   python3 ./src/py/main.py -i ./xml/dblp.xml -p 
   ```

4. 后端将部署在 localhost:5000 上

## 运行二进制

我们提供打包好的后端可执行文件。您可以直接运行它，而不需要构建。

运行需要

- 准备好xml数据库，放在`./xml`下面，或者将已有的`db`文件夹和`main.exe`放至同一文件夹
- 运行`main.exe -i ./xml/dblp.xml`构建数据库
- 运行`main.exe -g` 运行额外的分析，包括最大团分析等
- 运行`main.exe` 运行后端服务器。可指定参数`-p`来预加载部分数据库到内存。
- 也可以运行`main.exe --standalone`，这样不需要部署反代，您可以直接访问`localhost:6767`，实现开箱即用。

## 其他信息

本项目不包括数据集。您需要自行下载数据集并将其放置在 `GCScholarInsight/xml` 目录下。

```bash
# DBLP 数据集:
https://dblp.uni-trier.de/xml/
```

为了最好的性能，您需要自行准备一个中间件软件作为反向代理服务器，并配置

```bash
/api -> localhost:5000
/ -> localhost:5173 或前端的构建目录 .../dist/index.html
```

推荐的中间件软件有 Nginx (Linux) 或 IIS (Windows)。

若您的电脑上没有现有的中间件软件，我们提供了开箱即用模式。这里，Python Flask框架会兼任代理静态文件和路由的任务。只需要指定

```bash
python3 ./src/py/main.py --standalone
```

即可。您需要在启动之前构建前端项目。
