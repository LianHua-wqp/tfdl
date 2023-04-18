.PHONY: all build run gotool clean help test

BINARY="tfdl"

PREFIX="/opt/pkg/petalinux/2018.3/tools/linux-i386/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-"
GOCC=${PREFIX}gcc
GOAR=${PREFIX}ar


SRCS := fly_uart.c fly_dma.c libaxidma.c fpga.c fly_gpio.c fly_bd.c
PATH_PREFIX = "./partc/"
# FULL_SRCS = $(if $(SRCS), $(addprefix $(PATH_PREFIX),$(SRCS)), %)	#addprefix 添加前缀函数
FULL_SRCS = $(addprefix $(PATH_PREFIX),$(SRCS))	#addprefix 添加前缀函数
OBJ_O = $(patsubst %.c,%.o,$(FULL_SRCS))
TEMP_A = $(patsubst %.c,lib%.a,$(SRCS))
OBJ_A = $(addprefix $(PATH_PREFIX),$(TEMP_A))

MY_LIB = libfly.a	#生成的静态库名称，记得在main里面导入

all: build

test:
	@echo ${FULL_SRCS}
	@echo ${OBJ_O}
	@echo ${OBJ_A}

	@${GOCC} -c ${FULL_SRCS}

build:
	@${GOCC} -c ${FULL_SRCS}
	@${GOAR} -cr ${MY_LIB} *.o
	@CGO_ENABLED=1 GOOS=linux GOARCH=arm CC=${GOCC} go build -ldflags="-s -w" -o ${BINARY}
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