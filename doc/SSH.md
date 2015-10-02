## SSH连接
FQTerm自带SSH连接功能，但是它在公钥交换时没有认证服务器端公钥，会产生安全隐患。因此我重新启用```FQTermLocalSocket```,这样可以使用系统的```ssh(1)```登陆远程主机。以下是设置方法。

### 配置文件
在```~/.fqterm/fqterm.cfg```的```[global]```标签下加入```externSSH=ssh -p %0 %1```.

### 使用
在快速登录的协议部分，选择协议为```Local```.主机地址处要补充用户名，如```username@host.com```.

### 注意事项
如果使用公钥认证登录方式，一般不需要额外的设置，如果在登录过程中需要输入密码，请安装```x11-ssh-askpass```,因为FQTerm没有模拟pty.

### 其他
OpenSSH 7.0p1开始默认禁用了一些不太安全的功能，可以在`~/.ssh/config`中强制打开，比如对未名BBS可做如下设置。

```
Host bdwm.net
HostName bdwm.net
  KexAlgorithms +diffie-hellman-group1-sha1
```
