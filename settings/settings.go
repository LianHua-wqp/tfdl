package settings

import (
	"bytes"
	"fmt"
	"os"

	"github.com/fsnotify/fsnotify"
	"github.com/spf13/viper"
)

var Conf = new(AppConfig)

type AppConfig struct {
	Mode       string `mapstructure:"mode"`
	Port       int    `mapstructure:"port"`
	*Hardware  `mapstructure:"hardware"`
	*LogConfig `mapstructure:"log"`
}

type Hardware struct {
	Uart string `mapstructure:"uart"`
	DMA  string `mapstructure:"dma"`
}

type LogConfig struct {
	Level      string `mapstructure:"level"`
	FileName   string `mapstructure:"filename"`
	MaxSize    int    `mapstructure:"max_size"`
	MaxAgent   int    `mapstructure:"max_agent"`
	MaxBackups int    `mapstructure:"max_backups"`
}

func ReadConfInBuff() {
	viper.SetConfigType("yaml") // or viper.SetConfigType("YAML")

	// any approach to require this configuration into your program.
	var yamlExample = []byte(`
# 注意空格间距
mode: "dev"
port: 8081

hardware:
  uart: "/dev/ttyPS0"
  dma: "/dev/fly_dma1"

log:
  level: "debug"
  filename: "./log/bluebell.log"
  max_size: 10  #M
  max_agent: 30 #最大备份天数
  max_backups: 5
`)

	viper.ReadConfig(bytes.NewBuffer(yamlExample))

	//viper.Get("name") // this would be "steve"
}

func Init(filename string) error {

	viper.SetConfigFile(filename)
	viper.WatchConfig()
	viper.OnConfigChange(func(in fsnotify.Event) {
		fmt.Println("配置文件被更改，可以在这里进行相应的操作")
		viper.Unmarshal(Conf)
	})

	// 如果 path 路径不存在，会有 err，然后通过 IsNotExist 判定文件路径是否存在，如果 true 则不存在，注意用 os.ModePerm 这样文件是可以写入的
	if _, err := os.Stat(filename); os.IsNotExist(err) {
		// mkdir 创建目录，mkdirAll 可创建多层级目录
		err = os.MkdirAll("./conf", os.ModePerm)
		ReadConfInBuff()
		viper.WriteConfigAs("config.yaml")
		return err
	}

	err := viper.ReadInConfig()
	if err != nil {
		return err
	}
	err = viper.Unmarshal(Conf)
	if err != nil {
		return err
	}
	fmt.Println(*Conf.Hardware)
	return err
}
