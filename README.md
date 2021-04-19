# triaging-binaries-toolkit
M1 research project

## Authors
Pascale-Jade Vallot, Jasmin et Tibane Galbrun

## Indirect Contributor
0Intro - https://github.com/0intro/libelf

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
```
Usage: tbt [-a ALGO|-o FILE|-c|-v|-V|-h] FILE|DIR
Compute Fuzzy Hashing

 -a ALGO,--algorithm ALGO       ALGO : CTPH|SIMHASH|ALL
 -c ,--compareHashes            Compare the hashes stored in the given file
 -o FILE,--output FILE          write result to FILE
 -v,--verbose                   verbose output
 -V,--version                   display version and exit
 -h,--help                      display this help
```

## Examples
Compute hash
```shell
./tbt -o hash.txt test/
```

Content of hash.txt
```
edit_dist_test:
	1:2:QXX5dffXXPXrVd5NfznlfPLBl3N1fPLBl3dffftDfjLffftzfjZzffrzXXrLBlX:cPXv5Nx9Pv5Pv3fff7BJdNVbv9ff33jnTfZpTfffX3ZffHffLXRffXRff3vjdjX
	2:36214d5d90e8f5a0cd8149e3779fbde6
simhash_test:
	1:256:EfffJNb5AUTrVqlO/hclZiPw4kfffF//fffUZghqb8HZvWoC63OxT2nsT7xLOKK:MNmOay6ZidMhqb8HV4xTHdOlOU5
	2:d5a69ec8d8fbf7390b1d60e608fe4a0b
ctph_test:
	1:512:bXTmUHBz6zzeeuddE7Rffi+b1ia3OuErTQg:wUvzcadYffi+7iagn
	2:d1ad8cc9feabe7ad011806aea91ad28f
shingle_table_test:
	1:128:4bWb5YRxHe5ffsN43fZLsF7futdrl37OTdLfFKfZI:4bWfYRxQ5ffsNalsFS1rl9OTTfn
	2:d5a39e08beeaf74981752eaaafde2e5a
```

Compare hash
```shell
./tbt -c hash.txt 
```

Output
```
--- CTPH ---

edit_dist_test :

simhash_test :
[ 082 % ] ctph_test
[ 079 % ] shingle_table_test

ctph_test :
[ 082 % ] simhash_test

shingle_table_test :
[ 079 % ] simhash_test

--- SIMHASH ---

edit_dist_test :
[ 009.38 % ] shingle_table_test

simhash_test :
[ 040.62 % ] ctph_test
[ 040.62 % ] shingle_table_test

ctph_test :
[ 040.62 % ] simhash_test
[ 034.38 % ] shingle_table_test

shingle_table_test :
[ 040.62 % ] simhash_test
[ 034.38 % ] ctph_test
[ 009.38 % ] edit_dist_test
```