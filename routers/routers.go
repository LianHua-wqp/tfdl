package routers

import (
	"github.com/gin-gonic/gin"
	"net/http"
	"tfdl/logger"
)

func SetupRouter() *gin.Engine {
	gin.SetMode(gin.ReleaseMode)
	r := gin.New()
	r.Use(logger.GinLogger(), logger.GinRecovery(true))
	//r := gin.Default()

	r.GET("/", func(context *gin.Context) {
		context.String(http.StatusOK, "pong")
	})

	//v1 := r.Group("/api/v1")
	//v1.POST("/login", controller.LoginHandler)
	//v1.POST("/signup", controller.SignUpHandler)
	//v1.GET("/refresh_token", controller.RefreshTokenHandler)

	return r
}
