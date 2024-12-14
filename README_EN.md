# GCScholarInsight

## Build Instructions (with WSL2)

### Frontend

To build the GCScholarWeb Frontend, follow these steps:

1. Navigate to the project directory

   ```bash
   cd GCScholarWeb
   ```

2. Install pnpm denendency

   ```bash
   pnpm add -D sass
   ```

3. After that we can run frontend

   ```bash
   pnpm dev
   ```

4. Frontend will run on localhost:5173

### Backend

To build the GCScholarInsight Backend, follow these steps:

1. Navigate to the project directory

   ```bash
   cd GCScholarInsight
   git submodule init
   git submodule update
   cp /path/to/dblp.xml ./xml
   ```

2. Use Ninja to build C++ part

   ```bash
   cmake -S ./ -B ./ -G Ninja
   ninja
   ```

3. After that we can run python backend

   ```bash
   python3 -m venv .venv
   source .venv/bin/activate
   pip install -r requirements.txt
   # Just install any package if there are still errors
   python3 ./src/py/main.py -i ./xml/dblp.xml -p 
   ```

4. Backend will run on localhost:5000

## Running the Binary

We provide a pre-packaged backend executable file. You can run it directly without needing to build it.

Requirements for running:

- Prepare the XML database and place it under `./xml`, or place the existing `db` folder and `main.exe` in the same root directory.
- Run `main.exe -i ./xml/dblp.xml` to build the database.
- Run `main.exe -g` to perform additional analyses, including maximum clique analysis.
- Run `main.exe` to start the backend server. You can specify the -p parameter to preload part of the database into memory.
- You can also run `main.exe --standalone`, which eliminates the need for deploying a reverse proxy. You can directly access localhost:6767 for out-of-the-box use.

## Other information

This project does not include the dataset. You need to download the dataset yourself and place it in the GCScholarInsight/xml directory.

```bash
# DBLP dataset:
https://dblp.uni-trier.de/xml/
```

To experience the full functionality, you need to prepare a middleware software as a reverse proxy server and configure it as follows:

```bash
/api -> localhost:5000
/ -> localhost:5173 or the build directory of the frontend .../dist/index.html
```

Recommended middleware software includes Nginx (Linux) or IIS (Windows).

If your computer doesn't have the necessary middleware software installed, we offer a ready-to-use mode. In this mode, the Python Flask framework will handle both serving static files and routing tasks. You only need to specify:

```bash
python3 ./src/py/main.py --standalone
```
