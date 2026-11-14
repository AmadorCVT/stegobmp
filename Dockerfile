# Entorno de compilación igual al de la consigna: Ubuntu 22.04 + gcc + OpenSSL.
# El código no se copia a la imagen: docker.sh monta el repo en /work.
FROM ubuntu:22.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
       build-essential libssl-dev valgrind gdb xxd file \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
CMD ["bash"]
