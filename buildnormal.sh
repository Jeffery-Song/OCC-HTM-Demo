SH_PATH="`pwd`"/"`dirname \"$0\"`"
export OCC_ROOT=$SH_PATH
rm -rf $OCC_ROOT/build
mkdir $OCC_ROOT/build
pushd $OCC_ROOT/build
echo "cmake cmd is : cmake .. -DCMAKE_USE_RTM_TXN=OFF -DCMAKE_USE_RTM_STORE=OFF"
cmake .. -DCMAKE_USE_RTM_TXN=OFF -DCMAKE_USE_RTM_STORE=OFF
make
popd