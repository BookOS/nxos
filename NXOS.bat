@echo Set environment for NXOS.
@set NXOS_NAME=NXOS
@set NXOS_ROOT_DIR=%cd%
@set NXOS_SRC_DIR=%cd%/Src
@set NXOS_EXEC_PATH=%NXOS_ROOT_DIR%\Tools\x86_64-elf-gcc\bin
@set PATH=%NXOS_EXEC_PATH%;%PATH%;
@echo [ROOT DIR] %NXOS_ROOT_DIR%
@echo [SRC DIR] %NXOS_SRC_DIR%
@echo [EXEC PATH] %NXOS_EXEC_PATH%