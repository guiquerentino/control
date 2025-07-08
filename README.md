# Control

Aplicativo para Windows que permite bloquear aplicativos e sites em horários definidos.

## Build

Requer MSVC ou MinGW. Compile todos os arquivos em `src` e o recurso em `resources/resource.rc`.

### MSVC
```
cl /EHsc /DUNICODE /D_UNICODE src/*.cpp resources/resource.rc user32.lib shell32.lib comctl32.lib
```

### MinGW
```
windres resources/resource.rc resource.o
g++ -municode src/*.cpp resource.o -lole32 -lshlwapi -lcomctl32 -static -o control.exe
```

O arquivo de configuraçao será salvo em `%APPDATA%\control.ini`.

## Uso

Execute o programa. A janela lista as regras. Use o botão **Pause** para pausar ou retomar os bloqueios inserindo a senha configurada no arquivo.
