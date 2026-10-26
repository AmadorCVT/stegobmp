# stegobmp

Herramienta de línea de comandos en C para ocultar y recuperar archivos dentro de imágenes BMP mediante esteganografía. Trabajo Práctico de Criptografía y Seguridad.

> Estado actual: setup inicial. El programa solo muestra un mensaje de bienvenida y una ayuda básica. Las operaciones `-embed` y `-extract` todavía no están implementadas.

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
./stegobmp
./stegobmp --help
```

## Estructura del proyecto

```
stegobmp/
├── README.md
├── .gitignore
├── Makefile
├── src/            # Código fuente (.c)
│   └── main.c
├── include/        # Headers (.h)
├── tests/          # Pruebas
├── samples/
│   ├── bmp/        # Imágenes BMP de ejemplo
│   └── secrets/    # Archivos a ocultar de ejemplo
└── docs/           # Documentación e informe
```
