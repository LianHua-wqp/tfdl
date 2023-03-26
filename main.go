package main

import (
	"fmt"
	"tfdl/logger"
	"tfdl/partcgo"
	"tfdl/settings"
)

func main() {

	err := settings.Init(settings.FILE_PATH + "/" + settings.FILE_NAME)
	if err != nil {
		fmt.Println("read config.yaml failed", err)
		return
	}
	if err = logger.Init(settings.Conf.LogConfig, settings.Conf.Mode); err != nil {
		fmt.Printf("init logger failed, err:%v\n", err)
		return
	}

	////初始化路由
	//router := routers.SetupRouter()
	//swag.InitRoutes(router) //注册swag路由
	//zap.L().Info("http://localhost:8081/swagger/index.html")
	//err = router.Run(fmt.Sprintf(":%d", settings.Conf.Port))
	//if err != nil {
	//	fmt.Printf("run server failed, err:%v\n", err)
	//	return
	//}

	partcgo.Init()
}
