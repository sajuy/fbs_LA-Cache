# Delayed-Hits Simulator
Here provides the basic realizations of algorithms under delayed-hits case. The simulator here contains components described in our paper "Towards Latency Awareness for Content Delivery Network Caching".


# 代码修改
该部分为我们在运行源代码过程中，为了使代码可以顺利运行，对源代码进行的修改工作。
## （a）原型：配置ATS
1、prototype/trafficserve r-8.0.3/configs/records.config.default第129行-第133行修改为：
```
CONFIG proxy.config.cache.ram_cache.size INT 4G
# no specific: unmodified traffic server
CONFIG proxy.config.cache.vdisk_cache.algorithm STRING LA
CONFIG proxy.config.cache.vdisk_cache.memory_window INT 12582912
CONFIG proxy.config.cache.vdisk_cache.n_extra_fields INT 1
```
2、prototype/trafficserve r-8.0.3/configs/storage.config.default第53行将512M修改为512G
   
## （b）"fbs/Simulator/Runs.py"
第51行将地址补充完整：
```
CmdRoot = "/home/candy/Desktop/fbs/Simulator/Delayed-Source-Code/build/bin/"
```

第55行将地址补充完整：
```
TrRoot = "./Example/Traces/"
```
第56行将地址补充完整：
```
OutRoot = "./Results/"
```

## （c）"fbs/Example/Example-Plots.ipynb"
### 1、路径前加入：D:/VSCode/fbs

第3段代码第3行：
```
Path = "D:/VSCode/fbs/Example/ProcRes/" + Name + ".txt"
```
第12段代码第45行： 
```
PBusrt = "D:/VSCode/fbs/Example/Burst/" + Name + "_b.txt"
```
第12段代码第49行：
```
PLats = "D:/VSCode/fbs/Example/AvgLats/" + Name + "//" + algo + "_" + str(CSize) +".txt"
```
第17段代码第20行： 
```
RPath = "D:/VSCode/fbs/Example/Verify/" + Name + "_" + str(D) + "l"
```

### 2、画图修改
第5段代码第6-7行；第8段代码第6-7行；第10段代码第6-7行；第13段代码第7-8行；第18段代码第6-7行；
修改前：
```
fpath = os.path.join(rcParams["datapath"],"fonts/ttf/cmr10.ttf")
prop = fm.FontProperties(fname=fpath)
```
修改后：
```
default_font = fm.findSystemFonts()
font_path = default_font[0]
prop = fm.FontProperties(fname=font_path)
```
### 3、将basex=2删除
第5段第67行：

修改前：
```
ax1.set_xscale("log",basex=2)；
```
修改后：
```
ax1.set_xscale("log")
```
第5段第114行：

修改前：
```
ax2.set_xscale("log",basex=2)；
```
修改后：
```
ax2.set_xscale("log")
```
第10段第72行：

修改前：
```
ax1.set_xscale("log",basex=2)；
```
修改后：
```
ax1.set_xscale("log")
```
第10段第126行：

修改前：
```
ax2.set_xscale("log",basex=2)；
```
修改后：
```
ax2.set_xscale("log")
```
以下是算法功能及使用方法的介绍


# Usage
1. Requirement: Ubuntu 18.04, "GCC 7.0+", "CMake 3.10+", "Boost C++ v1.70+"

2. The "./Simulator" contains codes and scripts to run simulations. More details can be found in "./Simulator/README.md"

3. The "./Prototype" includes codes about apache trafficserver and prototypes. More details can be found in "./Prototype/README.md"

4. The "./ProcData" is used to process simulation results and get some results to draw plots. Please go to "./ProcData/README.md" to get more details

5. Folder "./Example" gives the all processed data and a file named "Example-Plots.ipynb" which uses the data to draw plots

# Trace Format
Request traces are expected to be in a space-separated format with 3 columns: request time, content id and content size.

| Time | ID | Size |
|:----:|:----:|:----:|
| 0 | 1 | 1024 |
| 1 | 23 | 1024 |
| 2 | 15 | 2048 |
| 3 | 10 | 1024 |


# Experiments
We have implemented LA-Cache and a bunch of state-of-the-arts in simulators including but not limited to:
- LA-Cache
- LRU-MAD
- LHD-MAD
- LRU
- LHD
- LRU-K
- 2Q
- LFU
- Belady
- Offline Delay

To draw the plots in our paper, you should install Python3.0+ at first. More details can be found in folder "./Example".


# Citation
If you use the simulator or some results in our paper for a published project, please cite our work by using the following bibtex entry

```
@inproceedings{yan2022towards,
  title={Towards Latency Awareness for Content Delivery Network Caching},
  author={Yan, Gang and Li, Jian},
  booktitle={Proc. of USENIX ATC},
  year={2022}
}
```
