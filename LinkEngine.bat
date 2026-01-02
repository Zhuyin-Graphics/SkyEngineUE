REM @echo off

setlocal

mkdir "ThirdParty"

set libs=crc32c sfmt taskflow boost rapidjson

set "engine_path=D:\Code\Engine\SkyEngine\cmake-build-release\build\install"
set "thirdparty_path=D:\Code\sky3rd_win32_output"
set "link_path=ThirdParty\SkyEngineRuntime"

mklink /J "%link_path%" "%engine_path%"

for %%a in (%libs%) do (
    mklink /J ThirdParty\%%a %thirdparty_path%\%%a
)

pause