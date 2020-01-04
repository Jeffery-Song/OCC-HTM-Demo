SH_PATH="`pwd`"/"`dirname \"$0\"`"
export OCC_ROOT=$SH_PATH
rm -rf $OCC_ROOT/rtm_store_build
mkdir $OCC_ROOT/rtm_store_build
pushd $OCC_ROOT/rtm_store_build
echo "cmake cmd is : cmake .. -DCMAKE_USE_RTM_TXN=OFF -DCMAKE_USE_RTM_STORE=ON"
cmake .. -DCMAKE_USE_RTM_TXN=OFF -DCMAKE_USE_RTM_STORE=ON
make
popd