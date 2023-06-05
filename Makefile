all:
	$(CC) -o otp otp.c `pkg-config --cflags --libs libsodium`
