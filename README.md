# triaging-binaries-toolkit
M1 research project

## Authors
Pascale-Jade Vallot, Jasmin et Tibane Galbrun

# Installation

## Requirements
- Unix(-like) System
- Lib openssl

## Install OpenSSL lib

### Debian(Ubuntu, ...)
```shell
sudo apt-get install libssl-dev
```

### MacOS
```shell
brew update
brew install openssl
ln -s /usr/local/opt/openssl/include/openssl /usr/local/include
ln -s /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib /usr/local/lib/
ln -s /usr/local/opt/openssl/lib/libssl.1.0.0.dylib /usr/local/lib/
ln -s /usr/local/Cellar/openssl/[version]/include/openssl /usr/bin/openssl
```

## Clone
```shell
git clone https://github.com/jashbin/triaging-binaries-toolkit.git
```

## Compilation

For makefile options
```shell
make help
```

To compile
```shell
make all
```

To compile test
```shell
make test
```

## Executable
```shell
Usage: tbt [-a ALGO|-o FILE|-c|-v|-V|-h] FILE|DIR
Compute Fuzzy Hashing

 -a ALGO,--algorithm ALGO       ALGO : CTPH|SIMHASH|ALL
 -c ,--compareHashes			Compare the hashes stored in the given file
 -o FILE,--output FILE		    write result to FILE
 -v,--verbose			        verbose output
 -V,--version			        display version and exit
 -h,--help			            display this help
```