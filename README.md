# stegobmp

Herramienta de línea de comandos en C para ocultar y recuperar archivos dentro de imágenes BMP mediante esteganografía. Trabajo Práctico de Criptografía y Seguridad.

> Estado actual: etapa 2. La línea de comandos, la lectura/escritura de BMP, el formato del payload (`tamaño || archivo || extensión`) y el cálculo de capacidad funcionan. Las funciones de `lsb.c`, `lsbi.c` y `crypto.c` están declaradas con el cuerpo vacío y las tienen que implementar sus responsables. El archivo a ocultar debe tener extensión.

## Requisitos

- Ubuntu 22.04 (o WSL2 con Ubuntu 22.04)
- `gcc`
- `make`

Instalación de las herramientas:

```bash
sudo apt update
sudo apt install build-essential
```

## Compilación

```bash
make
```

Genera el ejecutable `./stegobmp`. Los archivos objeto quedan en `build/`.

Para compilar y correr las pruebas (`tests/test_*.c`):

```bash
make test
```

Para borrar todo lo generado:

```bash
make clean
```

## Ejecución

```bash
./stegobmp --help
./stegobmp -embed -in mensaje.txt -p imagen.bmp -out salida.bmp -steg LSB1 [-a aes128|aes192|aes256|3des] [-m ecb|cfb|ofb|cbc] [-pass password]
./stegobmp -extract -p salida.bmp -out mensaje -steg LSB1 [-a ...] [-m ...] [-pass password]
```

Los parámetros pueden ir en cualquier orden, distinguen mayúsculas y minúsculas y usan el guion común `-`. Sin `-pass` no se cifra; con `-pass` y sin `-a`/`-m` se usa `aes128` y `cbc`.

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
├── src/            # Código fuente (.c)
│   ├── main.c  cli.c  bmp.c  payload.c
│   ├── steg.c  lsb.c  lsbi.c  crypto.c
│   └── stegobmp.c
├── include/        # Headers (.h)
├── tests/          # Pruebas
├── samples/
│   ├── bmp/        # Imágenes BMP de ejemplo
│   └── secrets/    # Archivos a ocultar de ejemplo
└── docs/           # Documentación e informe
```
