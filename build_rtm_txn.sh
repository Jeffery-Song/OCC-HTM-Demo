SH_PATH="`pwd`"/"`dirname \"$0\"`"
export OCC_ROOT=$SH_PATH
rm -rf $OCC_ROOT/rtm_txn_build
mkdir $OCC_ROOT/rtm_txn_build
pushd $OCC_ROOT/rtm_txn_build
echo "cmake cmd is : cmake .. -DCMAKE_USE_RTM_TXN=ON -DCMAKE_USE_RTM_STORE=OFF"
cmake .. -DCMAKE_USE_RTM_TXN=ON -DCMAKE_USE_RTM_STORE=OFF
make
popd