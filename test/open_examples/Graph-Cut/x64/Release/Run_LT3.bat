cd ..\Samples\Graph_Cut\x64\Release

for /l %%x in (1, 1, 1) do (
	Graph_Cut.exe -i:LT3_node.10240x1440.F32 -j:LT3_hor_edge.10240x1440.F32 -k:LT3_ver_edge.10240x1440.F32 -w:2560 -h:1440 -r:32
)

pause
