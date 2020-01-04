# OCC Demo

## Build Command:

```bash
mkdir build
cd build
cmake ..
make

# run test:
./coretest
```

RTM is enabled by default. To disbale RTM, use
```bash
cmake .. -DCMAKE_USE_RTM_STORE=OFF -DCMAKE_USE_RTM_TXN=OFF
```