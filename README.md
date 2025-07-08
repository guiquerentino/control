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

Execute o programa. A janela lista as regras. No primeiro acesso o aplicativo solicitará o cadastro de uma senha, que fica salva de forma criptografada no `control.ini`. Utilize os botões **Pause** e **Resume** para alternar os bloqueios. Também é possível incluir novos aplicativos através do botão **Add App**, que abre o Explorador de Arquivos para escolher o executável. A senha pode ser modificada usando **Change Password**.
