cd ..\Samples\Graph_Cut\x64\Release

for /l %%x in (1, 1, 1) do (
	Graph_Cut.exe -i:LT1_node.F32 -j:LT1_hor_edge.F32 -k:LT1_ver_edge.F32 -w:640 -h:360 -r:32
)

pause
