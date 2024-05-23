@echo off
setlocal enabledelayedexpansion

for %%f in (shaders\*.vert shaders\*.frag) do (
    %VULKAN_SDK%\Bin\glslc.exe %%f -o %%f.spv
)

pause