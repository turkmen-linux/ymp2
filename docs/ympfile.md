# Ymp package
Ymp package is actually zip file. 
```
test_1.0_1.x86_64.ymp
  - metadata.yaml
  - files
  - links
  - data.tar.gz
    - /usr/bin/test
    - ...
```

## metadata.yaml
Metadata file defines package information. 
```
ymp:
  package:
    name: test
    version: 1.0
    release: 1
    description: test package
    group:
     - app.misc
    depends:
     - foo
     - bar
    archive-hash: 4b81087c9aa8c4e631ca3a51d2c5c749f6077183
    arch: x86_64
    archive-size: 14530529

```

## files
File list and hashes.

Format: `sha1sum filepath`
```
3760de851dfe0e88af01362680c8354252c59026 usr/bin/example
b7f123b49345e9274f3f894b2e8aed7ed0716ca0 usr/lib64/libexample.so.1
c909ac4e6081822eedfdcb30e158b374be553c7f usr/share/man/man1/example.1
```

## links
File symlink and path

Format: `linkpath target`
```
usr/lib64/libexample.so.1 libexample
usr/bin/example ../libexec/example
```
