#! /usr/bin/env bash

name="${1}"
from="${2}"

mkdir -p "bin"

cd "bin" || exit 1

ln -s "../${from}" "${name}" || exit 1
