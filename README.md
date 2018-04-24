# In-Depth Buffers
I3D 2018

[Songfang Han](https://hansf1992.wixsite.com/home)
Ge Chen
[Diego Nehab](http://w3.impa.br/~diego/)
[Pedro V. Sander](http://www.cse.ust.hk/~psander/)

Maintained by Songfang Han (<shanaf@connect.ust.hk>)

Tested on Qt 4.8, Windows 7.

## Setup

### Dependencies

To install the dependencies, run:

    cd indepthbuffers

### DAGClustering

To generate indepth buffers:

    dag-merger.exe -m modelpath -s viewsub -l sampleTriSub -c cacheDir
 
An example usage is in the.

### DAG Order Upsample

To generate order from coarse model

    dag-merger.exe -m modelpath -s viewsub -l sampleTriSub -c cacheDir

### Viewer
An example render with preprocessed in-depth buffers is given in dag-viewer.
To use it:
 
    dag-merger.exe -m modelpath -s viewsub -l sampleTriSub -c cacheDir 
  
Final code will be released by publication time.
