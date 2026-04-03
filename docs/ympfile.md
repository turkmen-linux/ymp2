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

