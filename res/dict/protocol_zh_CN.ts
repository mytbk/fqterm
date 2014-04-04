<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="zh">
<defaultcodec>UTF-8</defaultcodec>
<context>
    <name>FQTerm::FQTermSSH1Channel</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="88"/>
        <source>Server refused pty allocation!</source>
        <translation>服务器拒绝pty分配!</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH1Kex</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_kex.cpp" line="72"/>
        <source>Kex exchange failed!</source>
        <translation>Kex交换失败!</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_kex.cpp" line="91"/>
        <source>startKex: First packet is not public key</source>
        <translation>startKex: 第一个包非公钥</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH1PacketReceiver</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh1_packet.cpp" line="95"/>
        <source>parseData: The packet is too big</source>
        <translation>解析数据: 包过大</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh1_packet.cpp" line="132"/>
        <source>parseData: bad CRC32</source>
        <translation>解析数据: bad CRC32</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH1PasswdAuth</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="62"/>
        <source>UserCancel</source>
        <translation>用户取消</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="85"/>
        <source>Strange response from server</source>
        <translation>未识别的服务器响应</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="93"/>
        <source>User canceled</source>
        <translation>用户取消</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH2Channel</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="240"/>
        <source>Server refuces to open a channel.</source>
        <translation>服务器拒绝开启通道.</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="245"/>
        <source>Server error when opening a channel.</source>
        <translation>开启通道时服务错误.</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="288"/>
        <source>Server refused pty allocation!</source>
        <translation>服务器拒绝pty分配!</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="363"/>
        <source>Channel closed by the server.</source>
        <translation>服务器关闭通道.</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="424"/>
        <source>Can&apos;t open a shell.</source>
        <translation>无法开启shell.</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_channel.cpp" line="427"/>
        <source>Unsupported packet.</source>
        <translation>不支持的包格式.</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH2Kex</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh2_kex.cpp" line="120"/>
        <source>Key exchange failed!</source>
        <translation>Key交换失败!</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh2_kex.cpp" line="139"/>
        <source>startKex: First packet is not SSH_MSG_KEXINIT</source>
        <translation>startKex: 第一个包非SSH_MSG_KEXINIT</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh2_kex.cpp" line="210"/>
        <source>Expect a SSH_MSG_KEXDH_REPLY packet</source>
        <translation>期望得到SSH_MSG_KEXDH_REPLY</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh2_kex.cpp" line="298"/>
        <source>Expect a SSH_MSG_NEWKEYS packet</source>
        <translation>期望得到SSH_MSG_NEWKEYS</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH2PacketReceiver</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh2_packet.cpp" line="188"/>
        <source>parseData: packet too big</source>
        <translation>解析数据: 包过大</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSH2PasswdAuth</name>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="166"/>
        <source>Authentication failed!</source>
        <translation>身份验证失败!</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="169"/>
        <source>Unexpected packet</source>
        <translation>未期望的包</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="177"/>
        <source>Expect a SSH2_MSG_SERVICE_ACCEPT packet</source>
        <translation>期望得到 SSH_2_MSG_SERVICE_ACCEPT</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="184"/>
        <source>Error when sending username and password.</source>
        <translation>发送用户名密码错误.</translation>
    </message>
    <message>
        <location filename="../../src/protocol/internal/fqterm_ssh_auth.cpp" line="207"/>
        <source>UserCancel</source>
        <translation>用户取消</translation>
    </message>
</context>
<context>
    <name>FQTerm::FQTermSSHSocket</name>
    <message>
        <location filename="../../src/protocol/fqterm_ssh_socket.cpp" line="222"/>
        <source>Unknown SSH version. Check if you set the right server and port.</source>
        <translation>未知的SSH版本. 检查服务器及端口设置.</translation>
    </message>
</context>
</TS>
