# Estegoanálisis - grupo 12

La cátedra nos mandó 4 bmp (`grupo12/`). Según la consigna, uno tiene algo oculto con LSB1, otro
con LSB4, otro con LSBI, y el cuarto "de otra forma". Todo lo que vamos sacando queda en
`analisis/`.

Los comandos se corren desde la raíz del repo. `./stegobmp` es binario de Linux: en Mac va con
`./docker.sh` adelante (ej. `./docker.sh ./stegobmp -extract ...`). Los pasos que usan `unzip`
van en la Mac (o en cualquier lado que lo tenga), porque la imagen de docker no lo trae.

```sh
./docker.sh make
mkdir -p analisis
```

## Resumen

| Archivo | Método | Qué tenía |
|---|---|---|
| `kings.bmp` | LSB1 | `kings.png`: captura de un buscaminas, con un zip pegado después del `IEND` |
| `paris.bmp` | LSBI | `paris.pdf`: "La password es sorpresa" |
| `loimposible.bmp` | otro: texto pegado al final del archivo | "al .png cambiar extension por .zip y descomprimir" |
| `lima.bmp` | LSB4, AES-128 OFB, password `sorpresa\r` | `lima.wmv`: un fragmento de video |

## Paso 1: probar los 3 métodos en cada imagen

Sin saber nada, probamos todas las combinaciones sin password. Si el método no es el correcto,
el tamaño que se lee de los primeros 4 bytes es basura y el programa lo rechaza.

```sh
for f in kings paris lima loimposible; do
  for m in LSB1 LSB4 LSBI; do
    ./stegobmp -extract -p grupo12/$f.bmp -out analisis/${f}_$m -steg $m
  done
done
```

Resultado:

| | LSB1 | LSB4 | LSBI |
|---|---|---|---|
| kings | **sale un `.png`** | tamaño 4008636142 > capacidad | tamaño 14476578 > capacidad |
| paris | tamaño 1276135438 > capacidad | tamaño 3785374702 > capacidad | **sale un `.pdf`** |
| lima | tamaño 501098861 > capacidad | tamaño **908065**, pero la extensión no termina en `\0` | tamaño 3207867483 > capacidad |
| loimposible | tamaño 4294967295 (`FF FF FF FF`) | tamaño 4294967295 | la extensión no es válida |

- **kings** es LSB1 y **paris** es LSBI.
- **lima** con LSB4 da un tamaño razonable (908065 bytes, menos que la capacidad de 13 MB, cosa
  que con 4 bytes al azar pasa ~0.3% de las veces) pero lo que sigue no tiene la forma
  `archivo || extension`: es candidato a estar **cifrado**.
- **loimposible** tiene todos los LSB en 1, no parece haber nada con estos métodos: es el cuarto.

Dejamos los dos que salieron con nombre más corto:

```sh
mv analisis/kings_LSB1.png analisis/kings.png
mv analisis/paris_LSBI.pdf analisis/paris.pdf
```

## Paso 2: paris.pdf → password

Abriendo [`paris.pdf`](../analisis/paris.pdf) dice:

> La password es sorpresa

Todavía no sabemos para qué es.

## Paso 3: loimposible.bmp → texto al final del archivo

El tamaño del archivo no coincide con el que dice el header del bmp (`bfSize`, offset 2, 4 bytes
little endian):

```sh
stat -c %s grupo12/loimposible.bmp        # 8400103   (en Mac: stat -f %z)
xxd -s 2 -l 4 -e grupo12/loimposible.bmp  # 00802cb6 = 8400054
```

Sobran 8400103 - 8400054 = 49 bytes, pegados después de los píxeles:

```sh
tail -c 49 grupo12/loimposible.bmp | tee analisis/loimposible_tail.txt
```

> al .png cambiar extension por .zip y descomprimir

Este es el archivo que oculta "de otra forma": el mensaje no está en los píxeles sino agregado al
final, donde los visores de bmp no lo muestran (leen hasta `bfSize`).

## Paso 4: kings.png → zip después del IEND

Un png termina en el chunk `IEND` (`49 45 4E 44` + CRC `AE 42 60 82`). Mirando ahí, después del
CRC sigue `50 4B 03 04` = `PK\x03\x04`, la firma de un zip:

```sh
xxd analisis/kings.png | grep -A2 IEND
```

