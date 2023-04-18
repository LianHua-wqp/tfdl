package main

import (
	"flag"
	"fmt"
	"sync"
	"tfdl/logger"
	"tfdl/logic"
	"tfdl/partcgo"
	"tfdl/settings"

	"go.uber.org/zap"
)

var port int
var ip string

func init() {
	//go run .\main.go -ip 122.114.122.174 -port 40241
	flag.StringVar(&ip, "ip", "122.114.122.174", "公网测试服务器IP")
	//flag.StringVar(&ip, "ip", "122.114.122.174", "公网测试服务器IP")
	flag.IntVar(&port, "port", 36161, "公网测试服务器PORT")
	flag.Parse()
}

func main() {

	// partcgo.SimpleTest(ip, uint16(port))
	// return

	err := settings.Init(settings.FILE_PATH + "/" + settings.FILE_NAME)
	if err != nil {
		fmt.Println("read config.yaml failed", err)
		return
	}

	// fmt.Println(*settings.Conf.LogConfig)
	// return

	if err = logger.Init(settings.Conf.LogConfig, settings.Conf.Mode); err != nil {
		fmt.Printf("init logger failed, err:%v\n", err)
		return
	}

	////初始化路由
	//router := routers.SetupRouter()
	//swag.InitRoutes(router) //注册swag路由
	//zap.L().Debug("http://localhost:8081/swagger/index.html")
	//err = router.Run(fmt.Sprintf(":%d", settings.Conf.Port))
	//if err != nil {
	//	fmt.Printf("run server failed, err:%v\n", err)
	//	return
	//}

	if err = logic.TcpServerInit(); err != nil {
		zap.L().Error(err.Error())
		return
	}

	if err = partcgo.Init(); err != nil {
		zap.L().Error(err.Error())
		return
	}

	zap.L().Debug("start app")

	wg := sync.WaitGroup{}
	wg.Add(1)
	wg.Wait()
}
