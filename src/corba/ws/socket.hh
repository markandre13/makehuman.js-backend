#pragma once

void ignore_sig_pipe();
int create_listen_socket(const char *service);
int make_non_block(int fd);
