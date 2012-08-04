REM Process BOX
Model_Packer.exe d3d -ExportJustRenderObject pCubeShape1 -input box.lta -output box.ro -stream0 position normal uv1 basisvectors
pause

REM Process Cylinder
Model_Packer.exe d3d -ExportJustRenderObject pCylinderShape1 -input cylinder.lta -output cylinder.ro -stream0 position normal uv1 basisvectors
pause

REM Process HighPolySphere
Model_Packer.exe d3d -ExportJustRenderObject pSphereShape2 -input highpolyshere.lta -output highpolyshere.ro -stream0 position normal uv1 basisvectors
pause

REM Process LowPolySphere
Model_Packer.exe d3d -ExportJustRenderObject pSphereShape1 -input lowpolyshere.lta -output lowpolyshere.ro -stream0 position normal uv1 basisvectors
pause

REM Process TeaPot
Model_Packer.exe d3d -ExportJustRenderObject TEAPOT01Shape -input teapot.lta -output teapot.ro -stream0 position normal uv1 basisvectors
pause

REM Process Wall
Model_Packer.exe d3d -ExportJustRenderObject pCubeShape2 -input wall.lta -output wall.ro -stream0 position normal uv1 basisvectors
pause