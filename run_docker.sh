#!/bin/sh

docker build --platform=linux/amd64 -t test-malloc .
docker run --platform=linux/amd64 --rm -v $PWD:/app test-malloc /app/run.sh "$@"
