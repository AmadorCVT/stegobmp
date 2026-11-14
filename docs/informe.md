# Informe

Aca ponemos cosas que despues queremos poner en el informe

## lsbi

en el paper habla de el ultimo bit del pixel pero despues en el ejemplo muestra que es el ultimo bit del byte xd

por ahora hice que si era igual la cantidad de modified y unmodified que no invierta



en el paper tiran esta  

For pattern ‘01’ (C), we cannot check how
many pixels were changed and how many
were not, because there is only one pixel,
comparison cannot occur. 

a nosotros la comparacion nos va a dar 1 > 0 entonces si lo tenemos en cuenta y modificamos o no segun el caso (revisar si lo queremos dejar asi). Yo creo que si y de ultima lo aclaramos en el informe

## estegoanálisis (archivos del grupo 12)

Cómo se llegó a cada archivo oculto. Los comandos están en el README.

1. **kings.bmp, LSB1.** Los primeros 4 bytes leídos con LSB1 dan 42927, un tamaño coherente, y lo que sigue empieza con la firma de un PNG. Sale `kings.png`: un tablero de buscaminas de 6x6.
2. **loimposible.bmp, sin LSB.** El archivo mide 49 bytes más de lo que dice su cabecera. Esos bytes, agregados después de los píxeles, son el texto "al .png cambiar extension por .zip y descomprimir".
3. **kings.png como zip.** El PNG tiene un zip pegado después de su fin (`IEND`). Adentro está `sol12.txt`, que explica cómo leer el buscaminas: cada mina es un 1, cada fila es una letra y todos los ASCII empiezan con 01. Las filas dan `A e s O f b`, o sea AES con clave de 128 bits en modo OFB. También avisa que la password está en otro archivo y que hay un `.wmv` cifrado y oculto.
4. **paris.bmp, LSBI.** Sale `paris.pdf`, que dice "La password es sorpresa".
5. **lima.bmp, LSB4 + AES-128 OFB.** Los primeros 4 bytes leídos con LSB4 dan 908065. No es múltiplo de 16, lo que cuadra con un modo de flujo como OFB, que no usa padding. Sale `lima.wmv`, de 908056 bytes.

### la password de lima.bmp

Con `-pass sorpresa` el descifrado da basura. La password real es `sorpresa` seguida de un retorno de carro (`0x0D`): lo más probable es que la cátedra la haya leído de un archivo o una consola con fin de línea de Windows. El resto es exactamente lo que pide el enunciado: PBKDF2 con SHA-256, salt de 8 bytes en cero, 10000 iteraciones, y la key y el IV separados de la misma salida.

Cómo se encontró:

- Para descartar un error de extracción se midió la diferencia entre píxeles vecinos a lo largo de la imagen. La zona con ruido en los 4 bits bajos termina exactamente en el byte 1816138 del cuerpo, que es 2 x (4 + 908065). Eso confirma LSB4, el orden físico de los bytes y el tamaño.
- Como se sabía que el archivo era un `.wmv`, se conocían los primeros bytes del texto plano: el tamaño real (908056) y la firma ASF. En OFB cada bloque de keystream es el cifrado del anterior, así que con dos bloques de texto plano conocido se puede probar una clave sin conocer el IV.
- Con ese test se probaron variantes de la password y la única que pasó fue `sorpresa\r`.

Verificación: el texto descifrado empieza con el tamaño 908056, sigue la firma ASF y termina justo en `.wmv\0`, sin bytes de sobra.

### qué muestra el video

Clip de 26 segundos de una serie (logo de FOX), doblado al español. Dos personas revisan un e-mail de ruptura y una nota que "este archivo es más grande de lo que debería", por lo que hay un error de compresión o "nuestro agente ocultó datos en el archivo". El portador es el archivo del e-mail y lo que delata el ocultamiento es su tamaño. El clip corta antes de mostrar qué datos eran.
