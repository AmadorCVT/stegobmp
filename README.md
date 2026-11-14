# stegobmp

Herramienta de línea de comandos en C para ocultar y recuperar archivos dentro de imágenes BMP mediante esteganografía. Trabajo Práctico de Criptografía y Seguridad.

> Estado actual: etapa 3 (integración). La línea de comandos, la lectura/escritura de BMP, el formato del payload (`tamaño || archivo || extensión`), el armado del flujo cifrado (`tamaño del cifrado || datos cifrados`) y el cálculo de capacidad están hechos. Las funciones de `lsb.c`, `lsbi.c` y `crypto.c` están declaradas con el cuerpo vacío y las tienen que implementar sus responsables. El archivo a ocultar debe tener extensión.

## Requisitos

- Ubuntu 22.04 (o WSL2 con Ubuntu 22.04)
- `gcc`
- `make`
- OpenSSL (`libssl-dev`) para el cifrado

Instalación de las dependencias:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

## Compilación

```bash
make
```

Genera el ejecutable `./stegobmp` (enlaza con `-lcrypto`). Los archivos objeto quedan en `build/`. Por defecto se compila con `-g` para poder depurar.

Para una versión optimizada y sin información de depuración:

```bash
make release
```

Para borrar todo lo generado (y volver a compilar desde cero):

```bash
make clean
make
```

## Con Docker (cualquier sistema, incluida Mac)

Si no tenés Ubuntu, `docker.sh` corre cualquier comando dentro de un contenedor Ubuntu 22.04 con `gcc` y OpenSSL, con el repo montado en `/work`. Solo hace falta tener Docker instalado y corriendo. La imagen (`stegobmp-dev`) se construye sola la primera vez.

```bash
./docker.sh make
./docker.sh ./stegobmp -extract -p salida.bmp -out recuperado -steg LSB1
./docker.sh            # shell interactiva dentro del contenedor
./docker.sh --rebuild  # reconstruye la imagen si cambió el Dockerfile
```

El ejecutable generado es de Linux: se corre con `./docker.sh ./stegobmp ...`, no directamente en la Mac. Si alternás entre compilar en el contenedor y compilar nativo, hacé `make clean` antes.

## Ejecución

```bash
./stegobmp --help
./stegobmp -embed -in mensaje.txt -p imagen.bmp -out salida.bmp -steg LSB1 [-a aes128|aes192|aes256|3des] [-m ecb|cfb|ofb|cbc] [-pass password]
./stegobmp -extract -p salida.bmp -out mensaje -steg LSB1 [-a ...] [-m ...] [-pass password]
```

Los parámetros pueden ir en cualquier orden, distinguen mayúsculas y minúsculas y usan el guion común `-`. Sin `-pass` no se cifra; con `-pass` y sin `-a`/`-m` se usa `aes128` y `cbc`.

### Ejemplos

Sin password:

```bash
./stegobmp -embed -in mensaje.txt -p imagen.bmp -out salida.bmp -steg LSB1
./stegobmp -extract -p salida.bmp -out recuperado -steg LSB1
# genera recuperado.txt (a -out se le agrega la extensión del archivo oculto)
```

Con password (aquí `aes256` en modo `cbc`):

```bash
./stegobmp -embed -in mensaje.txt -p imagen.bmp -out salida.bmp -steg LSBI -a aes256 -m cbc -pass secreto
./stegobmp -extract -p salida.bmp -out recuperado -steg LSBI -a aes256 -m cbc -pass secreto
```

El programa devuelve 0 si todo salió bien, 1 ante un error de ejecución (archivo inexistente, BMP inválido, no entra en el portador, datos corruptos o password incorrecta) y 2 ante un error en los parámetros. Si falla el descifrado o los datos recuperados no son coherentes, no se escribe ningún archivo.

## Módulos y contratos

| Módulo | Archivos | Responsabilidad |
| --- | --- | --- |
| Tipos comunes | `include/stegobmp.h`, `src/stegobmp.c` | `StegMethod`, `CipherAlgo`, `CipherMode`, `Options`, códigos de error |
| CLI | `include/cli.h`, `src/cli.c`, `src/main.c` | Parseo de argumentos y flujo `-embed` / `-extract` |
| BMP | `include/bmp.h`, `src/bmp.c` | `bmp_load`, `bmp_save`, `bmp_free` |
| Payload | `include/payload.h`, `src/payload.c` | Formato `tamaño \|\| archivo \|\| extensión` y cifrado |
| Esteganografía | `include/steg.h`, `src/steg.c`, `src/lsb.c`, `src/lsbi.c` | `steg_embed` / `steg_extract` y `lsb1_*`, `lsb4_*`, `lsbi_*` |
| Cifrado | `include/crypto.h`, `src/crypto.c` | `crypto_encrypt` / `crypto_decrypt` (PBKDF2 + OpenSSL) |

Cambiar una firma de los headers requiere acuerdo del grupo.

## Estructura del proyecto

```
stegobmp/
├── README.md
├── .gitignore
├── Makefile
├── Dockerfile      # Ubuntu 22.04 + gcc + OpenSSL
├── docker.sh       # corre comandos dentro del contenedor
├── src/            # Código fuente (.c)
│   ├── main.c  cli.c  bmp.c  payload.c
│   ├── steg.c  lsb.c  lsbi.c  crypto.c
│   └── stegobmp.c
├── include/        # Headers (.h)
├── samples/
│   ├── bmp/        # Imágenes BMP de ejemplo
│   ├── secrets/    # Archivos a ocultar de ejemplo
│   └── catedra/    # Archivos de la cátedra
└── docs/           # Documentación e informe
```
