cmake_minimum_required(VERSION 3.9)
project(pus-09-kryptografia-openssl)

add_executable(zad01-cipher_ecb src/cipher_ecb.c)
target_link_libraries(zad01-cipher_ecb crypto)

add_executable(zad02-cipher_cbc src/cipher_cbc.c)
target_link_libraries(zad02-cipher_cbc crypto)

add_executable(zad03-cipher_bio src/cipher_bio.c)
target_link_libraries(zad03-cipher_bio crypto)

add_executable(zad04-digest src/digest.c)
target_link_libraries(zad04-digest crypto)

add_executable(zad05-mac src/mac.c)
target_link_libraries(zad05-mac ssl)

add_executable(zad06-server src/server.c)
target_link_libraries(zad06-server ssl crypto)

add_executable(zad06-client src/client.c)
target_link_libraries(zad06-client ssl crypto)

add_executable(zad07-rsa src/rsa.c)
target_link_libraries(zad07-rsa crypto)

add_executable(zad07-client_rsa src/client_rsa.c)
target_link_libraries(zad07-client_rsa ssl crypto)

add_executable(zad07-server_rsa src/server_rsa.c)
target_link_libraries(zad07-server_rsa ssl crypto)