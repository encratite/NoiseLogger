output=NoiseLoggerServer
sourceFiles=Server/Source/*.cpp
objectDirectory=Server/Object
outputDirectory=Output
includes="-I../Fall -IServer -ICommon"
libraries="-L../Fall/Output -lFall -LOutput -lNoiseLoggerCommon -llzma -lssl -lcrypto -lpq"
compiler=g++

mkdir -p $objectDirectory
mkdir -p $outputDirectory

objects=
for sourceFile in $sourceFiles
do
	outputFile=$(basename $sourceFile | cut -d. -f1).o
	object=$objectDirectory/$outputFile
	commandLine="$compiler -std=c++11 -g -Wall $includes -c $sourceFile -o $object"
	echo $commandLine
	$commandLine
	if [[ $? != 0 ]]
	then
		exit 1
	fi
	objects="$objects $object"
done

outputPath=$outputDirectory/$output
commandLine="$compiler -o $outputPath$objects $libraries"
echo $commandLine
$commandLine