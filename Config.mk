CXXFLAGS += -pthread -std=c++14 -Wall -Wextra -pedantic -O2 -g -fno-omit-frame-pointer -Wold-style-cast -Wuninitialized -Wunreachable-code -Wstrict-overflow=3 -D_FORTIFY_SOURCE=2 -ffunction-sections -fdata-sections
LDLIBS += -lstdc++ -lpthread -lz3
LDFLAGS += -Wl,-O1 -Wl,--hash-style=gnu -Wl,--sort-common -Wl,--gc-sections
