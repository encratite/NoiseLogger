output=NoiseLoggerClient
sourceFiles=Client/Source/*.cpp
objectDirectory=Client/Object
outputDirectory=Output
libraries="-L../Fall/Output -lFall -lasound -llzma"
includes="-I../Fall -IClient"
compiler=g++

mkdir -p $objectDirectory
mkdir -p $outputDirectory

objects=
for sourceFile in $sourceFiles
do
	outputFile=$(basename $sourceFile | cut -d. -f1).o
	object=$objectDirectory/$outputFile
	echo Building $sourceFile
	$compiler -std=c++11 -g -Wall $includes -c $sourceFile -o $object $libraries
	if [[ $? != 0 ]]
	then
		exit 1
	fi
	objects="$objects $object"
done

outputPath=$outputDirectory/$output
echo Building $outputPath
$compiler -o $outputPath$objects $libraries