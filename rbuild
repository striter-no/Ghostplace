#!/usr/bin/env bash

gcc-14 -pedantic -Wall -Wextra -O2 -o "$1" ./"$1".c ./csrs/* -I src -lm -lutf8proc \
&& printf "\n\n" \
&& ./"$1"

mv ./"$1" ./bin