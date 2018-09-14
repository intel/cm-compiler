cd ..\Samples\Graph_Cut\x64\Release

for /l %%x in (1, 1, 1) do (
	Graph_Cut.exe -i:LT2_node.F32 -j:LT2_hor_edge.F32 -k:LT2_ver_edge.F32 -w:1280 -h:720 -r:32
)

pause
