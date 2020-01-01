SH_PATH="`pwd`"/"`dirname \"$0\"`"
export OCC_ROOT=$SH_PATH
rm -rf $OCC_ROOT/rtmbuild
mkdir $OCC_ROOT/rtmbuild
pushd $OCC_ROOT/rtmbuild
echo "cmake cmd is : cmake .. -DCMAKE_USE_RTM_TXN=ON -DCMAKE_USE_RTM_STORE=ON"
cmake .. -DCMAKE_USE_RTM_TXN=ON -DCMAKE_USE_RTM_STORE=ON
make
popd