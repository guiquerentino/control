# Control

Aplicativo para Windows que permite bloquear aplicativos e sites em horários definidos.

## Build

Agora o projeto pode ser compilado com **CMake** usando MSVC ou MinGW.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

O executável `control.exe` será criado dentro da pasta `build`.

O arquivo de configuração será salvo em `%APPDATA%\control.ini`.

## Uso

Execute o programa. A janela lista as regras. Utilize os botões **Pause** e **Resume** para alternar os bloqueios. No primeiro acesso será solicitado que você defina uma senha. Ela é armazenada de forma criptografada no `control.ini` e pode ser modificada usando **Change Password**.
