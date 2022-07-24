targetdir=../src_parser

rm -rf $targetdir
mkdir -p $targetdir
java -jar ./antlr-4.10.1-complete.jar -Dlanguage=Cpp  -visitor -o $targetdir Pinlang.g4
