# In-Depth Buffers
I3D 2018

[Songfang Han](https://hansf1992.wixsite.com/home)
Ge Chen
[Diego Nehab](http://w3.impa.br/~diego/)
[Pedro V. Sander](http://www.cse.ust.hk/~psander/)

Maintained by Songfang Han (<shanaf@connect.ust.hk>)

Tested on Qt 4.8, visual studio 2015, Windows 7.

## Setup

### Dependencies

This project is build with Boost 1.61. For the rendering example, glew, glfw, flm and freeImage is used.

    Download thirdparty.zip
    unzip thirdparty to root

### DAGClustering

To generate indepth buffers:

    dag-merger.exe -m modelpath -s viewSub -l sampleTriSub -e epsilonParam -c cacheDir -p priorityMetric -i relaxIteration
 
An example is given in the root/bin/example.bin.

### DAG Order Upsample

To generate order from coarse model

    dag-upsample.exe -m modelpath -n coarseModelpath -s viewSub -l sampleTriSub -e epsilonParam-c cacheDir -b carsemodelCacheDir

An example is given in the root/bin/example.bin.

### Viewer
An example render using preprocessed in-depth buffers is given in dag-viewer.
To use it:
 
    dag-viewer.exe -m modelpath -c cacheDir
  
An example is given in the root/bin/example.bin.
