#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail

# ./pinlang -f ../example_code/test.pin > a.ll && cat -n a.ll
# clang-12 -x ir ./a.ll -O0  && echo "---" && ./a.out ; echo -e "\nexitcode=$?"

for file in $(ls ../code/test/test_*.pin); do
	printf "%-50s " "$file "
	./pinlang -f $file > a.ll 2> /dev/null
	clang-12 -x ir ./a.ll -O0 2> /dev/null
	./a.out
	echo "pass"
done

echo "all pass"
