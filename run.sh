clear
echo  PROJECT IMAGE COMPRESSION
echo
echo Compiling Compressor, Please Wait....
gcc -lm compress.c
clear
echo Running Compressor:
./a.out 
echo Compressor Execution Completed.
echo
while test 1
do
echo "Proceed To Decompressing? (y/n)"
read input
if test $input = 'n' || test $input = 'N' 
 then
   echo Project Execution Completed.
   exit
fi
if test $input = 'y' || test $input = 'Y'
 then
  break
fi
done
echo Compiling Decompressor, Please Wait....
gcc -lm decompress.c
clear
echo Running Decompressor:
./a.out
echo Decompressor Execution Completed.
echo Project Execution Completed.
