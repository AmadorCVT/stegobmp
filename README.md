# stegobmp

Herramienta de línea de comandos en C para ocultar y recuperar archivos dentro de imágenes BMP mediante esteganografía. Trabajo Práctico de Criptografía y Seguridad.

> Estado actual: etapa 1 (esqueleto y contratos). La línea de comandos y la lectura/escritura de BMP funcionan. `-embed` escribe una copia idéntica del portador (todavía no oculta nada) y `-extract` informa que no está implementado. Los cuerpos de `lsb.c`, `lsbi.c`, `crypto.c` y `payload.c` son marcadores a reemplazar por sus responsables.

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
