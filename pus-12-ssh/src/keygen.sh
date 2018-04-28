#!/bin/bash
ssh-keygen -q -b 2048 -t rsa -N passphrase -C LIBSSH2_KEY -f key
mv key private.key
mv key.pub public.key
echo "1. Copy \"public.key\" file to the \".ssh\" directory in user's home directory."
echo
echo "2. If file \"authorized_keys\" exists, execute the following command:"
echo "   mv authorized_files authorized_files.bak"
echo
echo "3. Rename \"public.key\" to \"authorized_keys\":"
echo "   mv public.key authorized_keys"