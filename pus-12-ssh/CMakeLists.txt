cmake_minimum_required(VERSION 3.9)
project(pus-12-ssh)

add_executable(zad01-auth_list src/libcommon.c src/authlist.c)
target_link_libraries(zad01-auth_list ssh2)

add_executable(zad02-exec src/libcommon.c src/exec.c)
target_link_libraries(zad02-exec ssh2)

add_executable(zad03-exec_rsa src/libcommon.c src/exec_rsa.c)
target_link_libraries(zad03-exec_rsa ssh2)

add_executable(zad04-sftp src/libcommon.c src/sftp.c)
target_link_libraries(zad04-sftp ssh2)

add_executable(zad05-sftp_get src/libcommon.c src/sftp_get.c)
target_link_libraries(zad05-sftp_get ssh2)