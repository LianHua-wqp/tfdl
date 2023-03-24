.PHONY: all build run gotool clean help

BINARY="tfdl"

PREFIX="/opt/pkg/petalinux/2018.3/tools/linux-i386/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-"
GOCC=${PREFIX}gcc
GOAR=${PREFIX}ar

SRCS := fly_uart.c fly_dma.c hello.c
# APP_OBJS := fly_uart.o hello.o
# ${GOAR} -cr ${MY_LIB} ${APP_OBJS}		#在不同目录时，需要使用的写法
MY_LIB = libfly.a	#生成的静态库名称，记得在main里面导入

all: build

build:
	${GOCC} -c ${SRCS}
	${GOAR} -cr ${MY_LIB} *.o	
	CGO_ENABLED=1 GOOS=linux GOARCH=arm CC=${GOCC} go build -o ${BINARY}

run:
	@go run ./

gotool:
	go fmt ./
	go vet ./

clean:
	@if [ -f ${BINARY} ] ; then rm ${BINARY} ; rm *.a *.o;fi
	
help:
	@echo "make - 格式化 Go 代码, 并编译生成二进制文件"
	@echo "make build - 编译 Go 代码, 生成二进制文件"
	@echo "make run - 直接运行 Go 代码"
	@echo "make clean - 移除二进制文件和 vim swap files"
	@echo "make gotool - 运行 Go 工具 'fmt' and 'vet'"