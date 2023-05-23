all:
	$(CC) -g -o otp otp.c `pkg-config --cflags --libs libsodium`
