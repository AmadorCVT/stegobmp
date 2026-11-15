# Solucion

En la consigna nos mandaron los siguientes archivos:
- kings.bmp
- paris.bmp
- loimposible.bmp
- lima.bmp

Sabiendo que ocultaban mensajes uno con lsb1, otro con lsb4, otro con lsbi y uno mas de otra forma

## Paso 1

Probamos todos los metodos en todos los archivos

```sh
for f in kings paris lima loimposible; do
  for m in LSB1 LSB4 LSBI; do
    ./stegobmp -extract -p grupo12/$f.bmp -out analisis/$f -steg $m
  done
done
```

Como salida obtuvimos `kings.png` y `paris.pdf`

## Paso 2

Vemos los archivos

- `kings.png` tiene un buscaminas
- `paris.pdf` dice que la password es *sorpresa*

todavia no sabemos para que sirven estas cosas.

## Paso 3

Sabemos que hay uno con lsb4 que como en el paso 1 no obtuvimos el archivo, suponemos que esta encriptado. Otro no sabemos como oculta el mensaje asi que investivamos un poco sobre los archivos

Al abrir `loimposible.bmp` con un editor de texto vemos que al final dice

```
al .png cambiar extension por .zip y descomprimir
```

lo guardamos en un archivo para tener

```sh
tail -c 49 ./grupo12/loimposible.bmp > ./analisis/loimposible_tail.txt
```

## Paso 4

Hacemos lo que nos dice el mensaje oculto de `loimposible.bmp`

```sh
cp analisis/kings.png analisis/kings.zip
unzip -o analisis/kings.zip -d analisis/
```

y obtenemos `sol12.txt` que tiene el siguiente contenido

```
cada mina es un 1.
cada fila forma una letra.
Los ascii de las letras empiezan todos en 01.
Asi encontraras el algoritmo que tiene clave de 128 bits y el modo
La password esta en otro archivo
Con algoritmo, modo y password hay un .wmv encriptado y oculto.
```

## Paso 5

Resolvemos el buscaminas, hacemos lo que nos dice `sol12.txt`

| Fila | Minas    | `01` + minas | ASCII |
| ---- | -------- | ------------ | ----- |
| 1    | `000001` | `0x41`       | A     |
| 2    | `100101` | `0x65`       | e     |
| 3    | `110011` | `0x73`       | s     |
| 4    | `001111` | `0x4F`       | O     |
| 5    | `100110` | `0x66`       | f     |
| 6    | `100010` | `0x62`       | b     |

Asi que entendemos que el archivo lima esta con lsb4 encriptado con AES de 128 bits en modo OFB y que la password es `sorpresa`

## Paso 6

probamos con

```sh
./stegobmp -extract -p grupo12/lima.bmp -out analisis/lima -steg LSB4 -a aes128 -m ofb -pass sorpresa
```

pero da

```sh
datos corruptos: el tamaño declarado (4236261204 bytes) es mayor que los datos disponibles (908061 bytes) (¿método -steg, password, -a o -m incorrectos?)
```

Asi que empezamos a probar diferentes variantes de mayusculas y minusculas de la palabra *sorpresa* sin éxito

Luego empezamos a probar con `\n`, `\r\n`, y finalmente `\r` al final, resultando en que la password era `sorpresa\r`

Creemos que quizas la password fue levantada de algun archivo que usaba `\r\n` como salto de linea con algun script que parseaba con `\n` y quedó el `\r` como parte de la contraseña.

Finalmente obtemenos el mensaje del archivo `lima.bmp`

```sh
./stegobmp -extract -p grupo12/lima.bmp -out analisis/lima -steg LSB4 -a aes128 -m ofb -pass $'sorpresa\r'
```

obteniendo `lima.wmv`