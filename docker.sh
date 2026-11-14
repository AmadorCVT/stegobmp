#!/usr/bin/env bash
# Corre un comando dentro del contenedor Ubuntu 22.04 con el repo montado en /work.
#   ./docker.sh                -> shell interactiva
#   ./docker.sh make           -> compila
#   ./docker.sh ./stegobmp ... -> ejecuta el programa
# La imagen se construye sola la primera vez (o si cambia el Dockerfile: ./docker.sh --rebuild).
set -euo pipefail

IMAGE=stegobmp-dev
ROOT="$(cd "$(dirname "$0")" && pwd)"

if [[ "${1:-}" == "--rebuild" ]]; then
    shift
    docker build -t "$IMAGE" "$ROOT"
elif ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    docker build -t "$IMAGE" "$ROOT"
fi

TTY=()
if [[ -t 0 && -t 1 ]]; then
    TTY=(-it)
fi

# -u: que los archivos generados queden a nombre del usuario y no de root
exec docker run --rm ${TTY[@]+"${TTY[@]}"} -u "$(id -u):$(id -g)" \
    -v "$ROOT":/work -w /work "$IMAGE" "$@"