```
0000a650: 0000 4945 4e44 ae42 6082 504b 0304 1400  ..IEND.B`.PK....
0000a660: 0000 0800 d9ad 615b 1c30 44cb ad00 0000  ......a[.0D.....
0000a670: 0401 0000 0900 1c00 736f 6c31 322e 7478  ........sol12.tx
```

Siguiendo la pista de loimposible, lo renombramos y lo descomprimimos. `unzip` avisa que hay
42586 bytes de más al principio (el png) pero lo saca igual, porque un zip se lee desde el final:

```sh
cp analisis/kings.png analisis/kings.zip
unzip -o analisis/kings.zip -d analisis/
cat analisis/sol12.txt
```

[`sol12.txt`](../analisis/sol12.txt):

```
cada mina es un 1.
cada fila forma una letra.
Los ascii de las letras empiezan todos en 01.
Asi encontraras el algoritmo que tiene clave de 128 bits y el modo
La password esta en otro archivo
Con algoritmo, modo y password hay un .wmv encriptado y oculto.
```

## Paso 5: el buscaminas → algoritmo y modo

[`kings.png`](../analisis/kings.png) es un tablero de 6x6. Los contadores de arriba marcan 3 minas
sin marcar y hay 14 banderas, o sea que hay 3 minas en las 5 casillas sin abrir. Se deducen con
los números:

- Fila 1: el `1` de la columna 1 ya tiene su mina (la bandera de abajo), así que (1,2) no es mina.
  El `1` de la columna 4 también (bandera en (2,4)), así que (1,3) tampoco.
- El `5` de (3,4) tiene 4 banderas alrededor: le falta una, que solo puede ser (4,5).
- El `4` de (5,6) tiene una bandera, (5,5): las otras 3 son (4,5), (4,6) y (6,5).

Con eso todos los números del tablero cierran. Poniendo 1 en cada mina (bandera o deducida):

| Fila | Tablero | Minas | `01` + minas | ASCII |
|---|---|---|---|---|
| 1 | 1 · · 1 3 F | `000001` | `0x41` | A |
| 2 | F 3 2 F 5 F | `100101` | `0x65` | e |
| 3 | F F 4 5 F F | `110011` | `0x73` | s |
| 4 | 3 4 F F · · | `001111` | `0x4F` | O |
| 5 | F 3 3 F F 4 | `100110` | `0x66` | f |
| 6 | F 2 1 3 · 2 | `100010` | `0x62` | b |

(`F` = bandera, `·` = casilla sin abrir)

```sh
for f in 000001 100101 110011 001111 100110 100010; do
  printf "\\x$(printf %x $((2#01$f)))"
done; echo
# AesOfb
```

**AES Ofb** → AES de 128 bits (lo dice el txt) en modo OFB. La password es la del paso 2.

## Paso 6: lima.bmp → el .wmv

Con lo anterior, lima tendría que salir con:

```sh
./stegobmp -extract -p grupo12/lima.bmp -out analisis/lima -steg LSB4 -a aes128 -m ofb -pass sorpresa
```

pero da:

```
datos corruptos: el tamaño declarado (4236261204 bytes) es mayor que los datos disponibles (908061 bytes)
```

O sea: el tamaño de afuera (908065) se lee bien y el descifrado no falla (OFB no tiene padding,
nunca "falla"), pero lo descifrado es basura: el tamaño de adentro no tiene sentido y no aparece
la cabecera de un ASF/wmv (`30 26 B2 75 8E 66 CF 11`). Que sea un modo de flujo cierra: 908065 no
es múltiplo de 16, así que no puede ser ECB ni CBC (con padding PKCS5 el cifrado siempre es
múltiplo del bloque).

Probamos sin éxito los 16 pares `-a`/`-m`, mayúsculas, la frase entera, otras derivaciones de
clave (PBKDF2 con otros hashes/iteraciones, `EVP_BytesToKey`) y otros órdenes de bits en LSB4.

Lo que faltaba: la password lleva un **`\r` (carriage return) al final**. En el pdf, "sorpresa"
termina la línea, y quien cifró se quedó con el fin de línea de Windows (`\r\n`) cortado a la
mitad. Probando la password con `\r`, `\n`, `\r\n` y espacios, en todos los algoritmos y modos,
el único que da bien es `sorpresa\r` con AES-128 OFB, justo lo que decía el buscaminas.

En bash, `$'...'` permite meter el `\r` en el argumento:

```sh
./stegobmp -extract -p grupo12/lima.bmp -out analisis/lima -steg LSB4 -a aes128 -m ofb -pass $'sorpresa\r'
file analisis/lima.wmv
# analisis/lima.wmv: Microsoft ASF
xxd -l 16 analisis/lima.wmv
# 00000000: 3026 b275 8e66 cf11 a6d9 00aa 0062 ce6c  0&.u.f.......b.l
```

Sale [`lima.wmv`](../analisis/lima.wmv), de 908056 bytes: los 908065 descifrados menos 4 del
tamaño y 5 de `.wmv\0`. Arranca con la cabecera de ASF (Windows Media Video 9 + Windows Media
Audio 9.2, según los metadatos). La metadata no dice de qué película es: hay que mirarlo.

## Archivos en `analisis/`

| Archivo | De dónde sale |
|---|---|
| `kings.png` | paso 1, `kings.bmp` con LSB1 |
| `paris.pdf` | paso 1, `paris.bmp` con LSBI |
| `loimposible_tail.txt` | paso 3, últimos 49 bytes de `loimposible.bmp` |
| `kings.zip` | paso 4, copia de `kings.png` |
| `sol12.txt` | paso 4, lo que tiene el zip |
| `lima.wmv` | paso 6, `lima.bmp` con LSB4 + AES-128 OFB + `sorpresa\r` |
