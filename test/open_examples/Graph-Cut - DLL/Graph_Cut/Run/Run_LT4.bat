cd ..\Samples\Graph_Cut\x64\Release

for /l %%x in (1, 1, 1) do (
	Graph_Cut.exe -i:LT4_node.20480x2880.F32 -j:LT4_hor_edge.20480x2880.F32 -k:LT4_ver_edge.20480x2880.F32 -w:5120 -h:2880 -r:32
)

pause
 