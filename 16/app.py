from flask import Flask, request
import hashlib
import time

app = Flask(__name__)

# 用于存储用户信息的字典，简单起见存储在内存中
# 格式: { "nickname": {"password": "pwd", "email": "email", "code": "hash", "expire_time": timestamp, "activated": False} }
users = {}

HTML_FORM = """
<!DOCTYPE html>
<html>
<head>
    <title>用户注册系统</title>
</head>
<body>
    <h2>用户注册</h2>
    <form action="/register" method="post">
        昵称: <input type="text" name="nickname" required><br><br>
        口令: <input type="password" name="password" required><br><br>
        邮箱: <input type="email" name="email" required><br><br>
        <input type="submit" value="提交">
    </form>
</body>
</html>
"""

@app.route('/', methods=['GET'])
def index():
    return HTML_FORM

@app.route('/register', methods=['POST'])
def register():
    nickname = request.form.get('nickname')
    password = request.form.get('password')
    email = request.form.get('email')
    
    current_time = time.time()
    
    # 清理过期未激活的用户，释放昵称
    for user in list(users.keys()):
        if not users[user]['activated'] and current_time > users[user]['expire_time']:
            del users[user]
            
    if nickname in users:
        return "该昵称已被占用或正在等待激活！<br><a href='/'>返回</a>"
        
    # 生成验证码 (简单的昵称哈希值，这里稍微加了点时间戳防止碰撞)
    hl = hashlib.md5()
    hl.update((nickname + str(current_time)).encode(encoding='utf-8'))
    code = hl.hexdigest()
    
    # 过期时间为1小时后 (3600秒)
    expire_time = current_time + 3600
    
    users[nickname] = {
        "password": password,
        "email": email,
        "code": code,
        "expire_time": expire_time,
        "activated": False
    }
    
    # 模拟发送邮件: 打印到控制台
    verification_link = f"http://127.0.0.1:5000/verify?user={nickname}&code={code}"
    
    print("-" * 50)
    print(f" [模拟邮件系统] 发送至: {email}")
    print(f" 主题: 账号激活")
    print(f" 内容: 您好，{nickname}！请点击以下链接激活您的账号 (该链接在1小时内有效):")
    print(f" {verification_link}")
    print("-" * 50)
    
    return f"注册提交成功，一封激活邮件已发送至 {email}（由于是简单实现，请查看运行此程序的终端控制台输出获取链接）。<br><a href='/'>返回</a>"

@app.route('/verify', methods=['GET'])
def verify():
    user = request.args.get('user')
    code = request.args.get('code')
    
    current_time = time.time()
    
    if user not in users:
        return "验证失败，用户不存在或该昵称因为超时未激活已被释放。"
        
    user_info = users[user]
    
    if user_info['activated']:
        return "该账户已经激活成功啦，不要重复验证！"
        
    # 检查是否过期（一小时内）
    if current_time > user_info['expire_time']:
        del users[user]  # 释放昵称
        return "验证链接已过期（已超过1小时），您的昵称已被释放，请重新注册。"
        
    # 验证哈希代码
    if user_info['code'] == code:
        users[user]['activated'] = True
        return f"<h1>激活成功！</h1>欢迎你，尊贵的会员：{user}。"
    else:
        return "验证码错误！"

if __name__ == '__main__':
    # 启动应用
    print("服务已启动，请在浏览器中访问: http://127.0.0.1:5000")
    app.run(debug=True, port=5000)
