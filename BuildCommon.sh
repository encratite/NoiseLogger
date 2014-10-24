output=NoiseLoggerCommon
sourceFiles=Common/Source/*.cpp
objectDirectory=Object
outputDirectory=Output
includes="-I../Fall -ICommon"
compiler=g++

mkdir -p $objectDirectory
mkdir -p $outputDirectory

objects=
for sourceFile in $sourceFiles
do
	outputFile=$(basename $sourceFile | cut -d. -f1).o
	object=$objectDirectory/$outputFile
	commandLine="$compiler -std=c++11 -static -g -Wall $includes -c $sourceFile -o $object"
	echo $commandLine
	$commandLine
	if [[ $? != 0 ]]
	then
		exit 1
	fi
	objects="$objects $object"
done

outputPath=$outputDirectory/lib$output.a
commandLine="ar rcs $outputPath$objects"
echo $commandLine
$commandLine