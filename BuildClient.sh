output=NoiseLoggerClient
sourceFiles=Client/Source/*.cpp
objectDirectory=Client/Object
outputDirectory=Output
includes="-I../Fall -IClient -ICommon"
libraries="-L../Fall/Output -lFall -LOutput -lNoiseLoggerCommon -lasound -llzma"
compiler=g++

mkdir -p $objectDirectory
mkdir -p $outputDirectory

objects=
for sourceFile in $sourceFiles
do
	outputFile=$(basename $sourceFile | cut -d. -f1).o
	object=$objectDirectory/$outputFile
	echo Building $sourceFile
	$compiler -std=c++11 -O3 -Wall $includes -c $sourceFile -o $object $libraries
	if [[ $? != 0 ]]
	then
		exit 1
	fi
	objects="$objects $object"
done

outputPath=$outputDirectory/$output
echo Building $outputPath
$compiler -o $outputPath$objects $libraries