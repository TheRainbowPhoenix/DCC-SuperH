#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
output_dir=$(pwd)
build_dir=$(mktemp -d /tmp/dcc-superh.XXXXXX)
cp CMakeLists.txt "$build_dir/"
cp -r src include tests "$build_dir/"
cd "$build_dir"
fxsdk build-cp
cp DCC-SuperH.hh3 "$output_dir/"
