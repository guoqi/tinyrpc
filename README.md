# TinyRPC - a simple rpc framework for C++

## Blueprint

- Construct a simple generic network library
- Support simple remote procedure call (RPC) as call a local procedure
- Simple and easy to use and manage
- Support self-defined IDL
- Support service register and service dicovery

## Example
```Bash
$ cd example && mkdir build && cd buid
$ cmake ..
$ make
```

## Building

```Bash
$ mkdir build && cd build
$ cmake ..
$ make
```
After that, there will be a static library named ```libtinyrpc.a``` in folder ```build``` 
and you could link yourself server code with it to get an executable file, 
which is the final rpc server composed multiple servers and services.

## Usage

See also example/rpcsvr
