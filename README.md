# Hyperactivity

A decentralized safe superintelligent operating system language.

## Setup

Requires a C compiler (gcc or clang) and make.

```
git clone https://github.com/avikj/hyperactivity.git
cd hyperactivity
make
./hyperactive
```

`make` builds the machine (`build/cubical`); `./hyperactive` is the machine run as the loop. The code
and the documents are not files: they are the source, `store/`, read and written only through
`./hyperactive`. From there, `hyperactive` teaches you the rest.
