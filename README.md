# NSF BigData project Parallel Bioinformatics Library

New name: **Bliss** ??

Please take a look at our [Wiki](https://bitbucket.org/AluruLab/pbil/wiki/Home).

## Building

Build requirements are:

`cmake`, `boost_log`, `boost_system`, `boost_thread`, `boost_program-options`


### Cloning:

```sh
git clone git@bitbucket.org:AluruLab/pbil.git
cd pbil
git submodule init
git submodule update
```

### Building

```sh
mkdir build
cd build
cmake ../
make
```

### Running the tests

```sh
ctest -T Test
```

### Building the documentation

```sh
make doc
```


## Configuring Eclipse:

Cmake typically uses a out-of-source build.  to generate eclipse compatible `.project` and `.cproject` files, supply
```
-G"Eclipse CDT4 - Unix Makefiles"
```
to cmake.

install ptp, egit, cmake ed.

