cd ..\Samples\Graph_Cut\x64\Release

Graph_Cut.exe -i:LT2_node.F32 -j:LT2_hor_edge.F32 -k:LT2_ver_edge.F32 -w:1280 -h:720 -r:32
Graph_Cut.exe -i:LT3_node.10240x1440.F32 -j:LT3_hor_edge.10240x1440.F32 -k:LT3_ver_edge.10240x1440.F32 -w:2560 -h:1440 -r:32
Graph_Cut.exe -i:LT4_node.20480x2880.F32 -j:LT4_hor_edge.20480x2880.F32 -k:LT4_ver_edge.20480x2880.F32 -w:5120 -h:2880 -r:32


pause
