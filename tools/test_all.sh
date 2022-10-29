#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail

./pinlang -f ../example_code/array.pin
./pinlang -f ../example_code/class.pin
./pinlang -f ../example_code/generics.pin
./pinlang -f ../example_code/assignment.pin
./pinlang -f ../example_code/if.pin
./pinlang -f ../example_code/type.pin
./pinlang -f ../example_code/tuple.pin
./pinlang -f ../example_code/algorithm.pin
./pinlang -f ../example_code/string.pin
./pinlang -f ../example_code/dynlib.pin
./pinlang -f ../example_code/int.pin

echo "all pass"
