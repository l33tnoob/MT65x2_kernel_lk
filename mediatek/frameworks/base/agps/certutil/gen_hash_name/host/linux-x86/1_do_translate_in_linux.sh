#!/bin/sh

chmod 750 ./bin/certutil
chmod 750 ./bin/hash_md5.sh
chmod 750 ./bin/hash_sha1.sh
chmod 750 ./bin/hash_md5_DER.sh

./bin/hash_md5.sh
./bin/hash_sha1.sh
./bin/hash_md5_DER.sh
